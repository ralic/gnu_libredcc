/* \file
 A kernel module to generate and/decode DCC signal using GPIO pins and on-board timers for the Raspberry Pi
*/

/*
 * Copyright (C) 2015 Andre Gruning 
 *
 * Credits go to Peter Jay Salzman, Michael Burian, Ori Pomerantz for their The Linux Kernel Module Programming Guide availble at http://www.tldp.org/LDP/lkmpg/2.6/html/index.html
*/

#define DRIVER_AUTHOR "Andre Gruning <libredcc@web.de>"
#define DRIVER_DESC   "Send and receive DCC signals on a GPIO"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>


#include "dcc_module.h"

// module parameters:
int dcc_gpio = DEFAULT_DCC_GPIO;
module_param(dcc_gpio, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(dcc_gpio, "GPIO pin used for the [input|output] of the dcc signal.");

// file operations:
//static int dcc_open(struct inode *, struct file *);
//static int dcc_release(struct inode *, struct file *);
//static ssize_t dcc_read(struct file *, char *, size_t, loff_t *);
//static ssize_t dcc_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "dcc"

static int major;   // major device number
static int opened = 0; // has the device been opened?

static struct file_operations fops = {
  //	.read = dcc_read,
  //	.write = dcc_write,
  //	.open = dcc_open,
  //	.release = dcc_release
};


			
// the beginning and the end:

int __init dcc_init(void)
{
	printk(KERN_INFO "DCC service starting.\n");
	
	major = register_chrdev(0, DEVICE_NAME, &fops);




	if (major < 0) {
	  printk(KERN_ALERT "Registering device " DEVICE_NAME" failed with %d\n", major);
	  return major;
	}

	return 0;
}

void __exit dcc_exit(void)
{

  int ret = unregister_chrdev(major, DEVICE_NAME);
  if (ret < 0) 
    printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
  printk(KERN_INFO "DCC service ending.\n");
}

module_init(dcc_init);
module_exit(dcc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("dcc_out");
