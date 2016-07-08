#ifndef PWM_H
#define PWM_H

#include <linux/pwm.h>

/// number of global PWM device to request
#define PWM_NUMBER 0

/// pointer to platform specific pwm device
extern struct bcm2835_pwm *pwm;

/// pointer to 
extern struct pwm_device *pd;

/// init the pwm
int pwm_init(void);

/// undp the init
void pwm_unwind(void);

/** hardware definitions lacking from the kernel tree so far or not
    accessible as they are not exported in a header.
    Compare "BCM2835 ARM Peripherals", Sect 9.
    \todo should go to some platform file
*/

/// PWM Control register and its bits
#define PWM_CTL 0x0 // offset from PWM_BASE

#define PWEN1 1 << 0  /// enable
#define MODE1 1 << 1  /// 0 is PWM mode, 1 is serial mode
#define RPTL1 1 << 2  /// repeate last word in FIFO mode
#define SBIT1 1 << 3  /// state of pwm output pin with no tranmission
#define POLA1 1 << 4  /// inversion of polariy when transmitting
#define USEF1 1 << 5  /// use fifo
#define CLRF1 1 << 6  /// clear fifo
#define MSEN1 1 << 7  /// do not use sophisticatted PWM algo

/// PWM DMA Configuration and its bits
#define PWM_DMAC 0x8

#define ENAB 1 << 31
#define PANIC(__x) ((__x) << 8)
#define DREQ(__x) ((__x) << 0)

/// PWM Channel 1 Range register
#define PWM_RNG1 0x10 // duration of one cycle

/// PWM Channel 1 Data register
#define PWM_DAT1 0x14 // data for the cycle

/// PWM Channel 1 Fifo Input
#define PWM_FIF1 0x18 // data via FIFO

#endif
