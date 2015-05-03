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
#include "chip.h"

//#define _SFR_IO_REG_P(sfr) ((sfr) < 0x20 + __SFR_OFFSET)


	
;; #if __SFR_OFFSET == 0x20
;; #warning Offset is 0x20
;; #elif __SFR_OFFSET == 0x0
;; #warning Offset is 0x00
;; #else
;; #error no offset at all
;; #endif
	
;;; $Id$
	
	.section .text
	.global __vector_1 	; gcc uses this symbol for the INT0 isr.

;;; ISR that starts up timer2 when there is a falling edge at PINT0 (PD2):
__vector_1:
	;;  none of the below changes SREG:
	push r24
	#warning also set prescale factor centrally!
	ldi r24, PRESCALER_8(DCCTIMER) 			; start timer2 with prescaler 8.
	sts _SFR_MEM_ADDR(TCCRxB(DCCTIMER)), r24		; start timer2,
				; could I not use the bit set or bit
				; copy command?
#if _SFR_IO_ADDR(EIMSK) < 0x20 			
	cbi _SFR_IO_ADDR(EIMSK), INT0		; disable this interrupt.
#else
 	in r24, _SFR_IO_ADDR(SREG)
	push r24
        in r24,_SFR_IO_ADDR(EIMSK) 	  ; disable this interrupt.
        andi r24, ~_BV(INT0)     
	out _SFR_IO_ADDR(EIMSK), r24
	pop r24
	out _SFR_IO_ADDR(SREG), r24
	#warning EIMSK is not a5 IO reg
#endif

#if _SFR_IO_ADDR(EIFR) < 0x20
        sbi _SFR_IO_ADDR(EIFR), INTF0		; clear any pending
				; interrupts to ourselves (writing a
				; one) why am I doing this?
#else
	ldi r24,_BV(INTF0)
	out _SFR_IO_ADDR(EIFR), r24
	#warning EIFR is not a5 IO REG

#endif
	pop r24
	reti
