# Copyright 2014 André Grüning <libredcc@email.de>

# This file is part of LibreDCC

# LibreDCC is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# LibreDCC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with LibreDCC.  If not, see <http://www.gnu.org/licenses/>.

LIB = uart
INCLUDE_DIR = ../../include
LIB_DIR = ..

OBJ = uart_init.o uart_tx.o uart_rx.o uart_stdio.o

SYMBOLS = 

MCU_TARGET = atmega328p
#MCU_TARGET = attiny22
OPTIMIZE = -O3
#OPTIMIZE = -O0        -fsee        -fsection-anchors        -mno-long-calls        -mno-longcall

DEFS = -D F_CPU=16000000 -D DEBUG=5 -I ../../include
LIBS =

CC = avr-gcc
AS = avr-gcc
AR = avr-ar

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(SYMBOLS) # -fno-jump-tables -
LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) #-nostdlib $(DEFS) # -fnojum
AFLAGS = -g -Wall -mmcu=$(MCU_TARGET) $(DEFS) $(SYMBOLS) -c
ARFLAGS = rcs


OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

all: lib$(LIB).a 

mv:
	mv lib$(LIB).a $(LIB_DIR)

# compiling is done by an implicit rule.
# assembling:

%.o: %.S
	$(AS) $(AFLAGS) -o $@ $< 

# building a library:

%.a: $(OBJ) 
	$(AR) $(ARFLAGS) $@ $?


#linking -- do we need this?
$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# dependencies (does this work for all OBJ files?
$(OBJ): $(INCLUDE_DIR)/$(LIB).h

clean:
	rm -rf *.o #$(LIB).a
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

purge: clean
	rm lib$(LIB).a

lst: $(LIB).lst

%.lst: %.a # does this work? We will see...
	$(OBJDUMP) -h -S $< > $@