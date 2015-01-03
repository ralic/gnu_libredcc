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

;;; $Id$
	
	.section .text
	.global __vector_1 	; gcc uses this symbol for the INT0 isr.

;;; ISR that starts up timer2 when there is a positive edge at PINT0 (PD2):
__vector_1:
	;;  none of the below changes SREG:
	push r24
	ldi r24, _BV(CS21) 			; start timer2 with prescaler 8.
	sts _SFR_MEM_ADDR(TCCR2B), r24		; do start timer2,
				; could I not use the bit set or bit
				; copy command? 
	cbi _SFR_IO_ADDR(EIMSK), INT0		; disable this interrupt.
	sbi _SFR_IO_ADDR(EIFR), INTF0		; clear any pending
				; interrupts to ourselves (writing a
				; one) why am I doing this? 
	pop r24
	reti
