#include "clock_manager.h"
#include "pwmdma-bcm2835.h"
#include "defs.h"

#include <linux/io.h>
#include <linux/delay.h> // for udelay


#define P_DCC  58 // 58us period -- \todo get this from a DCC encoder header

// \todo use the clk-bcm2835.h
// \todo pwm-bcm2835 and the clocks in the dtbs need changing with the clocks and the dma channel!
// \todo check correct size of clock manage module -- but where?
#define CM_SIZE 0x100

static void* cm_base = NULL;

int init_clockmanager (void) {

  cm_base = ioremap(CM_BASE, CM_SIZE); // @todo: do managed remap
  if(cm_base == NULL) {
    printk(KERN_INFO DEVICE_NAME ": Could not ioremap physical %x of clock manager base.\n", CM_BASE);
    return -ENOMEM;
  }
  return 0;
}

/**
   @param clock offset of clock from clockmanager base address.
   \todo make div and also src parameters of this function
 */
void set_clock(u32 clock) {

  u32 cm_ctl;

  // trigger stop clock
  cm_ctl = ioread32(cm_base + clock + CM_CTL);
  cm_ctl &= ~(CLK_ENAB);
  iowrite32(cm_ctl | CLK_PASSWD, cm_base + clock + CM_CTL); 
  
  // wait until clock has stopped
  while( ioread32(cm_base + clock + CM_CTL) & CLK_BUSY);

  // set clock ticks to duration of one bit being shifted out:
  iowrite32(CLK_PASSWD | CLK_DIVI(DCC_DIVI(P_DCC)), cm_base + clock + CM_DIV); 

  cm_ctl = CLK_PASSWD | CLK_MASH(0) | CLK_SRC(SRC_XTAL);
  iowrite32(cm_ctl, cm_base + clock + CM_CTL); 
  udelay(1000);   // \todo: do I have to wait here? like Gert does?
  
  // finally enable clock again (never change CLK_ENAB while also changing other bits of the CM_CTL register):
  iowrite32(cm_ctl | CLK_ENAB, cm_base + clock + CM_CTL); 



}

