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
 * @todo whether hardware (electrical current limitiation?) really can deal with switching two outputs at the
 * same time (eg by programming them to the same address). 
 * @todo how to protect outputs that belong to the same port from
 * being activated at the same time?
 * @todo deactive when activating for paired outputs,
 * @todo a command queue for the outputs -- outputs are only switched on sequentially??? (at least if
 * activation times are pulsed. 
 */

#include <share/io.h>
#include <avr/chip.h>
#include <avr/io_hw.h>
#include <avr/io.h>


#ifdef NO_LOCAL_STATICS
static uint8_t button = 1;  
static uint8_t count = 0;
#endif

/**
 * A port consists of two consecutively numbered outputs.
 * \todo this is perhaps not always so...
 */
#define OUTPUTS (2 * (PORTS))

/* some ideas to avoid that several outputs are switched on a the same time:
typedef struct {
  uint8_t ticks; // to be kept in RAM
  uint8_t ontime; // ought to be loaded from eeprom...
  uint8_t mask; // ought to be loaded from progmem, well as an
  initialised variable it is...
} port_data;
*/




#if PORTS == 2
static uint8_t output_timer[OUTPUTS] = {0,0,0,0}; 
static const uint8_t output_ontime[OUTPUTS] = { 0, 0, 5, 5}; // x 16ms. 
#elif PORTS == 3
static uint8_t output_timer[OUTPUTS] = {0,0,0,0,0,0}; 
static const uint8_t output_ontime[OUTPUTS] = { 5, 5, 7, 7, 0, 0}; // x 16ms. 
#else 
#error Change the above manualy re number of ports
#endif

#if SDCC_pic14
#warning check whether sdcc can init this one? Yes! -- Check. Because we would not need to init this separately.
#endif

/**
 * counts down and switches on / off the pulsed outputs,
 */
static inline void tick_outputs() {
  
  uint8_t i;

  for(i = 0; i < OUTPUTS; i++) {
    if(output_timer[i]) {
      --output_timer[i];
      set_output(i); 
    }
    else {
      /* \todo This is a workaround for avoind that signal (ie permamnet) outputs are set to zero -- as long as we do have a more elegant global implementation */
      if(output_ontime[i] != 0) {
	reset_output(i);
      }
    }
  }
}

/**
 * This function is called repeatedly when the prog button has been pressed (and released)
 * It then alternatingly activates two outputs of port
 * 
 * \param port whose outputs to toggle.
 */
static inline void toggle_port(const uint8_t port) {
#ifndef NO_LOCAL_STATICS
  static uint8_t count = 0; 
#endif
  
  // ca every 2seconds.
  if(count == 0) { 
    activate_output(port << 1);
  }
  else if(count == 0x80) { 
#warning this will only work for paired outputs
    activate_output((port << 1) + 1);
  }
  count++;
} 

//! indicates whether we are in prog mode and which port is being programmend next.:
volatile uint8_t button_count = 0;

/**
 * The code below is to be called periodically.
 * Essentially the code below does the followng
 * 1. Test whether the programiing button has been released (ok we could do this
 *    via an input changed interrupt in a more elegant world).
 * 2. If the button has been pressed, we are in programming mode. If
 *    we are in progamming mode, we toggle the port, that corresponds to the port being programmed.
 * 3. Finally, we tick down all (timed) outputs, both if we are in normal mode
 *     or in programmingh mdode
 */
inline void tick() {

#ifndef NO_LOCAL_STATICS
  static uint8_t button = 1; 
#endif

  // 1. Poll Progbutton.
  const uint8_t button_new = get_progbutton(); 

  if(button_new && !button) { // button just released.
    INCR(button_count, PORTS);
  }
  button = button_new;

  // 2. If in progmode toggle responsing port
  if(button_count) {
    toggle_port(button_count-1);
  }

  // 3. Tick down timed outputs.
  tick_outputs();
}

/**
 * activates an output. What that exaclty means depends on the ontime value of the output:
   - if ontime is 0, then it is a permanent output and it is switch on indefintely
   - if ontime is non 0, then it is a pulsed output switched on for ontime times 16ms
   - if a pulsed output is activated while the output is on, this subsequent activation is ignored (centrals often send multiple on-commands)
   - if an output is activated its paired output is switched off.
  \todo in the middle run a more OO-style implementation is sought

  @param output to be activated

 */
void activate_output(const uint8_t output) {

  if(output_ontime[output] == 0) {
    reset_output(output^1); // switch off the paired output
    set_output(output); // switch on this output
  }
  else {
    if(output_timer[output] == 0) { 
      output_timer[output^1] = 0;
      output_timer[output] = output_ontime[output];
    }
  }
}

/**
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
