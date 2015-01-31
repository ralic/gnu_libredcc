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
//#include <mach/irqs.h>


#include "dcc_module.h"

// module parameters:
static int gpio = DEFAULT_DCC_GPIO;
module_param(gpio, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(gpio, "GPIO pin used for the [input|output] of the dcc signal.");

static int major = DEVICE_MAJOR; // major device number
module_param(major, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(major, "Major device number used for dcc device.");

static int irq; // irq used for pin

// how far did the init go successfully?
static enum {level_nothing, level_gpio, level_irq, level_device} init_level = level_nothing; 


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

static irqreturn_t my_gpio_handler(int irq, void* dev_id) {
  // should not do this:
  printk(KERN_ERR "GPIO INT");
  return IRQ_HANDLED;
}




// the beginning and the end:

int __init dcc_init(void)
{
  
  int ret;

	printk(KERN_INFO "DCC service starting.\n");

	ret = gpio_request(gpio, "DCC Pin");
	if(ret < 0) {
	  printk(KERN_ALERT "Requesting GPIO %d failed with %d.\n", gpio, ret);
	  return ret;
	} 
	else {
	  printk(KERN_INFO "Successfully requested GPIO %d.\n", gpio);

	}

	init_level = level_gpio;
	

	ret = gpio_cansleep(gpio);
	if(ret) {
	  printk(KERN_ALERT "Aborting because selected GPIO %d can sleep.\n", gpio);
	  return ret;
	}

	ret = gpio_direction_input(gpio);
	if(ret < 0) {
	  printk(KERN_ALERT "Setting up GPIO %d as input failed with %d.\n", gpio, ret);
	  return ret;
	}

	ret = gpio_get_value(gpio);
	if(ret < 0) {
	  printk(KERN_ALERT "Reading GPIO %d failed with %d.\n", gpio, ret);
	  return ret;
	}
	else {
	  printk(KERN_INFO "Read GPIO %d as %d.\n", gpio, ret);
	}	  
	  
	irq = gpio_to_irq(gpio);
	if(irq < 0) {
	  printk(KERN_ALERT "Getting interrupt no for GPIO %d failed with %d\n", gpio, irq);
	  return irq;
	} else {
	  printk(KERN_INFO "Got IRQ %d for GPIO %d.\n", irq, gpio);
	}
	
	ret = request_irq(irq, my_gpio_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "dcc handler", NULL);
	if(ret < 0) {
	  printk(KERN_ALERT "Reqesting interrupt  %d for GPIO %d failed with %d\n", irq, gpio, ret);
	  return ret;
	} else {
	  printk(KERN_INFO "Interrupt handler for IRQ %d set.\n", irq);
	}




	// setup hardware
	// \todo What's the pinctrl subsystem? And does it leave on the raspi? Read Documentation/pinctrl.txt

	/*
- gpio_direction_output(gpio_num, true);
- gpio_direction_input(gpio_num);
- gpio_set_value(gpio_num, value);
- etc....
	*/


		// setup chardev 	
	major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);

	if (major < 0) {
	  printk(KERN_ALERT "Registering device " DEVICE_NAME " failed with %d\n", major);
	  return major;
	}
	printk(KERN_INFO "DCC service has major device number %d.\n", major);

	return 0;
}

void __exit dcc_exit(void)
{

  //free_irq(irq);

  gpio_free(gpio);

  unregister_chrdev(major, DEVICE_NAME); 

  printk(KERN_INFO "DCC service ending.\n");
}

module_init(dcc_init);
module_exit(dcc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("dcc_out");
