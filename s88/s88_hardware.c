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

/* \file
   This file contains (most of) the hardware dependent part of the S88
   driver. It is here most changes will need to occur to adapt to
   other microcontrollers etc.  
   
   Essentially what it does is to initialise the hardware -- more
   hardware relevant code to set or read ports and pins of a
   microcontroller is implemented as macros in the corresponding
   header file s88_hardware.h. 
 */

#include "s88_hardware.h"

/*! initialise the hardware on the Arduino board. The function is naked and in
  section .init8 in order to be executed automatically before main.
  
  Essentially it does the following:
  1. set up used ports and pins for input or output
  2. set up timer 0 / OCR0A so that interrupt TIMER0_COMPA is
  triggered every 30us. The corresponding ISR is in s88.c
*/
void init_s88_hardware() __attribute__((naked));
void init_s88_hardware() __attribute__((section(".init8")));
void init_s88_hardware() {

  // set up ports:
  all_off(); 

  // make the following pins outputs:
  S88_DDR |= _BV(S88_CLOCK_PIN) | _BV(S88_LOAD_PIN) | _BV(S88_RESET_PIN) | _BV(S88_TEST_PIN);

  // make the following pins input:
  //DDRB &= ~(_BV(S88_DATA_PIN))

  // switch on pull-up:
  S88_PORT |= _BV(S88_DATA_PIN);
  
  /* set up s88 interrupt and timer. As s88 is not so time critical,
     we take the timer with the lowest priority. This is timer 0. */

  // select CTC mode with OCRA as top:
  TCCR0A  = _BV(WGM01);

  /* Timing calculations (from http://www.opendcc.de/s88/s88_n/s88-timing.html)
     tcycle > 30us, we choose tcycle = 60us dh tlow = thigh = 30us --
     this is about half the maximum theoretical speed -- to be on the safe side.
   */

#ifndef F_CPU
#error "F_CPU not defined"
#endif
#define TICKS_PER_US (F_CPU / 1000000)

#define PRESCALER 8 
  
#define S88_TICKS (30 * TICKS_PER_US) / PRESCALER // interrupt every 30us -- this is period is 60us, ie 16.7 kHZ
#if S88_TICKS > 248 
#warning S88_TICKS too high (> 248)
#elif S88_TICKS < 8
#warning S88_TICKS too low (<8)
#endif

  OCR0A = S88_TICKS; 
  TCNT0 = 0; // reset the counter.
  
  // select prescaler and start of timer:
#if PRESCALER == 8
#define PRESCALER_MASK _BV(CS01)
#elif PRESCALER == 64
#define PRESCALER_MASK (_BV(CS01) | _BV(CS00))
#else
#error "No Prescaler defined or not setting defined for given prescaler"
#endif
  TCCR0B = PRESCALER_MASK;

}
