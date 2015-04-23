/* 
 * Copyright 2014 André Grüning <libredcc@email.de>
 *
 * This file is part of LibreDCC
 *
 * LibreDCC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LibreDCC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LibreDCC.  If not, see <http://www.gnu.org/licenses/>.
 */
// $Id$

#include <avr/io_hw.h>
#include "error.h"
#include <avr/chip.h>

// AVR

/** @todo
- check EINT und timer2 are enables / disabled in a time locked fashion
- check that for running EINT must occur before TIM2_int can occur again
- simplify TIM2 and TIM0 interrupt handling
- disable and enable the interrupts alternatingly, early sei() in the timer interrupt to allow early EINT0, and to really have strong time locking
*/

/*
 * The code uses interrupt INT0 to let timer2 rum for 3/4 of
 * PERIOD_1. (\see eint0_isr.S for the actual ISR)
 * whenever we get a rising edge of the DCC signal. If the timer has
 * elapsed then we sample the DCC signal again (using the timer2
 * match interrupt). 
 * If the signal is still high, then the current bit is a 0 and if it
 * has changed back to low, it is a 1, compare to [NRMA].
 * 
 * This code owes much to [XXXX] released under a GNU licence. However
 * as released on 20/12/2011 it contained a number of inaccurucies
 * etc. I reimplemented the code following the ideas there in a more
 * structured way. 
 *
 * We will use TMER2 for the timing of DCC signals as it is the timer
 * with the highedst priorty.
 * 
 * For the ATdmega328p pin PD2 doubles as INT0 -- so this is the pin we
 * are using to feed the DCC signal.
 */

#include<share/compose_packet.h>

#include<avr/power.h>
#include<avr/interrupt.h>
#include<share/bitqueue.h>

/**
 * useful for 16MHz -- might need to be chosen differently for a
 * different F_CPU
 */
#define PRESCALER 8 

/**
 * We sample the DCC signal 3/4 of the period of a 1 signal after a
 * rising edge. This corresponds to the following number of timer
 * ticks. (This is the average of half the standard durations of a
 * PERIOD_1 and a PERIOD_0 -- to sample the signal with a maximal
 * margin to the respective negative signal edges.
 *signal 
 */
#ifndef F_CPU
#error "F_CPU not defined"
#endif
#define TICKS_PER_US (F_CPU / 1000000)
#define SAMPLE_TICKS ((3 * PERIOD_1 * TICKS_PER_US) / (PRESCALER * 4))

// non essential error check -- not compulsory,
#if SAMPLE_TICKS > 250 
#error SAMPLE_TICKS to high (> 250)
#elif SAMPLE_TICKS < 10
#error SAMPLE_TICKS to low (<10)
#endif
//# "Sample ticks are " ## SAMPLE_TICKS
								 
/**
 * A c version of the ISR in \see eint0_isr.S.
 *
 * Assembler is not really necessary, as timing is not really
 * critically, but it is annoying to see how many instructions the
 * compiler uses when looking at the disassembled code.
 */
/*
ISR(INT0_vect) {
// start timer 2 with prescaler 8. 
  TCCR2B = _BV(CS21); 
  EIMSK &= ~(_BV(INT0)); // disable ourselves (why is it INT0 in the assembler file_
  EIFR |= _BV(INTF0); // clear any spurios interrupt that might have occoured -- is this neccessary
} */




/**
 * Interrupt service routine to sample the signal when timer2 has
 * gone SAMPLE_TICKS. It converts the signal into a bit. It then calls
 * the function compose_packet which composed the bits into a DCC
 * packet.
 */


ISR(TIMERx_COMPA_vect(DCCTIMER)) {
  // bit = 1 if PIND2 is high and bit = 0 if PIND2 is still low 87us after a falling edge.
  const uint8_t bit = sample_dccpin(); 

  // stop timer
  TCCRxB(DCCTIMER) = 0; 

  // reset timer
  TCNTx(DCCTIMER) = 0;


  queue_bit(bit); // from here onwards it is no longer hardware dependent.

  /* clear interrupt flag in case there was a falling edge in the
   * mean time, in order to ignore any pending interrupts that might
   * have occured while we where processing the bit. Compare eg 
   * 13.2.3 of [328].
   */
  EIFR |= _BV(INTF0);  
  EIMSK |= _BV(INT0); // reenable interrupt INT0.

}

/**
 * This function initialises the AVR hardware, especially timer2 and
 * PD2/INT0. It is executed before any main method.
 */  
void init_dcc_receiver() __attribute__((naked));
void init_dcc_receiver() __attribute__((section(".init8"))); // to be executed before main.
void init_dcc_receiver() {

  // enable timer2:
  power_timer_enable(DCCTIMER);

  // enable INT0 on falling edge -- this is often clearer than the rising edge of the signal. (Also a machine instruction might be saved later on when converting the read potential to the DCC bit).
  EICRA |= _BV(ISC01);  
  EIMSK |= _BV(INT0);

  // set timer2 to normal mode, no outputs needed:
  TCCRxA(DCCTIMER) = 0; 

  // set output compare register:
  OCRxA(DCCTIMER) =   SAMPLE_TICKS; // 87us // whatever came out up sample ticks
#warning Adatabe SAMPLE_TICKS and also the PRESCALER that is set in the assembler interrupt routine.

  // enable compare match interrupt
  TIMSKx(DCCTIMER) = _BV(OCIExA(DCCTIMER));
  
  // switch on pull-up for PD2/PINT0 (as optocoupler only goes to mass, or diode) --
  // DDRD &= ~(_BV(PD2)); // make pin 2 of portD input
  pullup_dccpin(); // switch on pull-up
}
