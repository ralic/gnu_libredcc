#include "dcc_encoder_core.h"

dcc_packet packet = {.len = 0}; 

volatile uint8_t preamble_len = ENCODER_PREAMBLE_LEN;

//! holds number of 1s to send in the preamble -- this is different
//! for operations and progeamming mode.
//!  \todo remove volatile here

/**
   generates DCC conform bit sequence from the bytes in packet, bib by
   bit on sunsequence calls.

   Initially calls generate the require number of DCC preamble
   bits. If there is no new packet to send avaiable it continous to
   generate preamble 1 bits until there is a new packet available.

   Further calls then generate the bit sequence of the DCC packet
   until all bit have been sent. The function notifies that the global
   variable packet can be released (and eg updated with the next
   packet to send) by calling done_with_packet().

   It then continunes to generate preamble bits (at least the minimal
   number required) until a new packet is available for bit stream encode.

   @return next bit of the DCC stream generated from packet.
 */
uint8_t next_bit() {

  //! states to go through for DCC signal generation.
  enum {PREAMBLE, START_BIT, BYTE, STOP_BIT};

  static uint8_t state = PREAMBLE;
  static uint8_t bytecount = 0;


  switch (state) {
  case PREAMBLE:
    {

      static uint8_t preamble_ones = 0;
      preamble_ones++;

      // we move to the start bit state if we have had enough 1s in the preamble
      if(preamble_ones >= preamble_len) { 
	preamble_ones = 0;
	state = START_BIT;
	end_of_preamble_hook();
      }
      return 1; 
    }

  case START_BIT:
    // check whether there is a new packet to send
    if(is_new_packet_ready()) {
      state = BYTE;
      return 0;
    }
    else { // just create more preamble bits, if there is no packet to send. :-)
      return 1; 
    }

  case BYTE: // when we reach here first we know we have a valid packet to send, and we know we are at the beginning of a packet, so we have a bit to return!
    {
      static uint8_t bitmask = 0x80;

      const uint8_t bit = (packet.pp.byte[bytecount] & bitmask);

      bitmask >>= 1;
      if(bitmask == 0) {
	bitmask = 0x80;
	state = STOP_BIT;
      }
      return bit;
    }

  case STOP_BIT:
    bytecount++;
    if(bytecount >= packet.len) {
      done_with_packet(); // make it clear we are done with the current packet
      bytecount = 0;
      state = PREAMBLE; // oops we arecreating a extra preamble bit here!
      //      free(packet); // should this be done outside the ISR? Does it
		    // take too much time? We will see. 
      //      preamble_ones++;
      //      goto CASE_PREAMBLE; // is this valid?
      return 1;
	  // we are done with this packet
    }
    else { // send next byte
      state = BYTE;
      return 0;
    }
    break;
  }
  // cant happen? or shall we restructure the switch statement, so it falls through to here if not a 0 bit has to be output?
  return 1;




}
