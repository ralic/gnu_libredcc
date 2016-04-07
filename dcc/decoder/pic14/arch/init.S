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
;;; $Id$

;;; We replace the startup code that sdcc would provide with this code. sdcc code is not suitable for out purposes

;;; must not be longer than 4 instrution as interrupt will jump to 0x0004
	extern	__sdcc_gsinit_startup ; sdcc init routine, this jumps internally to _main
	extern  _first

	global _main

STARTUP	code 0x0000
	pagesel __sdcc_gsinit_startup
	goto __sdcc_gsinit_startup
_main	pagesel _first 		; this is where execution of the c programme starts.
	goto _first

;;; global definitions of general registers as in sdcc

	;; global PSAVE
	global SSAVE
	global WSAVE
	;; global STK12
	;; global STK11
	;; global STK10
	;; global STK09
	;; global STK08
	;; global STK07
	;; global STK06
	;; global STK05
	;; global STK04
	global STK03
	global STK02
	global STK01
	global STK00
	
sharebank udata_ovr 0x0070
	
	;; we actually do not need PSAVE
	;; PSAVE	res 1
SSAVE	res 1
WSAVE	res 1
;; STK12	res 1
;; STK11	res 1
;; STK10	res 1
;; STK09	res 1
;; STK08	res 1
;; STK07	res 1
;; STK06	res 1
;; STK05	res 1
;; STK04	res 1
STK03	res 1
STK02	res 1
STK01	res 1
STK00	res 1

	
END
