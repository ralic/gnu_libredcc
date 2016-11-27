/* 
 * Copyright 2014-2016 André Grüning <libredcc@email.de>
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
 * along with LibreDCC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
   * \todo test ga with very high and very low address and with broadcast address
   * \todo test with optimidation -fess und with something with las
*/ 

#ifndef DCC_H
#define DCC_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


  /**
   * Global defs for the DCC protokol
   */



  /* defines the basic accessory packet */

  /* just a forward definition, so we can use pointer to dcc_packet in ba.h etc
     struct dcc_packet; */

  /** basic accessory with 9+2bit addresses */
#include <ba.h>

  /** multifunction packet with 14bit */
#include <mf14.h>

  /// service mode packets
#include <sm.h>

  /**
   * duration of 1 signal [us]. Compare to [NMRA]
   */
#define PERIOD_1 116 

  /**
   * duration of 0 signal [us].
   * In fact it can be longer..., see [NMRA]
   */
#define PERIOD_0 (2*PERIOD_1) 

  /** 
   * minimal number of 1 bits in preamble for the decoder to latch onto the message.
   * According to [NMRA1], decoders must not accept packets with less than 10 one bits...
   */
#define DECODER_PREAMBLE_MIN_LEN 10

  /** minimal number of 1 bits in the premable for the encoder: */
#define ENCODER_PREAMBLE_MIN_LEN 14
#define ENCODER_LONG_PREAMBLE_MIN_LEN 20

  /** mimimal number of 1 bits u#in the preamble for the encoder for service mode */
  //#define SERVICE_MODE_PREAMBLE_MIN_LEN 20


  /** the number of bits we are using in the preamble: */


#define ENCODER_PREAMBLE_LEN (ENCODER_PREAMBLE_MIN_LEN + 3)
#define ENCODER_LONG_PREAMBLE_LEN (ENCODER_PREAMBLE_MIN_LEN + 3)

  /**
   * maximal length of a dcc packet in bytes including the xor checksum.
   */
#define MAX_PACKET_LEN 6

  /**
   * minimal length of a dcc packet in bytes including the xor checksum:
   */
#define MIN_PACKET_LEN 3

  /** 
   * A struct to hold a dcc packet:
   */
  typedef struct {
    uint8_t len;
    union {
      uint8_t byte[MAX_PACKET_LEN]; // addressing byte-wise
      uint16_t packet2; // to deal with the two first bytes at the same time.
      ba_packet ba; // for parsing as a ba packet.
      mf_packet mf; // packet for multifunction (ie loco decoders) with long addresses
      smd_packet sm; // service mode direct packet
    } pp; // the different packet type -- I would like to leave it an unnamed union, but then initialising it gets difficult.
  } dcc_packet;

  /**
     checks whether the packet is the idle packet, check with
     documentation.

     The packet does not need to full fill any preconditions as it is
     assumed that this is the first check done because idle packages are
     so frequent.
 
     @param p a pointer to that dcc_packet that is to be checked
     @return true if the packet is the idle packet.
  */
#define is_idle_packet(__packet) (((__packet)->pp.byte[0] == 0xFF) && ((__packet)->len == 3) && ((__packet)->pp.byte[1] == 0x00) && ((__packet)->pp.byte[2] == 0xFF))

#define is_reset_packet(__packet) (					\
				   (					\
				    (__packet).pp.byte[0] == 0x00	\
									) && \
				   (					\
				    (__packet).len == 3			\
									) && \
				   (					\
				    (__packet).pp.byte[1] == 0x00	\
									) && \
				   (					\
				    (__packet).pp.byte[2] == 0x00	\
									) \
									)


  /// calulates and returns the xor_checksum of a dcc_packet
#ifdef SDCC_pic14
#warning SDCC cannot compile this with const before packet
  uint8_t xor_checksum(const dcc_packet * packet);
#else
  uint8_t xor_checksum(const dcc_packet  * const packet);
#endif

  /// returns the decoder (not the port) address of a ba packet
  /// perhaps I should rename it do get_ba_decoder_address??
#ifdef SDCC_pic14
#warning SDCC cannot compile this with const before packet
  uint16_t get_ba_address(const dcc_packet * packet);
#else
  uint16_t get_ba_address(const dcc_packet * const packet);
#endif

  /// various fixed packets:
  extern const dcc_packet idle_packet;
  extern const dcc_packet reset_packet;


#ifdef __cplusplus
}
#endif

#endif
