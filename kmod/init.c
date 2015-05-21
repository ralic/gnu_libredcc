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

#include "init.h"
#include "buffer.h"
#include "dma_init.h"

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
//#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>

//#include <linux/moduleparam.h>
#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/interrupt.h>
//#include <linux/platform_device.h>
//#include <asm/io.h>
//#include <linux/pwm.h>
//#include <linux/delay.h>

static enum {init_nothing, init_gpio, init_dma, init_pwm} init_level = init_nothing;

static void unwind(void);

int __init init(void) {
  
int ret = gpio_init();
if(ret) {
return ret;
}
init_level = init_gpio;

ret = dma_init();
if(ret) {
unwind();
return ret;
}
      
init_level = init_dma;

ret = pwm_init();
if(ret) {
unwind();
return ret;
}
init_level = init_pwm;

printk(KERN_INFO "DCC PWM service starting.\n");

/* 
send a dummy packet!
 */

unsigned *data = kmalloc(sizeof(*data), GFP_DMA); // make dev_kmalloc
data[0] = 0x55555555; // test changing the order of this an the below line
struct scatterlist* sgl = map_dma_buffer(data, sizeof(*data));
//dma_cookie_t cookie = 
submit_one_dma_buffer(sgl);
return ret;

}



static void unwind() {

//  int status = dma_async_is_tx_complete(dma, cookie, NULL, NULL);
//  printk(KERN_INFO "Status of submitted tx is: %u.\n", status);


	printk(KERN_INFO "Unwinding from init level %d.\n", init_level);

	switch(init_level) {
	default:
	case init_pwm: 
	  pwm_unwind();
	  //	pwm_free(pd); pwm_disable();
	case init_dma:
	  dma_unwind();
	  //		dma_unmap_sg(NULL, sgl, 1, DMA_MEM_TO_DEV);
	case init_gpio:
	  gpio_unwind();
	case init_nothing:  {
		// nothing to unwind 
	}
	}
}

void __exit exit(void)
{
/*	unwind_gpio();
	printk(KERN_INFO "GPIO service ending for DCC.\n");
	unwind_dma();
	printk(KERN_INFO "DMA service ending for DCC.\n");
	unwind_pwm();
	printk(KERN_INFO "PWM service ending for DCC.\n"); */
	unwind();
}

module_init(init);
module_exit(exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andre Gruning");	
MODULE_DESCRIPTION("Support for DMA to PWM for BCM2835");
//MODULE_SUPPORTED_DEVICE("pwm_out");
