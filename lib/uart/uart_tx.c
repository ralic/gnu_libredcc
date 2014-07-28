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
#include <avr/interrupt.h>

// to be executed before main -- also make sure that uart_init.o is included!
void uart_tx_init() __attribute__((naked)) __attribute__((section(".init8"))); 
void uart_tx_init() {
    UCSR0B |= _BV(TXEN0); // enable transmitter.
}

//! length of transmit buffer
#define TX_BUFFER_SIZE 128

//! transmit buffer that holds the chars to be transmitted
static volatile uint8_t tx_buffer[TX_BUFFER_SIZE]; 

//! index to the next byte to be written (post increment scheme)
static volatile uint8_t tx_write_idx = 0; 

/** 
    Places a byte into the transmission buffer to be sent over the UART.

    The caller needs to tolerate that interrupts are temporily disabled.
    Interrupts are unconditionally enabled after the return from this function.
    
    If the transmit buffer is full, the new byte is ignored, but the
    last byte of the transmit buffer is set to "H" as a marker. 

    @param byte to be written into the transmit buffer

    @todo implement sensible behavbuffer[7] = 'H'; 
    @todo perhaps there is still an error here -- test with a really
    small or really large buffer.
 */
void uart_putc_buffered(const uint8_t byte) {

  cli(); // would it be enough to just disable the UART?

  //! check whether buffer is full
  if(tx_write_idx >= TX_BUFFER_SIZE) {
    tx_buffer[TX_BUFFER_SIZE-1] = 'H'; // as a marker -- only for debugging.
  }
  //! if not, write new byte into buffer.
  else { 
    tx_buffer[tx_write_idx++] = byte;
    // indicating that we have something to tx.
    UCSR0B |= _BV(UDRIE0); 
  }

  sei(); 
}

void uart_putc_blocking(const uint8_t byte) {

  while(!uart_tx_free());
  uart_putc_buffered(byte);

}


/*! check how empty tx buffer is 
  @return the number of bytes that the tx buffer can still hold.
 */
uint8_t uart_tx_free() {
  return TX_BUFFER_SIZE - tx_write_idx;
}

/**
 * This is the ISR that actually sends the bytes over the UART. Will
 * only be called when the UART is ready to send a new byte, ie when
 * the UDR (uart data register ready) interrupt is enabled.
 */
ISR(USART_UDRE_vect) {

  //! points always to the next byte to be read. (post increment scheme)
  static uint8_t tx_read_idx = 0; 

  /* @todo do we need to check whether tx_read_idx <= tx_write_idx at the
     beginning? Not if we make sure that the interrupt is enabled only
     when there is something to tx_write... and disable this interrupt if
     there is nothing to tx_write  */  
  UDR0 = tx_buffer[tx_read_idx++];
  
  // we disable the UDRIE (ourselves) if there is nothing more to send currently
  // and at the same time reset the buffer:
  // should we perhals disable UDRIE in the setup? Can it be that is enabled for the Arduino board?
  if(tx_read_idx >= tx_write_idx) {
    UCSR0B &= ~(_BV(UDRIE0));
    tx_read_idx = 0;
    tx_write_idx = 0;
    // like the bit buffer I should also check here to what depth the buffer is actually used
  }
} 

/**
 * caller needs to make sure this is not interrupted with other
 * interrupts
 */    
/*
void tidy_tx_buffer() {

  const uint8_t size = tx_write_idx - tx_read_idx;

  memcpy((void*) tx_buffer, (void*) tx_buffer+tx_read_idx, size); // memcpy is coded such that
					 // it works alright if dest
					 // < source if they are
					 // overlapping as it is
					 // working forward in
					 // memory. This might break
					 // on a different CPU?? 
  tx_write_idx = size;
  tx_read_idx = 0;
}

uint8_t is_tx_buffer_halffull() {

  return (tx_write_idx > TX_BUFFER_SIZE / 2);
}
*/

