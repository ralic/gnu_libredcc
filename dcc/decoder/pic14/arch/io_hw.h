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

#ifndef IOHW_H
#define IOHW_H 1

#include <pic/picutil.h>
#include <arch/chip.h>

// functions for io ticks:

#define io_tick() (TMR1H & 0x80) // MSB bit set every 16ms
#define acknowledge_io_tick() (TMR1H &= 0x7F) // we might miss an iotick as we do not stop the counter while changing it!? -- depends on how this intstructzion compiles

// FUNCTION TO SET THE OUTPUT pinS:

#define make_output(_output) do { OUT_TRIS &= ~(_output);} while(0)
#define set_output(_output) do { OUT_PORT |= _output;} while(0)
#define clear_output(_output) do { OUT_PORT &= ~(_output);} while(0)





/*! function or macro to read the programming button
  * @return 0 if the button is pressed, and not 0 otherwise
  */
#define get_progbutton() (PROG_PORT &= _BV(PROG_PIN))  // we use MCLR as this for many PICs can only be an input anyway, but might not be a good idea as I do not know whether this as a weak pull-up on all devices.

/*! function or macro to set error indicator */

// this below could be eliminated and made generic (see io_hw.c)

#define set_error_indicator(dummy) do{ ERROR_PORT |= _BV(ERROR_PIN);} while(0) 
#define clear_error_indicators(dummy) do {ERROR_PORT &= ~(_BV(ERROR_PIN) | _BV(WARNING_PIN));} while(0)
#define set_warning_indicator(dummy) do {ERROR_PORT |= _BV(WARNING_PIN);} while(0)

/*! function or macro to set the outputs. The outputs to set are 1 bits in the mask>
    @param mask 8bit or mask with the output bits to set
*/
//#define set_output(_output) do { OUT_PORT|= (output_mask[_output]); } while (0)

/*! function or macro to reset the outputs.
    @param mask 8bit with one bits for the outputs to set
*/
#define reset_output(_output) do { OUT_PORT &= ~(output_mask[_output]); } while(0)

/*! function or macro to read the programming button
  * @return 0 if the button is pressed, and not 0 otherwise
  */
/*! function or macro to set error indicator */

//! array that contains the or masks for the output activations.
extern const uint8_t output_mask[]; 

/*! function that sets up the io ports for the decoder */
void init_io_hw();

#endif
