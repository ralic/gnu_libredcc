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
#ifndef ERROR_H
#define ERROR_H 1

#if DEBUG 
// size_error is just a constant to get the maximum used value of this enum

enum {
  no_error,
  preamble_too_short,
  checksum_nonzero,
  dcc_fall_through,
  //  size_errors,
  lost_bit,
  packet_too_long,
  size_error,
  NUM_ERRORS};  

extern char* error_msg[];

#include <uart.h>
#define INFO(str) fputs((str), &uart)
#define ERROR(code) fputs(error_msg[(code)], &uart)

#else // debug not defined!
#define ERROR(code) do{} while(0)
#define INFO(str) do{} while(0)
#endif

#define RESET_ERROR(dummy) do{} while(0)
#define WARNING(str) INFO(str)

#endif
