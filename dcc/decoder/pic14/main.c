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
// $Id$

#include<pic14regs.h>
#include<pic/picutil.h>

#include "interrupt.h" 
#include <share/compose_packet.h>
#include "io_hw.h"
#include <share/io.h>

/*
 * RC0: set if we miss a preamble bit, cleared routinely in the output -- we still mis preamble bit if we are handling a command
 * timer loop or if we have a xor problem or for xor error.
 * RC1: measures timing between external int and timer0 (and should be one of the ports)
   RC3: Set if we a lose a DCC bit, ie bit was not read until a new bit is available, cleared routinely in output timer
*/

#include <share/decoder.h>

//! pin for external INT on pic16f690.
//#define INT_PIN RA2  // if I change this here, I must also change that in the assembler isr!
//! pin for external int also has analog input function -- we must disable this first.
//#define INT_ANS ANS2

/*
 * TODO:
 * - change AVR part for simpler use of timer2, and also whether
 *   checking of the bit can be moved more towards the front, 
 * - check when which interrupts are en/disabled (BOTH), and when
 *   interrupt flags are clear -- so that we do not get a deadlock if
 *   one interrupt is missed because the execution of itself or another
 *   just takes too long, and  
 * - check when the timer is running and when stops (for AVR)
 */

// asssume the clock is running with F_CPU Hz, than we have F_CPU Hz/ 4  instruction clock,

/// actually the below is almost the same as for the AVR -- so perhaps
/// should go into a separate header?
#define PRESCALER 1 
// just the prescaler is different, due to the
		      // different CPU clock.
#define TICKS_PER_US (F_CPU / 4000000) 
#define SAMPLE_TICKS ((3 * PERIOD_1 * TICKS_PER_US) / (PRESCALER * 4))

//77 non essential error check -- not compulsory, but nice to have.
#if (250 < SAMPLE_TICKS)
#error SAMPLE_TICKS too high (> 250)
#elif (SAMPLE_TICKS < 30)
#error SAMPLE_TICKS too low (<30) -- will be eaten by assembler code.
#endif

/**
 * This function initialises the PIC hardware, especially timer0 and
 * the external int. It must be executed before any main method.
 */  
/*  is there something like sections also in sdcc (ie assembler segments??) -- yes, but sdcc does not makes use of that. */

static inline void init_pic() { //__naked {

#if F_CPU == 8000000
  // select 8Mhz from the internal clock
  IRCF2 = 1;
  IRCF1 = 1;
  IRCF0 = 1;  // 0 is 4 Mhz, 1 is 8 Mhz
#else
#error Please set IRCF flags to match cpu frequency
#endif



  // make RC0 a proper output -- to detect we have a timer interrupt
  // and perhaps we have to do it in a different order: first set the
  // output to zero, unless we want to get spurios pulses!
  // for debugging only! we need it for error!
  // ANS4 = 0;  // we are restting all ANSEL/ANSELH in init_io_hw anyway


  // RA2 doubles as external interrupt input pin. Make sure, it is an input and
  // the pull up switched on.
#ifdef NOT_RABPU
  NOT_RABPU = 0; // port AB pull ups on as INT_PIN is RA2 and PROG_PIN is RA3 -- perhaps this is sinking too much current as all pulll ups on PORTA and PORTB are switched on. -- perhaps better define this in ports?
#endif
  #warning should NOT_RABPU also be runs over ports.h
#ifdef INPUT_WPU
  INPUT_WPU |= _BV(INPUT_PIN);
#endif
  INTEDG = 0; // negative edge to trigger external int
  INTF = 0; // clear any previous interrupt events (should not be after a reset!)
  INTE = 1; // enable external interrupts.

  //  prescaler is 1:1 for timer 0 (which counts the lenght of the DCC
  //  period, so one timer tick is one CPU tick -- error here as well
  //  if F_CPU is not 8MHz
  PSA = 1;
  T0CS = 0; // OPTION reg, timer clock source is cpu clock.
}

/*! the config it has 14 bits 
 * - make RA3 a normal input by disable the MCLR functio
 * - switch on internal oscialator and make all pins normal IO -- s
 */
#warning check whether this is the right stuff for 12f286 and whether it should go to ports.h?
__code uint16_t __at(0x2007) config  = _MCLRE_OFF & _INTOSCIO;
//__code uint16_t __at(0x2007) config  =  _INTOSCIO;


int first(void) { // bit_buffered!

  init_interrupt();
  init_pic(); // for the pic we must make sure manually that this
	      // works, as there are no init sections (or at least
	      // sdcc does not implement a correpsopndign attribute. 
  init_io();
  init_compose_packet();
  init_decoder();

  GIE = 1;

  while(1) { // loop must call tick.
    
    //if(count <= 8) // so we still have four bits available in the bibbuttger
    while(!has_next_bit()) {; // wait until we have at least one bit to process. -- perhaps we never get here with the new code... 
      // perhaps this is not a good idea, as this could lead to starvation of this part of code if the bit_buffer is never empty...
      // on the other hand we must ensure it never overruns even if DCC commands are sent at max speed and each command send is a valid command for this decoder.
	// we perhas could also check whehter count has at least 2 bits or so left...
#warning "find out what the period if the wdt is"
	
	// if there is no new bit, we check whether enough time as elapsed to tick down the outputs
	//      if((state == PREAMBLE) && (TMR1IF)) { // state must be volatile!
	//      if(TMR1IF) { // here change to checking another bit in TMR1H ! -- and see whether it drives up the use of the bitbuffer 
#warning wieso wird der code nie ausgefuert, wenn ich tested, ob wir im Zustand preamble sind?? Wir muessten doch p state muess vielleicht volatile seind?
	//    TMR1IF = 0;
	if((TMR1H & 0x08) /* && count < 16 */) { // if highest bit in TMR1
					   // is set and make sure we
					   // have at least 4 bit
					   // space in the buffer... 
	  TMR1H &= 0x07; // reset that bit (am I resetting the right
			 // bit here?
	  
	  // RESET_ERROR();

	  // where is the watchdog setup?
	  clrwdt(); // but what if timing is such that we always have
		    // a new bit -- then this code is never
		    // executed. Experimental evidence with a max
		    // DCC stream speed shows, we currently also ways
		    // have times when count is zero.re
	  tick(); // instead of an interrupt, we are polling for the , every 32ms, reduce this to 16ms by preloading TMR1!
	}
      }
    //while(!count); // make sure we have at least one bit to process.
  
    compose_packet(next_bit()); 
  }
}

