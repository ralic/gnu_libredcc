#include "pwm.h"

#include <linux/module.h>
#include <asm/io.h> // for readl / writel
#include <linux/pwm.h>

#ifdef DUMMY
#warning "DUMMY is defined"
#define __io_address(x) NULL
#define CM_BASE NULL
#define PWM_BASE NULL
#else
#include <mach/platform.h> // for __io_address
#endif


#include "defs.h"
#include "clock_manager.h"


struct pwm_device *pd = NULL;
struct bcm2835_pwm *pwm = NULL;


int __init pwm_init(void) {

  /* acquire pwm device 
     \todo more elegant via pwm_get / device tree
     \todo use managed pwm
  */

#define PWM_NUMBER 0

  pd = pwm_request(PWM_NUMBER, "bcm2835-pwm"); 
  if (IS_ERR(pd)) {
  printk(KERN_ALERT "Requesting PWM %d failed with %ld.\n", PWM_NUMBER, PTR_ERR(pd));
  //unwind_setup(init_level);
  return PTR_ERR(pd);

}
  pwm = to_bcm2835_pwm(pd->chip);

  //  init_level = level_got_pwm; 

#if 1
  init_clock();
  set_clock();
#else
#warning not setting the clock!
#endif


#if 0
#warning not doing the real stuff!


#define TICKS_NS 1000000000ul // 1sec
  int ret = pwm_config(pd, TICKS_NS / 2, TICKS_NS);
  if(ret < 0) {
    printk(KERN_ALERT "Configuring PWM %d with %ld/%ld failed with %d.\n", PWM_NUMBER, TICKS_NS/2, TICKS_NS, ret);
    //unwind_setup(init_level);
    return ret;
  }
  
  ret = pwm_enable(pd);
  if(ret < 0) {
    printk(KERN_ALERT "Enabling PWM %d failed with %d.\n", PWM_NUMBER, ret);
    //unwind_setup(init_level);
    return ret;
  }
  
#else


#define WORDLENGTH 32 // send 32bits serially.
  //#define DATUM  0x99999999 // blink at 2*58us = 116 us period -- is there a gap?

writel(0, (pwm->base + PWM_DMAC)); // is this necessary?


  writel(WORDLENGTH, (pwm->base + PWM_RNG1)); 
  //writel(DATUM, __io_address(pwm->base + PWM_DAT1)); 
  //writel(DATUM, __io_address(pwm->base + PWM_FIF1));


  //  writel(PWEN1 | MODE1 | RPTL1 | SBIT1 | POLA1 | USEF1 | CLRF1 | MSEN1,
  //  writel(PWEN1 | MODE1 | RPTL1 | POLA1 | USEF1 | CLRF1 | MSEN1, 
  writel(MODE1 | RPTL1 | POLA1 | USEF1 | CLRF1 | MSEN1, 

	 (pwm->base + PWM_CTL)); 

  writel(ENAB | PANIC(7) | DREQ(7), (pwm->base + PWM_DMAC));
	
#endif
  return 0; // currently no error conditions foreseen.
}

void pwm_unwind(void) {
  /* doing nothing currently, but should
     1. switch off pwm device, and probably free the clock
     2. do pwm_put or similar in case we are using the pwm module instead of going direct
     3. and should be doing completely different stuff if I used the device tree stuff
  */
  pwm_free(pd);
  printk(KERN_INFO __FILE__ " service ending.\n");
}


