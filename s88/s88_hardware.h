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

#ifndef S88_HARDWARE_H
#define S88_HARDWARE_H 1

#include<avr/io.h>

//! AVR ports to be used (currently port B)
#define S88_PORT PORTB
#define S88_DDR  DDRB
#define S88_PIN PINB


//! Connect this AVR pin (Arduino Uno #X) to the LOAD pin of the S88 bus
#define S88_LOAD_PIN PB0

//! Connect this AVR pin (Arduino Uno #X) to the RESET pin of the S88 bus
#define S88_RESET_PIN PB1

//! Connect this AVR pin (Arduino Uno #X) to the CLOCK pin of the S88 bus
#define S88_CLOCK_PIN PB2

#if DEBUG
//! This is the LED on the Ardino Uno board -- for testing and debugging purposes only
#define S88_TEST_PIN PB5
#endif 

//! Connect this AVR pin (Arduino Uno #X) to the LINE pin of the S88 bus
#define S88_DATA_PIN PB3

#define load_on() S88_PORT |= _BV(S88_LOAD_PIN)
#define reset_on() S88_PORT |= _BV(S88_RESET_PIN)
#define clock_on() S88_PORT |= _BV(S88_CLOCK_PIN)
#define test_on() S88_PORT |= _BV(S88_TEST_PIN)

#define load_off() S88_PORT &= ~_BV(S88_LOAD_PIN)
#define reset_off() S88_PORT &= ~_BV(S88_RESET_PIN)
#define clock_off() S88_PORT &= ~_BV(S88_CLOCK_PIN)
#define test_off() S88_PORT &= ~_BV(S88_TEST_PIN)

#define all_off() S88_PORT &= ~(_BV(S88_CLOCK_PIN) | _BV(S88_LOAD_PIN) |  _BV(S88_RESET_PIN) | _BV(S88_TEST_PIN))

#define read_s88_data() (S88_PIN & _BV(S88_DATA_PIN))


/*! Start reading the S88 bus. For AVR, this enables overflow
    interrupt generation and that's it as we set up all other things
    beforehand. */
#define start_s88() do {TIMSK0 = _BV(OCIE0A);} while(0)

#endif
