#ifndef CHIP_H
#define CHIP_H 1

#include <avr/io.h>
// \todo how to test that timer 1 does not have 16bit? Well, check whether there is TCNTxH?

/**
   The INT0 pin is the DCCPIN:
 */
#if  defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#define DCCPORT D
#define PROGPORT DCCPORT

#define DCCPIN PD2
#define PROGPIN PD3

#define IOPORT B

#define IOTIMER 0
#define DCCTIMER 2

#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__)
#define DCCPORT B
#define DCCPIN PB2
#define PROGPORT DCCPORT
#define PROGPIN PB5

#define IOPORT B

#define IOTIMER 1
#define DCCTIMER 0

#define HELPERPIN PB3

#else
#error Pins not yet defined for this AVR chip
#endif

#define PINx(__x) __PINx(__x)
#define PORTx(__x) __PORTx(__x)
#define DDRx(__x) __DDRx(__x)

#define __PINx(__x) PIN ## __x
#define __PORTx(__x) PORT ## __x
#define __DDRx(__x) DDR ## __x

// registers for setting up the INT0 external interrupt:
#ifndef EIFR
#define EIFR GIFR
#endif

#ifndef EIMSK
#define EIMSK GIMSK
#endif

#ifndef EICRA 
#define EICRA MCUCR
#endif

#if defined TCNT1 && !defined TCCR1A
#define TCCR1A TCCR1
#define TCCR1B TCCR1
#endif

#define __CONCAT3(__x, __y, __z) __x ## __y ## __z

#ifdef TIMSK0
#define TIMSKx(__x) __CONCAT3(TIMSK, __x, )
#else
#define TIMSKx(__x) TIMSK
#endif

#define CSx0(__x) __CONCAT3(CS, __x, 0)
#define CSx1(__x) __CONCAT3(CS, __x, 1)
#define CSx2(__x) __CONCAT3(CS, __x, 2)
#define CSx3(__x) __CONCAT3(CS, __x, 3)

// prescalers:

#define PRESCALER_STOP(__x) 0
#define PRESCALER_1(__x) _BV(CSx0(__x))
#define PRESCALER_8(__x) __CONCAT3(PRESCALER_8_, __x,)  
#define PRESCALER_1024(__x) __CONCAT3(PRESCALER_1024_, __x,)
#define PRESCALER_1024_0 _BV(CS02) | _BV(CS00)
#define PRESCALER_8_0 _BV(CS01)

#ifdef __AVR_ATmega328P__  

#define PRESCALER_8_1 _BV(CS11)
#define PRESCALER_8_2 _BV(CS21)

#define PRESCALER_1024_1 _BV(CS12) | _BV(CS10)
#define PRESCALER_1024_2 _BV(CS22) | _BV(CS21) | _BV(CS20)

//#warning No fuse programmed b/o Arduino

#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__)

#define PRESCALER_8_1 _BV(CS12)
#define PRESCALER_1024_1 _BV(CS13) | _BV(CS11) | _BV(CS10)

#else
// #warning no fuses being programmed
#warning unknown architecture
#endif

#define PREACALER_1024(__x) __CONCAT3(PRESCALER_1024_, __x,)

#define TIMERx_COMPA_vect(__x) __CONCAT3(TIMER, __x, _COMPA_vect)
#define TIMERx_OVF_vect(__x) __CONCAT3(TIMER, __x, _OVF_vect)

#define TOIEx(__x) __CONCAT3(TOIE, __x,)

#define TCCRxB(__x) __TCCRxB(__x)
#define __TCCRxB(__x) TCCR ## __x ## B

#define TCCRxA(__x) __TCCRxA(__x)
#define __TCCRxA(__x) TCCR ## __x ## A

#define OCRxA(__x) __OCRxA(__x)
#define __OCRxA(__x) OCR ## __x ## A

#define OCIExA(__x) __OCIExA(__x)
#define __OCIExA(__x) OCIE ## __x ## A

#define TCNTx(__x) __TCNTx(__x)
#define __TCNTx(__x) TCNT ## __x 

#define nop() do { asm volatile ( "nop"); } while(0)

#endif
