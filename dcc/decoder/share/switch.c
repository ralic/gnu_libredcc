#include <share/switch.h>
#include <share/port.h>
#include <arch/io_hw.h>

/// initialise a pair of ports
static inline void pair_init(const uint8_t this) {
  make_output(ports[this].output[0]);
  make_output(ports[this].output[1]);
}

/// initialise a single port (the one with index 0)
static inline void single_init(const uint8_t this) {
  make_output(ports[this].output[0]);
}

/// do nothing -- a dummy
static void do_nothing(const uint8_t port) {
  // nothing to do
  return;
}

/** Functions for pulsed paired outputs */

static void pulsed_tick(const uint8_t this) {

  ports[this].timer--;

  if(ports[this].timer == 0) {
    clear_output(ports[this].output[0]);
    clear_output(ports[this].output[1]);
    ports[this].tick = do_nothing;
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
static void pulsed_activate(const uint8_t this, const uint8_t gate) {

  if(ports[this].tick != do_nothing) return;

  ports[this].timer = ports[this].ontime[gate];
  set_output(ports[this].output[gate]); // switch on this output
  ports[this].tick = pulsed_tick;
}

void pulsed_init(const uint8_t this) {
  pair_init(this);
  ports[this].tick = do_nothing;
  ports[this].activate = pulsed_activate;
  ports[this].timer = 0;
}

/** methods for paired ports that are switched on or off alternatingly */

static void permanent_activate(const uint8_t this, const uint8_t gate) {
  clear_output(ports[this].output[1 ^ gate]); // switch off the paired output
  set_output(ports[this].output[gate]); // switch on this output
}

void permanent_init(const uint8_t this) {
  pair_init(this);
  ports[this].tick = do_nothing;
  ports[this].activate = permanent_activate;
}

/** a single output that is switched on and off */

static void single_permanent_activate(const uint8_t this, const uint8_t gate) {
  if(gate == 0) 
    set_output(ports[this].output[0]); // switch on this output
  else 
    clear_output(ports[this].output[0]); // switch off the paired output
}

void single_permanent_init(const uint8_t this) {
  single_init(this);
  ports[this].tick = do_nothing;
  ports[this].activate = single_permanent_activate;
}

/** for a single blinking output */

static void single_blink_on(const uint8_t this);
static void single_blink_off(const uint8_t this);

static void single_blink_activate(const uint8_t this, const uint8_t gate) {

  if(gate == 0) {
    ports[this].timer = 1;
    ports[this]. tick = single_blink_on;
  }
  else {
    clear_output(ports[this].output[0]); 
    ports[this].tick = do_nothing;
  }
}

static void single_blink_off(const uint8_t this) {
  ports[this].timer--;
  if(ports[this].timer == 0) {
    ports[this].timer = ports[this].ontime[0];
    ports[this].tick = single_blink_on;
    clear_output(ports[this].output[0]); // switch on this output
  }
}
  
static void single_blink_on(const uint8_t this) {
  ports[this].timer--;
  if(ports[this].timer == 0) {
    ports[this].timer = ports[this].ontime[0];
    ports[this]. tick = single_blink_off;
    set_output(ports[this].output[0]); // switch on this output
  }
}

void single_blink_init(const uint8_t this) {
  single_init(this);
  ports[this].tick = do_nothing;
  ports[this].activate = single_blink_activate;
  
}

