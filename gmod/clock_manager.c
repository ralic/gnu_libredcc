#include "clock_manager.h"
#include "defs.h"

#include <linux/io.h>
#include <linux/delay.h> // for udelay


#define P_DCC  58 // 58us period 


  //	writel(0x5A0000000, __io_address(CM_PWMCTL)); 

// \todo check correct size of clock manage module.
#define CM_SIZE 0x100

void* cm_base = NULL;

int init_clock (void) {
  cm_base = ioremap(CM_BASE, CM_SIZE);
  if(cm_base == NULL) {
    printk(KERN_INFO "Could not ioremap physical address %x.\n", CM_BASE);
    return -ENOMEM;
  }
  return 0;
}

int set_clock() {

  u32 clock = ioread32(cm_base + CM_PWM + CM_CTL);
  clock &= ~(CLK_ENAB);
  writel(CLK_PASSWD | clock, cm_base + CM_PWM + CM_CTL); 
  
  while( readl(cm_base + CM_PWM + CM_CTL) & CLK_BUSY);
  // countdown avaraibale hire timeout and call cpu_relax();?
  // do I have to check whether it is busy here?
  writel(CLK_PASSWD | CLK_DIVI(DCC_DIVI(P_DCC)), cm_base + CM_PWM + CM_DIV); 
  // do I have to wait here? like Gert does?
  //	writel(CLK_PASSWD | CLK_ENAB | CLK_SRC_XTAL, __io_address(CM_PWMCTL)); 
  udelay(1000);
  //	writel(0x5A000011, __io_address(CM_PWMCTL)); 
  //while( readl(__io_address(CM_PWMCTL)) & CLK_BUSY);
  writel(0x5A000001, cm_base + CM_PWM + CM_CTL); 
  udelay(1000);
  //while( readl(__io_address(CM_PWMCTL)) & CLK_BUSY);
  writel(0x5A000011, cm_base + CM_PWM + CM_CTL); 
}

