/* @file
 *
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

#include <avr/interrupt.h>
#include <string.h>

/** 
 * initialises the UART for sending.
 *
 * \pre make sure uart_init is also linked into the excutable for receiving.
 * 
 * @todo The receiver part seems to work -- at least as long as I do not try to send something.
 *
 * @note ATmega328p has an internal receive buffer of two bytes. And
 * therefore we probably do not need a receiver buffer as serial
 * transmission is slow compared to CPU freq of the AVR?
 
 @todo von den unscharfen <= auf != und == übergehen because this might help detect any errors

 */
void init_uart_rx() __attribute__((naked)) __attribute__((section(".init8")));
void init_uart_rx() {

  /*! @todo  is the below redundant or was it only necessarey because of the arduino bootloader?
    setup RXD0 pin (doubling as PORTD0, PD0) */
  PORTD &= ~(_BV(PD0)); 
  DDRD &= ~(_BV(PD0)); 
	     
  // enable reciever and enable the receive interrupt:
  UCSR0B |= _BV(RXEN0) | _BV(RXCIE0);
}

//! length of receiver buffer
#define RX_BUFFER_LEN 64 

//! the receiver buffer
volatile static uint8_t rx_buffer[RX_BUFFER_LEN];

//! index of the buffer position to be written to next
volatile static uint8_t rx_write_idx = 0;

/** 
 ISR that reads a byte from the UART into the rd buffer

 \todo improve dealing with buffer overruns.
*/
ISR(USART_RX_vect) {

  // is there still space in the receiver buffer?
  if(rx_write_idx >= RX_BUFFER_LEN) {
    // no space left
    rx_buffer[RX_BUFFER_LEN-1] = '$'; // marker that we had a buffer overflow. // only for debugging
    UCSR0B &= ~ (_BV(RXCIE0)); // disable the receiver (ourselves) until we are reenabled when the buffer has been cleared.
  }
  else {
    // there is still space in the receive buffer.
    rx_buffer[rx_write_idx++] = UDR0;
  }
}

#if 0
/**
 * interrupts must be enabled (in fact it is enough if the transmit
 * interrupt interrupt is disabled)
 */
static inline void tidy_rx_buffer() {

  cli(); // make sure we cannot be interrupted, acutally it would be
	 // enough to disable the rx_interrupt -- but perhaps the
	 // interrupt cannot be disabled atomocally?

  const uint8_t size = rx_write_idx - rx_read_idx;
  
  // using memmove to be on the safest side...
    memmove( (void*) rx_buffer, (void*) rx_buffer +rx_read_idx, size); // memcpy is coded such that
  // memcopy is faster
  // memcpy( (void*) rx_buffer, (void*) rx_buffer +rx_read_idx, size); // memcpy is coded such that
					 // it works alright if dest
					 // < source if they are
					 // overlapping as it is
					 // working forward in
					 // memory. This might break
					 // on a different CPU as it depsned on glibc implementation
  rx_write_idx = size; 
  rx_read_idx = 0;

  //! @todo if we had disable the receiver interrupt (as the receive
  //! buffer was full), we could enable it here again -- as we now
  //! have more space.
  sei();
}
#endif

/** byte position of the byte to be read next from the buffer. */
static uint8_t rx_read_idx = 0; // only to be read in the main thread.


/*! \pre to be called on main thread only.

  Function checks whether the the read index into the buffer has
  advanced to the write index -- that there are no more bytes to read
  from the buffer. If so, it reset both indices to the beginning of
  the buffer.
  \note should be called regularly to avoid a receiver buffer overrun.
  \post interrupts will be enabled.
 */
static inline void uart_rx_tidy_buffer() {
  if(rx_read_idx != 0 && rx_read_idx >= rx_write_idx) { // if no more chars left in read buffer
    cli();  // disable all interrupts -- infact RXCIE0 would be enough, if this works as fast as the cli -- does it?
    // then tidy buffer, double-check idiom
    if(rx_read_idx >= rx_write_idx) { 
      rx_write_idx = 0;
      rx_read_idx = 0;
      UCSR0B |=  _BV(RXCIE0); // re-enable receiver interrupt, in case
			      // it was disabled as we had reached the
			      // end of the buffer  
    }
    sei();
  }
}
    
/** 
    gets a byte from the buffer or blocks until one is there

    \pre caller must make sure that interrupts are enabled when this 
      method is called as it relies on the buffer being filled by the
      uart 
    \pre  it must not be called from an other interrupt
    handler

    @todo write a proper non-blocking version.

*/
unsigned char uart_getc_buffered() {

  /// \todo is it necessary that we tidy buffer every time? Or is it a
  /// waste of time?
  uart_rx_tidy_buffer();

  // block until a new byte gets delivered into the buffer, perhaps
  // with sleeping -- later on! 
  while(rx_read_idx >= rx_write_idx); // and now really wait for the
				      // next byte via the UART -- blocking!
  return rx_buffer[rx_read_idx++];
}

//! @return number of bytes ready to be read from the buffer.
uint8_t uart_rx_received() {
  uart_rx_tidy_buffer();
  return rx_write_idx - rx_read_idx;
}
