/*! \file
 * \copyright Copyright 2014 André Grüning <libredcc@email.de>
 *
 * \license
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
 *
 */

#include <avr/io.h>
#include <avr/power.h>

/** initialises the UART. The baudrate is hardcoded at compile time to
    the value of macro BAUD. If BAUD is undefined, the default is
    9600bd.

    The (naked) function is included in section .init7 so that is
    executed before main. \see avr-glibc
    manual. \todo add link to avr-glibc manual

    \param BAUD a macro that defines the baudrate. If undefined, BAUD
    will be set to 9600.
    \param BAUD_TOL a macro that defines the permitted tolarance of
    baud rate settings.

    \todo make baudrate settable in software to create a more flexible
    library.
 */
void uart_init() __attribute__((naked)) __attribute__((section(".init7"))); 
void uart_init() {

  // enabling the UART:
  power_usart0_enable();

#ifndef BAUD_TOL
#define BAUD_TOL 3 
#endif

#ifndef BAUD
#define BAUD 9600
#endif

  // This include defines the macros used below to calculate the baud rate
#include <util/setbaud.h>
  
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(USBS0); // 8bit, 2stop, no partity
}
