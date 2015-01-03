#ifdef DEBUG

#include "error.h"

char* error_msg[NUM_ERRORS] = { 
  [no_error] = "OK",
  [dcc_fall_through] = "DCC fall through -- coding error",
  [preamble_too_short] = "Preamble too short",
  [packet_too_long] = "Packet too long",
  [checksum_nonzero] = "Checksum error",
  [size_error] = "Size Errors",
  [lost_bit] = "Lost bit"
};

#endif
