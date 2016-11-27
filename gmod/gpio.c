#include <linux/module.h>


#include <linux/gpio.h>
#include "gpio.h"

#include "defs.h"

/** \todo: get this from the pwm device tree entry?
 possible are GPIOs 12 , 40, 18 (pwm0) and
 13, 19 for pwm1 -- only 18 is possible on my rpi (alt5)  
*/
static int pwm_pin =   18; 
module_param(pwm_pin, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(pwm_pin, "GPIO pin used for the output of the pwm signal.");

static char* const compatible_gpio_chips[] = {"bcm2708_gpio", "pinctrl-bcm2835"}; // init only?
static const size_t NUM_COMP_GPIO_CHIPS = sizeof(compatible_gpio_chips) / sizeof(char*); // init only?

// helper function inspired by lirc_rpi.c
inline static int __init is_compatible_gpio_chip(struct gpio_chip *chip, void *dummy) {

  int i;
  printk(KERN_INFO "GPIO Chip %s\n", chip->label);

  for(i = 0; i < NUM_COMP_GPIO_CHIPS; i++) {
    if (strcmp(compatible_gpio_chips[i], chip->label) == 0)
      return 1;
  }
  return 0;
}

static enum {gpio_nothing, gpio_got_pin} gpio_init_level = gpio_nothing; 

int __init gpio_init(void) {

  int ret; 

  // using pinctrl would be cleaner.
  ret = gpio_request(pwm_pin, "PWM Pin");
  if(ret < 0) {
    printk(KERN_ALERT "Requesting GPIO %d failed with %d.\n", pwm_pin, ret);
    gpio_unwind();
    return ret;
  } 
  gpio_init_level = gpio_got_pin;
	
  struct gpio_chip* gc = gpiochip_find(NULL, is_compatible_gpio_chip);
  if(gc == NULL) {
    printk(KERN_ALERT "No gpiochip found.\n");
    gpio_unwind();
    return -ENODEV;
  }

  /*
    struct pinctrl *pinctrl;
    int gpio;
    //	struct device;
    struct device dev;

    pinctrl = devm_pinctrl_get_select_default(&dev);
    if(IS_ERR(pinctrl)) {
    printk(KERN_INFO "No pinctrl default device: %i\n", -1 //ERR_INT(pinctrl)
    );
    }
	

    gpio = devm_gpio_request(&dev, pwm_pin, "gpio18");
    if(gpio < 0) {
    printk(KERN_INFO "Could not request GPIO %s: %i\n", "foo", gpio);ERT "Aborting because selected GPIO %d can sleep.\n", pwm_pin);
    unwind_setup(init_level);
    return ret;
    }
  */

  //	ret = gpio_direction_output(dcc_in, GPIOF_INIT_LOW);
  //if(ret < 0) {
  //  printk(KERN_ALERT "Setting up GPIO %d as output failed with %d.\n", dcc_in, ret);
  //  unwind_setup(init_level);
  //  return ret;
  //}

  printk(KERN_INFO "Successfully requested pin %u for PWM.", pwm_pin);
  return ret;
}

void gpio_unwind(void) {

  switch(gpio_init_level) {
  default:
  case gpio_got_pin:
    // and set pin back to input -- so that we have no voltages at the pin
    gpio_free(pwm_pin);

  case gpio_nothing: {
    // nothing to do.
  }
  }
}