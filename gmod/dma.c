#include "pwm.h" // for PWM_FIF1
#include "dma.h"

#include <linux/dmaengine.h>
#ifdef DUMMY
#warning "Dummy is defined!"
#define PWM_BASE 0
#else
#include <mach/platform.h> // for PWM_BASE
#include "defs.h"
#endif



// static struct pwm_device *pd;

struct dma_chan * pwm_dma;
static enum {dma_nothing, dma_got_channel} dma_level = dma_nothing;


int __init dma_init(void) {

  // \todo instead of a slave, we could probalby use a cyclic buffer?

  // acquire DMA channel:
  dma_cap_mask_t caps;
  dma_cap_zero(caps);
  dma_cap_set(DMA_SLAVE, caps); // \todo DMA_ITERRUOT, DMA_PRIVATE?

  pwm_dma = dma_request_channel(caps, NULL, NULL); // BCM_LITE here? 
  if(pwm_dma==NULL) {
    printk(KERN_INFO "Could not get a DMA channel\n");
    dma_unwind();
    return -EBUSY;
  }
  dma_level = dma_got_channel; 

  // for io addresses:
#define __to_bus1(x) (((x) & 0x00FFFFFF) | 0x7E000000) // applicatable to virt and phys addresses
#define __to_bus2(x) ((x) - BCM2708_PERI_BASE + 0x7E000000) // applicatable to phys addresses only

#define BCM2708_PWM_DREQ 5 // \todo move to some platform file?

#warning should not be a local var because that goes out of scope or can it go out of scope?
  static struct dma_slave_config slave_config = {
  .direction = DMA_MEM_TO_DEV,
    .dst_addr = __to_bus1(PWM_BASE + PWM_FIF1), //  bus address of the PWM_controler.
    .dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES,
    .slave_id = BCM2708_PWM_DREQ,
    .device_fc = true, // ? what for? does not seem to be evalued by BCM2708
    };
	
  // what to do with the return value? error value?
  int ret = dmaengine_slave_config(pwm_dma, &slave_config);
  if(ret) {
    printk(KERN_INFO "Could not set up slave with error %d\n", ret);
    dma_unwind();
    return ret;
}

  printk(KERN_INFO "Acquired DMA channel %p and set up as slave for PWM peripherial\n", pwm_dma);
  return ret;
}

void dma_unwind(void) {
  
  switch(dma_level) {
  default:
  case dma_got_channel: {
    //    int status = dma_async_is_tx_complete(pwm_dma, cookie, NULL, NULL);
    // printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
    dma_release_channel(pwm_dma); }
  case dma_nothing: {
    // nothing to do
  }
  }
}


