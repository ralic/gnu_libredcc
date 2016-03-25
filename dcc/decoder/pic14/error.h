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
#ifndef ERROR_H
#define ERROR_H 1

//! $Id$


#include "io_hw.h"
#include "ports.h"

// codes for errors and warnings:
enum {no_error, preamble_too_short, checksum_nonzero, dcc_fall_through};

#ifdef DEBUG 

#define ERROR(code) set_error_indicator(code)
#define RESET_ERRORS(dummy) clear_error_indicators()

#else 

#define ERROR(code) do{} while(0)
#define RESET_ERROR(dummy) do{} while(0)

#endif

#if DEBUG >= 2
#define WARNING(code) set_warning_indicator(code)
#else
#define WARNING(code) do{} while(0)
#endif

#if DEBUG >= 3
#define INFO(str) do{} while(0) // we have no way of sending messages
#else
#define INFO(str) do{} while(0) 
#endif


#endif
