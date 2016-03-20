#ifndef PORT_H
#define PORT_H 1

#include <stdint.h>

typedef struct _port port_t;
typedef void activate_t(port_t* const this_port, const uint8_t gate);
typedef void tick_t(port_t* const this_port);
typedef void init_t(port_t* const this_port);
typedef uint8_t output_t; /// in this case, a bitmask to indentify the pins to set/clear

/// structure to hold a port
struct _port {
  uint16_t id; /// its id (a proxy of its BA address
  activate_t* activate; /// the function to call to activate this port
  tick_t* tick; /// the function to tick this port
  init_t* init; /// function to init this port
  uint8_t timer; /// timer ticks
  uint8_t ontime[2]; /// ontime in number of ticks for each gate;
  output_t output[2]; /// information to set the output
};

/// the ports
extern port_t ports[];

/// number of ports this decoder controls
extern const uint8_t num_ports;

/// toggle a port if ticked
void port_toggle(port_t * const port);



#ifdef HAS_NO_INIT
inline void init_ports(); 
#endif

#endif
