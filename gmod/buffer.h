#include<linux/scatterlist.h>
#include <linux/dmaengine.h>


struct scatterlist* map_dma_buffer(void* data, size_t size); // probably not needed if I integrate it with the below?
dma_cookie_t submit_one_dma_buffer(struct scatterlist* sgl);

dma_cookie_t submit_dma_single(dma_addr_t dma_addr, size_t size);
