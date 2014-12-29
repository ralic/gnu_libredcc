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

#include <uart.h>

/** @file
    The function in this file serve to interfacemore primitive
    functions for reading and writing from the uart to the stdio
    library.  

    \todo split this source file into two, one each for reading and
    writing so that they can be imported separatly.

    See also section for avr-glib on stdlib
    \todo Insert url to avr-glib
 */

/**
   wraps primitive uart_putc_buffered() to something that is stdlib
   compatible
   \returns always 0 (should return error code in case of error)

   \todo implement EOF return

   @param byte to write to stream
   @param stream file stream to write to

 */   
static int uart_put(const char c, FILE* const stream) {
  uart_putc_buffered(c);
  return 0;
}


/**
   wraps uart_getc_buffer() to be used with stdio.

   @return a byte that is read from stream
   @todo implement handling of EOF
   @param stream file stream to read from.
 */
static inline int uart_get(FILE* const stream) {
  return uart_getc_buffered();
}


/**
   provides a file stream structure to read from and write to the uart
   for use with stdio functions.
 */
FILE uart = FDEV_SETUP_STREAM(uart_put, uart_get, _FDEV_SETUP_RW);
