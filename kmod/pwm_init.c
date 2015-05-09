/* \file
   A kernel module to generate/decode DCC signals using GPIO pins and
   on-board timers for the Raspberry Pi.
*/ 

// todo: change to managed resources?

/*
 * Copyright (C) 2015 Andre Gruning 
 *
 * Credits go to Peter Jay Salzman, Michael Burian, Ori Pomerantz for
 * their The Linux Kernel Module Programming Guide availble at
 * http://www.tldp.org/LDP/lkmpg/2.6/html/index.html 
 *
 * Credits also go to Gert van Loo -- 
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
//#include <linux/interrupt.h>
#include <linux/platform_device.h>
//#include <asm/io.h>
#include <linux/pwm.h>
#include <linux/delay.h>

#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>

// module parameters:
static int pwm_out =   18; // possible are GPIOs 12 , 40, 18 (pwm0) and
// 13, 19 for pwm1 -- only 18 is possible on
// my rpi (alt5)  
module_param(pwm_out, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(pwm_out, "GPIO pin used for the output of the pwm signal.");


// helper function copied from lirc_rpi.c
inline static int is_right_chip(struct gpio_chip *chip, void *data)
{
	printk(KERN_INFO "GPIO Chip %s\n", chip->label);
	if (strcmp(data, chip->label) == 0)
		return 1;
	return 0;
}

static enum {level_nothing, level_got_pin, level_got_pwm, level_dma_mapped, level_got_dma, } init_level = level_nothing;
static void unwind_setup(int level);

extern int bcm2708_gpio_set_function(struct gpio_chip *gc, unsigned offset, int function);


static struct pwm_device *pd;

static struct scatterlist sgl[1];

static struct dma_chan * dma;

dma_cookie_t cookie;


int __init pwm_init(void)
{
	int ret;
  
	printk(KERN_INFO "PWM service starting.\n");

#if 1
	// pinctrl would be cleaner.
	ret = gpio_request(pwm_out, "PWM Pin");
	if(ret < 0) {
		printk(KERN_ALERT "Requesting GPIO %d failed with %d.\n", pwm_out, ret);
		unwind_setup(init_level);
		return ret;
	} 
	printk(KERN_INFO "Successfully requested GPIO %d.\n", pwm_out);
	init_level = level_got_pin;
	

	// 	struct gpio_chip* gc = gpiochip_find("pinctrl-bcm2835", is_right_chip);
	
#define GPIOCHIP_NAME "bcm2708_gpio"
	struct gpio_chip* gc = gpiochip_find(GPIOCHIP_NAME, is_right_chip);
	if(gc == NULL) {
		printk(KERN_ALERT "No gpiochip %s found.\n", GPIOCHIP_NAME);
		unwind_setup(init_level);
		return -ENODEV;
	}
	printk(KERN_ALERT "Gpiochip %s found.\n", gc->label);

	/*
	  struct pinctrl *pinctrl;
	  int gpio;
	  //	struct device;
	  struct device dev;

	  pinctrl = devm_pinctrl_get_select_default(&dev);
	  if(IS_ERR(pinctrl)) {
	  printk(KERN_INFO "No pinctrl default device: %i\n", -1 /*ERR_INT(pinctrl)
	  );
	  }
	

	  gpio = devm_gpio_request(&dev, pwm_out, "gpio18");
	  if(gpio < 0) {
	  printk(KERN_INFO "Could not request GPIO %s: %i\n", "foo", gpio);ERT "Aborting because selected GPIO %d can sleep.\n", pwm_out);
	  unwind_setup(init_level);
	  return ret;
	  }
	*/

	//	ret = gpio_direction_output(dcc_in, GPIOF_INIT_LOW);
	//if(ret < 0) {
	//  printk(KERN_ALERT "Setting up GPIO %d as output failed with %d.\n", dcc_in, ret);
	//  unwind_setup(init_level);
	//  return ret;
	//}

	ret = bcm2708_gpio_set_function(gc, pwm_out, GPIO_FSEL_ALT5);
	if (ret) {
		printk(KERN_ALERT "Setting up GPIO %d as function %d failed with %d.\n", pwm_out, GPIO_FSEL_ALT5, ret);
		unwind_setup(init_level);
		return ret;
	}
