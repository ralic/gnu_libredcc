#include <share/switch.h>
#include <share/port.h>
#include <avr/io_hw.h>

/**
 * activates an output. What that exaclty means depends on the ontime value of the output:
 - if ontime is 0, then it is a permanent output and it is switch on indefintely
 - if ontime is non 0, then it is a pulsed output switched on for ontime times 16ms
 - if a pulsed output is activated while the output is on, this subsequent activation is ignored (centrals often send multiple on-commands)
 - if an output is activated its paired output is switched off.
 \todo in the middle run a more OO-style implementation is sought

 @param output to be activated

*/
void pulsed_activate(port_t * const this_port, const uint8_t gate) {

  if(this_port->timer == 0) {
    this_port->timer = this_port->ontime[gate];
    clear_output(this_port->output[1 ^ gate]); // switch off the paired output
    set_output(this_port->output[gate]); // switch on this output
  }
}

void pulsed_tick(port_t * const this_port) {

  if(this_port->timer) {
    this_port->timer--;
  }
  else {
    clear_output(this_port->output[0]);
    clear_output(this_port->output[1]);
  }
}

void pulsed_init(port_t * const this_port) {

  make_output(this_port->output[0]);
  make_output(this_port->output[1]);

}



void permanent_activate(port_t * const this_port, const uint8_t gate) {
    clear_output(this_port->output[1 ^ gate]); // switch off the paired output
    set_output(this_port->output[gate]); // switch on this output
}

void permanent_tick(port_t * const this_port) {
  // nothing to do
}

void permanent_init(port_t * const this_port) {
  pulsed_init(this_port);
}

