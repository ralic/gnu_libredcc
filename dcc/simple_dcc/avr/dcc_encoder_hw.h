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
#ifndef DCC_ENCODER_HW_H
#define DCC_ENCODER_HW_H 1

#include <avr/io.h>
#include <dcc.h>


#ifdef __cplusplus
extern "C" {
#endif

  //! length of the DCC preamble to send (ie number of leading 1s). This is initialiased to ENCODER_PREAMBLE_LEN
  //    volatile extern uint8_t preamble_len;

  //! checks whether the dcc encoder is still busy with the last packet or ready to send a new one.
  //  uint8_t busy_with_last_packet();

  //! switches signal generation on
  void dcc_on();

  //! swirches dcc signal generation off, however waits until current packet has been sent. Restart DCC with dcc_on()
  void dcc_off();


  //! decide whether we compile with or without shortcut alarm
#define SHORTCUT 0

#ifdef SHORTCUT
  //! \return whether the shortcut warning is triggered. This assumes a short cut is a low-level on PD2
#define is_shortcut() (PD2 == 0)

  //! switches dcc off immediately, for example when a short-cut has arisen. 
  void emergency_dcc_off();

  //* switches encoder to service mode -- may be called from both service or operations mode
  void service_mode_on();

  //* switches encoder to operations mode
  void service_mode_off();

#endif

#ifdef __cplusplus
}
#endif

#endif
