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
#ifndef SM_H
#define SM_H 1

/**  @see NRMA RP 9.2.3 for reference */

#include<stdint.h>

/** General constants for progamming sequences -- these are the minimal values */

#define SM_RESET_PACKETS 3
#define SM_CMD_REPEAT 5
#define POWERON_CYCLE_LEN 20
#define SM_RECOVERY_PACKETS 6

/** Constants for Direct Mode programming sequence */

// bit pattern for the 4 high bits of all service mode packets.
#define SM_PREFIX 0x7

// packet for "direct" programming mode of CVs (RP 9.2.3, Sect E)

#define SM_DIRECT_PACKET_LEN 4
#define SM_DIRECT_WR 0x3
//#define SM_DIRECT_VY 0x1
//#define SM_CURECT_BIT 0x2 

/*! Note  CV #1 address is 0x0 and CV# 1024 is address 0x3FF */

typedef //union {
  struct {
    // first byte
    uint8_t cv_h: 2; // high bits of CV address 
    uint8_t cmd: 2; // the direct mode command (wr,vy,bit)
    uint8_t prefix: 4; // must be SM_PREFIX
    // second byte
    uint8_t cv_l; // low byte of CV address
    // thrid byte: 
    //    #warning data renamed data_byte!
    uint8_t data_byte; // value of CV to be written / verified (or more complex for bit manipulation
    // fourth byte is xor checksum
    uint8_t checksum;
  //  };
} smd_packet;

//#define SM_DM_CHECKSUM_IDX 3;

#define is_sm_direct_packet(__packet) ( ( (__packet).sm.prefix == SM_PREFIX) && \
					( (__packet).len == SM_DIRECT_PACKET_LEN))

#define CV_HIGH(__cv) ( ( (__cv) >> 8) ) // 2 MSB of 10
#define CV_LOW(__cv) ( (uint8_t) (__cv)) // 8 LSB of 10

//! minimal allowed CV address
#define CV_MIN 1

//! maximal allowed CV address
#define CV_MAX 1024

#endif
