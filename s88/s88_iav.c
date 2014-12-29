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

    IAV stand for "Intefaccia ad alta velocita".

    Implements the HSI88 protocol for communication over USB with a desktop etc.
    Also contains the main function for the current S88 driver.

    For the HSI88 protocol see downloads on \see http://www.ldt-infocenter.com/dokuwiki/doku.php?id=en:hsi-88

    \todo Currently only the commands are implemented that are used by the Rocrail 
    software command station, see source code of file
    rocdigs/impl/hsi88.c of rocrail, \see
    https://github.com/rocrail/Rocrail/blob/master/rocdigs/impl/hsi88.c
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

#ifdef TEST
static uint8_t terminal_mode = 1;
#else
/*! flag to determin whether commincation over uart is in terminal
    mode or binary mode.  In terminal mode bytes are transmitted as
    two ascii chars in hexadecimal encoding, and in binary mode simply
    as the byte.
*/
static uint8_t terminal_mode = 0;
#endif

/** This function reads byte from the uart respecting the terminal_mode setting.
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
    @param  byte byte to write
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

/*! send all current readings (as stored in readings) via the UART
    in the format as specified in the HSI88 specification. 

    This function is currently used as follows:
    - when the s command is executed.
    - when the m command is executed.
    It is not used to communicate sensor updates while the s88 is
    running.

    @param letter "i" if called in course of setting chain lenghts, "m" if
    requested by the "m" command.

    @pre num_sensor is not zero, otherwise undefined behaviour.

    @todo check whether volatile on num_sensor and READINGS is really
    needed as we will probably switch off s88 anyway during calls to set_chain_length().

    @note this method seems to be the only place outside the ISR
    where readings is read on the main thread.

    \todo check whether the caller sneed to stop polling the s88 while this
    command is executed.
*/
static void send_all_readings(const char letter) {
  
  fputc(letter, &uart);

  //! \todo check whether this change works alright -- it is the only
  //! (?) one I made while commenting these files.
  const uint8_t num_modules = ((num_sensor - 1) / 16) + 1;  // assert(num_sensor !=0)
  fputc_iav(num_modules, &uart);  // reporting all sensors / modules.
	    
  uint8_t i;
  for(i = 1; i <= num_modules; i++) {

    fputc_iav(i, &uart); // module number
    fputc_iav(readings.module[i] / 0x100, &uart); // high byte first!
    fputc_iav(readings.module[i] % 0x100, &uart); // low byte second!
  }
    fputc(EOL_CHAR, &uart); 
}

/*! sets the number of sensors per chain - ie the length of the chain(s).

  \note We read in 3 values (one for each of 3 possible chains), but
  add them up to form one long chain of sensor.

  \todo currently deals only with one chain. Should be 3 as in HSI88.

  \todo make changing of chain length safe even if S88 is already running.
*/
void set_chain_lengths() {

  uint8_t modules = 0;

  uint8_t i;
  for(i = 0; i < MAX_CHAINS; i++) {
    modules+= fgetc_iav(); // read in length of each chains and add up
			   // to form 1 long chain.
  }

  if(fgetc(&uart) == EOL_CHAR) { // confirmation that we have a complete command

    modules = (modules > MAX_MODULES) ? MAX_MODULES : modules;
    
    fputc('s', &uart);
    fputc_iav(modules, &uart); // new number of modules.
    fputc(EOL_CHAR, &uart);
    // if chain is running
    // do ths
    //     // stop the chain, if it is running 
    // empty the send_queue?
    // else -- we are doing it for the first time
    // must also make sure that S88 stops if chain length is set to zero.
    num_sensor = 16*modules;
    // officially we should scan here, but that implemntation would be
    // in elegant, so we send just all readings as initialised and
    // there after start the scanning 
    // stop_s88();
    send_all_readings('i');
    start_s88();
  }
  // else do nothing
}

