/*
 * Copyright 2006 Wolfgang Kufer <kufer@gmx.de>
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
 * The code in this file is based on code initially published under
 * the GNU general public license 2 by Wolfgang Kufer as part of
 * OpenDCC project (see http://www.opendcc.de) in 2006. I (Andre) owe him
 * a lot for my understanding of how to decode DCC signals reliably.
 *  
 * Major changes 
 *   - removed (now unnessary) optimisations.
 *   - introduced other (unnecessary) optimisations.
 *   - restructured in what I think (incorrectly) to be a nicer
 *     structure making it easier to adapt it to other uC (eg PIC)
 *   - separated (hard-independent) DCC part from hardware dependent-part.
 *   - (too) verbose commenting
 *
 * Basic code to decode signal bits from the DCC signal and compose bits into a DCC packet.
 * The DCC packet itself to be parsed elsewhere, in \code handle_packet
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
  uint8_t bits;
} local_statics; 

#define state local_statics.state
#define xor local_statics.xor
#define bits local_statics.bits

#endif

/** This function gets fed bits from the DCC stream and composes them 
 *  into a dcc packet, compare [NMRA].  
 *
 * It then calls the external function handle_packet which needs to be provided elsewhere. 
 * It is declared external to allow different efficient implementations for different 
 * types of decoders.
 * 
 * At the point handle_packet is called, a valid packet is in the global variable packet. 
 * handle_packet should return quickly if compose_packet is executed during an ISR. 
 * It is assumed that packet can be overwritten once handle_packet
 * returns. 
 *
 * @param bit the current bit of the dcc stream
 */

void compose_packet(const uint8_t bit) {

/* We optimised compose_packet with the PIC in mind -- but these
   changes should also lead to efficient code on other uC 
   1. switch(state) statement replaced with series of if-else (this
      is ugly)
   2. dec/inc to move to next state instead of assinging state directly.
   3. state of DCC signal and count of preamble bits are contracted
      in the same variable, negative values mean we are still in the
      preamlbe and counting up towards state == 0 
*/

  // reflects the different states the bit stream can be in, compare [NMRA]:
  typedef enum {START_BIT = 0, BYTE = 1, STOP_BIT = 2} DCC_SIGNAL_STATE;

#ifndef NO_LOCAL_STATICS
  static uint8_t state = -DECODER_PREAMBLE_MIN_LEN;
  static uint8_t xor = 0;
#endif

  //  INFO(bit ? "1" : "0"); // for debugging to test whether 0 and 1 arrive.
  //  UDR0 = bit ? '1' : '0';
  //  return;


  if( (int8_t) state < 0) {
    state++;   // inc state, but only
    if(!bit) { // in case we really got a 1 bit
      state = -DECODER_PREAMBLE_MIN_LEN; // reset preamble in case of a zero bit
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
    else { // if bit is 1 packet is complete.
      if(!xor)  
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
