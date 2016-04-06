/* 
 * Copyright 2014-2016 André Grüning <libredcc@email.de>
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
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include<stdint.h>

#warning I must create a new linker script -- otherwise PCL might be pointing to the moon when interrupt is called    

#define sei() GIE = 1


// below here is the bit queue -- try to use a common file
extern volatile uint8_t bit_buffer; // does not need to be initialised!
extern volatile uint8_t bit_pointer;

/*! initialised the interript routines, eg to set varaible to defined
  values */
#define init_interrupt() do { bit_pointer = 0;} while(0)

/*! check whether a least one bit is available */
#define has_next_bit(dummy) (bit_pointer != 0)


/*! Before calling this function, it must have been asserted with
     has_next_bit whether there is a next bit
     @return 0 if next bit is zero and !0 if next bit is one */
uint8_t next_bit();

#endif
