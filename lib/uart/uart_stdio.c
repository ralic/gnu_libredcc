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

/// perhaps make these not static again?

/// should I split these into two files?
static inline int uart_put(const char c, FILE* const stream) {
  uart_putc_buffered(c);
  return 0;
}

static inline int uart_get(FILE* const stream) {
  return uart_getc_buffered();
}

FILE uart // ; // = //FDEV_SETUP_STREAM(NULL, uart_getc_buffered, _FDEV_SETUP_READ);
= FDEV_SETUP_STREAM(uart_put, uart_get, _FDEV_SETUP_RW);