#endif

	// now acquire pwm device (more elegant via pwm_get / device tree)

#if 0

#define PWM_NUMBER 0

	pd = pwm_request(PWM_NUMBER, "bcm2835-pwm"); // or use bcm2708-pwm or checkwith device tree -- is it inclded? and is the device tree copied to // also checl the pwm interfsce what it generally provides.
and is there a similar devietree method to get a pinctrl device?
	//pd = pwm_get(NULL, NULL);
	if(IS_ERR(pd)) {
		printk(KERN_ALERT "Requesting PWM %d failed with %ld.\n", PWM_NUMBER, PTR_ERR(pd));
		unwind_setup(init_level);
		return PTR_ERR(pd);
	}
	init_level = level_got_pwm; 

#define TICKS_NS 1000000000ul // 1sec
	ret = pwm_config(pd, TICKS_NS / 2, TICKS_NS);
	if(ret < 0) {
		printk(KERN_ALERT "Configuring PWM %d with %ld/%ld failed with %d.\n", PWM_NUMBER, TICKS_NS/2, TICKS_NS, ret);
		unwind_setup(init_level);
		return ret;
	}

	ret = pwm_enable(pd);
	if(ret < 0) {
		printk(KERN_ALERT "Enabling PWM %d failed with %d.\n", PWM_NUMBER, ret);
		unwind_setup(init_level);
		return ret;
	}

#else
	// deal with PWM chip direct as long as I cant get the
	//pwm-bcm2835 to work

	//#define PWM_BASE (BCM2708_PERI_BASE + 0xC000) // add to


	//#relevant header 0x7E20C0000x7E20C000


	// for clockmanager:

#define CM_PWMCTL (CM_BASE + 0xA0)


#define CLK_PASSWD (0x5A << 24)
#define CLK_MASH(__stage) ((__stage & 0x3) << 9) // stage can be 0,1,2,3 -- see manual
#define CLK_BUSY (1 << 7)
#define CLK_ENAB (1 << 4)
#define CLK_SRC(__source) (__source & 0xF)

#define CLK_SRC_GND 0
#define CLK_SRC_XTAL 1 // 19.2 MHz
#define CLK_SRC_PLLA 4
#define CLK_SRC_PLLC 5
#define CLK_SRC_PLLD 6
#define CLK_SRC_HDMIAUX 7

#define CM_PWMDIV (CM_BASE + 0xA4)

#define CLK_DIVI(__divi) ((__divi & ((1 << 12) -1)) << 12)
#define CLK_DIVF(__divf) ((__divf & ((1 << 12) -1)))

#define F_XTAL 19200000 // 19.2 MHz
#define P_DCC  58 // 58us period 

#define DCC_DIVI ((F_XTAL * P_DCC) / 1000000)

	//	writel(0x5A0000000, __io_address(CM_PWMCTL)); 

	u32 clock = readl(__io_address(CM_PWMCTL));
	clock &= ~(CLK_ENAB);
	writel(CLK_PASSWD | clock, __io_address(CM_PWMCTL)); 

	while( readl(__io_address(CM_PWMCTL)) & CLK_BUSY);
	// countdown avaraibale hire timeout and call cpu_relax();?
	// do I have to check whether it is busy here?
	writel(CLK_PASSWD | CLK_DIVI(DCC_DIVI), __io_address(CM_PWMDIV)); 
	// do I have to wait here? like Gert does?
	//	writel(CLK_PASSWD | CLK_ENAB | CLK_SRC_XTAL, __io_address(CM_PWMCTL)); 
	udelay(1000);
	//	writel(0x5A000011, __io_address(CM_PWMCTL)); 
	//while( readl(__io_address(CM_PWMCTL)) & CLK_BUSY);
	writel(0x5A000001, __io_address(CM_PWMCTL)); 
	udelay(1000);
	//while( readl(__io_address(CM_PWMCTL)) & CLK_BUSY);
	writel(0x5A000011, __io_address(CM_PWMCTL)); 

#define PWM_CTL 0x0

