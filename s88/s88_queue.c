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

/** @file 
    This file contains code that realises a queue data structure (of
    finite maximal length) that serves to exchange sensor readings
    between the interrupt service routine in s88.c and the code for
    handling the readings in the main program code, for example in
    s88_iav.c 

    \page queue_size

    We discuss here the size of the queue.
    
    We assume the queue is long enough, changed sensor readings are
    a rare event, and that serial transmission (which pops off the
    queue) is sufficiently fast, so that the queue is never at full
    capacity -- hence we assume queuing a new reading always
    succeeds. So no error catching at the line below.
    \todo calculate the margines for the serial transmission with respect to the
    maximum speed of the s88 bus!
*/

#include "s88_queue.h"

#include<inttypes.h>
#include<avr/interrupt.h>

/** Maximal length of queue. 
    \todo Do timing calculation so show this is sufficient 
    \see the page above
*/
#define SENSOR_BUFFER_LEN 256

//! the sensor buffer, to be accessed both from the interrupt and the main thread.
volatile static reading_t sensor_buffer[SENSOR_BUFFER_LEN];

/*! index of the position to be written next -- to be accessed 
    from the main and interrupt thread 
*/
volatile static uint8_t write_idx = 0;

/*! index of the postion to be read next. To be accessed only from
    the reader of the queue on the main thread 
*/
static uint8_t read_idx = 0;

/*! removes items from the queue that have already been read and are
  hence obsolate. Needs to be called regularly to avoid an overrun of
  the queue.
 */
void tidy_sensor_buffer() {
    if(read_idx >= write_idx) { // if no more chars left to read in read buffer
      cli();  // disable all interrupts [1]-- in fact the S88_interrupt would be enough

      // then tidy buffer, double-check idiom
      if(read_idx >= write_idx) { // we could check here whether
				      // rx_write_idx is not zero to
				      // save a few CPU cycle when we
				      // are idle, but the buffer has
				      // already been reset.
	write_idx = 0;
	read_idx = 0;
      }
      // reenable interrupt (or whatever we had disabled before at [1])
      sei();
    }
}

/*! @return whether the queue has an element or not
  Intended to be called on a non-interrupt thread only
 */
bit_t has_reading() {
  tidy_sensor_buffer();
  /** \note write_idx will be advanced only in the interrupt routine (and
      read_idx only on the thread on which current has_reading is
      being called) -- so no race condition, we
      maximially fail to realise that we have got an element to read) */
  return write_idx > read_idx;
}

/*!  adds a sensor reading to the queue. Intended to be called on the
     interrupt only.
     @param the sensor reading 

     \todo we are not dealing with a potential buffer overflow as we
     assume the end of the queue is never reached.

     \todo speed up this function by passing the value as a
     reference, not as a value.

     \post reading can be destroyed because it has been copied to
     sensor_buffer (or is otherwise discarded).
*/ 
void queue_reading(const reading_t reading) {

  // is there still space in the buffer?
  if(write_idx >= SENSOR_BUFFER_LEN) {
    /* \todo deal with the buffer overflow. Eg Disable s88 interrupt
       to avoid more s88 is being read, (or introduce a wait cycle, eg
       a flag the start of the ISR?) and wait until the main thread
       has read elements from queue? -- This could quite simply be
       done by the ISR checking whether the queue has free spaces.
    */
    // do nothing at buffer overflow.
  }
  else {
    // there is still space in the buffer.
    sensor_buffer[write_idx++] = reading;
  }
}

/*! gets an element from the queue. Intended to run on main thread --
    call only if you know that queue has an element, checked with
    has_reading().
    \pre had_reading() must return nonnull.
*/
reading_t dequeue_reading() {
  return sensor_buffer[read_idx++];
}
