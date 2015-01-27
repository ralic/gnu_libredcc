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

extern volatile uint8_t io_ticks;
#define io_tick() io_ticks
#define acknowledge_io_tick() io_ticks--

extern const uint8_t output_mask[];

/**
 the pseudo fucntion that returns the state of the programming button.
*/
#define get_progbutton() (PIND & _BV(PD3))
#define set_output(_output) do { PORTB |= output_mask[_output]; } while (0)
#define reset_output(_output) do { PORTB &= ~(output_mask[_output]); } while(0)

#endif
