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
#ifndef IOIO_H
#define IOIO_H

#include<stdint.h>
//! number of ports that the decoder has -- the number of physical
//! outputs is twice this number. 
#define PORTS 3

/**
 * counts the number of button presses of programme button 
 */
extern volatile uint8_t button_count;

//! activates an output
void activate_output(const uint8_t output); 

//! to be periodically executed for io timings -- currently every 16ms
inline void tick();

//! Cyclically increment variable var up to _lastval (inclusive) and then set to 0
#define INCR(_var, _lastval) do {		\
  (_var)++;					\
  if( (_var) > (_lastval)) {			\
    (_var) = 0;					\
  }						\
  } while(0)

#ifdef HAS_NO_INIT
inline void init_ports(); 
#endif

#endif
