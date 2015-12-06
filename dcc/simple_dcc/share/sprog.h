#ifndef SPROG_H
#define SPROG_H -1

#include "dcc.h"
//#include <stdio.h>

//! listens for Sprog commands on stdin and writes reply to out to stdout. Parses and executes the sprog command.
void sprog();

//! an sprog command is maximally 64 chars long including the terminating \r
#define INPUT_LINE_LEN 64 

//! store maximally 64 chars plus the terminating null char.
extern char line[]; 

//! white space to separate arguments and cmds -- we also include some
//! end-of-line char to be more tolerant than the original SPROG at line format
#define WHITE_SPACE " \t\n\r"

/*! we assume the longest command has no more arguments than bytes in a
 * dcc packet. In other words we assume the "O" command is the longerst
 * @todo sprog say 6 is MAX_ARG, but we should perhaps allow for max(MAX_PACKET_LEN, 6)
 */
#define MAX_ARG MAX_PACKET_LEN 

//! answer sent in response to successful commands.
#define OK "OK"




#endif
