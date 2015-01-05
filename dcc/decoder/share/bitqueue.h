#ifndef BITQUEUE_H
#define BITQUEUE_H 1

#include <stdint.h>

typedef uint8_t bit_t;
//! holds the bits mask pointer to the bitqueue.
extern uint8_t bit_pointer;

/**
 @return true if there is at least one bit waiting to be processed in the bitqueue.
 check whether at least one bit is available. Dummy argument to be ignored. \todo can the dummy argument be removed */
#define has_next_bit(dummy) (bit_pointer != 0)

/**
@return the next bit to be processes from the bitqueue.
@requires has_next_bit has been called before with positive outcome.
@post global interruts will be enabled.
@pre to be called from main thread only with global interruts enabled.
*/
bit_t next_bit();

/** queue a bit */
void queue_bit(bit_t bit);

#endif
