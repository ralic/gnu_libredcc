//! add to platform.h
#define PWM_BASE (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */
#define CM_BASE (BCM2708_PERI_BASE + 0x101000) /* Clock manager */ 




#warning modules complains that it cannot map f220c000 -- this is the pwm? (but than can be solved by loading the pwm driver first
#warning it also complains re the clock manage -- so we need to make this first, too!


//! add to mach/gpio.h, remove from ~/build/arch/arm/mach-bcm2708/bcm2708_gpio.c
enum { GPIO_FSEL_INPUT, GPIO_FSEL_OUTPUT,
	GPIO_FSEL_ALT5, GPIO_FSEL_ALT_4,
	GPIO_FSEL_ALT0, GPIO_FSEL_ALT1,
	GPIO_FSEL_ALT2, GPIO_FSEL_ALT3,
};

