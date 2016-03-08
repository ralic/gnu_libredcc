/* \file
 A kernel module to generate/decode DCC signals using GPIO pins and
 on-board timers for the Raspberry Pi.
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
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
//#include <mach/hardware.h>
#include <asm/io.h>

#include "dcc_module.h"

// module parameters:
static int dcc_in = DEFAULT_DCC_IN_GPIO;
module_param(dcc_in, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(dcc_in, "GPIO pin used for the input of the dcc signal.");

static int timer = DEFAULT_TIMER;
module_param(timer, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(timer, "Number 0--3 of System Timer to use.");

static int period = DEFAULT_PERIOD;
module_param(period, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(period, "(Half)period of signal to produce");

static int latency = 0;    
module_param(latency, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(latency, "Maximal latency");






// IRQ handlers

extern int (*rt_handle_IRQ)(unsigned int);

static void do_handle(void);

static int rt_handler(unsigned int irq) {

  if(irq != IRQ_TIMER(timer)) 
    return IRQ_NONE;

  //uint32_t pending = readl(__io_address(ARM_BASE + 0x200));
  // uint32_t match = readl(__io_address(ST_BASE + 0x0)); // is it the system timer interrupt? 

  do_handle();

  //acknowledge System Timer Match and clear interrupt at the same time
  writel(Mn(timer), __io_address(ST_BASE + CS));

  return IRQ_HANDLED;
}


static void do_handle(void) {
  static int toggle = 0;
  static uint32_t next = 0; 
  static uint32_t settling = 10000; // number of calls for settling before we get serious with recording lateness

  
    // read free running system timer
    uint32_t clo = readl(__io_address(ST_BASE + CLO));
    gpio_set_value(dcc_in, toggle & 0x1);
    toggle++;

    if((!settling) && (next)) {
      if(clo-next > latency) {
	latency = clo-next;
      }
    } else {settling--;}

    next = clo+period;

    // set next System Timer Match :
    writel(next, __io_address(ST_BASE + Cn(timer)));
 
}

static int has_run = false;

static irqreturn_t my_timer_handler(int irq, void* dev_id) {
  has_run = true;
  // acknowledge System Timer Match and clear interrupt at the same time
  writel(Mn(timer), __io_address(ST_BASE + CS));
  return IRQ_HANDLED;
}
// */

// the beginning and the end:

typedef enum {level_nothing, level_dcc_in, level_irq_in, level_irq_timer, level_device} init_level_enum;
static init_level_enum init_level = level_nothing; 
static void unwind_setup(init_level_enum level);

int __init dcc_init(void)
{
  int ret;

	printk(KERN_INFO "DCC service starting.\n");

	ret = gpio_request(dcc_in, "DCC Pin");
	if(ret < 0) {
	  printk(KERN_ALERT "Requesting GPIO %d failed with %d.\n", dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	} 
	printk(KERN_INFO "Successfully requested GPIO %d.\n", dcc_in);

	init_level = level_dcc_in;
	
	ret = gpio_cansleep(dcc_in);
	if(ret) {
	  printk(KERN_ALERT "Aborting because selected GPIO %d can sleep.\n", dcc_in);
	  unwind_setup(init_level);
	  return ret;
	}

	// \todo do I have to switch off the pull-down?
	ret = gpio_direction_output(dcc_in, GPIOF_INIT_LOW);
	if(ret < 0) {
	  printk(KERN_ALERT "Setting up GPIO %d as output failed with %d.\n", dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	}

	ret = gpio_get_value(dcc_in);
	if(ret < 0) {
	  printk(KERN_ALERT "Reading GPIO %d failed with %d.\n", dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	}
	printk(KERN_INFO "Read GPIO %d as %d.\n", dcc_in, ret);
	
	ret = request_irq(IRQ_TIMER(timer), my_timer_handler, 0 /*Flags */ , "timer handler", NULL);
	disable_irq(IRQ_TIMER(timer));
	if(ret < 0) {
	  printk(KERN_ALERT "Requesting timer interrupt  %d for GPIO %d failed with %d\n", IRQ_TIMER(timer), dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	} 
	printk(KERN_INFO "Interrupt handler for IRQ %d set.\n", IRQ_TIMER(timer));

	init_level = level_irq_timer;

	int i;
	for(i = 0; i < 4; i++) {
	  int val = readl(__io_address(ST_BASE + Cn(i)));
	  printk(KERN_INFO "Timer Match %i is %x.\n", i, val);
	}
	
	unsigned long flags;
	local_irq_save(flags);
	rt_handle_IRQ = &rt_handler;
	local_irq_restore(flags);

	uint32_t clo = readl(__io_address(ST_BASE + CLO));
	writel(clo + period, __io_address(ST_BASE + Cn(timer)));
	enable_irq(IRQ_TIMER(timer));

	printk(KERN_INFO "Module initialised");
	return 0;
}

static void unwind_setup(init_level_enum level) {

  printk(KERN_INFO "Unwinding from init level %d.\n", level);
  printk(KERN_INFO "Maximal Lateness: %d.\n", latency);

  switch(level) {
  default:
  case level_device:
    //      unregister_chrdev(major, DEVICE_NAME); 
  case level_irq_timer:
    disable_irq(IRQ_TIMER(timer));
    free_irq(IRQ_TIMER(timer), NULL);
    unsigned long flags;
    local_irq_save(flags);
    rt_handle_IRQ = NULL;
    //acknowledge spurious System Timer Match and clear interrupt at the same time
    writel(Mn(timer), __io_address(ST_BASE + CS));
    local_irq_restore(flags);

  case level_irq_in:
    //    free_irq(dcc_in_irq, NULL);
  case level_dcc_in:
    gpio_free(dcc_in);
  case level_nothing: 
    // nothing to unwind
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
