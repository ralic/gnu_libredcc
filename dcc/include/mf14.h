/* 
 *  Copyright 2014 André Grüning <libredcc@email.de>
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
#ifndef MF14_H
#define MF14_H 1


/** the different commands for bits 5,6,7 of the first byte */
enum {advanced_op = 1}; // used

/** the different subcommands for advanced_op to be assigned to data1, RP9.2.1, p5 **/
enum {adv_speed_128 = 0x1F // 128 speed step operation
};

/** address type: short or long **/
enum MF_ADDR_TYPE {dcc_short, dcc_long};

#define ADDR_SHORT_PREFIX = 0


typedef struct  {
  uint8_t addr: 7;  // 7 bit for address 0-127
  uint8_t prefix: 1; // prefix ADDR_SHORT_PREFIX
} addr_short;

// 0b11
#define ADDR_LONG_PREFIX = 3 
#define MF14_PACKET_LEN =  6 // 2 for addr, 3 for command, 1 for xor (do all MF14_Packet have the same length?

typedef struct  {
  uint8_t addr_high: 6; // 6 bit of address information
  uint8_t prefix: 2; // ADDR_LONG_PREFIX;
  uint8_t addr_low;
} addr_long;
    

/** the advance step operation uses byte1 so: */
typedef struct {
  uint8_t speed: 7;
  uint8_t dir: 1; // shoudnlt this be 1?
} speed128;

//! speed value for stop:
#define SPEED_STOP 0
#define SPEED_EMERGENCY_STOP 1
#define FORWARD 1
#define REVERSE 0

typedef struct {
  // 1st byte:
  uint8_t data1: 5; // bits 0-4
  uint8_t cmd: 3; // bits 5,6,7
  // 2nd byte
  union {
    uint8_t data2;
    speed128 speed;
  };
  // 3rd byte
  uint8_t data3;
} mf_cmd;

/*
typedef struct {
  addr_short addr;
  mf_cmd cmd;
} short_addr;

typedef struct {
  addr_long addr;
  mf_cmd cmd;
} long_addr;
*/


typedef union {
  struct {
    addr_short addr;
    mf_cmd cmd;
  } mf_short;
  struct {
    addr_long addr;
    mf_cmd cmd;
  } mf_long;
} mf_packet;



#endif
