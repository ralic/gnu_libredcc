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

/** \file
 * \todo is there another version of error_hw.c?
 */


#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include <error.h>

/*! When all is setup, processing occurs only on interrupts for avr,
  so we try to save energy by sleeping */
void exit() __attribute__((naked)) __attribute__((noreturn));
void exit(int retval) {
  set_sleep_mode(SLEEP_MODE_IDLE);
  while(1) {
    sleep_enable();
    sleep_cpu();
  }
}

/*! to be executed before any setup of the individual modules.
  essentially does some clean up after the Arduino bootloader and
  switches off all devices
*/
void init_avr() __attribute__((naked)) __attribute__((section(".init5"))); 
void init_avr() {

  // the arduino bootloader not only leaves PD0 and PD1 in UART mode,
  // it also seems to enable and then not disable the eeprom
  // interrupt:  
  EECR &= ~(_BV(EERIE));
  power_all_disable(); // to save as much power as possible.
}

//int main(void) __attribute__((noreturn));
/*! not much to do, except enabling the interripts */
int main(void) {
  sei();
  INFO("Starting Decoder");
  return 0;
}
