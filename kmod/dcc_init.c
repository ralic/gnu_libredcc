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
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
//#include <mach/irqs.h> /// @todo remove.
#include <mach/hardware.h>
#include <asm/io.h>

#include "dcc_module.h"

// module parameters:
static int dcc_in = DEFAULT_DCC_IN_GPIO;
module_param(dcc_in, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(dcc_in, "GPIO pin used for the input of the dcc signal.");

/*
static int dcc_out = DEFAULT_DCC_OUT_GPIO;
module_param(dcc_out, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(dcc_out, "GPIO pin used for the output of the dcc signal.");
*/

static int major = DEVICE_MAJOR; // major device number
module_param(major, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(major, "Major device number used for dcc device.");

static int dcc_in_irq; // irq used for pin

// \todo how far did the init go successfully?


// file operations:
//static int dcc_open(struct inode *, struct file *);
//static int dcc_release(struct inode *, struct file *);
//static ssize_t dcc_read(struct file *, char *, size_t, loff_t *);
//static ssize_t dcc_write(struct file *, const char *, size_t, loff_t *);


//static int opened = 0; // has the device been opened?

static struct file_operations fops = {
  //	.read = dcc_read,
//	.write = dcc_write,
// 	.open = dcc_open,
// 	.release = dcc_release
};


//static int dcc_open(struct inode *, struct file *) {
// return -EINVAL;
//}
//static int dcc_release(struct inode *, struct file *);
//static ssize_t dcc_read(struct file *, char *, size_t, loff_t *);


// IRQ handlers

// \todo why did the message below appear multiple times although I just have one shot?
static irqreturn_t my_gpio_handler(int irq, void* dev_id) {

  // dont to this
printk(KERN_ERR "GPIO INT");
return IRQ_HANDLED;
}

// \todo should I have the TIMER flag??
static irqreturn_t my_timer_handler(int irg, void* dev_id) {
  static int toggle = 0;
  
  // read free running system timer
  uint32_t clo = readl(__io_address(ST_BASE + 0x04));

  // acknowledge System Timer Match 0:
  writel(1 << 0, __io_address(ST_BASE + 0x0));
  
  gpio_set_value(dcc_in, toggle & 0x1);
  toggle++;

#define DCC_CYCLES 100 // 100us
  // @todo check STC_FREQ_HZ == 1000000 (1 million).

  // set next System Timer Match 0:
  writel(clo + DCC_CYCLES, __io_address(ST_BASE + 0xC));

  return IRQ_HANDLED;
}

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

	/*
	ret = gpio_direction_input(dcc_in);
	if(ret < 0) {
	  printk(KERN_ALERT "Setting up GPIO %d as input failed with %d.\n", dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	}
	*/

	// \todo do I have to switch off the pull-down?
	ret = gpio_direction_output(dcc_in, GPIOF_INIT_HIGH);
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
		  
	  
	dcc_in_irq = gpio_to_irq(dcc_in);
	if(dcc_in_irq < 0) {
	  printk(KERN_ALERT "Getting interrupt no for GPIO %d failed with %d\n", dcc_in, dcc_in_irq);
	  unwind_setup(init_level);
	  return dcc_in_irq;
	} 
	printk(KERN_INFO "Got IRQ number %d for GPIO %d.\n", dcc_in_irq, dcc_in);
	
	ret = request_irq(dcc_in_irq, my_gpio_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "dcc handler", NULL);
	if(ret < 0) {
	  printk(KERN_ALERT "Reqesting interrupt  %d for GPIO %d failed with %d\n", dcc_in_irq, dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	}
	  
	printk(KERN_INFO "Interrupt handler set for IRQ %d.\n", dcc_in_irq);
	init_level = level_irq_in;

#ifndef IRQ_TIMER0 
#define IRQ_TIMER0 -1
#warning "Using fake IRQ_TIMER0"
#endif

	ret = request_irq(IRQ_TIMER0, my_timer_handler, IRQF_ONESHOT, "timer handler", NULL);
	if(ret < 0) {
	  printk(KERN_ALERT "Requesting timer interrupt  %d for GPIO %d failed with %d\n", IRQ_TIMER0, dcc_in, ret);
	  unwind_setup(init_level);
	  return ret;
	} 
	printk(KERN_INFO "Interrupt handler for IRQ %d set.\n", IRQ_TIMER0);

	init_level = level_irq_timer;


	//- gpio_direction_output(gpio_num, true);
	// - gpio_direction_input(gpio_num);
	//- gpio_set_value(gpio_num, value);

	// setup chardev 	
	major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);
	if (major < 0) {
	  printk(KERN_ALERT "Registering device " DEVICE_NAME " failed with %d\n", major);
	  unwind_setup(init_level);
	  return major;
	}
	printk(KERN_INFO "DCC service has major device number %d.\n", major);

	init_level = level_device;

	return 0;
}


static void unwind_setup(init_level_enum level) {

  printk(KERN_INFO "Unwinding from init level %d.\n", level);

  switch(level) {
  default:
  case level_device:
      unregister_chrdev(major, DEVICE_NAME); 
  case level_irq_timer:
    free_irq(IRQ_TIMER0, NULL);
  case level_irq_in:
    free_irq(dcc_in_irq, NULL);
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
