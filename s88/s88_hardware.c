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

#include "s88_hardware.h"

void init_s88_hardware() __attribute__((naked));
void init_s88_hardware() __attribute__((section(".init8"))); // to be executed before main.
void init_s88_hardware() {

  // set up ports:

  all_off(); 
  // make the following pins outputs:
  S88_DDR |= _BV(S88_CLOCK_PIN) | _BV(S88_LOAD_PIN) | _BV(S88_RESET_PIN) | _BV(S88_TEST_PIN);

  // make the following pins input:
  //DDRB &= ~(_BV(S88_DATA_PIN)) // needed?
  // switch on pull-up:
  S88_PORT |= _BV(S88_DATA_PIN);
  
  // set up s88 interrupt and timer. As s88 is not so time critical,
  // we take the timer with the lower priority
  // this is timer0

  // select CTC mode with OCRA as top:
  TCCR0A  = _BV(WGM01);

  // normal mode:
  // TCCR0A  = 0;
  

  // We need x us for S88, hence 
  /* Timing calculations (from http://www.opendcc.de/s88/s88_n/s88-timing.html)
     tcycle > 30us, we choose tcycle = 60us
     dh tlow = thigh = 30us -- this is about half the maximum
     theoretical speed.
   */

#ifndef F_CPU
#error "F_CPU not defined"
#endif
#define TICKS_PER_US (F_CPU / 1000000)

#define PRESCALER 8 // should 8 -- but for testing we make it slow!
  
#define S88_TICKS (30 * TICKS_PER_US) / PRESCALER // interrupt every 30us -- this is period is 60us, ie 16.7 kHZ
#if S88_TICKS > 250 
#warning S88_TICKS too high (> 250)
#elif S88_TICKS < 10
#warning S88_TICKS too low (<10)
#endif

  OCR0A = S88_TICKS; // 60 currently.
  //OCR0A = 240; // for testing we are at quater of the speed!


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
