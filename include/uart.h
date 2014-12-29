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
#ifndef UART_H
#define UART_H 1

/** @file
    Header file for communication with the UART.
*/


#include <stdio.h>
//#include <avr/io.h>


#ifdef __cplusplus
extern "C" {
#endif

  /**
   * read a char from the uart. It blocks if buffer is empty. 
   */
  unsigned char uart_getc_buffered();

  /** write a char to the uart -- if buffer full, an error char is
      outputted and the original char to be written is discarded.  */
  void uart_putc_buffered(const uint8_t ch);

  ///  write a char to the uart -- blocking if buffer is full.
  void uart_putc_blocking(const uint8_t byte);

  /// returns currently available (ie free) bytes in the uart transmission buffer.
  uint8_t uart_tx_free();

  /// returns currently available (ie received) bytes in the uart receiver buffer.
  uint8_t uart_rx_received();

  //! file structure for the UART using the bloc
  extern FILE uart;

#ifdef __cplusplus
}
#endif

#endif

