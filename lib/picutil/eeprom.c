/* 
 *  Copyright 2014 André Grüning <libredcc@email.de>
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
/* 
 *  Copyright 2014 André Grüning <libredcc@email.de>
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
#include "pic/picutil.h"
#include <pic14regs.h>

/**
- should i not include the picutil header here?
- should i disable interrupts during this function?
- it is not clear to me whether a read to EEPROM can be started while WR is still on -- so do we have to wait for WR to be come clear when we want to read.
- finally why should interrupts be disable during this sequence? if it is clear that interrupts will not write the EEPROM?
- manual 12f683 mentions something about the cycle count, but that for 16f88x does not.
- we could also return the WRERR bit
@sideeffects: clears the watchdog timer.

*/
void eeprom_write_byte(const uint8_t adr, const uint8_t value) {

  clrwdt();

#ifdef EEPGD // only defined if specific PIC can also programme the progmem
  EEPGD = 0;
#endif
  EEADR = adr; 
  EEDAT = value;
  WREN = 1; 
  EECON2 = 0x55; 
  EECON2 = 0xAA; 
  WR = 1; 

  WREN = 0;
  while(WR); // wait until eeprom write is done to ensure we can read straight away
  //  do {
  //__asm { sleep } __endasm; // is it a good idea to sleep here? Or must interrupts be on. 
  // yes the corresponding EE interupt flag would have to be on.
    // clrwdt();
  //} while(WR);

}

#if 0
#error "Never used, never finished"
/** copies len bytes from source (in RAM) to dest (in EEPROM), but only overwrite individual bytes, if their contents is changed -- to save unnecessary EEPROM write cycles and write wear

    \param len number of bytes to copy
    \param dest destination address in EEPROM to copy to
    \param source source address to copy from in RAM
*/
void eeprom_update(const uint8 dest, void * const source, uint8_t len) {
#error "Check whether corrrectly implemented"
  WREN = 1;
  EEADR = dest;
  while(len) {
    // is it necessary to complete a write cycle before the next read cycle?
    while(WR);
    RD = 1;
    // is the new value the same as the old?
    if(EEDAT != *source) {
      // no, then write eeprom cell
      EEDAT = *source;
      // while(WR); wait here is it is not necessary to wait before a read cycle.
      EECON2 = 0x55;
      EECON2 = 0xaa;
      WR = 1;
    }
    source++;
    EEADR++;
    len--;
  };
  WREN = 0;
}
#endif
  

    

