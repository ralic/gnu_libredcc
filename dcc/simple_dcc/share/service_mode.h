/* Copyright 2014 André Grüning <libredcc@email.de>
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
#ifndef ServiceMode 
#define ServiceMode 1

#include <dcc.h>

void service_mode_on();
void service_mode_off();

/** \file This file contains functions to send service mode packets, see NRMA RP 9.2.3 
 */

/** send a direct mode packet embedded into the appropriate number of resets and repitition and decoder recovery time **/
uint8_t send_sm_dm_sequence(const dcc_packet * const packet);

#endif
