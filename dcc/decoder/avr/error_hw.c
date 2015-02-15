#ifdef DEBUG

#include "error.h"
#include <share/defs.h>

char* error_msg[NUM_ERRORS] = { 
  [no_error] = "OK" EOLSTR,
  [dcc_fall_through] = "DCC fall through -- coding error" EOLSTR,
  [preamble_too_short] = "Preamble too short" EOLSTR,
  [packet_too_long] = "Packet too long" EOLSTR,
  [checksum_nonzero] = "Checksum error" EOLSTR,
  [size_error] = "Size Errors" EOLSTR,
  [lost_bit] = "Lost bit" EOLSTR,
  [no_reset_source] = "No reset source" EOLSTR,
};

#endif
