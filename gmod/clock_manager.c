#include "clock_manager.h"
#include "dcc.h"
#include "defs.h"

#include <linux/io.h>
#include <linux/delay.h> // for udelay


#define P_DCC  58 // 58us period -- \todo get this from a DCC encoder header

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

void set_clock() {

  u32 settings;

  // trigger stop clock
  u32 clock = ioread32(cm_base + CM_PWM + CM_CTL);
  clock &= ~(CLK_ENAB);
  iowrite32(CLK_PASSWD | clock, cm_base + CM_PWM + CM_CTL); 
  
  // wait until clock has stopped
  while( ioread32(cm_base + CM_PWM + CM_CTL) & CLK_BUSY);

  // set clock ticks to duration of one bit being shifted out:
  iowrite32(CLK_PASSWD | CLK_DIVI(DCC_DIVI(P_DCC)), cm_base + CM_PWM + CM_DIV); 

  settings = CLK_PASSWD | CLK_MASH(0) | CLK_SRC(SRC_XTAL);
  iowrite32(settings, cm_base + CM_PWM + CM_CTL); 
  udelay(1000);   // \todo: do I have to wait here? like Gert does?
  
  // finally enable clock again:
  iowrite32(settings | CLK_ENAB, cm_base + CM_PWM + CM_CTL); 



}

