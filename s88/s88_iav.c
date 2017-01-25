/* 
 * Copyright 2014, 2017 André Grüning <libredcc@email.de>
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

    IAV stands for "Interfaccia ad alta velocita" (ie high speed interface).

    Implements the HSI88 protocol for communication over USB with a desktop etc.
    Also implements the main functions of the current S88 protocol.

    For the HSI88 protocol see downloads on \see
    http://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:hsi-88  

    \todo Currently only the commands are implemented that are used by
    the Rocrail/JMRI software command stations.
*/

#include "s88.h"
#include "s88_queue.h"
#include "s88_iav.h"
#include "s88_hardware.h"

#include <stdio.h>
#include <uart.h>
#include <avr/interrupt.h>

/*! selected output function: if 1, we use the HSI88 format; if
  undefined we use a more human readable format. */
#define IAV 1

/*! macro if defined some test code and test outputs are produced. */
#undef TEST

/*! flag to determin whether communication over UART is in terminal
  mode or binary mode.  In terminal mode bytes are transmitted as
  two ascii chars in hexadecimal encoding, and in binary mode simply
  as the byte.
*/
static uint8_t terminal_mode = 0;

/** stores number of modules */
static uint8_t num_modules = 0;

/** reads a byte from the uart respecting the terminal_mode setting.
    @returns byte read from the uart.
*/
inline static unsigned char fgetc_iav() {
  if(terminal_mode) {
    // read 2 ascii chars and convert to byte;
    uint8_t byte;
    fscanf(&uart, "%2hhx", &byte); // read 2 digits as hex number and convert to unsigned char
    return byte;
  }
  else { // binary mode,
    return uart_getc_buffered();
  }
}

/** write a byte to a stream respecting the terminal_mode setting.
    @param  byte to write
    @param f stream to write to
*/
static inline void fputc_iav(const uint8_t byte, FILE* const f) {
  if(terminal_mode) {
    fprintf(f, "%02x", byte); 
  } 
  else {
    fputc(byte, f);
  }
}

/**
   As header of the "i" and "m" replies, send the letter and the number
   of registered modules.

   @param letter the letter (either "i" or "m") to mark start of reply.
*/
static inline void send_registered_modules(const char letter) {
  fputc(letter, &uart);
  fputc_iav(num_modules, &uart);  // num of all modules.
}


/**
   send the reading of a module to the UART. 
   Note internally module indices start from 0, however module numbers
   are reported starting from 1.

*/
static inline void send_module(const uint8_t module, const uint16_t value) {

  // module number is one more than its index.
  fputc_iav(module+1, &uart); 
  fputc_iav(value / 0x100, &uart); // high byte first!
  fputc_iav(value % 0x100, &uart); // low byte second!
}


/*! send all current readings via the UART
  in the format as specified in the HSI88 specification. 

  This function is currently used as follows:
  - after the s command for the first sweep through the sensor chain.
  - when the m command is executed.

  s88 operation should be stopped when this function is executed.

  @param letter "i" if called in course of setting chain lengths, "m" if
  requested by the "m" command.

  @note blocks if UART buffer is full!
*/
static void send_all_readings(const char letter) {

  send_registered_modules(letter);
	    
  for(uint8_t i = 0; i < num_modules; i++) {
    // wait for uart buffer:
    while(uart_tx_free() < 6); 
    send_module(i, readings.module[i]);
  }

  // wait again
  while(uart_tx_free() < 10);
  fputc(EOL_CHAR, &uart); 
}

/*! sets the number of sensors per chain - ie the length of the chain(s).

  \note We read in 3 values (one for each of 3 possible chains), but
  add them up to form one long chain of sensors.

  \todo currently deals only with one chain. Should be 3 as in HSI88.
*/
void set_chain_lengths() {

  uint8_t new_modules = 0;

  uint8_t i;
  for(i = 0; i < MAX_CHAINS; i++) {
    new_modules+= fgetc_iav(); // read in length of each chain and add up
  }

  // check command is complete.
  if(fgetc(&uart) == EOL_CHAR) {

    // stop s88
    end_s88();

    // module number in range?
    num_modules = (new_modules > MAX_MODULES) ? DEFAULT_MODULES : new_modules;
    
    fputc('s', &uart);
    // output new number of modules.
    fputc_iav(num_modules, &uart); 
    fputc(EOL_CHAR, &uart);

    /* officially we should scan here, but that implemntation would be
       inelegant, so we send all just readings as initialised.
    */
    send_all_readings('i');

    // start s88 operation now.
    begin_s88(16*num_modules);
  }
  // else do nothing [or error msg or just a cr in reply -- not clear from docu what Hsi88 does.]
}