#define PWEN1 1 << 0  // enable
#define MODE1 1 << 1 // 0 is PWM mode, 1 is serial mode
#define RPTL1 1 << 2 // repeate last word in FIFO mode
#define SBIT1 1 << 3 // state of pwm output with no tranmission
#define POLA1 0 << 4// inversion of polariy?
#define USEF1 1 << 5// use fifo?
#define CLRF1 1 << 6 // clear fifo?
#define MSEN1 1 << 7 // do not use sophisticatted PWM algo

#define PWM_DMAC 0x8

#define ENAB 1 << 31
#define PANIC(__x) ((__x) << 8)
#define DREQ(__x) ((__x) << 0)


#define PWM_RNG1 0x10 // duration of one cycle
#define PWM_DAT1 0x14 // data for the cycle
#define PWM_FIF1 0x18 // data via FIFO


#define WORDLENGTH 32 // send 32bits serially.
#define DATUM  0x99999999 // blink at 2*58us = 116 us period -- is there a gap?

	writel(0, __io_address(PWM_BASE + PWM_DMAC));


	writel(WORDLENGTH, __io_address(PWM_BASE + PWM_RNG1)); 
	writel(DATUM, __io_address(PWM_BASE + PWM_DAT1)); 
	//writel(DATUM, __io_address(PWM_BASE + PWM_FIF1));


	writel(PWEN1 | MODE1 | RPTL1 | SBIT1 | POLA1 | USEF1 | CLRF1 | MSEN1, 
	       __io_address(PWM_BASE + PWM_CTL)); 

	writel(ENAB | PANIC(7) | DREQ(7), __io_address(PWM_BASE + PWM_DMAC));
	


	/*	printk(KERN_INFO "PWM Period: %x\n", readl(__io_address(PWM_BASE + PWM_RNG1))); 
		printk(KERN_INFO "PWM Duty: %x\n", readl(__io_address(PWM_BASE + PWM_DAT1))); 
		printk(KERN_INFO "PWM Control: %x\n", readl(__io_address(PWM_BASE + PWM_CTL))); 
		printk(KERN_INFO "CM_PWMControl: %x\n", readl(__io_address(CM_PWMCTL))); 
		printk(KERN_INFO "CM_PWMDIV: %x\n", readl(__io_address(CM_PWMDIV))); 
	*/

#endif


#if 1


	// get DMA ready buffer

//	static unsigned data[8192]; 
	unsigned *data = devm_kmalloc(4, GFP_DMA);
	data[0] = 1;
//	int i;
//	for(i = 0; i < 8192; i++) {
//		data[i] = 0xF0F0F0F0;
//	}

	printk(KERN_INFO "Pointer to data: %x\n", (unsigned) data);

	printk(KERN_INFO "Pointer to data: %x\n", (unsigned) virt_to_dma(NULL, data));


	/*
	  = {
	  [0] = {
	  .page_link = virt_to_page(data), // meddle with the lower bits?
	  .offset = (unsigned long) data & ~PAGE_MASK, // is this the offset?
	  .length = 2*4 //sizeof(data), // size in byte??
	  }
	  }
	*/

//	sgl[0].page_link = virt_to_page(data); // meddle with the lower bits? warning maks int from pointer?
//	sgl[0].offset = (unsigned long) data & ~PAGE_MASK; // is this the offset?
//	sgl[0].length = 2*4; //sizeof(data), // size in byte??

//	sg_set_page(sgl, virt_to_page(data), 2*4, offset_in_page(data));
	sg_set_buf(sgl, data, 4); // length is in bytes


//	sgl[0].page_link = virt_to_page(0xC0000000); // meddle with the lower bits? warning maks int from pointer?
//	sgl[0].offset = 0;
//	sgl[0].length = 8192; //sizeof(data), // size in byte??


/* no specific dma device needed -- I could get it back from dma_channel struct*/
int nents2 = dma_map_sg(NULL, sgl, 1, DMA_MEM_TO_DEV);
if(nents2 == 0) {
	printk(KERN_INFO "Cound not map the buffer to DMA memory.\n");
	unwind_setup(init_level);
	return -ENOMEM;
}
printk(KERN_INFO "Pointer to dma buffer: %x\n", sgl[0].dma_address);

init_level = level_dma_mapped;

// acquire DMA:

