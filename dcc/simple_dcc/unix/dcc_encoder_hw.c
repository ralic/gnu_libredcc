#include <dcc.h>
#include "../share/dcc_encoder_core.h"

/** The function below will talk to the dcc module via the device or
    proc fs.
    \todo implement later */
uint8_t is_dcc_on() {
  return -1;
}

void dcc_on() {
  return;
}

void dcc_off() {
  return;
}

/**
   most likely I can take the implmeneration from avr -- it seems
   generic and move both into the "shared" folder.
*/
void service_mode_on() {
  return;
}
void service_mode_off() {
  return;
}

/** in this implementation, this hook is not used hence it should always return true**/
uint8_t is_new_packet_ready() {
  return 1;
}

/** handshaking with next_bit **/

static int has_next_dccbit = 0;

static inline int has_next() {
  return has_next_dccbit;
}

void end_of_preamble_hook() {
  has_next_dccbit = 0;
  return;
}

/** hook not used */
void done_with_packet() {
  return;
}



/**
   A dcc packet maximally consists of preample followed by 6*[start
   bit (0),
   8 data bits] folloy by preamble len 1 bits. 
   For our setup each 0 is encode in 4 bits via dma, and 2 bits per
   1 bit.
   Data has maximal lenght when all data bits are 0 (and this can in
   principle happen). Hence the maxinal length in encode bits is:
   - 6*(8 data bist + 1 stop bits) * 4 = 6*9*4 = 216
   - In my implementation: maximally 23 preamble bits a 2 encoded bits: 46
   -> 216+46 = 262
   hence we need 262/32 = 9 32bit values.
   -> finally we currently want the last words to be only "one" bits. -> We will have at least 16 preamble bits anyway. However in the worst case the beginning two (in case of an overlap) or four bits of 32bit word belong to a zero-code => then we send 30/2 + 32/2 = 31 preamble bits or (28+32)/2 = 30 preamble bits. => we need one extra word
   -> finally we currently want the last words to be only "one" bits. -> We will have at least 16 preamble bits anyway. Hower, so we add 8*sizeof(int).
   -> 10 words to fill

*/

//! PWM bits needed to encode a zero DCC bit
#define CODE0_LEN 4
//! PWM bits needed to encode a one DCC bit
#define CODE1_LEN 2
//! maximal length in PWM bits of DCC packet:
#define CODE_BITS (MAX_PACKET_LEN * (8+1) * CODE0_LEN + ENCODER_LONG_PREAMBLE_LEN * CODE1_LEN)
//! number of words needed to accommodate this number of bits plus one extra word that will consist of only 1s
#define WORDS ((CODE_BITS - 1) / sizeof(unsigned) + 1 + 1)
#define BITS_PER_WORD (8*sizeof(signal[0]))

//static struct  {
static unsigned word_p = 0;
static unsigned signal[WORDS]; // = {0,0,0,0,0,0,0,0,0,0};
static int bit_p = BITS_PER_WORD;
//} dcc_data;

static inline void generate_packet(const uint8_t bit) {

  if(bit == 0) {
    bit_p-=2;
    signal[word_p] |= 0b11 << bit_p;
    bit_p-=2;
  }
  else { // a one bit
    bit_p-=2;
    signal[word_p] |= 0b01 << bit_p;
  }
 
  if(bit_p <= 0) {
    word_p++;
    bit_p+= BITS_PER_WORD;
  }
}    

static inline void finalise_packet() {

  if(bit_p < BITS_PER_WORD) {
    do {  // fill with endocded 1 bits (ie make preamble longer)
      bit_p-=2;
      signal[word_p] |= 0b01 << bit_p;
      // I can shorting this by shifiting 0x55555555 left and fill with zero enough times
    } while (bit_p > 0);
    word_p++;
    bit_p = BITS_PER_WORD;
  }

  // the last word (which will be repeated if DMA queue runs empty:
  if(signal[word_p-1] != 0x55555555) {
    signal[word_p] = 0x55555555;
    word_p++;
  }
}


static inline void send_packet(const unsigned signal[], const unsigned count) {
  
  //  printf("We have %u word\n", count);
  //for(int i = 0; i < count; i++) {
  // for(int bit = 30; bit >=0; bit-=2) {
    
  send_it_to_dma();
      
}





void commit_packet(const dcc_packet* const new_packet) {
#warning implement like avr and than adttional stuff -- ie we need to call next_bit and fill a data structure with bits and the final  

  packet = *new_packet; // it would in this case suffice to have a
			// pointer and not to the full copying.

  word_p = 0;

  // the below shorter with memcopy or memfill?
  int i;
  for(i = 0; i < WORDS; i++) {
    signal[i] = 0;
  }
  has_next_dccbit = 1;

  while(has_next()) {
    generate_packet(next_bit());
  }
  finalise_packet();

  send_packet(signal, word_p);

}


