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
#ifndef COMPOSE_PACKET_H
#define COMPOSE_PACKET_H 1

/* \file
 */

#include <dcc.h>

/** This function composes subsequently read DCC bits to DCC
    packets. Once a complete packet has been read this is stored in
    packet (below). It then calls handle_packet() which must be
    provided externally.  
    \param bit a bit read from the DCC raw data stream 
    \sideeffect calls handle_packet once a complete packert has been
    read.
    \sideeffect sets packet.
*/
void compose_packet(const uint8_t bit); 

/**
 * This function needs to be provided elsewhere.
 * It will be called from within compose_packet. It should process the
 * DCC packet provided in the global variable packet. An
 * implementation of handle_packet needs to be aware that on return
 * from handle_packet, the global variable packet might changes. Hence
 * handle_packet needs to make a deep copy of packet if the content of
 * it is needed in some functions that handle_packet might call even
 * after its return.
 * Precondition is that the message has been xor checked and is longer than one byte. 
 * It is assume that execute_packet accesses packet below readonly, and that upon return of execute packet the packet can be * overwritten
 * The code in dcc_receiver calls handle_packet when it has a packet ready. The packet is passed in the global var packet below. o 
 */
void handle_packet();

/** 
    The packet itself. Use of passing a packet between compose_packet
    and handle_packet. For efficiency reasons, especially on a PIC,
    this is done via a global variable (and not a function parameter).
*/
extern dcc_packet packet; 

#ifdef NO_LOCAL_STATICS
/** method to initialise the "globalised" local statics in the form of a struct 
 @todo should this rather run under NO_STRUCT_INIT? */
void init_compose_packet();
#endif

#endif
