#include <share/port.h>
#include <share/switch.h>
#include <avr/io.h>

//#define     TWOSWITCHES_TWOSIGNALS
#define FOUR_BLINK
#if defined TWOSWITCHES_TWOSIGNALS
port_t ports[] = {
  [0] = { init: pulsed_init,
	  ontime: {5,5},
	  output: {_BV(2), _BV(1)},
  },
  [1] = { init: pulsed_init,
	  ontime: {10,10},
	  output: {_BV(4), _BV(3)},
  },
  [2] = { init: single_permanent_init,
	  output: {_BV(0)},
  }, 
  [3] = { init: single_permanent_init,
	  output: {_BV(5)},
  }, 
};
#elif defined FOUR_BLINK
port_t ports[] = {
  [0] = { init: single_permanent_init,
	  ontime: {10},
	  output: {_BV(0)},
  },
  [1] = { init: single_permanent_init,
	  ontime: {60},
	  output: {_BV(1)},
  },
  [2] = { init: single_permanent_init,
          ontime: {30},
	  output: {_BV(3)},
  }, 
  [3] = { init: single_permanent_init,
	  ontime: {50},
	  output: {_BV(4)},
  }, 
};
#else 
#error No Port configuration defined!
#endif

// const uint8_t NUM_PORTS = sizeof(ports) / sizeof(ports[0]);

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
  for(uint8_t i = 0; i < NUM_PORTS; i++) {
    ports[i].init(ports + i);
  }
}

