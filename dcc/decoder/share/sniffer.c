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

/** @file
    @todo in the makefile get rid of main.c as it is not needed, but it introduced symbols that sniffer does not need.
    - eg put main method in sniffer.c and in decoder.c 
 */


#include <share/compose_packet.h>

#include<stdint.h>

//#include "sniffer.h"
#include<dcc.h>

#include <eeprom_hw.h>
#include <error.h>

#include <dcc.h>

#include "io.h"

#ifdef __AVR
/* #elif SDCC_pic14 -- not yet implemented due to lack of implementation of PIC uart yet. And lack of fprintf support in the librariess for PIC*/
#else 
#error "Architecture not implemented"
#endif




/** pretty prints the current packet to the uart */



void handle_packet() {

  uint8_t i;

  INFO("Packet: ");

  for(i = 0; i < packet.len; i++) {
    fprintf(&uart, " %02x" ,packet.pp.byte[i]);
  }
  fputc('\n', &uart);
  return;
}

