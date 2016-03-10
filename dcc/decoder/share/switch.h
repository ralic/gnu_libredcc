#ifndef SWITCH_H
#define SWITCH_H

#include <inttypes.h>
#include <share/port.h>

void pulsed_activate(port_t * const this_port, const uint8_t gate);
void pulsed_tick(port_t * const this_port);
void pulsed_init(port_t * const this_port);

void permanent_activate(port_t * const this_port, const uint8_t gate);
void permanent_tick(port_t * const this_port);
void permanent_init(port_t * const this_port);



#endif
