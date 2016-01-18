#include <linux/dmaengine.h>

/// \todo Why is this here? -- Is this the only thing we use from dma? Also pwm and gpio are not exporting anything direct.
extern struct dma_chan * pwm_dma;

void dma_unwind(void);
int dma_init(void);
