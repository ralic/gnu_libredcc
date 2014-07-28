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
#include "s88_queue.h"

#include<inttypes.h>
#include<avr/interrupt.h>

#define SENSOR_BUFFER_LEN 256

//! the sensor buffer, to be accessed both from the interrupt and the main thread.
volatile static reading_t sensor_buffer[SENSOR_BUFFER_LEN];

//! index of the position to be written next -- to be accessed only from the main thread.
volatile static uint8_t write_idx = 0;


static uint8_t read_idx = 0;

//! to run on main thread
void tidy_sensor_buffer() {
    if(read_idx >= write_idx) { // if no more chars left in read buffer
      cli();  // disable all interrupts -- infact the S88_interrupt
      // would be enough
      // then tidy buffer, double-check idiom
      if(read_idx >= write_idx) { // we could check here whether
				      // rx_write_idx is not zero to
				      // save a few CPU cycle when we
				      // are idle, but the buffer has
				      // already been reset.
	write_idx = 0;
	read_idx = 0;
	// reenable interrupt (or whatever we had disable before)
      }
      sei();
    }
}

/*! whether the queue has an element or not 
  note write_idx will be advanced only in the interrupt routing (and
  read_idx only on the main thread) -- so no race condition, we
  maximially fail to realise that we have got an element to read) */
bit_t has_reading() {
  tidy_sensor_buffer();
  return write_idx > read_idx;
}



//! runs on interrupt
void queue_reading(const reading_t reading) {
    // is there still space in the buffer?
  if(write_idx >= SENSOR_BUFFER_LEN) {
    // deal with the buffer overflow? Disable s88 interrupt and exit
    // is without reading? and then try to resent when it is enabled
    //rx_buffer[RX_BUFFER_LEN-1] = '$'; // marker that we had a buffer overflow. // only for debugging
    // UCSR0B &= ~ (_BV(RXCIE0)); // disable the receiver (ourselves)
    // until we are reenabled when the buffer has been cleared. 
  }
  else {
    // there is still space in the buffer.
    sensor_buffer[write_idx++] = reading;
  }
}

// introduce sensor type and bit_t;

//! runs on main thread -- call only if you know that queue has an element!
reading_t dequeue_reading() {
  return sensor_buffer[read_idx++];
}

