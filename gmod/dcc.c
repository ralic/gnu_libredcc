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
   \todo do we need to use volatile in the kernel?
   \todo should we allow asynchronous notification? For IAV?
   \todo can we assume that the OS does not break down short writes into shorter ones?
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#include <asm/io.h> // for readl / writel
#include <mach/platform.h> // for __io_address
#include "pwm.h"



#include "dcc.h" // \todo inc
//#include "../dcc/simple_dcc/unix/dcc_encoder_hw.h" \todo use this include file.
#include "gpio.h"
#include "pwm.h"
#include "dma.h"
#include "buffer.h"

/**** module parameters ****/


/// major device number
static int major = DEVICE_MAJOR; 
module_param(major, int, S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(major, "Major device number used for " DEVICE_NAME " device.");

/**** module global variable -- \todo perhaps better in a device structure? */

/// \todo what is the class name used for?
static struct class* class = NULL;

/// \todo shall I do my own device structure
static struct device* device = NULL;

/** true if we are currently producing an output signal, false
    otherwise. */
static bool signal = true;


/** encodes the level of resource initialisation and allcation we have
    successfully done. 
    \todo for some of the resource there is probably manage allocation. */
static enum {level_nothing, level_chrdev, level_class, level_device, level_sysfs, 
	     level_gpio, level_pwm, level_dma, 
	     level_running} init_level = level_nothing;

/** function to release / undo resource allocation.*/
static void unwind(void); 


/**** device fops ****/

/// atomic flag to indicate whether dcc device is in use or available. 
static atomic_t available = ATOMIC_INIT(1); 

/** open the dcc device. Allow only one client have device open at a
    time.

    \todo deny  opening for writing
    \todo allow several openers because in the end all is reentrend up
    \todo move all the initialise from init to open.
    \todo up to the point where we commit to DMA? -- make it reentrent.
*/
static int open (struct inode * i, struct file * f) {

  if( !atomic_dec_and_test(&available)) {
    atomic_inc(&available); 
    return -EBUSY; 
  }

  nonseekable_open(i,f); 
  printk(KERN_INFO DEVICE_NAME " opened.\n");
  return 0;
}

/**
   release fop
   \todo do all the tyding up in here.
 */
static int release (struct inode * i, struct file * f) {

  atomic_inc(&available);
  printk(KERN_INFO DEVICE_NAME " closed.\n");
  
  return 0;
}

/** @todo should block if no data available (from [bigbook]?)
    @todo just give back chuncks of data that are convenient (think of
    IAV implementation)
    
    Currently does nothing as we do not intend to read much from here.
*/
static ssize_t read (struct file * f , char __user * u, size_t s, loff_t * l) {
 
  printk(KERN_INFO DEVICE_NAME " read attempt.\n");
  return -EINVAL;
}

/** 
    @todo
    - must be reentrant due to copy_user which can wait.
    - could I block? Yes, no problems with that?
    - If I accepted less bytes than count the caller will most
    probably retry immediately -- ie the driver will be stuck as it is
    expecting more bytes,  
    @todo perhaps just raise a warning if the data to be send is not a
    multiple of the dma channel width.
    @todo was there a convention to first return 0 bytes and the an error?
*/

#define WIDTH sizeof(u32) // \todo get this value direct from the DMA headers
//#define ROUNDUP(_size, _width) ( ( ( (_size) + (_width) - 1) / (_width) ) * (_width) )

static ssize_t write (struct file * f, const char __user * user, size_t size, loff_t * l ) {

  if(size > PAGE_SIZE) size = PAGE_SIZE; 
  if(size % WIDTH) 
    printk(KERN_INFO "Submitting a number of bytes not aligned with width of DMA channel"); 

  u32 *data = kmalloc(size, GFP_DMA | GFP_KERNEL); // \todo replace with pool?
  if(data == NULL) {
    printk(KERN_INFO "No DMA mappable memory.\n");
    // block if no dma mappable memory -- most probably not necessary for DCC application
    return -ENOMEM;
  }

  int written = size - copy_from_user(data,user,size); // \todo can copy_from_user return an error?

  dma_addr_t dma_handle = dma_map_single(NULL, data, written, DMA_MEM_TO_DEV); // add the device -- but which? PWM?
  if (dma_mapping_error(NULL, dma_handle)) { // first argument is dev?
    printk(KERN_INFO "Can't map to DMA address space.");
    // block if no mapping into DMA space?
    kfree(data);
    return -ENOMEM; // or return value of mapping error?
  } 

  submit_dma_single(dma_handle, written, data);

  printk(KERN_INFO DEVICE_NAME "actually written %x bytes to DMA\n", written);

  return written;

}

/// file operations this module supports.
static struct file_operations fops = {
  .read = NULL, 
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

  // insert here dma_pause and dma_resume, or stop the pwm directly?

  u32 pwm_ctl = readl(__io_address(PWM_BASE + PWM_CTL));


  if (strncmp("on", buf, 2) == 0) {
    writel(pwm_ctl | PWEN1, __io_address(PWM_BASE + PWM_CTL)); 
    signal = true;
  }
  else {
    writel(pwm_ctl & ~(PWEN1), __io_address(PWM_BASE + PWM_CTL)); 
    signal = false;
  }


  printk(KERN_INFO DEVICE_NAME " signal generation switched %s.\n", signal ? "on" : "off");

  return count;
}

/// \todo get rid of the warning on this line.
static DEVICE_ATTR(signal, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP, show_signal, store_signal);

/**** lifecycle functions of the module ****/

/** \todo aquicre resource only in open **/
int __init dcc_init(void) {

  int ret = 0;
  
  /// setup chardev
  /// @todo: transfer to new device interface.
  major = register_chrdev(major, DEVICE_NAME, &fops);
  if (major < 0) {
    printk(KERN_ERR "Registering device " DEVICE_NAME " failed with %d\n", major);
    unwind();
    return major;
  }
  init_level = level_chrdev;
  printk(KERN_INFO "DCC service has major device number %d.\n", major);

  /** create virtual device class 
      @todo why required?
      @todo better chose an existing device class?
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

 
  /* sysfs files:
   * dev_attr_signals comes from the DEVICE_ATTR(...) earlier */
  ret = device_create_file(device, &dev_attr_signal);
  if (ret < 0) {
    printk(KERN_ERR "failed to create /sys endpoint '%s'\n", "signal");
    unwind();
    return ret;
  }
  init_level = level_sysfs ;

  /*** init dma resources we require *****/

  ret = gpio_init(); // could also be given the device structure.
  if (ret < 0) {
    printk(KERN_ERR "failed to acquire gpio pin.\n");
    unwind();
    return ret;
  }
  init_level = level_gpio;

  /**** acquire pwm resources ****/
  ret = pwm_init(); // could also be given the device structure as
		      // an argument.
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


  /** here goes the next step of init */



  init_level = level_running;
  printk(KERN_INFO "DCC service starting sucessfully.\n");
  return ret;
}

/** release resources in the reverse order in which they were
    acquired, starting from the latest successfull level. */
static void unwind(void) {

  printk(KERN_INFO "Unwinding from init level %d.\n", init_level);

  switch(init_level) {
  default:
  case level_running:
  case level_dma:
    dma_unwind();
  case level_pwm:
    pwm_unwind();
  case level_gpio:
    gpio_unwind();
  case level_sysfs:
    device_remove_file(device, &dev_attr_signal);
  case level_device:
    device_destroy(class, MKDEV(major, 0));
  case level_class:
    class_destroy(class);
  case level_chrdev:
    unregister_chrdev(major, DEVICE_NAME); 
    printk(KERN_INFO DEVICE_NAME " unregistered major %d.\n", major);
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
