/* 
 *  Copyright 2014 André Grüning <libredcc@email.de>
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

// outputs are entirly driven in an isr
ISR(TIMER0_OVF_vect) {
  tick();
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
  PORTB &= ~(_BV(PB2) | _BV(PB1)); // this switches off the ports and the pull-ups.
  DDRB |= _BV(PB2) | _BV(PB1); // this makes the port an output

  // enable overflow interrupt
  TIMSK0 = _BV(TOIE0); 
  TCNT0 = 0;    
	
  // timer normal mode, no outputs needed.
  TCCR0A = 0; 

  // start timer with prescaler 1:1024.
  TCCR0B = _BV(CS02) | _BV(CS00); 
  // that is 
}

const uint8_t output_mask[2*PORTS] = { _BV(2), _BV(1) }; // RC1 and RC2 on PIC // PB1 and PB2 on AVR
