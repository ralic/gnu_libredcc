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

PRG = s88
OBJ = $(PRG).o  s88_iav.o s88_queue.o s88_hardware.o

LIBS1 = uart
LIBS = -luart

MCU_TARGET = atmega328p
OPTIMIZE = -O2
#OPTIMIZE = -O0

DEFS = -D F_CPU=16000000 -D DEBUG=5
INCLUDE_DIR = ../include
LIB_DIR = ../lib/avr

CXX = avr-g++
CC = avr-gcc
AS = avr-gcc

CFLAGS = -std=gnu11 -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -I $(INCLUDE_DIR) # -fno-jump-tables -
CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics 
#CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics -fexceptions

LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -u uart_init -u lok -L $(LIB_DIR) 
#LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -lstdc++ -u uart_init -u lok -L $(LIB_DIR) 
AFLAGS = -g -Wall -mmcu=$(MCU_TARGET) $(DEFS) -c


OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

all: hex ehex

list: $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@



# compiling is done by an implicit rule.

# assembling:

%.o: %.S
	$(AS) $(AFLAGS) -o $@ $< 

#linking:
$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(LIB_DIR)/lib%.a: 
	$(MAKE) -C $@

# dependency:
#s88.o: uart.h 
#uart.c: uart.h

clean:
	rm -rf *.o $(PRG).elf 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

list: $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images
hex: $(PRG).hex

%.hex: %.elf
#	$(OBJCOPY) -j .text -O ihex $< $@
	$(OBJCOPY) -j .text -j .data -O ihex $< $@


bin: $(PRG).bin

%.bin: %.elf
	$(OBJCOPY) -j .text -O bin $< $@
#	$(OBJCOPY) -j .text -j .data -O ihex $< $@


# Rules for building the .eeprom rom images
ehex: $(PRG)_eeprom.hex

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ || { echo empty $@ not generated; exit 0; }

# Rules for Uploading to the Arduino board:
upload: all
	avrdude -p m328p -c arduino -P /dev/ttyACM0 -Uflash:w:$(PRG).hex

upload2: all
	avrdude -p m328p -c arduino -P /dev/ttyACM1 -Uflash:w:$(PRG).hex
