#include <linux/dmaengine.h>


dma_cookie_t submit_dma_single(dma_addr_t dma_addr, size_t size, void* buffer);
