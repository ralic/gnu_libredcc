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

/**
 * $Id$
 * @todo whether it really can deal with swithcing two outputs at the
 * same time (eg by programming them to the same address). 
 * @todo how to protect outputs that belong to the same port from
 * being activated at the same time_?
 * @todo also how to prevent (thread-safty and interrupt-wise) that
 * outputs are activated for longer than specifies (eg by changing
 * output_timer concurrently?) -- volatile is not neccessary when we
 * just can be sure that methods are executed sequentially. In fact
 * pushing/popping register could be done without if only one
 * interrupt is running at any one time...
 * @todo deactive when activating, and add an onset delay -- so that
 * always only one input is active...
 *
 * \todo Rewrite so that we have a command queue for the outputs -- so that
 * outputs are only switched on sequentially??? (at least if
 * activation times are short! -- or a separete queue for each port --
 * but so that if a switch on is requested twice in a row one command
 * is ignored! (at least optionally!)
 */

#include "io.h"
#include <io_hw.h>

#ifdef NO_LOCAL_STATICS
static uint8_t button = 1;  
static uint8_t count = 0;
#endif

/**
 * A port consists of two consecutively numbered outputs, ie a port is
 * a pair of outputs  
 * \todo this is perhaps not always so...
 */
#define OUTPUTS (2 * PORTS)

/* some ideas to avoid that several outputs are switched on a the same time:
typedef struct {
  uint8_t ticks; // to be kept in RAM
  uint8_t ontime; // ought to be loaded from eeprom...
  uint8_t mask; // ought to be loaded from progmem, well as an
  initialised variable it is...
} port_data;
*/

static volatile uint8_t output_timer[OUTPUTS]; // and what is volatile here?
					// All of it? Check that, and kann it be initialised? either staticaslly or via a timer?
// can sdcc init this one? Yes!

#if PORTS != 2
#error Change the below manualy re number of ports
#endif
static const uint8_t output_ontime[OUTPUTS] = { 5, 5, 5, 5}; // x 16ms. 1sec -- much too? (currently it is 32ms wotj tje pic) {32s seems to work for the LGB motore


/**
 * core parts need to be made thread safe... as activate output will
 * be running on a different thread
 * 
 * This runs on a low priorty interruptable interrupt or on the main
 * thread.
 */
// long in real life
// make that the two outputs of a single port can never be active at
// the same time
static inline void tick_outputs() {
  
  uint8_t i;

  for(i = 0; i < OUTPUTS; i++) {
    if(output_timer[i]) {
      // if activate outputs is called between the test and the
      // decrement, nothing happens as activate_outputs will only set
      // the output timer if it is zero -- at worst we miss one
      // activation because output timer is just about to go zero --
      // but that is intentional. 
      --output_timer[i];
      // on the PIC the above might be implemented atomocially, if
      // DECF INDF is used (which is possible because ALL ram can be
      // accessed thus. -- check whether it really is.
      // on the AVR, as output timer is volatile, it will not be held
      // in a register for all the time, but a ram location cannot be
      // directly decrement, so we have a secuqnece like ldd r0,
      // output_time, dec r0, std output_timer, r0
      // if active outputs is called inbetween the ldd and std
      // operation, it will find output_timer still different from
      // zero, and do nothing. If output timer was 1, and is being
      // decremented to zero in the next time step, we have maximally
      // missed one activieatin, -- also that is intetnional, and no
      // harm done.
      // 
      set_output(i); 
    }
    // if activate_outputs has been called in the mean time, 
    // then nothing is lost, as the correct number of ticks is ticked down.
    else {
      /* \todo This is a workaround for avoind that signal (ie permamnet) outputs are set to zero -- as long as we do have a more elegant global implementation */
      if(output_ontime[i] != 0) {
	reset_output(i);
      }
    }
  }
}

/**
 * This function is called repeatedly (eg from ...).
 * It then alternatingly activates two two output of port
 * 
 * \param port the port whose outputs to toggle.
 */
static inline void toggle_port(const uint8_t port) {
#ifndef NO_LOCAL_STATICS
  static uint8_t count = 0; // needs to be initialised elsewhere for
			    // sdcc, not critical to be initalised
			    // though
#endif
  
  // ca every 2seconds.
  if(count == 0) { 
    activate_output(port << 1);
  }
  else if(count == 0x80) { 
    activate_output((port << 1) + 1);
  }
  count++;
} 

/**
 * this variable indicated two things:
 * - if !=0 we are in programming mode
 * - and its value tells how many time the button has been pressed to
 * cycle through the different ports
 */
volatile uint8_t button_count = 0;

/** 
 * If we are running with F_CPU = 16MHz, and a prescaler 1:1024, then
 * there is a timer0 overflow every 15.625 times in a ms, that is one
 * tick is 0.064ms. If we count a byte up, then ports are updated
 * about every 16ms. 16ms is also enough for debouncing of the button.
 */