/**
   This function reads a reading from the queue and outputs it to the UART

   \pre There are readings to handle (eg established via
   has_reading()) before calling this function).  

   \note This function does not handle a reading (ie returns without
   effect) when there are less then 10 bytes free in the uart
   transmission queue (via uart_free_tx()) -- this is to make sure
   that all characters can indeed be written to the uart tx queue
   without overflowing. 

   \todo UART could have a better handling of overflows (eg stop
   reading in the S88)
*/
inline static void handle_reading() { 

  /* make sure to write in one go as to not block!
     We are writing 10 chars terminal mode:
     message reads: "i01234567\r", ie 10 chars. */
  if(uart_tx_free() >= 10) { 
    const reading_t reading = dequeue_reading();

#ifdef IAV
    send_registered_modules('i');
    send_module(reading.sensor / 16, reading.module_val);
    fputc(EOL_CHAR, &uart); 
#else /* here we write 8 chars! For testing only we print the reading
	 more human-readably. */
    fprintf(&uart, "%u %s\n", reading.sensor, reading.value ? "On" : "Off"); 
#endif
  }
}

/**!
   Main function.
   - enables the interrupts -- all other initialisation has been done by
   naked functions in sections .init5 and .init8.
   - checks whether we have any new commands pending via the uart,
   parses and executed them.
   - checks whether we have any sensor updates and handles them.
   - is an eternal loop

   \todo generally handle the UART better. Currently the code will
   deadlock if the PC sends an incomplete command. This could be
   remedied by having a watchdog routine or a real scheduler to
   detected time-outs and then jump to the beginning of main again
   without reinitialisation.
*/


int main() __attribute__((OS_main));
int main() {

  sei(); // start the interrupts (needed for UART and later for S88)

  while(1) {

    // check whether we have a the beginning of the command
    if(uart_rx_received()) {
      // Single first letter decides command.
      const char cmd = fgetc(&uart);

      /*! \note The calls below might deadlock if the PC never sends
	the rest of the commnand. (Eg if we are in ASCII mode, but the
	central is in binary more, the central will sends strings that
	are shorter than expected.
	
	\todo look up watch dog in glibc for AVR and AVR data sheet:
        - does watch dog reset preserve all memory content (and the stack)? -- ie are all
	global vars still set?
	\todo orperhaps use a timer to time out?
      */
      switch(cmd) {
      case 's': 
	set_chain_lengths(); 
	break;
	
      case EOL_CHAR:
	// echo EOL_CHAR
	fputc(cmd, &uart); 
	break;
	
      case 'v': 
	if(fgetc(&uart) == EOL_CHAR) {
	  // original HSI88 has response length of 41 chars, not sure
	  // whether is imporant for any central?
	  //! \todo add an automatic version string from git
 	  fputs("ver. IAV88 (C) Andre Gruening 2014, 2017.", &uart);
	  fputc(EOL_CHAR, &uart);
	}
	break;

      case 't':
	if(fgetc(&uart) == EOL_CHAR) {
	  terminal_mode = !terminal_mode;
	  fputs(terminal_mode ? "t1" : "t0", &uart);
	  fputc(EOL_CHAR, &uart);
	}
	break;

      case 'm': 
	if(fgetc(&uart) == EOL_CHAR) {
	  end_s88();
	  send_all_readings('m');
	  begin_s88(16*num_modules);
	}
	break;

      default:
	// unknown cmd -- just echo the character, and do not block.
	fputc(cmd, &uart);
	break;
      }
    }

    // do we have new readings from the sensor?
    if(has_reading()) {
      // send them to UART
      handle_reading(); 
    }
  }
}