static dma_cap_mask_t caps;
//caps = DMA_SLAVE;
dma = dma_request_channel(caps, NULL, NULL); // BCM_LITE here? or slave -- where are the capabilities defined? 
if(dma==NULL) {
	printk(KERN_INFO "Could not get a DMA channel\n");
	unwind_setup(init_level);
	return -ENOMEM;
}
init_level = level_got_dma; 


#define __to_bus1(x) (((x) & 0x00FFFFFF) | 0x7E000000) // applicatable to virt and phys addresses
#define __to_bus2(x) ((x) - BCM2708_PERI_BASE + 0x7E000000) // applicatable to phys addresses only

#define BCM2708_PWM_DREQ 5 


#warning should not be a local var because that goes out of scope
static struct dma_slave_config slave_config = {
	.direction = DMA_MEM_TO_DEV,
	//	  .src_addr = physaddr_xx, // not needed for MEM_TO_DEVICE
	.dst_addr = __to_bus1(PWM_BASE + PWM_FIF1), //  bus address of the PWM_controler.
	//	  .src_addr_width = 4,
	.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES,
	//	  .src_maxburst = 6_xx, // not used by BCM2708
	//	  .dst_maxburst = 6_xx,
	.slave_id = BCM2708_PWM_DREQ,
	.device_fc = true, // ? what for? does not seem to be evalued by BCM2708
};
	
// what to do with the return value? error value?
ret = dmaengine_slave_config(dma, &slave_config);
if(ret) {
	printk(KERN_INFO "Could not set up slave with error %d\n", ret);
	unwind_setup(init_level);
	return ret;
}

// replace below with dmaengine_prep_slave_signle as it is a bit simpler (although it does the same calls)
struct dma_async_tx_descriptor * dma_desc =  
	dmaengine_prep_slave_sg(dma, sgl, 1, DMA_MEM_TO_DEV, 0); // 1 entry in the sgl, ehich flags?
//dmaengine_prep_dma_cyclic(dma, buf_addr, buf_len, period_len, DMA_MEM_TO_DEV);				// flags sgozld be DMA_PREP_INTERRUPT to show that we want an interrupt.
// need to set callback by hand.
if(dma_desc == NULL) {
	printk(KERN_INFO "Could not get a tx descriptor\n");
	unwind_setup(init_level);
	return -1;
}
// add a callback to the descriptor: mark used buffer as available
// add an idle value if no new real packet is availale
// callback is allowed to prepare and submit a new tranaction
cookie = dmaengine_submit(dma_desc);
if(cookie < 0) {
	printk(KERN_INFO "Submitting transaction resulted in error %u.\n", cookie);
	unwind_setup(init_level);
	return cookie;
}

dma_async_issue_pending(dma); // no return value.

int status = dma_async_is_tx_complete(dma, cookie, NULL, NULL);
printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
// I can DMA unmapas soon as the transaction is done##
// But when can I start overwriten the buffer?
// use dynamic_sybc to recycle streaming buffers
// straming mapping ops can be called from interrupt context
// can unmap when tranaction is finished (or sync?)
// I must sync for the device afterchanging the buffer content (but probably only in one direction -- for device)



#endif




	
return 0;
}


static void unwind_setup(int level) {

	printk(KERN_INFO "Unwinding from init level %d.\n", level);

	switch(level) {
	default:
	case level_got_dma: {
		int status = dma_async_is_tx_complete(dma, cookie, NULL, NULL);
		printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
		dma_release_channel(dma);
	}
	case level_dma_mapped:
		dma_unmap_sg(NULL, sgl, 1, DMA_MEM_TO_DEV);
	case level_got_pwm:
		pwm_disable(pd);
		pwm_free(pd);
	case level_got_pin:
		gpio_free(pwm_out);
	case level_nothing: 
		// nothing to unwind
		break;
	}
}

void __exit pwm_exit(void)
{

	unwind_setup(init_level);
	printk(KERN_INFO "PWM service ending.\n");
}

module_init(pwm_init);
module_exit(pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andre Gruning");	
MODULE_DESCRIPTION("Support for DMA to PWM for BCM2835");
//MODULE_SUPPORTED_DEVICE("pwm_out");
