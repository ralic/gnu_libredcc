#include "../dcc/include/dcc.h"

/**
   @param packet a dcc packet as defined by the NMRA. 
   @param signal a buffer that contains the encoded dcc signal of the packet)
   @pre signal is a least 32 bytes long.
   @pre signal is zeroed

   A logic one is encoded into the bit sequence 01.
   A logic zero is encoded into the bit sequence 0011.

   A dcc packet has maximally 6 bytes -- if each consists of only zero
   bits, we need 4*8*6 = 192 bits to store the corresponding raw
   signal. Each transmitted byte needs to be preceeded by a start bit
   (zero bit), consuming another 4*6 = 24 bits. Then each dcc packet
   needs to be followed by the preamble for the next one -- This is
   minmally 10 one-bits in op mode and usually we send 14 or 20 in
   programme mode. Hence this is another 10*2 = 20, 28 or 40 bits.
   In total this means we have 236, 244, 256 bits, and hence we need
   the buffer for signal to be at least 29.5, 30.5, 32 bytes long.
   
   @todo But I usually send 3 more preamble 1-bits, hence at least for
   prog mode I could be in a sitaution to need 33 bytes!


*/
int dcc_encode(dcc_packet* ppaket, int_fast8_t* signal) {

#if (MAX_PACKET_LEN*9*4 + 2 * ENCODER_PREAMBLE_MIN_LEN > 256) 
#error Buffer too short -- ensure that dcc_encode only gets buffers passed that can take sufficient bytes.
#endif

#define BITS (sizeof(*signal) * 8) // should be 32 on the ARM and   // 8 in a AVR etc.

  // not reentrant? -- only a problem if any of the below can sleep --
  // and I doubt that.
  dcc_encode_setup(ppacket);

  int bit_p = 32;
  int byte_p = 0;

  while(dcc_encode_has_bit()) {

    if(dcc_encode_next_bit() == 0) {
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
      bit_p+= BITS;
    }
  }    

  if(bit_p < BITS) {
    do {  // fill with 1 bits.
      bit_p-=2;
      signal[word_p] |= 0b01 << bit_p;
    } while (bit_p > 0);
    word_p++;
  }
  return word_ptr;
}
