#ifndef DCC_ENCODER_CORE_H
#define DCC_ENCODER_CORE_H 1

#include <inttypes.h>
#include <dcc.h>
#include "../share/dcc_encoder_core.h"

extern uint8_t volatile preamble_len;


/*! to hold the dcc packet to be sent.
  This should in principle be made volatile, but as a change does only
  matter between function calls (isr calls) where dcc_packet is used and we assume
  that functions read from / write back to memory at their start/end
  we can get away without making it volatile.
 */
extern dcc_packet packet; 


#ifdef KMOD

#define dcc_encode_setup(__packet) do {packet = __packet; flag2 = 1} 
#define dcc_encode_has_bit() flag2

#define done_with_packet_hook() /* do nothing at all */
//#define end_of_preamble_hook() { flag2 = 0};
#define is_new_packet_ready() 1

#else
#endif
//! \todo everything in here must persumable go back into dcc_encoder_hw

//! commits packet to be sent as a DCC signal
void commit_packet(const dcc_packet* const packet);

//! true if dcc power is on
uint8_t is_dcc_on();

//! switch dcc power on:
void dcc_on();

//! switch dcc power off:
void dcc_off();

//! returns next bit of dcc signal (packets needs to have been set up somewhere
uint8_t next_bit();

//! INTERNAL returns true when there is a new packet available. Called from next_bit();
uint8_t is_new_packet_ready();

//! INTERNAL called from next_bit() to make clear the packet has been dealt with.
void  done_with_packet(); 

//! called from next_bit() to make clear we have finished with the preample
void  end_of_preamble_hook();

#endif

