#include <share/switch.h>
#include <share/port.h>
#include <avr/io_hw.h>

/// initialise a pair of ports
static inline void pair_init(port_t * const this_port) {
  make_output(this_port->output[0]);
  make_output(this_port->output[1]);

}

/// initialise a single port (the one with index 0)
static inline void single_init(port_t * const this_port) {
  make_output(this_port->output[0]);
}

/// do nothing -- a dummy
static void do_nothing(port_t * const this_port) {
  // nothing to do
  return;
}

/** Functions for pulsed paired outputs */

static void pulsed_tick(port_t * const this_port) {

  this_port->timer--;

  if(this_port->timer == 0) {
    clear_output(this_port->output[0]);
    clear_output(this_port->output[1]);
    this_port->tick = do_nothing;
  }
}

/**
 * activates an output. What that exaclty means depends on the ontime value of the output:
 - if ontime is 0, then it is a permanent output and it is switch on indefintely
 - if ontime is non 0, then it is a pulsed output switched on for ontime times 16ms
 - if a pulsed output is activated while the output is on, this subsequent activation is ignored (centrals often send multiple on-commands)
 - if an output is activated its paired output is switched off.
 \todo in the middle run a more OO-style implementation is sought

 @param gate must be 0 or 1 -- otherwise undefined behaviour

*/
static void pulsed_activate(port_t * const this_port, const uint8_t gate) {

  if(this_port->tick != do_nothing) return;

  this_port->timer = this_port->ontime[gate];
  set_output(this_port->output[gate]); // switch on this output
  this_port->tick = pulsed_tick;
}

void pulsed_init(port_t * const this_port) {
  pair_init(this_port);
  this_port->tick = do_nothing;
  this_port->activate = pulsed_activate;
  this_port->timer = 0;
}

/** methods for paired ports that are switched on or off alternatingly */

static void permanent_activate(port_t * const this_port, const uint8_t gate) {
  clear_output(this_port->output[1 ^ gate]); // switch off the paired output
  set_output(this_port->output[gate]); // switch on this output
}

void permanent_init(port_t * const this_port) {
  pair_init(this_port);
  this_port->tick = do_nothing;
  this_port->activate = permanent_activate;
}

/** a single output that is switched on and off */

static void single_permanent_activate(port_t * const this_port, const uint8_t gate) {
  if(gate == 0) 
    set_output(this_port->output[0]); // switch on this output
  else 
    clear_output(this_port->output[0]); // switch off the paired output
}

void single_permanent_init(port_t * const this_port) {
  single_init(this_port);
  this_port->tick = do_nothing;
  this_port->activate = single_permanent_activate;
}

/** for a single blinking output */

static void single_blink_on(port_t * const this_port);
static void single_blink_off(port_t * const this_port);

static void single_blink_activate(port_t * const this_port, const uint8_t gate) {

  if(gate == 0) {
    this_port->timer = 1;
    this_port-> tick = single_blink_on;
  }
  else {
    clear_output(this_port->output[0]); 
    this_port->tick = do_nothing;
  }
}

static void single_blink_off(port_t * const this_port) {
  this_port->timer--;
  if(this_port->timer == 0) {
    this_port->timer = this_port->ontime[0];
    this_port->tick = single_blink_on;
    clear_output(this_port->output[0]); // switch on this output
  }
}
  
static void single_blink_on(port_t * const this_port) {
  this_port->timer--;
  if(this_port->timer == 0) {
    this_port->timer = this_port->ontime[0];
    this_port-> tick = single_blink_off;
    set_output(this_port->output[0]); // switch on this output
  }
}

void single_blink_init(port_t * const this_port) {
  single_init(this_port);
  this_port->tick = do_nothing;
  this_port->activate = single_blink_activate;
  
}

