/* 
 * Copyright 2014 André Grüning <libredcc@email.de>
 *
 * This file is part of LibreDCC
 *
 * LibreDCC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LibreDCC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LibreDCC.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SIMPLE_DCC_H
#define SIMPLE_DCC_H 1

#include "dcc.h"


//! an sprog command is maximally 64 chars long including the terminating \r
#define INPUT_LINE_LEN 64 

//! white space to separate arguments and cmds -- we also include some
//! end-of-line char to be more tolerant than the original SPROG at line format
#define WHITE_SPACE " \t\n\r"

/*! we assume the longest command has no more arguments than bytes in a
 * dcc packet. In other words we assume the "O" command is the longerst
 * @todo sprog say 6 is MAX_ARG, but we should perhaps allow for max(MAX_PACKET_LEN, 6)
 */
#define MAX_ARG MAX_PACKET_LEN 

//! maximum number of arguments a command can have.
// #define MAX_ARG 5 

//! answer sent in response to successful commands.
#define OK "OK"

#endif 
