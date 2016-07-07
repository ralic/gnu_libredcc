/**
 * @file   dcc.c
 * @author Copyright (C) 2015, 2016 Andre Gruning 
 *         GNU licence v3 or later
 * @date   Sun Jan 17 18:42:51 2016
 * 
 * @brief  
 * 
 * Credits to 
 * - Peter Jay Salzman, Michael Burian, Ori Pomerantz for their The Linux Kernel Module Programming Guide 
 *   <http://www.tldp.org/LDP/lkmpg/2.6/html/index.html>.
 * - Pete Batard <pete@akeo.ie> for teaching me how to use sysfs
     <http://pete.akeo.ie/search/label/kfifo> 
   - John Linn, "Linux DMA in Device Drivers"
 * \todo add the only book from O`Reilly
 * \todo add the slide set from xilinx
 * \todo make my own device structure.
   \todo run userspace programme with strace.
   \todo should we allow asynchronous notification? For IAV?
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#include "pwm.h"
#include "dcc.h" // \todo include "../dcc/simple_dcc/unix/dcc_encoder_hw.h" 
#include "gpio.h"
#include "pwm.h"
#include "dma.h"
#include "buffer.h"

#include "defs.h"



/**** module parameters ****/

static int major = DEVICE_MAJOR; 
module_param(major, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(major, "Major device number of " DEVICE_NAME " device.");

/**** module global variables -- \todo perhaps better in a device structure? */

/// \todo what is the class name used for?
static struct class* class;

/// \todo shall I do my own device structure
static struct device* device;

/** true if we are currently producing an output signal, false otherwise. */
static bool signal = false;


/** encodes the level of resource initialisation and allcation we have
    successfully done. 
    \todo for some of the resource there is probably manage allocation. */
static enum {level_nothing, level_chrdev, level_class, level_device, level_sysfs, 
	     level_pwm, level_dma, 
	     level_running} init_level = level_nothing;

/** release resources.*/
static void unwind(void); 


/**** fops ****/

static atomic_t available = ATOMIC_INIT(1); 

/** open the dcc device. Allow only one client have device open at a
    time.

    \todo deny opening for reading
    \todo allow several writers because in the end all is reentrend up
    \todo move all the initialise from init to open.
    \todo up to the point where we commit to DMA? -- make it reentrent.
*/
static int open (struct inode * i, struct file * f) {

  if( !atomic_dec_and_test(&available)) {
    atomic_inc(&available); 
    return -EBUSY; 
  }
  nonseekable_open(i,f); 
  return 0;
}

/**
   release fop
   \todo do all the tyding up in here.
 */
static int release (struct inode * i, struct file * f) {

  atomic_inc(&available);
  return 0;
}

static ssize_t write (struct file * f, const char __user * user, size_t size, loff_t * l ) {

  u32* data;
  int written;
  dma_addr_t dma_handle;

  if(size > PAGE_SIZE) size = PAGE_SIZE; 
  if(size % sizeof(*data)) 
    printk(KERN_WARNING "Submitting a number of bytes not aligned with width of DMA channel"); 

  data = kmalloc(size, GFP_DMA | GFP_KERNEL); // \todo replace with pool?
  if(data == NULL) {
    printk(KERN_ERR "No DMA mappable memory.\n");
    // \todo block if no dma mappable memory?
    return -ENOMEM;
  }

  written = size - copy_from_user(data,user,size); // \todo can copy_from_user return an error?

  dma_handle = dma_map_single(NULL, data, written, DMA_MEM_TO_DEV); // add the device -- but which? PWM?
  if (dma_mapping_error(NULL, dma_handle)) { // first argument is dev?
    printk(KERN_ERR "Can't map to DMA address space.");
    // block if no mapping into DMA space?
    kfree(data);
    return -ENOMEM; // or return value of mapping error?
  } 

  submit_dma_single(dma_handle, written, data);
  return written;

}

/// file operations this module supports.
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .write = write,
  .open = open,
  .poll = NULL, // \todo implement?
  .llseek = no_llseek, // required?
  .release = release
};


/**** sysfs operations ****/

static ssize_t show_signal(struct device *dev, struct device_attribute *attr, char *buf) {
  return scnprintf(buf, PAGE_SIZE, "%s\n", signal ? "on" : "off");
}

static ssize_t store_signal(struct device *dev, struct device_attribute *attr, char *buf, size_t count) {

  if (strncmp("on", buf, 2) == 0) {
    pwm_enable(pd);
    signal = true;
  }
  else {
    pwm_disable(pd); 
    signal = false;
  }
  return count;
}

/// \todo get rid of the warning on this line.
static DEVICE_ATTR(signal, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP, show_signal, store_signal);


/**** lifecycle functions of the module ****/

int __init dcc_init(void) {

  int ret = 0;
  
  /// setup chardev
  /// \todo do we need this as we are creating a new device anyway?

#if 1
  major = register_chrdev(major, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ERR "Registering device " DEVICE_NAME " failed with %d\n", major);
    unwind();
    return major;
  }
  init_level = level_chrdev;
  printk(KERN_INFO "DCC service has major device number %d.\n", major);
#endif

  /** create virtual device class 
      @todo why required?
      @todo better chose an existing device class?
      @todo deal with error in case the device class exists already?
  */
  class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(class)) {
    printk(KERN_ERR "failed to register device class '%s'\n", CLASS_NAME);
    unwind();
    return PTR_ERR(class);
  }
  init_level = level_class;
 
  /* create device */
  device = device_create(class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
  if (IS_ERR(device)) {
    printk(KERN_ERR "failed to create device '%s_%s'\n", CLASS_NAME, DEVICE_NAME);
    unwind();
    return PTR_ERR(device);
  }
  init_level = level_device;

 
  /* create sysfs files */
  ret = device_create_file(device, &dev_attr_signal);
  if (ret < 0) {
    printk(KERN_ERR "failed to create sysfs endpoint '%s'\n", "signal");
    unwind();
    return ret;
  }
  init_level = level_sysfs ;

#if 0
  /* optionally grap GPIO (pwm-bcm2835 does not do it) -- so that nobody else can get it */
  ret = gpio_init(); // could also be given the device structure.
  if (ret < 0) {
    printk(KERN_WARN "failed to acquire gpio pin.\n");
    return ret;
  }
  init_level = level_gpio;
#endif 

  /**** acquire pwm resources ****/
  ret = pwm_init(); 
  if (ret < 0) {
    printk(KERN_ERR "failed to acquire pwm device.\n");
    unwind();
    return ret;
  }
  init_level = level_pwm;


  /**** acquire dma resources ****/
  ret = dma_init(); // could also be given the device structure as
		      // an argument.
  if (ret < 0) {
    printk(KERN_ERR "failed to acquire dma device.\n");
    unwind();
    return ret;
  }
  init_level = level_dma;

  init_level = level_running;
  printk(KERN_INFO "DCC service starting sucessfully.\n");
  return ret;
}

static void unwind(void) {

  switch(init_level) {
  default:
  case level_running:
  case level_dma:
    dma_unwind();
  case level_pwm:
    pwm_unwind();
  case level_sysfs:
    device_remove_file(device, &dev_attr_signal);
  case level_device:
    device_destroy(class, MKDEV(major, 0));
  case level_class:
    class_destroy(class);
  case level_chrdev:
    unregister_chrdev(major, DEVICE_NAME); 
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
