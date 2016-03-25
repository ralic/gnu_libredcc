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
#include <share/io.h>
#include "io_hw.h"

#include<pic16regs.h>

void init_io() {

  #warning " so far only receiver_pic14.h makes sure the pullup is on."

  // configure input for progbutton:
  // switch on pull-up for progbutton:

  ANSEL = 0; // we are doing only analog io -- do all have this?
#ifdef ANSELH_ADDR
  ANSELH = 0; // do I need this?
#endif

  // RABPU = 0; // for 16f690 this has already been done in init_pic
#ifdef PROG_WPU
  PROG_WPU |= _BV(PROG_PIN);
#endif
  // TRISA3 = 1; // make it an input but it can only be an input anyway, so perhaps I do not need to do this.
	      // it does not have analog function, so no ANSx to set
	      // for it 
  // make RC1, RC2 outputs and switch them off:

    // #elif defined GPIO_ADDR
  // GP3 has not pull-up as it is also MCLR :-) (so we need an
  // external pull-up!
  // WPU3 = 1;


  OUT_PORT &= ~(_BV(OUT_0));
  OUT_PORT &= ~(_BV(OUT_1));

  OUT_TRIS &= ~(_BV(OUT_0));
  OUT_TRIS &= ~(_BV(OUT_1));

  // only need if errors/warning are indicated via ports

  ERROR_PORT &= ~(_BV(ERROR_PIN));
  ERROR_PORT &= ~(_BV(WARNING_PIN));

  ERROR_TRIS &= ~(_BV(ERROR_PIN));
  ERROR_TRIS &= ~(_BV(WARNING_PIN));

  // set up the 16bit timer so that we get an overflow about every 32ms (for F_CPU == 8MHz)
  #warning values below not checked for 12f683

  // when the overflow happens first does not matter, so we do not reset TMR1
  // TMR1H = 0; 
  // TMR1L = 0;
 
#ifdef TMR1GE // enable for timer
  TMR1GE = 0;
#endif
  TMR1CS = 0;

#if F_CPU != 8000000
#error Frequency must be 8MHz or change prescaler settings below.
#endif

  /* Calculation is as follows
   * F_CPU = 8000000 Hz from quartz
   * => F_OSC = F_CPU / 4 = 2000000 Hz
   * => with precaler 1:1, F_TMR1_OVR = F_OSC / 65536 = 30.518 Hz
   * => timer tick is about every 33ms -- double the time as for the AVR
   * Ok, I could easily use a different timer, or read out a different bit.
   *
   * If I went for a 8 bit timer (eg TMR2 if available) then the calculation would be:
   * => F_OSC / 256 = 7812.5 KHz for 1:1 prescaler If there was a 1:128 prescaler, I would get:
   * the timer wrapes with 61.035 Hz, ie at 16.384ms intervals. Hence
   * in princile 2x 8bit timer would be enough. TMR2 has pre and
   * postscaler, that could be combined to yield 1:128 etc, but then I
   * am not using this in anyway as an interrupt
   */

  // prescaler 1:1
  T1CKPS1 = 0;
  T1CKPS0 = 0;

  // start timer
  TMR1ON = 1;

  init_ports(); // resets only the internal timers for all ports

}

const uint8_t output_mask[2*PORTS] = { _BV(OUT_1), _BV(OUT_0) }; 
