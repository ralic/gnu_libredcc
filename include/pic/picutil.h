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
#ifndef PICLIB
#define PICLIB 1

#include <stdint.h>
#include <pic16regs.h>

/** 
 * assumptions:
 * - EEPGD = 0 (as per default) -- this is a wrong assumption?
 * - WREN = 0 and what more...?
 * - WR = 0
 *  
 * Does the code below work as expected regarding the return value?
 */
//uint8_t eeprom_read_byte(const uint8_t adr);

#define eeprom_read_byte(_adr) ( EEADR = (uint8_t) (_adr), RD = 1, EEDAT )

/* assumes:
 * EEPGD = 0
 * GIE = 0!!!!
 */
void eeprom_write_byte(const uint8_t adr, const uint8_t value);

#define eeprom_read_word(__adr) ( eeprom_read_byte(__adr) + (eeprom_read_byte((__adr)+1) << 8))

#define eeprom_update_byte(__adr, __val ) do { \
    if(eeprom_read_byte((__adr)) != (uint8_t) (__val)) {	\
      eeprom_write_byte( (uint8_t) (__adr), (__val));		\
    }} while(0)

#define eeprom_update_word(__adr, __val) do {		\
    eeprom_update_byte((__adr), (__val));		\
    eeprom_update_byte((__adr) + 1, ((__val) >> 8));	\
  } while(0)
  

// reset the watchdog timer
#define clrwdt() __asm clrwdt __endasm

//! analogue to avr-gcc, set a bit
#define _BV(_bit) (1 << (_bit))


#endif
