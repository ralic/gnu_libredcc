#include "pwm.h"

#include <linux/module.h>
#include <asm/io.h> // for readl / writel
#include <mach/platform.h> // for __io_address
#include <linux/delay.h> // for udelay


int __init pwm_init(void) {

  // now acquire pwm device (more elegant via pwm_get / device tree) \todo but I do not know how to get access to it.

#if 0
#define PWM_NUMBER 0

  pd = pwm_request(PWM_NUMBER, "pwm-bcm2835"); // or use bcm2708-pwm or checkwith device tree -- is it inclded? and is the device tree copied to // also checl the pwm interfsce what it generally provides.
  // \todo and is there a similar devietree method to get a pinctrl device?
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
#define CLK_SRC(__source) (__source & 0xF) // source can be one of the following below:

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

#define WORDLENGTH 32 // send 32bits serially.
  //#define DATUM  0x99999999 // blink at 2*58us = 116 us period -- is there a gap?

    writel(0, __io_address(PWM_BASE + PWM_DMAC)); // is this necessary?


  writel(WORDLENGTH, __io_address(PWM_BASE + PWM_RNG1)); 
  //writel(DATUM, __io_address(PWM_BASE + PWM_DAT1)); 
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

  return 0; // currently no error conditions foreseen.
}

void pwm_unwind(void) {
  /* doing nothing currently, but should
     1. switch off pwm device, and probavbly free the clock
     2. do pwm_put or similar in case we are using the pwm module instead of going direct
  */
}

#endif
