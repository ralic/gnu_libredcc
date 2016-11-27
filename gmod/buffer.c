#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "dma.h"
#include "buffer.h"

static dma_cookie_t cookie;

/** callback data */
struct cb_data {
  //  struct device* dev;
  dma_addr_t dma_addr;
  size_t size;
  // enum_data_direction dir;
  void* buffer;
};

/**
   Callback unmaps the DMA memory and frees the underlying buffer
   \todo will be called in tasklet context, so no waiting etc.
   \todo instead of NULL pass in a global device
 */ 
void callback(struct cb_data* cbd) {
  dma_unmap_single(NULL, cbd->dma_addr, cbd->size, DMA_MEM_TO_DEV);
  kfree(cbd->buffer);
  kfree(cbd);
}

void buffer_unwind(void) {
  // \todo what to do here?
}

dma_cookie_t submit_dma_single(dma_addr_t dma_addr, size_t size, void* buffer) {

  // \todo when is this struct freed?
  struct dma_async_tx_descriptor * dma_desc =  
    dmaengine_prep_slave_single(pwm_dma, dma_addr, size, DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT | DMA_CTRL_ACK); 

  if(dma_desc == NULL) {
    printk(KERN_INFO "Could not get a tx descriptor\n");
    dma_unmap_single(NULL, dma_addr, size, DMA_MEM_TO_DEV);
    kfree(buffer);
    return -ENOMEM;
  }

  // add a callback to the descriptor
  struct cb_data * cbd = kmalloc(sizeof(*cbd), GFP_KERNEL);
  cbd->dma_addr = dma_addr;
  cbd->size = size;
  cbd->buffer = buffer;

  dma_desc->callback = callback;
  dma_desc->callback_param = cbd;
  // callback is allowed to prepare and submit a new transaction
  cookie = dmaengine_submit(dma_desc);
  if(cookie < 0) {
    printk(KERN_INFO "Submitting transaction resulted in error %u.\n", cookie);
    callback(cbd); // free all memory and dma mapping held via cdb
    return cookie;
  }

  dma_async_issue_pending(pwm_dma); // no return value. // this should perhaps be called elsewhere? And I guess it is enough to be called once or everytime?

  return cookie;
}
