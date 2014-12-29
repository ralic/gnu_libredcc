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
#include<stdio.h>
#include "r_io.h"


/** 
 * Should read from a stream until a \r in encountered (unlike standard
 * fgets which reads until a \n is encountered.
 * \todo check whether SPROG strings can contain \r or \n as a value, and not as a terminating character.
 * \todo does not quite conform to fgets as it does not return NULL when an
 * error has happened. And it will be wrong, when a buffer of size 0
 * is given.
 * \todo extended h
 */
char* r_fgets(char* const str, const uint8_t size, FILE* const stream) {
  

  uint8_t idx = 0;
  int16_t ch; // so that we can distinguish between char 0xFF and EOF (-1),

#warning r_fgets accepts also \n as EOL, not only \r -- might fail with IAV_88!

  //  /*
  while( (idx < size-1) && ((ch = fgetc(stream)) != EOF) && (ch != '\r') && (ch != '\n')) {
    str[idx] = ch;
    idx++;
  }
  str[idx] = '\0';
  return str;
  // */
  //  return fgets(str, size, stream);
}

