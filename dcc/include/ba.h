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
#ifndef BA_H
#define BA_H 1

#include<stdint.h>
//#include <dcc.h>

/********** Basic Accessory **********/

#define BA_PREFIX 2
#define BA_PACKET_LEN 3
#define BA_BROADCAST_ADDR 0x1FF

/**
 * A DCC Basic Accessory packet
 */
typedef union  {
  struct {
    // first byte:
    uint8_t addr_l: 6; // bits 0-5
    uint8_t prefix: 2; // must be BA_PREFIX
    // second byte
    uint8_t gate: 1; // bit 0 // green or red? is this the right naming convention?
    uint8_t port: 2; // bits 1-2: port (ineffect an extended address
    uint8_t on: 1; // bit 3: whether to switch gate on or off.
    uint8_t addrnot_h: 3; // bits 4,5,6 are the complements of bits 6,7,8 of the 9 bit addr
    uint8_t one: 1; // must be 1.
    // third byte
    // uint8_t checksum: 8
  };
  //  uint16_t packet2;
} ba_packet;


/********** 
 */

/**
 * @returns true if packet is a basic accessory packet and 0 otherwise. [NMRA3], Section D
 * Assumes packet has a length of at least 1.
 * 
 */
#define is_ba_packet(__packet)  ( ( (__packet).ba.prefix == BA_PREFIX) && \
				  ( (__packet).len == BA_PACKET_LEN) && \
				  ( (__packet).ba.one))

/// could also be implemented as a macro
/**
 * checks whether packet is a basic accessory command and if it is, returns its 9bit address. Returns zero if it isnt not a BA address. Address zero is reserved for...
 * commented out because of c++ error */

// uint16_t get_ba_address(const dcc_packet*  const packet);

// man kopennte auch gleich den adjacenten teil mit konstanten bits hier integrieren, oder?
// portaddress ist gemain wie im p50ax protocol verstanden.
// das NRMA document ueber CV for accessory spricht auch keine Eindeutige sprache...
// es nennt "output address" was ich hier "port address" nenne.
// wir schneiden ueberzaehlige bits nicht ab, sondern vertrauen, dass die aufnehmende bitstruct passend zurecht geschnitten ist.  
#define BA_PORTADDR2PORT(__addr)     ( (__addr)) // & 0x3 // only the 2 LSB bits.
#define BA_PORTADDR2ADDR_L(__addr)   ( (__addr) >> 2) // & 0x3F (2bits already gone to the port, only 6 bit count)
#define BA_PORTADDR2ADDR_HNOT(__addr) (~( (__addr) >> 8)) // & 0x7 (only 3 bits count), 2+6 bits gone



#endif
