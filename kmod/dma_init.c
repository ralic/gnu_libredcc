#include "pwm.h" // for PWM_FIF1
#include "init.h"
#include "dma_init.h"

//#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <mach/platform.h> // for PWM_BASE

//static void unwinda_dma(void);

// static struct pwm_device *pd;

struct dma_chan * pwm_dma;
static enum {dma_nothing, dma_got_channel} dma_level = dma_nothing;


//static dma_cookie_t cookie;

int __init dma_init(void) {

  // acquire DMA channel:
  static dma_cap_mask_t caps; // does this need to be static, ie persistent, or can I give it as an function param direct??
  //caps = DMA_SLAVE;
  pwm_dma = dma_request_channel(caps, NULL, NULL); // BCM_LITE here? or slave -- where are the capabilities defined? 
  if(pwm_dma==NULL) {
    printk(KERN_INFO "Could not get a DMA channel\n");
    dma_unwind();
    return -ENOMEM;
  }
  dma_level = dma_got_channel; 

  // for io addresses:
#define __to_bus1(x) (((x) & 0x00FFFFFF) | 0x7E000000) // applicatable to virt and phys addresses
#define __to_bus2(x) ((x) - BCM2708_PERI_BASE + 0x7E000000) // applicatable to phys addresses only

#define BCM2708_PWM_DREQ 5 // move to some platform file?

    //	  .src_addr = physaddr_xx, // not needed for MEM_TO_DEVICE
#warning should not be a local var because that goes out of scope or can it go outof scope?
  static struct dma_slave_config slave_config = {
  .direction = DMA_MEM_TO_DEV,
    .dst_addr = __to_bus1(PWM_BASE + PWM_FIF1), //  bus address of the PWM_controler.
    //	  .src_addr_width = 4,
    .dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES,
    //	  .src_maxburst = 6_xx, // not used by BCM2708
    //	  .dst_maxburst = 6_xx,
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
  case dma_got_channel: 
    //int status = dma_async_is_tx_complete(dma, cookie, NULL, NULL);
    //printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
    dma_release_channel(pwm_dma);
  case dma_nothing: {
    // nothing to do
  }
  }
}


