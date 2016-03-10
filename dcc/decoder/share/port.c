#include <share/port.h>
#include <share/switch.h>
#include <avr/io.h>


port_t ports[] = {
  [0] = { activate: pulsed_activate,
	  tick: pulsed_tick,
	  init: pulsed_init,
	  timer: 0,
	  ontime: {5,5},
	  output: {_BV(2), _BV(1)},
  },
  [1] = { activate: pulsed_activate,
	  tick: pulsed_tick,
	  init: pulsed_init,
	  timer: 0,
	  ontime: {10,10},
	  output: {_BV(4), _BV(3)},
  },
  [2] = { activate: permanent_activate,
       tick: permanent_tick,
       init: permanent_init,
       //timer: 0,
       //ontime: {10,10},
       output: {_BV(0), _BV(5)},
  },

};


const uint8_t num_ports = sizeof(ports) / sizeof(ports[0]);


#ifdef NO_LOCAL_STATICS
static uint8_t count = 0;
#endif



/**
 * This function is called repeatedly when the prog button has been pressed (and released)
 * It then alternatingly activates the 2 gates of a port
 * 
 * \param port whose outputs to toggle.
 */
void port_toggle(port_t * const port) {
#ifndef NO_LOCAL_STATICS
  static uint8_t count = 0; 
#endif
  
  if(count == 0) { 
    port->activate(port, 0);
  }
  else if(count == 0x80) { 
    port->activate(port, 1);
  }
  count++;
} 

#ifndef HAS_NO_INIT
void init_ports() __attribute__((naked)) __attribute__((section(".init8"))); 
#endif
void init_ports() { 
  for(uint8_t i = 0; i < num_ports; i++) {
    ports[i].init(ports + i);
  }
}

