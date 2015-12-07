/* 
 * Copyright 2014, 2015 André Grüning <libredcc@email.de>
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

/** \file
    Emultates SPROG as far so that it can collaborate with rocrail

    For ROCRAIL we need cmds:

    running:
    - "-": Power Off
    - "+": Power On
    - "O": Send DCC packet given as a sequence of hex bytes.
    
    programming:
    - "C" -- direct mode programming -- write only
    - "V" -- obsolete (but required by NMRA) -- not planned to implement.

    extensions:

    - "M" -- direct mode programming (ie CVs are programmed direct) --
    to be implemented

*/


#define _GNU_SOURCE // needed?

#include "../share/sprog.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "sprog2packet.h"


int fd_dcc;
char* dcc_device_name = DCC_DEVICE_NAME;

int main(int argc, char** argv) {

fprintf(stderr, "No of Args: %i\b", argc);
if(argc >= 2) dcc_device_name = argv[1];

  //   fd_dcc = open(DCC_DEVICE_NAME, O_WRONLY | O_DIRECT | O_SYNC); // these different flags needed?
fd_dcc = open(dcc_device_name, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR); // these different flags needed?
  if(fd_dcc < 0) {
fputs(dcc_device_name, stderr);
perror(": Could not open " );
    exit(errno);
  }

sprog_init();

sprog();

return 0; 


}
