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
 * along with LibreDCC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @todo whether hardware (electrical current limitiation?) really can deal with switching two outputs at the
 * same time (eg by programming them to the same address). 
 * @todo how to protect outputs that belong to the same port from
 * being activated at the same time?
 * @todo deactive when activating for paired outputs,
 * @todo a command queue for the outputs -- outputs are only switched on sequentially??? (at least if
 * activation times are pulsed. 
 * @todo eliminate this file and distribute its contents elsewhere.
 */

#include <share/io.h>
#include <share/port.h>
#include <avr/io_hw.h> // for get_progbutton
#include <share/port.h>


#ifdef NO_LOCAL_STATICS
static uint8_t button = 1;  
#endif

#if SDCC_pic14
#warning check whether sdcc can init this one? Yes! -- Check. Because we would not need to init this separately.
#endif

//! indicates whether we are in prog mode and which port is being programmend next.:
volatile uint8_t button_count = 0;

/**
 * The code below is to be called periodically.
 * Essentially the code below does the followng
 * 1. Test whether the programiing button has been released (ok we could do this
 *    via an input changed interrupt in a more elegant world).
 * 2. If the button has been pressed, we are in programming mode. If
 *    we are in progamming mode, we toggle the port, that corresponds to the port being programmed.
 * 3. Finally, we tick down all (timed) outputs, both if we are in normal mode
 *     or in programmingh mdode
 */
void tick() {

#ifndef NO_LOCAL_STATICS
  static uint8_t button = 1; 
#endif

  // 1. Poll Progbutton.
  const uint8_t button_new = get_progbutton(); 

  if(button_new && !button) { // button just released.
    INCR(button_count, PORTS);
  }
  button = button_new;

  // 2. If in progmode toggle corresponding port
  if(button_count) {
    port_toggle(ports + (button_count-1));
  }

  // 3. Tick down timed outputs.
  for(int i = 0; i < num_ports; i++) {
    ports[i].tick(ports + i);
  }
}

