#ifndef SWITCH_H
#define SWITCH_H

#include <inttypes.h>
#include <share/port.h>

void pulsed_init(port_t * const this_port);
void permanent_init(port_t * const this_port);

void single_permanent_init(port_t * const this_port);
void single_blink_init(port_t * const this_port);

#endif
