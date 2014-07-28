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
#include <avr/io.h>
#include <avr/power.h>

void uart_init() __attribute__((naked)) __attribute__((section(".init7"))); 
void uart_init() {

    // enabling the UART:
    power_usart0_enable();

  
#ifndef BAUD_TOL
#define BAUD_TOL 3 
#endif

#ifndef BAUD
#warning "This should be settable in software -- so reread the macros as functions?"
#define BAUD 9600
#endif

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
