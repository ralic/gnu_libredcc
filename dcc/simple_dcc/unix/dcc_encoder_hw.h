#ifndef DCC_ENCODER_HW 
#define DCC_ENCODER_HW 1

#include "../../include/dcc.h"


//! PWM bits needed to encode a zero DCC bit
#define CODE0_LEN 4
//! PWM bits needed to encode a one DCC bit
#define CODE1_LEN 2
//! maximal length in PWM bits of DCC packet:
#define CODE_BITS (MAX_PACKET_LEN * (8+1) * CODE0_LEN + ENCODER_LONG_PREAMBLE_LEN * CODE1_LEN)
//! number of words needed to accommodate this number of bits plus one extra word that will consist of only 1s
#define WORDS ((CODE_BITS - 1) / sizeof(unsigned) + 1 + 1)
#define BITS_PER_WORD (8*sizeof(signal[0]))



void encoder_init(void);

#endif
