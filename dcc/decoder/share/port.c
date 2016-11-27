#include <share/port.h>
#include <share/switch.h>

#ifdef __AVR
#include <avr/io.h>

//#define     TWOSWITCHES_TWOSIGNALS
//#define FOUR_BLINK
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
// image of stop RETB stop board with TPWS warning light http://www.rmweb.co.uk/forum/download/file.php?id=65705 with this font http://www.roadsuk.com/downloads/fonts.html
port_t ports[] = {
  [0] = { init: single_blink_init,
	  ontime: {10},
	  output: {_BV(0)},
  },
  [1] = { init: single_blink_init,
	  ontime: {60},
	  output: {_BV(1)},
  },
  [2] = { init: single_blink_init,
          ontime: {30},
	  output: {_BV(3)},
  }, 
  [3] = { init: single_blink_init,
	  ontime: {50},
	  output: {_BV(4)},
  }, 
};
#else 
#error No Port configuration defined!
#endif

#elif defined SDCC_pic14
#include <pic14regs.h>
#define _BV(__bit) (1 << (__bit))

port_t ports[] = {
  [0] = { .init = single_blink_init,
	  .ontime = {10},
	  .output = {_BV(0)},
  },
  [1] = { .init = single_blink_init,
	  .ontime = {60},
	  .output = {_BV(1)},
  },
  [2] = { .init = single_blink_init,
          .ontime = {30},
	  .output = {_BV(3)},
  }, 
  [3] = { .init = single_blink_init,
	  .ontime = {50},
	  .output = {_BV(4)},
  }, 
};

#else
#error Unknown architecture
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
void port_toggle(const uint8_t this) {
#ifndef NO_LOCAL_STATICS
  static uint8_t count = 0; 
#endif
  
  if(count == 0) { 
    ports[this].activate(this, 0);
  }
  else if(count == 0x80) { 
    ports[this].activate(this, 1);
  }
  count++;
} 

#ifndef HAS_NO_INIT
void init_port() __attribute__((naked)) __attribute__((section(".init8"))); 
#endif
void init_port() { 
  uint8_t i;
  for(i = 0; i < NUM_PORTS; i++) {
    ports[i].init(i);
  }
}

