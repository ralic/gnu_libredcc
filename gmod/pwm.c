

#include <linux/module.h>
#include <linux/io.h>
#include <linux/pwm.h>

#include "defs.h"
#include "pwm.h"
#include "clock_manager.h"

/// os view of pwm device
struct pwm_device *pd = NULL;

/// platform view of pwm device -- need both.
struct bcm2835_pwm *pwm = NULL;


/** acquire pwm device 
    \todo more elegant via pwm_get / device tree, but cant get it to work
    \todo use managed pwm
    \todo or perhaps use sysfs to be able to set the registers from
    user space that are not accesible via the pwm.h functions -- but
    this would be a change to pwm-bcm2835.c
*/
int __init pwm_init(void) {

  pd = pwm_request(PWM_NUMBER, "bcm2835-pwm"); 
  if (IS_ERR(pd)) {
    printk(KERN_ALERT "Requesting PWM %d failed with %ld.\n", PWM_NUMBER, PTR_ERR(pd));
  return PTR_ERR(pd);

  }
  pwm = to_bcm2835_pwm(pd->chip);

  init_clockmanager();
  set_clock(CM_PWM);

  // switch off DMA for pwm peripherial in case it was running.
  iowrite32(0, (pwm->base + PWM_DMAC)); 

  // set to shift out 32 bits (-1 is just dummy in FIFO mode)
  pwm_config(pd, -1, 8*sizeof(u32)); // \todo any better way to get 32 here?
  
  // config the pwm device for use of fifo etc
  iowrite32(MODE1 | RPTL1 | USEF1 | CLRF1, pwm->base + PWM_CTL); 

  // set polarity -- \todo make this a module parameter
  pwm_set_polarity(pd, PWM_POLARITY_INVERSED); // \todo why does TAMS not switch off after this?


  // set up and enable DMA mode of pwm peripheral
  iowrite32(ENAB | PANIC(7) | DREQ(7), (pwm->base + PWM_DMAC));
	
  return 0; 
}

/** release pwm device
    \todo release clock */
void pwm_unwind(void) {
  
  // put DMAC back to its reset state:
  iowrite32(PANIC(7) | DREQ(7), (pwm->base + PWM_DMAC)); 
  
  // CTL is reset by free
  pwm_free(pd);
}


