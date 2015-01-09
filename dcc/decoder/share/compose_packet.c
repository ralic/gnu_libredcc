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

/**
 * \file
 *
 * Basic code to decode signal bits from the DCC signal and compose bits into a DCC packet.
 * The DCC packet itself to be parsed elsewhere, in \code handle_packet
 */

/** @todo:       
   - eleminate all unnecessary code to save memory space and then see
   whether we can fit all into one bank 
   - then check whether after the initialise we ever use another bank
   than bank 0 (ie for the special function registers -- if not we can
   write a script to delete all BANKSEL and PAGESEL commands... except
   from init code) 
   - // sdcc current version does not handle inline correctly
   - xor and packet_len should rather be initialised in START_BIT
*/

#include "compose_packet.h"
#include <error.h>


#ifdef DEBUG
#include <avr/io.h>
#endif

#ifdef NO_STRUCT_INIT
dcc_packet packet; 
#else
//! initialises len to 0 and leaves the rest undefined 
dcc_packet packet = {len: 0}; 
#endif 

#ifdef NO_LOCAL_STATICS
//! states the DCC receiver can be in:
typedef enum {START_BIT = 0, BYTE = 1, STOP_BIT = 2} DCC_SIGNAL_STATE;

static struct {
  uint8_t state; 
  uint8_t xor;
  uint8_t bits;  // rename
} local_statics; 

#define state local_statics.state
#define xor local_statics.xor
#define bits local_statics.bits

#endif

/** This function gets fed bits from the dcc stream and composes them 
 *  into a dcc packet, compare [NMRA].  
 *
 *  This function may be run on the (interruptable) main thread (as
 *  for PIC) or on an (interruptable or not) interrupt (as for ATmega)
 *  
 *  It then calls the external function handle_packet which needs to be provided elsewhere. 
 *  It is declared external to allow different efficient implementations for different 
 *  types of decoders.
 * 
 * When handle_packet is called, a valid packet is in the global variable packet. 
 * handle_packet should return quickly if compose_packet is executed during an ISR. 
 * It is assumed that packet can be overwritten once handle_packet
 * returns. So handle_packet would have to make its own copy of it.
 *
 * @param bit the current bit of the dcc stream -- if it is an integer
 * it must be either 0 or 1 -- other values will lead to unexpected
 * behaviour  
 */

//! @todo should I make this a bit type? and make it inline again...
//static inline 
void compose_packet(const uint8_t bit) {

/* We optimised compose_packet with the PIC in mind -- but these
     changes should also lead to efficient code on other uC 
     1. switch(state) statement replace with series of if-else (this
     is ugly)
     2. dec/inc to move to next state instead of assinging state directly.
     3. state of dcc signal and count of preamble bits are contracted
        in the same variable, negative values mean we are still in the
        preamlbe and counting up towards state == 0 
*/

  // reflects the different states the bit stream can be in, compare [NMRA]:
  // enum {PREAMBLE = 0, START_BIT = 1, BYTE = 2, STOP_BIT = 3};
  typedef enum {START_BIT = 0, BYTE = 1, STOP_BIT = 2} DCC_SIGNAL_STATE;

#ifndef NO_LOCAL_STATICS
  static uint8_t state = -DECODER_PREAMBLE_MIN_LEN; //! @todo integrate this into the enum.
  static uint8_t xor = 0;
#endif

  //INFO(bit ? "1" : "0"); // for debugging to test whether 0 and 1 arrive.

  //  UDR0 = bit ? '1' : '0';
  //  return;

  if( (int8_t) state < 0) {
    state++; // inc state, but only
    if(!bit) { // in case we really got a 1 bit
      state = -DECODER_PREAMBLE_MIN_LEN; // reset preamble in case of
					 // a zero bit
      ERROR(preamble_too_short);
    }
  } 
  else if (state == START_BIT) {
    if(!bit) { // waiting for a zero bit
      state++; // advance to state BYTE
    }
  }
  else if (state == BYTE) {

    static uint8_t bitpattern; // does not need to be initialised :-)
#ifndef NO_LOCAL_STATICS
    static uint8_t bits = 0; 
#endif

    //    INFO("BYTE\n");

    bitpattern <<= 1;
    if(bit) bitpattern++;

    bits++;
    if(bits >= 8) { // done with this byte as we have 8 bits
      bits = 0;
      packet.pp.byte[packet.len] = bitpattern;
      xor ^= bitpattern; 
      state++; // advance to state STOP_BIT
    }
  }
  else if(state == STOP_BIT) { 
    packet.len++;
    if(!bit) { // more bytes to come
      state--; // go back to state BYTE
      if(packet.len >= MAX_PACKET_LEN) { // packet too long, ignore. // 
	packet.len = 0;
	xor = 0;
	state = -DECODER_PREAMBLE_MIN_LEN;
	ERROR(packet_too_long);
      }
    }
    else { // bit is 1, packet is complete.
      if(!xor)  // no checksum error
	handle_packet(); // handles packet in global var "packet"
      else { 
	ERROR(checksum_nonzero);
      }
      xor = 0;
      state = -DECODER_PREAMBLE_MIN_LEN + 1; // the high stop bit may count as the first preamble,
      packet.len = 0; 
    }
  }
  else { // not one of the above states? Then our code is wrong.
    ERROR(dcc_fall_through);
  }
}

#ifdef NO_LOCAL_STATICS

void init_compose_packet() {
  state = -DECODER_PREAMBLE_MIN_LEN;
  xor = 0;
  bits = 0;
  packet.len = 0;
}

#endif
