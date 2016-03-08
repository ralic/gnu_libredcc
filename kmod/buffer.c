#include "dma_init.h"
#include "buffer.h"
#include <linux/dma-mapping.h>

static dma_cookie_t cookie;

static struct scatterlist sgl[1];

struct scatterlist* map_dma_buffer(void* data, size_t size) {

# warning always maps into the same static sgl!

  // get DMA-ready buffer
  sg_set_buf(sgl, data, size); // length is in bytes
	
  /** no specific dma device needed 
      \todo I could get it back from dma_channel struct for the sake of portabolity 
      \todo do I need to unmap as well? 
      \todo do I need the nents something for */
  int nents2 = dma_map_sg(NULL, sgl, 1, DMA_MEM_TO_DEV); // \todo do I need nents for anything later?
  if(nents2 == 0) {
    printk(KERN_INFO "Cound not map the buffer to DMA memory.\n");
    //kfree(data);
    return NULL;
  }
  printk(KERN_INFO "Pointer to dma buffer: %x\n", sgl[0].dma_address);

  return sgl; // \todo when and where do I unmap the sgl, and is this a costly operation so that we better ke eps the buffers we have got? 
}


/**
 \todo will this becalled in tasklet context? and what does this mean?
 */ 
void callback(void* parameters) {

  // unmap dma? and free memory?
  printk(KERN_INFO "done\n");

}



// now fill the dma_buffer with data -- do I need to synchronise memoery (this dma synchronise thingie?)
//   if I change the data? Do I need to remap and everything?
dma_cookie_t submit_one_dma_buffer(struct scatterlist* sgl) {

  // replace below with dmaengine_prep_slave_signle as it is a bit simpler (although it does the same calls)
  struct dma_async_tx_descriptor * dma_desc =  
    dmaengine_prep_slave_sg(pwm_dma, sgl, 1, DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT | DMA_CTRL_ACK); // 1 entry in the sgl, which flags?
  //dmaengine_prep_dma_cyclic(dma, buf_addr, buf_len, period_len, DMA_MEM_TO_DEV);				// flags should be DMA_PREP_INTERRUPT to show that we want an interrupt (do we really want an interrupt? -- yes -- to free the memory! or does kfree block?
  // need to set callback by hand.
  if(dma_desc == NULL) {
    printk(KERN_INFO "Could not get a tx descriptor\n");
    // do i need to unwind sth?
    return -1;
  }

  // add a callback to the descriptor: mark used buffer as available
  dma_desc->callback = callback;
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

void buffer_unwind(void) {
  //int status = dma_async_is_tx_complete(pwm_dma, cookie, NULL, NULL);
  //printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
}
