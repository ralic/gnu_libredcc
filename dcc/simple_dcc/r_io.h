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
#ifndef R_IO_H
#define R_IO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

char* r_fgets(char* str, const uint8_t size, FILE* const stream);
  //uint8_t r_fputs(const char* str, FILE* const stream);


#warning EOL is currently \r
#define EOLSTR "\r"
#define EOLCHAR '\r'
  //#define EOLSTR "\n"
  //#define EOLCHAR '\n'

#define FPUTL(string, fileptr)			\
  do {						\
    fputs(string EOLSTR, (fileptr));		\
  } while(0)

#ifdef __cplusplus
}
#endif

#endif

