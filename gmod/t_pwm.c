#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/pwm.h>
#include <asm/io.h> // for readl / writel
#include <mach/platform.h> // for __io_address

#include "defs.h"


/*** copy from pwm module ****/







/**** module parameters ****/

/// \todo what is the class name used for?
static struct class* class = NULL;

/// \todo shall I do my own device structure
static struct device* device = NULL;

/**** lifecycle functions of the module ****/

static struct pwm_device *pd = NULL;

/** \todo aquicre resource only in open **/
int __init test_pwm_init(void) {

  int ret = 0;

#if 0
  #define CLASS_NAME "Test-pwm32"
  class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(class)) {
    printk(KERN_ERR "failed to register device class '%s'\n", CLASS_NAME);
    return PTR_ERR(class);
  }

  #define DEVICE_NAME "Test=pwm"
  device = device_create(class, NULL, MKDEV(0, 0), NULL, DEVICE_NAME);
  //device = device_create(NULL, NULL, MKDEV(0,0), NULL, DEVICE_NAME);
  if (IS_ERR(device)) {
    printk(KERN_ERR "failed to create device '%s'\n", DEVICE_NAME);
    return PTR_ERR(device);
  }
#endif
  #define PWM_NUMBER 0
  //pd = pwm_get(0, "my label"); // or use bcm2708-pwm or checkwith device tree -- is it inclded? and is the devi
   pd = pwm_request(0, "my label");
  if(IS_ERR(pd)) {
    printk(KERN_ALERT "Requesting PWM %d failed with %ld.\n", PWM_NUMBER, PTR_ERR(pd));
    //unwind_setup(init_level);
    return PTR_ERR(pd);
  }

  ret = pwm_config(pd, 1000000, 2000000);
  if(ret) {
    printk(KERN_ALERT "Config PWM %d failed with %d.\n", PWM_NUMBER, ret);
    return ret;
  }

  ret = pwm_enable(pd);
  if(ret) {
    printk(KERN_ALERT "Enableing PWM %d failed with %d.\n", PWM_NUMBER, ret);
    return ret;
  }

  // noting is visible on the output -- perhaps the pwm-module or device-tree does not set gpio18


  printk(KERN_INFO __FILE__ "service starting sucessfully.\n");


  //  writel(0, __io_address(PWM_BASE)); // does not work w or w/o __io_address
  // iowrite32(0, __io_address(PWM_BASE)); // does not work // but 
  // does not work: the pwm does not come on -- set gpio pin?


  struct bcm2835_pwm *pc = to_bcm2835_pwm(pd->chip);

  printk(KERN_INFO __FILE__ " PWM base address: %x\n", pc);


  return ret;
}

/** release resources in the reverse order in which they were
    acquired, starting from the latest successfull level. */

void __exit test_pwm_exit(void)
{
  pwm_free(pd);
  printk(KERN_INFO __FILE__ " service ending.\n");
}

module_init(test_pwm_init);
module_exit(test_pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test-pWM");	
MODULE_DESCRIPTION("Test-pwm");

