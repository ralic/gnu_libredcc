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
 * Takes a BA_PACKET and extract the _decoder_ address from it. 
 * Valid decoder addresses are 0..XXX [NRMNAXXX, paga YY]
 * @param packet must be a valid BA packet (eg tested with is_ba_packet)
 * @return Returns the decoder address.
 *
 * @todo reimplement as a macro.
 */
#ifdef SDCC_pic14
#warning SDCC cannot compile this with const before packet
uint16_t get_ba_address(const dcc_packet * packet) {
#else
uint16_t get_ba_address(const dcc_packet * const packet) {
#endif
  // but 0 could be a legal address? It is the broadcast addresses as far as I can see from NRMA RP9.2.1, D, l429sqq -- So better return -1?
  return ( ( ( ~packet->pp.ba.addrnot_h) & 0x7) << 6) + packet->pp.ba.addr_l; 
}
