/*! @file 
 *
 * \copyright Copyright 2014, 2017 André Grüning <libredcc@email.de>
 *
 * This file is part of LibreDCC
 *
 * \licence 
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

/*! \file

  This file implements the driver for the S88 protocol. The only
  hardware dependent part is the use of ISR() further down. I refain
  from defining this as a normal function, and make an ISR() in the
  hardware dependent source code somewhere else which then calls this
  normal reason for performance reasons.

  \pre for AVR, timer0 needs to be set up so that the ISR() is called
  with a regular frequency as done in init_s88_hardware().  The ISR()
  then calls queue_reading() each time it finds a sensor on the S88
  bus has changed sate. The queue behind queue_reading() serves to
  transmit the sensor information to other code that reads a changed
  sensor from the queue and then processes the changes sensor state --
  in the current case this code is in s88_iav.c. 

  For S88 protocol details, \see http://www.s88-n.eu/index-en.html.


  \todo Currently we implement only one chain of sensors -- instead of 3
  as in the orginal HSI88.

  \page queue_size Estimating the necessary length of the queue.
  At each clock tick we read in one sensor. Clock ticks are maximally
  30us for s88 (and 60us in our case) -- and each changed reading
  creates on entry in the queue. These are then read in the main
  thread an forwarded to the serial port. Each reading is translated
  into 1+2+6+1 = 9 bits/30us = 3bits/10us = 0.3 10^6 / s = 300.000 baud.

  If we always wait for a whole module then we have 300.000/16 = 18.75 kbd

  \todo reasoning here about the chosen size of queue -- balancing max
  speed of generation of events with rate of transmission of messages
  via the uart.
 */

#include<uart.h>
#include<stdio.h>
#include<avr/interrupt.h>

#include "s88.h"
#include "s88_hardware.h"
#include "s88_queue.h"

/** Flag to indicate whether s88 chain sweeping is to stop. Contract:
    1. Set to true in the main thread to indicate to ISR to stop when
       chain sweep is complete.
    2. If ISR reads this flag = true at the end of a sweep, it stops s88 sweeping
       and sets stopping to true to acknowledge that is has finised
*/
static volatile bit_t stopping = 0;

/** Contains the current sensor readings. Its data is accessed from
    ISR. The main thread only accesss it when s88 is stopped. */
READINGS readings;

/*! Length of the sensor chain in bits. It is set in on the main
    thread, but only when the s88 is stopped -- hence no need to make
    volatile. */
static sensor_t num_sensor = 0; 

/*! Takes a sensor reading, checks whether the sensor value has
  changed from its previous stored value, and stores the new value. 

  If the value has changed, then the sensor reading is stored in the
  queue from s88_queue.c for passing on to a handler that runs on the
  main thread. 

  \pre sensor is less than num_sensor, otherwise undefined behaviour.

  @param sensor sensor number between  0 and num_sensor-1
  @param bit sensor value as read from layout

  @todo rewrite such that changes are only send at the of a sweep
  through the sensor chain. Would reduce load on the serial interface
  for Hsi88 protocol.
*/
inline static void handle_reading(const sensor_t sensor, const bit_t bit) {

  const sensor_t byte_p = sensor / 8;
  const uint8_t bit_mask = 1 << (sensor % 8);
  
  /* If current and new reading are different, queue the new reading
     for further handling.
     \note The line below is a logical XOR, \see
     \http://stackoverflow.com/questions/1596668/logical-xor-operator-in-c 
  */
  if((!(readings.byte[byte_p] & bit_mask)) != (!bit)) {
    readings.byte[byte_p] ^= bit_mask; // update bit in current reading.
   
    /* changed sensor states have this format due to the Hsi88
       protocol: */
    const reading_t new_reading = {
      .sensor = sensor,
      .value = bit,
      .module_val = readings.module[ sensor / 16 ]
    };

    /** see \ref queue_size. 
       \todo At this point we could check whether the queue has free space,
       and if not stop reading from the bus (ie stalling the clock
     */
    queue_reading(new_reading); 
  }
}

/** main S88 interrupt to drive the S88 bus. We use timer0 on the AVRs
    as this is the lowest priority timer and we except the S88 bus to be
    quite slow compared to other activities, and the S88 protocol is
    actually not time critical, as you could at will insert longer clock
    cycles (or stall the clock).
    For details of the S88 bus protocol and timings 
    \see http://www.opendcc.de/s88/s88_n/s88-timing.html

    \note Due to the structure of the function we read always at least
    sensors 0 and 1 -- even if num_sensor <= 2 
 */
