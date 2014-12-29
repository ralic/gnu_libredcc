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
// 1. Finalise sniffer!
// 2. Check whether it is a problem of copying the value, because I changed the way the variables are treated! So I need volatie somewhere or how do I need to copy?

#include "service_mode.h"
#include "dcc_encoder.h"

// #define POWERON_CYCLE_MSG "Power on cycle send"

//! Are we in service mode? */
//static uint8_t mode = OP_MODE;

/** 
    switches to service mode -- may be called from both service or operations mode
 */
void service_mode_on() {
  
  // switch to long preamble 
  preamble_len = ENCODER_LONG_PREAMBLE_LEN;
  // mode = SERVICE_MODE;

  // stop OperationsModeQueue (or even flush it completely??) and start it again when leaving Service mode
}

/** switches to operations mode -- may be called from bith service or operations mode
 */
void service_mode_off() {
  // switch to normal peramble
    preamble_len = ENCODER_PREAMBLE_LEN;
    // mode = OP_MODE;
}





/**
 to let decoders tune in on the dcc stream, however I want my own
 decoders to be able to operatate without this... 
*/
static uint8_t send_poweron_cycle()  {
  
  uint8_t i;
  for(i = POWERON_CYCLE_LEN; i > 0; i--) {
    commit_packet(&idle_packet);
  }
  return 0; // POWERON_CYCLE_MSG;
}
  



/** Sends a service mode instruction packet for direct mode, embedded into the required sequence of  reset packet and with the required number of repetions.

@param packet the service mode instruction to send
@return "Ok" on success

It would be here were we would need to check whether we have received an acknowledge etc
  // clear flag, enable irs, overcurrent detectng (or all of this could already happen when SM is enabled.

*/
 
uint8_t send_sm_dm_sequence(const dcc_packet * const packet) {

  // do_power-on cycle? or do it when we switch to SM mode?
  
  uint8_t i;
  for(i = SM_RESET_PACKETS; i > 0; i--) {
    commit_packet(&reset_packet);
  } 

  //  for(uint8_t i = SM_CMD_REPEAT + SM_RECOVERY_PACKETS; i > 0; i--)
  //  {
  for(i = SM_CMD_REPEAT; i > 0; i--) {
    commit_packet(packet);
  }

  // /* 
  for(i = SM_RECOVERY_PACKETS; i > 0; i--) {
    commit_packet(&reset_packet);
  }
  // */
  
  return 0;
}
