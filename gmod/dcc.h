#ifndef KDCC_H
#define KDCC_H 1

#define DRIVER_AUTHOR "Andre Gruening"
#define DRIVER_DESC "DCC encoder -- hardware side"

#define DEVICE_MAJOR 0 
#define DEVICE_NAME "pwmdma"

#define CLASS_NAME "dccc" // is this really necessary?


//* DDC specific constants: \todo import from dcc.h in the general tree

// #define MAX_PACKET_LEN 6
// #define ENCODER_LONG_PREAMBLE_LEN 20 /// \todo check value.

/// PWM bits needed to encode a zero DCC bit
// #define CODE0_LEN 4

/// PWM bits needed to encode a one DCC bit
// #define CODE1_LEN 2

/// maximal length in PWM bits of DCC packet:
// #define CODE_BITS (MAX_PACKET_LEN * (8+1) * CODE0_LEN + ENCODER_LONG_PREAMBLE_LEN * CODE1_LEN)

/** number of words needed to accommodate this number of bits 
    plus one extra word that will consist of only 1s */ 
//#define WORDS ((CODE_BITS - 1) / sizeof(unsigned) + 1 + 1)

/// size in bit of the date type used for signal
// #define BITS_PER_WORD (8*sizeof(signal[0]))

#endif
