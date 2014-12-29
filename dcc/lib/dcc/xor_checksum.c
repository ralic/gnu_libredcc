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
#include "dcc.h"

/**
 * 1. Calculates the xor checksum of a dcc_packet and returns it.
 * 2. Can be used to text the xor checksom of a packet, because
 * calulating the checksum over a packet that contains a xor checksum
 * must return zero.
 *
 * @param packet dcc packet over which to calculate the checksum
 * @return the checksum of the packer -- or if the packet already
 * contains a checksum, validate the packages and returns 0 is the
 * package is valid.
 *
 * Perhaps implementation as a macro would also make sense -- especially for PIC
 */
uint8_t xor_checksum(const dcc_packet const * packet) {

  uint8_t i = packet->len;
  uint8_t checksum = 0;
  while(i--) {
    checksum ^= packet->pp.byte[i];
  }
  return checksum;
}
