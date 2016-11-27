/* 
 * Copyright 2014, 2015 André Grüning <libredcc@email.de>
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
    Emultates SPROG as far so that it can collaborate with rocrail

    For ROCRAIL we need cmds:

    running:
    - "-": Power Off
    - "+": Power On
    - "O": Send DCC packet given as a sequence of hex bytes.
    
    programming:
    - "C" -- direct mode programming -- write only
    - "V" -- obsolete (but required by NMRA) -- not planned to implement.

    extensions:

    - "M" -- direct mode programming (ie CVs are programmed direct) --
    to be implemented

*/

#include <avr/interrupt.h>
#include <uart.h>
#include "../share/sprog.h"

int main() {

  stdout = &uart;
  stdin = &uart;
  stderr = &uart;

  // enable interrupts -- ie start uart input
  sei();
  // and listen to the uart for SPROG commands. All replies back to to uart.
  sprog();

  return 0; 


}
