/* 
 * Copyright 2014-2016 André Grüning <libredcc@email.de>
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
 * along with LibreDCC. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CHIP_H
#define CHIP_H 1

/*! File meant to work for both assembler and C -- so we need to geth port definitions right! */

// the first defined is for assembler files (and might collide with the internal definition of __DEVICE in gpasm because we use here the same symboll for the preprocessor

#ifdef __16f690 // PORTC_ADDR // for 16f690 PORTC is used as main output

#define PROG_PIN 3 // rename BUTTON_PIN?
#define PROG_PORT PORTA
#undef PROG_WPU

#define INPUT_PIN 2 // rename DCC_PIN?
#define INPUT_PORT PORTA
#undef INPUT_WPU


#define ERROR_PORT PORTC
#define ERROR_TRIS TRISC
#define ERROR_PIN 3
#define WARNING_PIN 0

#define OUT_PORT PORTC
#define OUT_TRIS TRISC
#define OUT_0 1
#define OUT_1 2

#elif defined __12f683 // GPIO_ADDR // for 12f683

#define PROG_PIN 3
#define PROG_PORT GPIO
#undef PROG_WPU

#define INPUT_PIN 2
#define INPUT_PORT GPIO
#define INPUT_WPU WPU

#define ERROR_PORT GPIO
#define ERROR_TRIS TRISIO
#define ERROR_PIN 5
#define WARNING_PIN 4

#define OUT_PORT GPIO
#define OUT_TRIS TRISIO
#define OUT_0 0
#define OUT_1 1

#else 
#error "Processor not defined, hence outputs cannot be defined"
#endif

#endif
