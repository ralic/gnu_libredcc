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

/** \file 
 */

//! \todo Some centrals might send activate packets continuously (LEnz), so only the first should be reacted to... (so we need to store our state...) -- what is the source of this? 
//! @todo: change attributes to near, pure, const

/* there were 2 problems with sdcc: 
   1. does not initialise static locals and 
   2. could not deal with const pointer in certain circumstance 
   3. and perhals cannot well initialise structs

   \todo WHY THE DECODER MIGHT GET STUCK:
   - TRYING TO use the uart while on the interrupt?
   - but would this survive the reset? does the crash survive a reaset?
*/

#include <share/defs.h>

#include<stdint.h>

#include "decoder.h"
#include<dcc.h>

#include <eeprom_hw.h>
#include <error.h>

#include <dcc.h>

#include "io.h"

static uint16_t port_id[PORTS];

#ifdef __AVR
#elif SDCC_pic14
#else 
#error "Architecture not implemented"
#endif

//! normal ba output mode.
inline static void handle_ba_opmode() {

  const uint16_t portid = BA_PORTID(packet);

  uint8_t i;
  for(i = 0; i < PORTS; i++) {
    if(port_id[i] == portid) {
      // bei vorherigem button progmode muessen wir noch ein wenig
      // warten bevor wir das naechste Packet annehmen?  
      activate_output((i << 1) + packet.pp.ba.gate); // \todo and deactive the other one?
      //      INFO("Execute BA packet" EOLSTR);
#if DEBUG
      fprintf(&uart, "for port/gate %u/%u\n", i, packet.pp.ba.gate);
      #warning remove this.
#endif
      return; // this precludes having two outputs programmed to the same address
    }
  }

  INFO("BA Packet not for us" EOLSTR);
}

/*!
  @param port -- the port to progamme in the range 0..PORTS-1
 */
inline static void handle_ba_progmode(const uint8_t port) {

  //  INFO("In BA Progmode");

  if(port < PORTS) {
    const uint16_t portid = BA_PORTID(packet);
    port_id[port] = portid; 
    eeprom_update_word(&(port_id_eeprom[port]), portid); // currently
							 // this waits
							 // until
							 // EEPROM is
							 // written --
							 // so this
							 // introduces
							 // a delay of
							 // about 5ms
							 // on PIC and
							 // ??ms on AVR
    //    INFO("BA Address programmed");
#ifdef DEBUG
    fprintf(&uart, "for port %u, address id %x\n", port, portid);
#warning remove this.
#endif

  }
  // we probably also need to delay here to enaure we get no programme twice in case the central sends same BA packet multiple times. 
  //  RESET_ERROR(); // we will have missed many DCC bits as EEPROM programming takes long. But restting here is futible.
}

inline static void handle_ba_packet() {

#if 0
  // future handling of BROADCAST packets here?
#endif
  
  // ignore packets that are off commands, we do our own timing.
  if(!packet.pp.ba.on) {
    INFO("Ignore BA off packet" EOLSTR);
    return; 
  } 

  //** @todo: we must have a timeout here, so that if the central sends
  // commands multiple times, subsequent addresses are not
  // reprogrammend. 
  // also there ought to be protection that two outputs of the same
  // gate are not activated at the same time :-)
  // finally red and green should be done properly.

  //  INFO("Got BA on packet" EOLSTR);

  if(button_count) { // progmode.
    INFO("BA prog mode");
#warning this is not thread-safe. as the prog button might be pressed between the previous and following line of code
    // on PIC this is not a problem as both the output routne and compose packet run on the main thread
    // on AVR this could be a problem if the DCC interrupt allows other interrupts to occur.
    // for AVR -- so I need to make sure the AVR does not reenable
    // other inttruts while running! At least not the timer
    // interrupts. (or if we change it so that the output routine will
    // run in its own interript, while the rest runs on the main programme)
    handle_ba_progmode(button_count-1);
    button_count++;
    if(button_count > PORTS) { // if we have reached the end of the ports, stop the programming mode
      button_count = 0;
    }
  }
  else { 
    INFO("BA opmode" EOLSTR);
    handle_ba_opmode();
  }
}
  
void handle_packet() {

  //  INFO("Got valid packet" EOLSTR);

  /*! postmode currently not utilised as the ba decoder does not
    accept any opmode commands that could also be progmode commands...
  */
  //typedef enum {opmode, premode, smmode, postmode} modes; 
  enum {opmode, premode, smmode, postmode};
#ifndef NO_LOCAL_STATICS
  static uint8_t previous_mode = opmode;
#endif
  uint8_t next_mode = opmode; // default is the next mode is opmode

  // ignore all op mode packets but BA backets:
  if (is_ba_packet(packet)) {
    //    INFO("BA Packet");
    handle_ba_packet();
  }
  else if(is_sm_direct_packet(packet)) {
    INFO("Got SM dircect packet");
    button_count = 0; // end button prog mode if we have a prog packet
    if(previous_mode == premode || previous_mode == smmode) {
      next_mode = smmode;
      /* we are not handling postmode as it cannot lead to any confusion anyway with our ba decoder
	 smmode: we enter it if previous was a reset packet, we stay in
	 as long as we are receiving sm packets, otherwise we leave it
	 to an intermediate state and thence only back to opmode if we
	 have received an oppacket that is not identical to an sm
	 packet. Here we can leave sm mode immedietalz to opmode as we
	 do not understand those opmode packets that could be similar to
	 an sm mode pacewt (some 4 byte long short ga packets that would
	 be).  So we only keep track of wether previous packet was reset
	 or sm: then we are in opmode. -- For all other packets we leave
	 sm_mode immediately.  */
      //How to implement the 20ms timeout to switch back from smmode? */
      // handle_sm_direct_packet();
    }
    else {
      // ignore sm packet as precondition to enter smmode have not
      // been met, next mode is opmode.
    }
  }
  else  if(is_reset_packet(packet)) {
    INFO("Reset Packet" EOLSTR);
    if(previous_mode == smmode) { // or postmode }
      // next_mode == opmode; // clear by default
    }
    else {next_mode = premode;} // ie for opmode, premode.
    // shall we perhaps only save the new cv values if we are leaving progmode, beause in progmode itself, we should not write to eeprom as it takes to long in case the central sends several sm packets in a row?
    // what else todo? deactiveate outputs?
    // reset progmode states? Perhaps rather not -- what if a central sends reset packets all the time? butten_counts
  }


  //  INFO("End Handling packet" EOLSTR);

  if ((previous_mode == smmode) && (next_mode != smmode)) {
    // just switching away from progmode at this step. -- disadvantage is that of this to be reached this requires the central to send one more non sm mode command (my central does this). However we must also take into account that the central does not do this (eg powerdown??) and hence call the function to write after the timeout of 20ms! Best would perhaps be to have smmode ticking away like on of the outputs and then burns it into the eeprom after 20ms... 
    // Also change the eeprom write routine for PIC so that it waits before a write, not after -- do we need to wait before a read as well?
    // write_cv_eeprom()? 
  }
  previous_mode = next_mode;
}

//! read addresses from eeprom into ram.
#ifndef HAS_NO_INIT
void init_decoder() __attribute__((naked)) __attribute__((section(".init8"))); 
#endif
void init_decoder() {
  uint8_t i;
  for(i = 0; i < PORTS; i++) {
    port_id[i] = eeprom_read_word(&(port_id_eeprom[i]));
  }
}
