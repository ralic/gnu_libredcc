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
#ifndef IOHW_H
#define IOHW_H 1

#include<share/io.h>
#include<avr/io.h>

/**
   pseudo functions to deal with timing of io (port) updates:
   If io_tick() returns true, then we should if possible run the function that updates the outputs.
   If the function to update the outputs is run, we acknowledge that we are running as early as possible.
 */
extern volatile uint8_t io_ticks;
#define io_tick() io_ticks
#define acknowledge_io_tick() io_ticks--


/**
 the pseudo function that returns the state of the programming button.
*/
#define get_progbutton() (PINx(PROGPORT) & _BV(PROGPIN)) //! @todo should think switching this off if we only have a helper button.


extern const uint8_t output_mask[];

#define set_output(_output) do { PORTx(IOPORT) |= output_mask[_output]; } while (0)
#define reset_output(_output) do { PORTx(IOPORT) &= ~(output_mask[_output]); } while(0)


/**
   pseudo function that samples (reads) the input with the DCC signal.
 */
#define sample_dccpin() (PINx(DCCPORT) & _BV(DCCPIN))

/** 
    pseudo function that switches the pullup of the DCC pin on:
*/
#define pullup_dccpin()  PORTx(DCCPORT) |= _BV(DCCPIN)


#define power_timer_enable(__x) __power_timer_enable(__x)
#define __power_timer_enable(__x) power_timer ## __x ## _enable()



#endif


