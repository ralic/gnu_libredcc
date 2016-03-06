#include <dcc.h>
#include "../share/dcc_encoder_core.h"
#include "sprog2packet.h"
#include <unistd.h>
#include "dcc_encoder_hw.h"

// should go to a privade header perhaps:

#define F_SYS_SIGNAL "/sys/devices/virtual/dccc/pwmdma/signal"
#define PWMDMA_NAME "kdcc"
#define PREAMBLE_WORD 0xAAAAAAAA


/** The function below will talk to the dcc module via the device or the
    sysfs.
    \todo implement later */
uint8_t is_dcc_on() {
  return -1;
}

/**
   most likely I can take the implmeneration from avr -- it seems
   generic and move both into the "shared" folder.
*/
void service_mode_on() {
  fputs("Switching Service mode on\n", stderr);
  return;
}
void service_mode_off() {
  fputs("Switching Service mode off\n", stderr);
  return;
}

/** in this implementation, this hook is not used hence it should always return true**/
uint_fast8_t is_new_packet_ready() {
  // fprintf(stderr, __FUNCTION__);
  return 1;
}

/** handshaking with next_bit **/

static int has_next_dccbit = 1;

static inline int has_next() {
  return has_next_dccbit;
}

void end_of_preamble_hook() {
  // fprintf(stderr, __FUNCTION__);
  has_next_dccbit = 0;
  return;
}

/** hook not used */
void done_with_packet() {
  // fprintf(stderr, __FUNCTION__ EOLSTR);
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


//static struct  {
static unsigned word_p = 0;
static unsigned signal[WORDS]; // = {0,0,0,0,0,0,0,0,0,0};
static int bit_p = BITS_PER_WORD;
//} dcc_data;

static inline void generate_packet(const uint_fast8_t bit) {

  if(bit == 0) {
    bit_p-=2;
    signal[word_p] |= 0b11 << bit_p;
    bit_p-=2;
  }
  else { // a one bit
    bit_p-=2;
    signal[word_p] |= 0b10 << bit_p;
  }
 
  // fprintf(stderr, "Word: %8x\n", signal[word_p]);

  if(bit_p <= 0) {
    word_p++;
    bit_p+= BITS_PER_WORD;
    // fprintf(stderr, "bit_p: %0x,\t word_p: %ox\n", bit_p, word_p);
  }
}    

static inline void finalise_packet() {

  if(bit_p < BITS_PER_WORD) {
    do {  // fill with endocded 1 bits (ie make preamble longer)
      bit_p-=2;
      signal[word_p] |= 0b10 << bit_p;
      // I can shorting this by shifiting 0xAAAAAAAA left and fill with zero enough times
    } while (bit_p > 0);
    word_p++;
    bit_p = BITS_PER_WORD;
  }

  // the last word which will be repeated if DMA queue runs empty:
  if(signal[word_p-1] != PREAMBLE_WORD) {
    signal[word_p] = PREAMBLE_WORD;
    word_p++;
  }
}

static inline void send_packet(const unsigned signal[], const unsigned count) {
  
  //  printf("We have %u word\n", count);
  //for(int i = 0; i < count; i++) {
  // for(int bit = 30; bit >=0; bit-=2) {
    
  int written = write(fd_dcc, signal, sizeof(signal[0])*count);
  if(written != count*sizeof(signal[0])) {
    perror(": Not all bytes written.");
  }
  
  return;

}


/**
   \todo not reentrant.
 */
void commit_packet(const dcc_packet* const new_packet) {
#warning implement like avr and than adttional stuff -- ie we need to call next_bit and fill a data structure with bits and the final  


  packet = *new_packet; // it would in this case suffice to have a
			// pointer and not to the full copying.

  /* int j; */
  /* for(j = 0; j < packet.len; j++) { */
  /*   fprintf(stderr, "%0x ", packet.pp.byte[j]); */
  /* } */




  word_p = 0;

  // the below shorter with memcopy or memfill?
  int i;
  for(i = 0; i < WORDS; i++) {
    signal[i] = 0;
  }
  has_next_dccbit = 1;

  while(has_next()) {
    const uint_fast8_t bit = next_bit();
    //fprintf(stderr, "Next bit: %s\n", bit ? "1" : "0");
    generate_packet(bit);
  }
  finalise_packet(); 
  //  word_p++;


  send_packet(signal, word_p);

}

void encoder_init() {
    // advance state of next_bit to bring it in the right state:
  fputs("\nInitialising \n", stderr);
  while(has_next()) {
    // fputs(next_bit() ? "Init: 1\n" : "Init: 0\n", stderr);
  }
}
  

void dcc_on() {

  FILE* f_signal = fopen(F_SYS_SIGNAL, "w");
  if(f_signal == NULL) {
    perror(__FILENAME__ ": Count not open" F_SYS_SIGNAL ". Check that " 
	   PWMDMA_NAME " is loaded and you have write access rights.");
    return;
  }
  fputs("on\n", f_signal);
  fclose(f_signal);

  // send preamble to trigger signal generation.
  const unsigned signal = PREAMBLE_WORD;
  send_packet(&signal, 1);

  fputs("DCC signal generation switched on.\n", stderr);
  return;
}

void dcc_off() {

  FILE* f_signal = fopen(F_SYS_SIGNAL, "r");
  if(f_signal == NULL) {
    perror(": Count not open" F_SYS_SIGNAL ". Check that " PWMDMA_NAME " is loaded and you have write access rights.");
    return;
  }
  fputs("off\n", f_signal);
  fclose(f_signal);
  // nothing else do at this level??
  fputs("DCC switched off.\n", stderr);
  return;
}

