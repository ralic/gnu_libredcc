/* \file




 */ 

/*
 * Copyright (C) 2015 Andre Gruning 
 *
 * Credits to 
 * - Peter Jay Salzman, Michael Burian, Ori Pomerantz for their The Linux Kernel Module Programming Guide 
 *   <http://www.tldp.org/LDP/lkmpg/2.6/html/index.html>.
 * - Pete Batard <pete@akeo.ie> for teaching me how to use sysfs
 <http://pete.akeo.ie/search/label/kfifo> 
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/kernel.h>
//#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
// #include <linux/gpio.h>
// #include <linux/interrupt.h>
//#include <mach/hardware.h>
//#include <asm/io.h>


#include "dcc.h"
//#include "../dcc/simple_dcc/unix/dcc_encoder_hw.h"


/**** module parameters ****/


/// major device number
static int major = DEVICE_MAJOR; 
module_param(major, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(major, "Major device number used for dcc device.");

static struct class* class = NULL;
static struct device* device = NULL;
static bool signal = false;




static enum {level_nothing, level_chrdev, level_class, level_device, level_sysfs, level_running} init_level = level_nothing; 
static void unwind(void);





/**** device fops ****/

static bool opened = false;

static int open (struct inode * i, struct file * f) {
  if(opened) return -EBUSY; // thread-save?
  // check requested permissions here?
  opened = true;
  printk(KERN_INFO DEVICE_NAME " opened.\n");
  return 0;
}

static int release (struct inode * i, struct file * f) {
  opened = false;
  printk(KERN_INFO DEVICE_NAME " closed.\n");
  return 0;
}

static ssize_t read (struct file * f , char __user * u, size_t s, loff_t * l) {
  printk(KERN_INFO DEVICE_NAME " read attempt.\n");
  return -EINVAL;
}

//* \todo ought to be in DMA memory right from the start?
static uint32_t buffer[WORDS];
//static unsigned buf_used = 0;


/** 
    - we should read only until the next 0xAAAAAAA word.
*/
static ssize_t write (struct file * f, const char __user * user, size_t size, loff_t * l ) {


  char* str_buf = (char*) buffer;

  size = (size / 4) * 4; // round to the nearest 4 byte boundary. 
  if(size == 0) return -EAGAIN;
  else {
    if(size > sizeof(buffer)) size = sizeof(buffer);
  }
  printk(KERN_INFO DEVICE_NAME "attempt writing %lx bytes to the buffer\n", size);

  size-= copy_from_user(buffer,user,size); // can copy_from_user return an error?

  printk(KERN_INFO DEVICE_NAME "actually written %lx bytes to the buffer\n", size);




  printk("%.*s\n", (int) size, str_buf);

  return size;

}


static struct file_operations fops = {
  .read = read,
  .owner = THIS_MODULE,
  .write = write,
  .open = open,
  .release = release
};




/**** sysfs operations ****/

static ssize_t show_signal(struct device *dev, struct device_attribute *attr, char *buf) {
  return scnprintf(buf, PAGE_SIZE, "%s\n", signal ? "on" : "off");
}

static ssize_t store_signal(struct device *dev, struct device_attribute *attr, char *buf, size_t count) {

  signal = (strncmp("on", buf, PAGE_SIZE) == 0) ? true : false;
  printk(KERN_INFO DEVICE_NAME " signal generation switched %s.\n", signal ? "on" : "off");

  return count;
}

static DEVICE_ATTR(signal, S_IWUSR | S_IRUSR, show_signal, store_signal);

/**** lifecycle functions of the module ****/

int __init dcc_init(void) {

  int ret = 0;
  
  //* setup chardev
  //* @todo: transfer to new device interface.
  major = register_chrdev(major, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ALERT "Registering device " DEVICE_NAME " failed with %d\n", major);
    unwind();
    return major;
  }
  init_level = level_chrdev;
  printk(KERN_INFO "DCC service has major device number %d.\n", major);



  /** create virtual device class */

  class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(class)) {
    printk(KERN_ALERT "failed to register device class '%s'\n", CLASS_NAME);
    unwind();
    return PTR_ERR(class);
  }
  init_level = level_class;
 
  /* create device for sysfs */
  device = device_create(class, NULL, MKDEV(major, 0), NULL, CLASS_NAME "_" DEVICE_NAME);
  if (IS_ERR(device)) {
    printk(KERN_ALERT "failed to create device '%s_%s'\n", CLASS_NAME, DEVICE_NAME);
    unwind();
    return PTR_ERR(device);
  }
  init_level = level_device;

 
  /* sysfs files:

   * dev_attr_fifo and dev_attr_reset come from the DEVICE_ATTR(...) earlier */
  ret = device_create_file(device, &dev_attr_signal);
  if (ret < 0) {
    printk(KERN_ALERT "failed to create /sys endpoint '%s'\n", "signal");
    unwind();
    return ret;
  }
  init_level = level_sysfs ;


  /** here goes the next step of init */

  init_level = level_running;
  printk(KERN_INFO "DCC service starting sucessfully.\n");
  return ret;
}

static void unwind(void) {

  printk(KERN_INFO "Unwinding from init level %d.\n", init_level);

  switch(init_level) {
  default:
  case level_running:
  case level_sysfs:
    device_remove_file(device, &dev_attr_signal);

  case level_device:
    device_destroy(class, MKDEV(major, 0));
  case level_class:
    class_destroy(class);
  case level_chrdev:
    unregister_chrdev(major, DEVICE_NAME); 
    printk(KERN_INFO DEVICE_NAME "unregistered major %d.\n", major);
  case level_nothing: 
    break;
  }
}

void __exit dcc_exit(void)
{
  unwind();
  printk(KERN_INFO "DCC service ending.\n");
}

module_init(dcc_init);
module_exit(dcc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("dcc_out");
