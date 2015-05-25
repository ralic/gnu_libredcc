#include "dma_init.h"
#include "buffer.h"
#include <linux/dma-mapping.h>

static dma_cookie_t cookie;

static struct scatterlist sgl[1];
//static dma_cookie_t cookie;

// do sth with the whole damn thing
//data[0] = 1;
//printk(KERN_INFO "Pointer to data: %x\n", (unsigned) data);
//printk(KERN_INFO "Pointer to data: %x\n", (unsigned) virt_to_dma(NULL, data));


struct scatterlist* map_dma_buffer(void* data, size_t size) {
# warning always maps into the same static sgl!

  // get DMA-ready buffer


  /*
    = {
    [0] = {
    .page_link = virt_to_page(data), // meddle with the lower bits?
    .offset = (unsigned long) data & ~PAGE_MASK, // is this the offset?
    .length = 2*4 //sizeof(data), // size in byte??
    }
    }
  */

  //	sgl[0].page_link = virt_to_page(data); // meddle with the lower bits? warning maks int from pointer?
  //	sgl[0].offset = (unsigned long) data & ~PAGE_MASK; // is this the offset?
  //	sgl[0].length = 2*4; //sizeof(data), // size in byte??
  
  //	sg_set_page(sgl, virt_to_page(data), 2*4, offset_in_page(data));
  sg_set_buf(sgl, data, size); // length is in bytes
	

//	sgl[0].page_link = virt_to_page(0xC0000000); // meddle with the lower bits? warning maks int from pointer?
//	sgl[0].offset = 0;
//	sgl[0].length = 8192; //sizeof(data), // size in byte??


/* no specific dma device needed -- I could get it back from dma_channel struct*/
  int nents2 = dma_map_sg(NULL, sgl, 1, DMA_MEM_TO_DEV);
 if(nents2 == 0) {
  printk(KERN_INFO "Cound not map the buffer to DMA memory.\n");
  //kfree(data);
  return NULL;
 }
 printk(KERN_INFO "Pointer to dma buffer: %x\n", sgl[0].dma_address);

 return sgl; // when and where do I unmap the sgl?
}



// now fill the dma_buffer with data -- do I need to synchronise memoery?

dma_cookie_t submit_one_dma_buffer(struct scatterlist* sgl) {

// replace below with dmaengine_prep_slave_signle as it is a bit simpler (although it does the same calls)
  struct dma_async_tx_descriptor * dma_desc =  
	dmaengine_prep_slave_sg(pwm_dma, sgl, 1, DMA_MEM_TO_DEV, 0); // 1 entry in the sgl, ehich flags?
//dmaengine_prep_dma_cyclic(dma, buf_addr, buf_len, period_len, DMA_MEM_TO_DEV);				// flags sgozld be DMA_PREP_INTERRUPT to show that we want an interrupt.
// need to set callback by hand.
if(dma_desc == NULL) {
	printk(KERN_INFO "Could not get a tx descriptor\n");
	// do i need to unwind sth?
	return -1;
}
// add a callback to the descriptor: mark used buffer as available
// add an idle value if no new real packet is availale
// callback is allowed to prepare and submit a new tranaction
cookie = dmaengine_submit(dma_desc);
if(cookie < 0) {
	printk(KERN_INFO "Submitting transaction resulted in error %u.\n", cookie);
	return cookie;
}

dma_async_issue_pending(pwm_dma); // no return value. // this should perhaps be called elsewhere? And I guess it is enough to be called once or everytime?

int status = dma_async_is_tx_complete(pwm_dma, cookie, NULL, NULL);
printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
// I can DMA unmapas soon as the transaction is done##
// But when can I start overwriten the buffer?
// use dynamic_sybc to recycle streaming buffers
// straming mapping ops can be called from interrupt context
// can unmap when tranaction is finished (or sync?)
// I must sync for the device afterchanging the buffer content (but probably only in one direction -- for device)

return cookie;
}

void buffer_unwind(void) {
  int status = dma_async_is_tx_complete(pwm_dma, cookie, NULL, NULL);
  printk(KERN_INFO "Status of submitted tx is: %u.\n", status);
}
