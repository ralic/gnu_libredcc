#include "pwm.h"

#include <linux/module.h>
#include <asm/io.h>
#include <linux/pwm.h>

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
  return PTR_ERR(pd);

  }
  pwm = to_bcm2835_pwm(pd->chip);

  init_clockmanager();
  set_clock();

  // switch of DMA for pwm peripherial. \todo is this necessary?
  iowrite32(0, (pwm->base + PWM_DMAC)); 

  // shift out 32 bits (-1 is just dummy in FIFO mode)
  pwm_config(pd, -1, 8*sizeof(u32)); // \todo any better way to get 32 here?
  

  iowrite32(MODE1 | RPTL1 | USEF1 | CLRF1, pwm->base + PWM_CTL); 
  pwm_set_polarity(pd, PWM_POLARITY_INVERSED); // \todo why does TAMS not switch off after this?


  // setting up and enabling DMA mode of pwm peripheral
  iowrite32(ENAB | PANIC(7) | DREQ(7), (pwm->base + PWM_DMAC));
	

  return 0; // currently no error conditions foreseen.
}

void pwm_unwind(void) {
  /* \todo switch off pwm device, and probably free the clock?
  */
  pwm_free(pd);
}


