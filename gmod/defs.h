#include <mach/platform.h> // for __io_address


//! add to platform.h


#define PWM_BASE (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */ 
/** still need it to calculate the physical address (or would need the physical address instead **/


#define CM_BASE (BCM2708_PERI_BASE + 0x101000) /* Clock manager */ 


#if 1
// not needed as pwm / device tree code does the setup
//! add to mach/gpio.h, remove from ~/build/arch/arm/mach-bcm2708/bcm2708_gpio.c
enum { GPIO_FSEL_INPUT, GPIO_FSEL_OUTPUT,
	GPIO_FSEL_ALT5, GPIO_FSEL_ALT_4,
	GPIO_FSEL_ALT0, GPIO_FSEL_ALT1,
	GPIO_FSEL_ALT2, GPIO_FSEL_ALT3,
};
#endif

/*** copy from pwm module 
     in order to get access to the virtual address of the pwm module 
     @todo Find a better place to put this, or put it into some kernel header and export the symbol
****/

#include <linux/pwm.h>
struct bcm2835_pwm {
        struct pwm_chip chip;
        struct device *dev;
        unsigned long scaler;
        void __iomem *base;
        struct clk *clk;
};

static inline struct bcm2835_pwm *to_bcm2835_pwm(struct pwm_chip *chip)
{
        return container_of(chip, struct bcm2835_pwm, chip);
}

