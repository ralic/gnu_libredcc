/* \file

*/ 

/*
 * Copyright (C) 2015 Andre Gruning 
 *
 * Credits go to Peter Jay Salzman, Michael Burian, Ori Pomerantz for
 * their The Linux Kernel Module Programming Guide availble at
 * http://www.tldp.org/LDP/lkmpg/2.6/html/index.html 
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/fs.h>
// #include <linux/gpio.h>
// #include <linux/interrupt.h>
//#include <mach/hardware.h>
//#include <asm/io.h>

#include "test.h"

// module parameters:
static int run = 0;
module_param(dcc, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(dcc, "GPIO pin used for input of the dcc signal.");

// static int period = DEFAULT_PERIOD;
// module_param(period, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
// MODULE_PARM_DESC(period, "(Half)period of signal to produce");

static int latency = 0;    
module_param(latency, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(latency, "Maximal latency");

// the beginning and the end:

static enum {level_nothing, level_dcc_in, level_irq_in, level_irq_timer, level_device} init_level = level_nothing; 
static void unwind_setup(int level);

int __init dcc_init(void) {

  int ret = 0;
  printk(KERN_INFO "DCC service starting.\n");
  return ret;
}

static void unwind_setup(int level) {

  printk(KERN_INFO "Unwinding from init level %d.\n", level);

  switch(level) {
  default:
  case level_device:
    //      unregister_chrdev(major, DEVICE_NAME); 
  case level_irq_timer:
  case level_irq_in:
  case level_dcc_in:
  case level_nothing: 
    break;
  }
}

void __exit dcc_exit(void)
{

  unwind_setup(init_level);
  printk(KERN_INFO "DCC service ending.\n");
}

module_init(dcc_init);
module_exit(dcc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("dcc_out");
