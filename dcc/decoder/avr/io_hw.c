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

#include "io_hw.h"
#include "../share/io.h"

#include<avr/power.h>
#include<avr/interrupt.h>

volatile uint8_t io_ticks = 0;

/*! The timer is setup so that it overfl.o-ws every 16ms
    Like on the PIC we could perhaps just poll the corresponding overflow flag?
    If we wanted the IO to run on the ISR itself, we would call tick() in the ISR.

 * If we are running with F_CPU = 16MHz, and a prescaler 1:1024, then
 * there is a timer0 overflow every 15.625 times in a ms, that is one
 * tick is 0.064ms. If we count an bit timer byte up, then ports are updated
 * about every 16ms. 16ms is also enough for debouncing of the button.
 */
ISR(TIMER0_OVF_vect) {
  io_ticks++; 
}

void init_io() __attribute__((section(".init8"))) __attribute__((naked));
void init_io() {

  // enable timer0
  power_timer0_enable();

  // configure input for progbutton:
  DDRD &= ~(_BV(PD3)); 
  // switch on pull up:
  PORTD |= _BV(PD3);
	    
  // configure outputs:

  #if PORTS != 2 
  #error This has to be adjusted manually for number of ports
  #endif
  PORTB &= ~(_BV(PB2) | _BV(PB1) | _BV(PB4) | _BV(PB3)); // this switches off the ports and the pull-ups.
  DDRB |= _BV(PB2) | _BV(PB1) | _BV(PB4) | _BV(PB3); // this makes the port an output

  // enable overflow interrupt
  TIMSK0 = _BV(TOIE0); 
  TCNT0 = 0;    
	
  // timer normal mode, no outputs needed.
  TCCR0A = 0; 

  /* start timer with prescaler 1:1024 -- at 16 MHz, this means a tick
     every 1024/16 us = 64us, and hence a timer overflow 256/16*1024 =
     16384 us = 16.4ms
    */
  TCCR0B = _BV(CS02) | _BV(CS00); 
}

// The below has to be adapted manual with the IO ports and pins of the outputs.
#if PORTS != 2
#error Adjust here for number of Ports
#endif
// \todo where is output mask used?
const uint8_t output_mask[2*PORTS] = { _BV(2), _BV(1), _BV(4), _BV(3)}; // RC1 and RC2 on PIC // PB1 and PB2 on AVR
