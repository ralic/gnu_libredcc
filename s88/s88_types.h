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

/** @file
    This file contains definitions of types that are used across a
    number of modules
*/
#ifndef S88_TYPES
#define S88_TYPES 1

#include <inttypes.h>

/*! type to hold readings of bits and/or logical values with the
  convention that a value of zero means logical FALSE or 0, and every
  other value TRUE or bit 1 */
typedef uint8_t bit_t;

/*! type that can hold the number of a sensor. We support up to 512
  sensor, hence the type must have at least 9 bits */
typedef uint16_t sensor_t;

/*! type that holds a sensor reading, ie a sensor number together with
  the reading.  */
typedef struct {
  sensor_t sensor; //! sensor number
  bit_t value; //! its value (off or on)

  /*! value of the module (ie block of 16 sensors) of which sensor is
    part -- needed for easy of implementation of unversed HSI88
    protocol, see s88_iav.c */
  uint16_t module_val; 
} reading_t;

#endif