ISR(TIMER0_COMPA_vect){

  /*! There are generally these states an S88 can be in:
    - load: loads the values from the actual input latches into the
      shift register
    - load_clock: a clock cycle during load to clock the latches.
    - reset: resets the input latches (if required for the decoder) --
      no clock needed
    - clock: reads out the shift register -- one bit in each clock cycle.
  */
  enum {load, load_clock, reset, clock}; // s88_state; 
 
  //! The variable state keeps track of the s88 state across
  //! subsequent calls to this function.
  static uint8_t state = load;
  
  /*! a counter that keeps track of the clock phase of signal we are
      in. Only last bit is relevant. Clock phase 1 means rising clock
      (or switching on one of the outpus).
      Clock phase 0 means falling clock (or switching off some
      outputs).
  */
  static uint8_t phase = 0; 
  phase++;

  //! for timings and protocol details @see http://www.opendcc.de/s88/s88_n/s88-timing.html
  if(!(phase & 0x1)) {
    // rising edge of clock or other signal
    // test_on();
    switch(state) {
    case load: 
      load_on(); 
      break;
    case reset:
      reset_on();
      break;
    default: // state is clock or load_clock.
      clock_on();
    }
  }
  else {
    // We are at falling edge of clock or other signal

    //! to keep track of sensor number being read between calls to this function.
    static sensor_t sensor = 0;

    switch(state) { 
    case load: 
      state = load_clock;
      /* advance S88 bus state, but nothing else to do, nothing to switch off!  */
      break;
    case load_clock:
      clock_off();
      handle_reading(0, read_s88_data()); // reading in first sensor 0
					  // while load is still high
					  // and handle it
      sensor = 1; // sensor 1 is to be read next.
      state = reset;
      break;
    case reset:
      state = clock;
      /* We switch off load and reset at the same time. This seems to be a
	 slight violation of the S88-N protocol. But hey, then this
	 protocol is not so well defined anyway. 
	 This does not cause any problems with any sensor I have, but
	 if it did, we could then extent \enum s88_state to have states
	 that allow first load to be switch off and then a clock cycle
	 later reset or vice-versa. */
      reset_off();
      load_off(); 
      break;
    default: /* state == clock  */ {
      clock_off();
      handle_reading(sensor, read_s88_data());
      sensor++;
      if(sensor >= num_sensor) {
	// commit_readings(); // this could be an alternative approach
	// to handle all readings when all sensors have been read. --
	// Not used here -- we handle readings via queue_reading about
	// whenever a reading is different from a previous reading.
	state = load;

	/* at this point:
	   1. all lines have been switched of
	   2. all sensord have been read in this sweep down the sensor
              chain, and the next clock cycle would begin a new sweep.
	   3. all reading have been put into the queie. 
	   So the sweep is comlete. Ideal time to check whether we
	   should be stopping:
	*/

	// main asks us to stop, so we do it here:
	if(stopping) {
	  // stop timer interrupt here.
	  stop_s88();
	  // acknowledge that we have stopped:
	  stopping = 0;
	}
      }
      break;
    }
    } 
  }
}


/**
gently stops s88 operation.
Must only be called from main.
 1. Requested ISR to stop via stopping flag
 2. empties queue;
 */
void end_s88() {

  if(num_sensor != 0) {
    // request ISR to stop
    stopping = 1;

    // wait until it has actually stopped:
    while(stopping) { /* wait */ }
    
    // at this point stopping is false: ie end acknowledged.

    num_sensor = 0;  
  }
}

/** start s88 operation
    @param sensors number of sensors on the s88 chain */
void begin_s88(const sensor_t sensors) {
  num_sensor = sensors;
  if(num_sensor) start_s88();
}

  
/*! initialises the hardware independent part of the s88 reader:
  - sets all initial readings to zero. */
void init_s88() __attribute__((naked));
void init_s88() __attribute__((section(".init8"))); //! to be executed before main.
void init_s88() {
  uint8_t i;
  for(i = 0; i < sizeof(readings); i++) {
    readings.byte[i] = 0;
  }
}

  