/** 
 * the code below is to be called repeatedly.
 * essentially the code below does the followng
 * 1. test whether the button has been released (ok we could do this
 * via an input changed interrupt...
 * 2. if the button has been pressed, we are in programming mode. If
 * we are in progamming mode, we toggle the port, that corresponds to 
 * 3. Finally, we tick down all outputs, both if we are in normal mode
 * or in programmingh mdode
 * 
 * I discuss below that for the normal operation, interrupts can be
 * enabled while this isr is carried out (perhaps somehow preventing
 * reentering this interrupt itself. 
 * 
 * This has been checked for normal mode.
 * - check this also for progmode -- not yet done
 * - check this also applies to reentering this interrupt. (we can
 * practically rule this out, as the execution of this interrupt takes
 * less than 16ms.
 */
inline void tick() {

#ifndef NO_LOCAL_STATICS
  static uint8_t button = 1; 
#endif

  const uint8_t button_new = get_progbutton(); 

  if(button_new && !button) { // ie the button has just been released.
    button_count++;
    if(button_count > PORTS) { // irgendwie ist es nicht schoen, hier
			       // dieselbe Abfrage wie in decoder.c zu
			       // haben...
                               // (und wieso ist die in decoder)
      button_count = 0;
      #warning "It would be enough to increment btton_count here, and then reset it in decoder.c -- but will that be thread safe?"
    }
  }
  button = button_new;

  if(button_count) {
    toggle_port(button_count-1);
  }

  //sei;
  tick_outputs();
}



/**
 * Should be run with interrupts disabled. Why???
 * Albeit typically it will be run on the timer interrupt (AVR) or on
 * the main thread (PIC)
 *  
 * On the PIC, both activate_output and tick_outputs will be run on
 * the main thread, and hence sequencially, and no problems are
 * expected to arrise.
 * 
 * On the AVR, tick_outputs runs on the timer0 interrupts, and
 * activate_output on the timer2 interrupt. So if none of them
 * reenables interrupts globally, that is fine then, they will run
 * sequentially. If then tick_output (on timer0) has its interrupts
 * enabled, nothing bad can happen if it is interrupted by timer2 as
 * discussed under tick_outputs. 
 * If also timer0 has its interrupts enabled, then activate_output can
 * be interrupted by timer2. We disucss here what happens then below
 * in the code.
 *
 * 
 * We are not discussing the case of multiple interrupts, or even
 * reentrant interrupting -- this should not happen, because all
 * interrupt isrs should have terminated by the time they are due
 * again, especially the timer0 isr will be called only every 16ms or
 * so. -- so unlikely to be reentering itself.
 *
 * Timer2 instead is running often and fairly long, so it should take
 * precuations that it enables the other interrupts, but not itself
 * after a cirtical phase. 
 *
 * From the discsiion of both activate_output and tick_outputs, I
 * conclude their interaction is non critical, and both could in
 * principle run with interrupts enabled -- as nothing bad can
 * happen. We will at most miss an activation of the output, but this
 * is intentional. 
 * 
 * This method might also be reentered on two different threads (eg
 * when toggle_outputs is running on timer0 and at the same time it is
 * called from timer0 interrupt, but the nothing bad habppend as
 * maximally output_timer is set to ontime twice, and perhaps ticks
 * then one 16ms tick longer... but also that is just a theoretical
 * possibility. 
 */
void activate_output(const uint8_t output) {

  // an ontime of 0 means we switch on the output permanently
  if(output_ontime[output] == 0) {
    reset_output(output^1); // switch off the paired output
    set_output(output); // switch on this output
  }
  else {
    if(output_timer[output] == 0) { // make sure we are not processing the
				// same command twice
    // we are interrupe here and output_timer is 0 nothing happens.
    // if we are interrupted here and output timer was 1, then we have
    // just intentionally missed an activation.
    //    RC2 = 1;
#warning we are here -- it seems output is activated at correct times, once the programming is done, LED is flashing -- why do I not see anything on the osci?? Now check when the output is really activated! For example in the function that actually sets the output to true
      output_timer[output^1] = 0;
      output_timer[output] = output_ontime[output];
    //    output_timer[output ^ 1] = 0; // never have the two outputs
    //    of one port on at the same time. -- Is this then still
    //    thread safe -- because if we interrupt tick_outputs, we have
    //    a problem that timer might here first be set 0 and then dec
    //    in the tick_outputs -- so this would be thread save if we
    //    where never to be interrupted by timer0 on which
    //    tick_outputs is running. As activate out is called from
    //    either timer0 itself (then no problem=) or timer2 we need to
    //    make sure that timer0 does not allow interruption by timer2
    //    -- but also the otherway round seems problematic - think
    //    about that.
  }
  }
}

/**
 * Must be run exclusively. A problem arises if 
 * - activate_output has checked null, then reset_output runs, and
 * then activate output set timer to a nonnull. But this is trivial.
 * - if ticks has done a nonnull check, then outputs are reset to
 * zero, and then ticks decrements to 0xFF --
 * Hence it must never be run will output_ticks are running -- ie it
 * must be run somewhere it can't be interrupted by the timer0 interrupt
 * 
 * But is this ever run from elsewhere? Where is this called in AVR? Is it? Yes explicitly from somewhere. But it could be made via init as well. Or initialised via a macro or automatically as a static variable.
 */
#ifndef HAS_NO_INIT
void init_ports() __attribute__((naked)) __attribute__((section(".init8"))); 
#endif
void init_ports() { 
  uint8_t i;
  for(i = 0; i < OUTPUTS; i++) {
    output_timer[i] = 0;
  }
}
