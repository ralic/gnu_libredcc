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

/*! main interrupt of s88 */

#define HSI

#include<uart.h>
#include<stdio.h>
#include<avr/interrupt.h>

#include "s88.h"
#include "s88_hardware.h"
#include "s88_queue.h"

//! holds the current readings -- may be accessed from both interrupt and main thread
volatile READINGS readings;

/*! length of the sensor chain in bits  -- curenntly we allow only one
    chain, but inprinciple we could have many.
Should be renamend num_sensor  */ 

volatile sensor_t max_sensor = 0;
// #define CHAINS (sizeof(chain_length) / sizeof(uint8_t))

/*! reads in a bit and stores in the readings.
  This function must only be called if global variable state points to
  a correct detector value -- I see I should rather have different
  variables for the two purposed 
  must make sure that state is only in the right range 0..max_chain_length_bits-1*/
inline static void read_bit(const sensor_t sensor, const bit_t bit) {

  const sensor_t byte_p = sensor / 8;
  const uint8_t bit_mask = 1 << (sensor % 8);
  
  // Shall I change this to state based stuff as in the DCC encoder
  // and decoders?

  /* if past reading and current are different (below is a logical
     XOR, see
     \url{http://stackoverflow.com/questions/1596668/logical-xor-operator-in-c}. 
  */
  if((!(readings.byte[byte_p] & bit_mask)) != (!bit)) {
    readings.byte[byte_p] ^= bit_mask; // toggle bit in reading testing

    
    /* we assume this always succeeds -- I should calculate the
       margines for the serial transmission with respect to the
       maximum speed of the s88 bus! */
    reading_t reading = {sensor: sensor, value:bit, module_val: readings.module[sensor / 16]};
      //    reading.sensor = state;
      //    reading.value = bit;
    queue_reading(reading); 
  }
}

//! how often is this to be called? Max frequency was 33kHz? Look it
//up again!
ISR(TIMER0_COMPA_vect){

  /*! these are the 3 states an s88 can be in:
    - load: loads the values from the actual input latches (are they latches?) into the shift
    register
    - reset: resets the input latches (if required for the decoder)
    - read: reads out the shift register (bit by bit).
  */
  typedef enum {load, load_clock, reset, clock} s88_state; 
 
  /*! natural start is with load or with reset? 
    state serves 2 purposes: 
    1. it keeps track of the s88 state
    2. if it has positive values, it is also counting which
    bit/decoder we are reading
    Starting with phase = 0 und state = reset means we enter the
    interrupt so at the first time that all outputs are switch off and
    next we start reading bits.
    
    Due to the structure of the method we read always at least sensors 0 and 1 -- even if max_sensor <= 2
  */
  static uint8_t state = load;
  
  // We are at rising clock (or rising other signal: -- does this case
  // statement work? Because we have define the labels so strangely --
  // how is it being converted?
  
  //! a counter that keeps track of the phase of signal we are in. Only last bit is relevant.
  static uint8_t phase = 0; 
  phase++;

  //! for timings @see http://www.opendcc.de/s88/s88_n/s88-timing.html
  if(!(phase & 0x1)) {
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
  else{
    // We are at falling edge of clock or other signal
    // all_off();

    static sensor_t sensor = 0;

    switch(state) { 
    case load: 
      state = load_clock;
      /* nothing else to do, nothing to switch off!  */
      break;
    case load_clock:
      clock_off();
      read_bit(0, read_s88_data()); // reading in first sensor while load is still high
      sensor = 1;
      state = reset;
      break;
    case reset:
      state = clock;
      reset_off();
      load_off(); // is it important that load is switched off a bit later than reset?
      break;
    default: /* state = clock  */ {
      clock_off();
      read_bit(sensor, read_s88_data());
      sensor++;
      if(sensor >= max_sensor) {
	// commit_readings();
	state = load;
      }
      break;
    }
    } 
  }
}

//! initialises the hardware independent part of the s88 reader in a hardware independent way.
void init_s88() __attribute__((naked));
void init_s88() __attribute__((section(".init8"))); // to be executed before main.
void init_s88() {
  uint8_t i;
  for(i = 0; i < sizeof(readings); i++) {
    readings.byte[i] = 0;
  }
}

  
