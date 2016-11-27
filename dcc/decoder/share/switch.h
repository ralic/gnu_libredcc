#ifndef SWITCH_H
#define SWITCH_H

//#include <inttypes.h>
#include<stdint.h>
#include <share/port.h>

void pulsed_init(const uint8_t this);
void permanent_init(const uint8_t this);

void single_permanent_init(const uint8_t this);
void single_blink_init(const uint8_t this);

#endif
