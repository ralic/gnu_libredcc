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

#include <stdio.h>
//#include <avr/io.h>


#ifdef __cplusplus
extern "C" {
#endif

  /**
   * read a char from the uart -- blocking if buffer is empty.
   */
  unsigned char uart_getc_buffered();

  /// write a char to the uart -- if buffer full, an error char is
  /// outputted and the original char to be written is discarded.  
  void uart_putc_buffered(const uint8_t ch);

  ///  write a char to the uart -- blocking if buffer is full.
  void uart_putc_blocking(const uint8_t byte);

  /// returns currently available (ie free) bytes in the uart transmission buffer.
  uint8_t uart_tx_free();

  /// returns currently available (ie received) bytes in the uart receiver buffer.
  uint8_t uart_rx_received();

  /// returns true if tx buffer is halffull.
  //uint8_t is_tx_buffer_halffull(); // is this used somewhere?
  
  /// tidy the buffer
  //void tidy_tx_buffer(); // is this used somewhere?

  /* ----- below is depreacted */

#if 0
  /** 
      sends a char to the uart -- not needed currently -- uart_tx_init must be included :-(
  */
#define uart_send(__char) do {			\
    while (!(UCSR0A & _BV(UDRE0)));		\
    UDR0 = __char;				\
  } while(0)


#define uart_send_nibble(__byte) uart_send(nibble2digit(__byte));

#define uart_send_byte(__byte) do {		\
    uart_send_nibble(__byte >> 4);		\
    uart_send_nibble(__byte);			\
  } while(0)
  
#define buffered_uart_send_byte(__byte) do {	\
    buffered_uart_send_nibble(__byte >> 4);	\
    buffered_uart_send_nibble(__byte);		\
  } while(0);

#define buffered_uart_send_nibble(__nibble) uart_putc_buffered(nibble2digit(__nibble));


  /** functions to wrap our own function such that they work with the standard IO streams*/

  // only used for internal purposes
  
  //  int uart_put_stdio(const char c, FILE* const stream);
  //int uart_get_stdio(FILE* const stream);
  
  // and the corresponding file stream:

#endif  

  //! file structure that writes and reads to the UART using the nonblocking write 
  extern FILE uart;

#ifdef __cplusplus
}
#endif

#endif

