#include <linux/dmaengine.h>

extern struct dma_chan * pwm_dma;
void dma_unwind(void);
int dma_init(void);
