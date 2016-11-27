#define DRIVER_AUTHOR "Andre Gruning <libredcc@web.de>"
#define DRIVER_DESC   "Send and receive DCC signals on a GPIO"

//#define DEVICE_MAJOR 0
#define DEVICE_NAME "dcc"

#define DEFAULT_DCC_IN_GPIO 23
//#define DEFAULT_DCC_OUT_GPIO 24

// System Timer Registers and bit masks, cp BCM2835 ARM Peripherals, Ch 12
#define CS 0x0
#define Mn(__timer) BIT(__timer)

#define CLO 0x04

// System Timer Compare Register for __timer
#define C0 0xC
#define Cn(__timer) (C0 + 0x4*(__timer)) 

#define DEFAULT_TIMER 1 // Timer 0 seems to be used from somewhere, Timer 3 is used as base of ticks

#define DEFAULT_PERIOD 10000 // 10ms = 10000us

#define IRQ_TIMER(__timer) (IRQ_TIMER0 + (__timer))

