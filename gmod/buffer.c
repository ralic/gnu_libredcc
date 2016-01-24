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
  //kfree(cbd);
}

void buffer_unwind(void) {
  // \todo what to do here?
}

// now fill the dma_buffer with data -- do I need to synchronise memoery (this dma synchronise thingie?)
//   if I change the data? Do I need to remap and everything?
dma_cookie_t submit_dma_single(dma_addr_t dma_addr, size_t size, void* buffer) {

  // replace below with dmaengine_prep_slave_signle as it is a bit simpler (although it does the same calls)
  struct dma_async_tx_descriptor * dma_desc =  
    dmaengine_prep_slave_single(pwm_dma, dma_addr, size, DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT | DMA_CTRL_ACK); // 1 entry in the sgl, which flags?
  //dmaengine_prep_dma_cyclic(dma, buf_addr, buf_len, period_len, DMA_MEM_TO_DEV);				// flags should be DMA_PREP_INTERRUPT to show that we want an interrupt (do we really want an interrupt? -- yes -- to free the memory! or does kfree block?
  // need to set callback by hand.
  if(dma_desc == NULL) {
    printk(KERN_INFO "Could not get a tx descriptor\n");
    // do i need to unwind sth?
    return -1;
  }

  // add a callback to the descriptor: mark used buffer as available

  struct cb_data * cbd = kmalloc(sizeof(*cbd), GFP_KERNEL);
  cbd->dma_addr = dma_addr;
  cbd->size = size;
  cbd->buffer = buffer;

  dma_desc->callback = callback;
  dma_desc->callback_param = cbd;
  // add an idle value if no new real packet is availale
  // callback is allowed to prepare and submit a new transaction
  cookie = dmaengine_submit(dma_desc);
  if(cookie < 0) {
    printk(KERN_INFO "Submitting transaction resulted in error %u.\n", cookie);
    return cookie;
  }

  dma_async_issue_pending(pwm_dma); // no return value. // this should perhaps be called elsewhere? And I guess it is enough to be called once or everytime?

  int status = dma_async_is_tx_complete(pwm_dma, cookie, NULL, NULL);
  printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
  // I can DMA unmapas soon as the transaction is done##
  // But when can I start overwriting the buffer?
  // use dynamic_sync to recycle streaming buffers
  // straming mapping ops can be called from interrupt context
  // can unmap when tranaction is finished (or sync?)
  // I must sync for the device after changing the buffer content (but probably only in one direction -- for device)

  return cookie;
}
