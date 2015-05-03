#include "bitqueue.h"
#include <avr/interrupt.h>

//! volatile as being used between two threads (main and interrupt)
volatile static uint8_t bit_buffer; // no need to initialise
volatile uint8_t bit_pointer = 0;

#undef SIMPLE_BQ
//#define SIMPLE_BQ 2

#if SIMPLE_BQ > 1
#warning SIMPLEST_BQ -- only for debugging.
void queue_bit(bit_t bit) {
  compose_packet(bit);
}
bit_t next_bit() {
return 0;
}
#elif defined SIMPLE_BQ 
#warning SIMPLE_BQ is on -- only for debugging

inline bit_t next_bit() {
  bit_pointer = 0; 
  return bit_buffer;

}

void queue_bit(bit_t bit) {

  bit_pointer = 1; 
  bit_buffer = bit;
}
#else

inline bit_t next_bit() {
  cli(); // is this necessary to disable global interrutpts? (On which architetures?)
  const bit_t bit = bit_pointer & bit_buffer;


  bit_pointer >>= 1; // decrement bit_pointer, ie shift right; If this
		     // is not compile to a single machine code
		     // instruction, we would need to put into the
		     // above interrupt-disabled block. 

    sei(); // yes, it is necessary at least to disable the interrupt that changes bit_pointer and bit_buffer -- to be sure they are consistnet.

  return bit;
}

void queue_bit(bit_t bit) {

  // error check whether bit pointer is full? -- but test with the
  // slower PIC showed that bitqueue use for more than 2 bits is
  // unlikely.  

  bit_pointer <<= 1; // increment pointer

  if(bit_pointer == 0) {
    // buffer overflow or buffer has been read empty.
    // in assembler we would perhaps simply check the carry flag for overflow.
    // according to experient this does not happen.
    bit_pointer++;
  }
  
  // in many assembler language this can be expressed shorted by first
  // setting the carry flag
  bit_buffer <<= 1;
  if(bit) bit_buffer++;
}
  
#endif
