#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/pwm.h>
#include <linux/clk.h>

#include "defs.h"

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

#if 1
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
    printk(KERN_ERR "failed to create device '%s with error %d'\n", DEVICE_NAME, PTR_ERR(device));
    return PTR_ERR(device);
  }
#endif


  #define PWM_NUMBER 0
  //pd = devm_pwm_get(NULL, "2020c000.pwm@0"); // or use bcm2708-pwm or checkwith device tree -- is it inclded? and is the devi
  pd = pwm_request(PWM_NUMBER, "my label");
  if(IS_ERR(pd)) {
    printk(KERN_ALERT "Requesting PWM %d failed with %ld.\n", PWM_NUMBER, PTR_ERR(pd));
    return PTR_ERR(pd);
  }

  ret = pwm_config(pd, 1000000, 2000000);
  if(ret) {
    printk(KERN_ALERT "Config PWM %d failed with %d.\n", PWM_NUMBER, ret);
    return ret;
  }

  ret = pwm_enable(pd);
  if(ret) {
    dev_err(device, "Enableing PWM %d failed with %d.\n", PWM_NUMBER, ret);
    return ret;
  }

  dev_info(device, "service starting sucessfully.\n");


  struct bcm2835_pwm *pc = to_bcm2835_pwm(pd->chip);

  printk(KERN_INFO __FILE__ " PWM base address: %x\n", pc->base);
  printk(KERN_INFO __FILE__ " PWM name: %s\n", pd->label);
  printk(KERN_INFO __FILE__ " Device name: %s\n", dev_name(pd->chip->dev));

  
  struct clk *clock = devm_clk_get(pc->dev, NULL);
  //struct clk *clock = clk_get(NULL, "pwm");
  if(IS_ERR(clock)) {
    dev_err(device, "Get clock failed with %ld", PTR_ERR(clock));
    return PTR_ERR(clock);
  }

  ret = clk_prepare(clock);
  if(ret < 0) {
    dev_err(device, "Clock prepare failed with %d\n", ret);
  }


  ret = clk_set_rate(clock, 110000000);
  if(ret < 0) {
    dev_err(device, "Clock set rate failed with %d\n", ret);
  }
  

  ret = clk_enable(clock);
  if(ret) {
    dev_err(device, "Could not enable clock: %d", ret);
  }




  u64 rate = clk_get_rate(clock);
  dev_info(device, "Clock runninng at rate %lu", rate);
  clk_disable(clock);

    //struct clk* parent = clk_get_parent(clock);
  return ret;
}

/** release resources in the reverse order in which they were
    acquired, starting from the latest successfull level. */

void __exit test_pwm_exit(void)
{
  pwm_free(pd);
  device_destroy(class, device->devt);
  class_destroy(class);
  printk(KERN_INFO __FILE__ " service ending.\n");
}

module_init(test_pwm_init);
module_exit(test_pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test-pWM");	
MODULE_DESCRIPTION("Test-pwm");

