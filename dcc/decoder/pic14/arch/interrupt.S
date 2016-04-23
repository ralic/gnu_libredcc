/* 
 * Copyright 2014-2016 André Grüning <libredcc@email.de>
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
 * along with LibreDCC. If not, see <http://www.gnu.org/licenses/>.
 */
;;;  for testing new code regarding the bit buffer

;;; $Id$
;;; Interrupt routine for dcc decoder

	include PROCESSOR_INC
	#include "chip.h"
	
	extern WSAVE
	extern SSAVE

BITS	UDATA 0x20 		; make sure _bits lies in first memory bank. Could this also be device dependent? How to I quiry the linker to give me the start of ram?ma
	global _bit_pointer, _bit_buffer, _next_bit

				; alternatively we could store it in the share bank?
	
_bit_buffer:	res 1		; the buffer bits
_bit_pointer:	res 1		; the bit pointer is a mask to the
				; current topmost bit, ie the one that
				; should be read next.

BIT_BUFFER CODE
	;; interrupt code assumes this is in the first bank and first page.

	
	;; before calling this, check that _bit_pointer must be !=0
	;; returned bit is in w
_next_bit:

	BANKSEL(_bit_pointer)

;;	is it really necessary to disable the global interrupt here? Perhaps by rearranging stuff, eg that the interrupt routing never rotates, only uses pointers?
	bcf INTCON, GIE
	btfsc INTCON, GIE	; see PIC AN576
	goto $-2

	movf _bit_pointer, w
	andwf _bit_buffer, w	; resulting bit is in w
	bsf INTCON, GIE	

	bcf STATUS, C
	rrf _bit_pointer, f	; can be done thread-safely outside
				; the interruot-disable part, we
				; maximally get indication of a buffer
				; overrun a bit earlier. 
	return


	
;; It takes 3-4 cycles to raise an EXTERNAL interupt (but nothing is
	;; set reagding an INTERNALT interrupt.
INTERRUPT CODE 0x0004

	movwf WSAVE
	swapf STATUS,w
	movwf SSAVE
	clrf STATUS 		; we make sure all following accesses
				; take place in register bank 0, not
				; needed for STATUS and INTCON it it
				; needed for pagesel? (as it is in all
				; banks. PORTC needs bank0 but is only
				; here for debuggine purposes. _bits
				; needs it as well 
	btfss INTCON, T0IE	; have we been interrupted via the
				; external interrupt? Why do we do the
				; checking via INTE and not via INTF? 
	goto external_isr		; no? Then it must be the

;;	swapf STATUS,w		; the STATUS needs only saving here.
;	movwf SSAVE
;   	clrf STATUS 		; we make sure all following accesses
				; are in bank 0  -- only necessary
				; after we have enabled  interrupts
				; use file registers in other banks --
				; this is probably only the case for
				; writing/reading the EEPROM..., so
				; one could disable the interrupts
				; during that time?

				; timer interrupt, as only the 2
				; interrupts INTE and T0IE are enabled 
	;; bcf PORTC, 1		; switch off LED2 -- this is for
	;; debugging the timing

	btfsc _bit_pointer, 2 		; for debugging to test up to what
				; depth the bit_buffer is being
				; used. -- it seems bit 2 is never set 
				; -- but bit 1 occassionally, and bit
				; 0 often -- so this means we have
				; maximum 2 bits in the queue when it
				; is called, so it might be filled up
				; to the third bit which however
				; seemsv to be consumed very fast
	bsf ERROR_PORT,WARNING_PIN		; debug

	;; 7 befehle am Kopf in write_bit, jetzt 6, equvialent to bcf STATUS,C rol _count, but sets also Z
	movfw _bit_pointer
	addwf _bit_pointer, f

	;; if buffer full
	btfsc STATUS,C 
	goto bit_buffer_full

	;; if buffer empty>

	btfsc STATUS, Z
	incf _bit_pointer,f		; does not set carry, as _count is incremented from 0 to 1
	
	
	;; count now points to the next available bit in buffer, now it is time to add the new bit to bufer

	;; bcf STATUS, C	; C should be clear from previous operation addwf
	btfsc INPUT_PORT, INPUT_PIN	; PIN for external interrupt high or low? -- this is where the DCC
			; signal is applied -- for all PICs? Certainly not...
	bsf STATUS, C		; for a high bit, set carry
	rlf _bit_buffer, f		; and rotate bit into _buffer

clean_up_timer0_isr:
	bcf INTCON, T0IE	; disable ourselves
	bsf INTCON, INTE	; enable external interrupt to detect next edge
	bcf INTCON, INTF	; clear external interrupt flag
	
;;	swapf SSAVE, w
;;	movwf STATUS

	goto end_isr
external_isr:	   ; none of the following changes flags -- so we
		   ; could save the code to save the flags, if we knew
		   ; we could only be called from programms that are
		   ; at bank 0 (page 0 we can ensure by just choosing
		   ; one bank!) TMR0 is in both even banks, INTCON in all.
		   ; 
	movlw 0x68		; preload for timer. Whence did I get this number??
	movwf TMR0		; load timer, timer is continuously running
	;; bsf PORTC, 1		; switch on LED2 for debugging purposes of the timing
	bcf INTCON, INTE	; disable external int
	bsf INTCON, T0IE	; enable timer int
	bcf INTCON, T0IF	; clear timer interruopt flag

end_isr:			; common end to both the timer0 and
				; external isr
	swapf SSAVE, w
	movwf STATUS
	swapf WSAVE, f
	swapf WSAVE, w
	retfie


bit_buffer_full:			; do nothing, but 

	rrf _bit_pointer, f		; revert bit_pointer
	bsf ERROR_PORT, ERROR_PIN		; we have lost a bit, so swithc on PC3
				; as error indicator for debugging
	goto clean_up_timer0_isr

END