/**
   This function 

   \pre there are readings to handle
   (eg established via has_reading()) before calling this function).

   \note This function does not handle a reading (ie returns without
   effect) when there are less then 10 bytes free in the uart
   transmission queue (via uart_free_tx()) -- this is to make sure
   that all characters can indeed be written to the uart tx queue
   without overflowing. 

   \todo uart could have a better handling of overflows (eg stop
   reading in the S88)

   \todo extract common code between this and send_all_readings() */
inline static void handle_reading() { 

  if(uart_tx_free() >= 10) { // we are writing 10 chars terminal mode
			     // or 6 in binary mode (and up to 7 or 8 in
			     // test mode.
    const reading_t reading = dequeue_reading();

#ifdef IAV
    fputc('i', &uart);
    fputc_iav(1, &uart); // we are always reporting the change of only one
		  // module!
    fputc_iav((reading.sensor / 16)+1, &uart); // module number =
					       // sensor /16 -- sensor
					       // numbers start from
					       // zero, module numbers
					       // from 1 in the HSI88 protocol

    fputc_iav(reading.module_val / 0x100, &uart); // high byte first!
    fputc_iav(reading.module_val % 0x100, &uart); // low byte second!
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

   xxxxx


   \todo add _noexit attribute
   \todo handle the uart better -- as it is currently the code will
    deadlock if the PC sends an incomplete command. THis could be
   remedied by a real scheduler or by having a watchdog routine.



 */
int main() {

  sei(); // start the interrupt, and hence polling the S88 bus.

#ifdef TEST
  fputs("Starting S88 -- Version $Rev$\n", &uart);
#endif

  while(1) {
    // check whether we have a the beginning of the command
    if(uart_rx_received()) {

#ifdef TEST
      fputc('x', &uart);
#endif

      const char cmd = fgetc(&uart);

      /*! \note The calls below might deadlock if the PC never sends
	the rest of the commnand. (Eg if we are in ASCII mode, but the
	central is in binary more, the central will sends strings that
	are shorter than expected.
	
	\todo look up watch dog in glibc -- does watch dog reset
	preserve all memory content (except the stack?) -- ie are all
	global vars still set?

	First letter decides command. see HSI for all the available commands.
 */
      switch(cmd) {
      case 's': // takes a number of parameters and has a complicated
		// output 
	set_chain_lengths(); // so it has its own function
	break;
      case '\n': /** any of the the chars acceptable as EOL. \note We
		     are accepting a wider range of EOL chars than the
		     HSI88 specification which uses only \r */
      case '\r':  //! \todo add case EOL_CHAR?
	fputc(cmd, &uart); // answer with the same EOL_CHAR
	break;
      case 'v': // no parameters -- answer for rocrail musst start
		// with a "'V'"

	//! \todo what is this line for -- testing I suppose -- delete.
	fputs("Version", &uart);
	if(fgetc(&uart) == EOL_CHAR) { // HSI has response length of
				       // 41 char, not sure whether
				       // that is decisive.
	  //! \todo think about an automatic version string from git
 	  fputs("Version XXX IAV88 (C) Andre Gruening ", &uart);
	  //! \todo replace this command with a concatented string
	  //! via macro processing in the above fputs()
	  fputc(EOL_CHAR, &uart);
	}
	break;
      case 't': // no arguments
	if(fgetc(&uart) == EOL_CHAR) {
	  terminal_mode = !terminal_mode;
	  fputs(terminal_mode ? "t1" : "t0", &uart);
	  fputc(EOL_CHAR, &uart);
	}
	break;
	// toggle binary or terminal mode -- not needed for rocrail as it starts working in binary mode
      case 'm': 
	/* not really needed as it is not sent by rocrail (but
	   others?) -- but we probably need to stop the readin process
	   or empty the queue. Also during ´m´ the polling of the bus
	   should be stopped -- because READINGS could still be
	   written to be the interrupt routien (although no change
	   events will be process -- as this currently would be
	   happening. on the same thread as this. */
      default: // unknown -- just do nothing, and do not block.
	fputc(cmd, &uart);
	break;
      }

    }

    // do we have new readings from the sensor?
    if(has_reading()) {
      handle_reading(); // sends them in appropriate format over the UART
    }
  }
}
