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

#ifndef S88_H
#define S88_H 1

#include "s88_types.h"

//! \file Header for s88.c

/*! HSI supports a maximum of 31 16 bit modules. Why not support 32?
  \see http://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:dl_hsi_88
 */
#define MAX_MODULES 32 

//! maximum number of parallel chains:
#define MAX_CHAINS 3

//#define LENGTH_BYTES(length_bits) (((length_bits) - 1) / 8 + 1)
//#define MAX_CHAIN_LENGTH_BITS 48 // or 31*16 = 496 according to HSI manual -- but we leave it with this lenght for the time being.

//! the length of the (single) chain of sensors, ie the total number of sensors.
extern volatile sensor_t num_sensor;

//! \todo is this needed somewhere outside? -- not for the IAV. So move into s88.c?
typedef union {
  uint16_t module[MAX_MODULES]; // for HSI where sensors come in modules of 16.
  uint8_t byte[2*MAX_MODULES];  // for easy of access in byte-wise operations on a 8 bit uC.
} READINGS;

//! holds the current readings -- may be accessed from both interrupt and main thread
//! \todo is this used somewhere else?
extern volatile READINGS readings;



#endif
