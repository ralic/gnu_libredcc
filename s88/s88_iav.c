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
#include "s88.h"
#include "s88_queue.h"
#include "s88_iav.h"
#include "s88_hardware.h"


#include <stdio.h>
#include <uart.h>
#include <avr/interrupt.h>

#define IAV 1

typedef void COMMAND(void) ;

/* 
   typedef struct {
   char letter;
  uint8_t bytes;
  COMMAND cmd;
} IAV_CMD;

IAC_CMD cmds[] = 
  { letter: 's'; bytes: MAX_CHAINS; cmd: set_chain_length},
  { letter: 'v'; bytes: 0; cmd: send_version}
  };

#define NUM_CMDS (sizeof{cmds} / sizeof{IAV_CMD})
*/

#ifdef TEST
static uint8_t terminal_mode = 1;
#else
static uint8_t terminal_mode = 0;
#endif

inline static unsigned char fgetc_iav() {
  if(terminal_mode) {
    // convert ascii_chars to byte;
    uint8_t byte;
    fscanf(&uart, "%2hhx", &byte); // read 2 digits as hex number and convert to unsigned char
    return byte;
  }
  else {
    return uart_getc_buffered();
  }
}

static inline void fputc_iav(const uint8_t byte, FILE* const f) {
  if(terminal_mode) {
    fprintf(f, " %02x", byte); // for real we have to delete the space!
  } 
  else {
    fputc(byte, f);
  }
}

static void send_all_readings(const char letter) {
  
  fputc(letter, &uart);

  uint8_t num_modules = (max_sensor / 16);  // assert(max_sensor % 16 = 0)

  fputc_iav(num_modules, &uart);  // report all senesors
	    
  uint8_t i;
  for(i = 1; i <= num_modules; i++) {

    fputc_iav(i, &uart); // module number
    fputc_iav(readings.module[i] / 0x100, &uart); // high byte first!
    fputc_iav(readings.module[i] % 0x100, &uart); // low byte second!
  }
    fputc(EOL_CHAR, &uart); 
}

void set_chain_lengths() {
  uint8_t i;
  uint8_t modules = 0;
  for(i = 0; i < MAX_CHAINS; i++) {
    modules+= fgetc_iav();
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
    max_sensor = 16*modules;
    // officially we should scan here, but that implemntation would be
    // in elegant, so we send just all readings as initialised and
    // there after start the scanning 
    send_all_readings('i');
    start_s88();
  }
  // else do nothing, or ungetc?
}

/*
void execute_iav_cmd(const char letter) {

  uint8_t i;
  for(i = 0; i < NUM_CMDS; i++) {
    if(letter == cmds[i].letter) {
      (cmd[i].cmd)();
      return;
    }
  }
}
*/

inline static void handle_reading() { 

  if(uart_tx_free() >= 10) { // we are writing 10 chars terminal mode
			     // or 6 in binary mode

    const reading_t reading = dequeue_reading();
#ifdef IAV
    fputc('i', &uart);
    fputc_iav(1, &uart); // we are always reporting the change of only one
		  // module!
    fputc_iav((reading.sensor / 16)+1, &uart); // module number =
					       // sensor /16 -- sensor
					       // numbers start from
					       // zero, module numbers
					       // from 1 

    fputc_iav(reading.module_val / 0x100, &uart); // high byte first!
    fputc_iav(reading.module_val % 0x100, &uart); // low byte second!
    fputc(EOL_CHAR, &uart); 
#else // here we write 7 chars!
    fprintf(&uart, "%u %s\n", reading.sensor, reading.value ? "On" : "Off");
#endif
  }
}

int main() {

  sei();

#ifdef TEST
  fputs("Starting S88 -- Version $Rev$\n", &uart);
#endif

  //  fputs("\r\r\r\r\r", &uart);

#if 0
  while(1) {
    if(uart_tx_free()) 
      fputs("x", &uart);
  }
#endif

  while(1) {
    if(uart_rx_received()) {

#ifdef TEST
      fputc('x', &uart);
#endif

      const char cmd = fgetc(&uart);

      switch(cmd) {
      case 's': // takes a number of parameters and has a complicated
		// output 
	set_chain_lengths(); // so it has its own function
	break;
      case '\n': // any of the the chars acceptable as EOL?
      case '\r': 
	// echo the same back;
	fputc(cmd, &uart); // answer with an EOL_CHAR
	break;
      case 'v': // no parameters // answer for rocrail musst start
		// with a "'V'"
	fputs("Version", &uart);
	if(fgetc(&uart) == EOL_CHAR) { // HSI has response length of 41 char.
 	  fputs("Ver. $Rev$ / $Date$ / IAV88 (c) Andre Gruening", &uart);
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
	// not really needed as it is not sent by rocrail (but
	// others?) -- but we probably need to stop the readin process
	// or empty the queue
      default: // unknown 
	// just do nothing! or await next EOL_CHAR?
	fputc(cmd, &uart);
	break;
      }

      // check next char is EOL_CHAR, only execute command cmd if it is followed by the correct EOL?
      // if not, e
      // we just do nothing.
    }
    //    }
    // do we have new readings from the sensor?
    if(has_reading()) {
      handle_reading(); // sends them in appropriate format over the UART
    }
  }
}
