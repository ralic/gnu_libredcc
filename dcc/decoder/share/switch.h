#ifndef SWITCH_H
#define SWITCH_H

#include <inttypes.h>
#include <share/port.h>

/// initialise a pair of ports
void pair_init(port_t * const this_port);

/// initialise a single port (the one with index 0)
void single_init(port_t * const this_port);

/// do nothing -- a dummy
void do_nothing(port_t * const this_port);



void pulsed_activate(port_t * const this_port, const uint8_t gate);
void pulsed_tick(port_t * const this_port);
#define pulsed_init pair_init

void permanent_activate(port_t * const this_port, const uint8_t gate);
#define permanent_tick do_nothing
#define permanent_init pair_init


void single_permanent_activate(port_t * const this_port, const uint8_t gate);
#define single_permanent_tick do_nothing
#define single_permanent_init single_init

void single_blink_activate(port_t * const this_port, const uint8_t gate);
void single_blink_tick(port_t * const this_port);
#define single_blink_init single_init







#endif
