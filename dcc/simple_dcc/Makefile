# Version: $Id: Makefile 1326 2014-12-27 16:21:01Z ag0015 $

PRG = simple_dcc
OBJ = $(PRG).o dcc_encoder.o r_io.o service_mode.o

LIBS1 = dcc uart
LIBS = -ldcc -luart

MCU_TARGET = atmega328p
OPTIMIZE = -O2

DEFS = -D F_CPU=16000000 -D DEBUG=5
INCLUDE_DIRS = -I ../include -I ../../include
LIB_DIRS = -L ../lib/avr -L ../../lib/avr

CXX = avr-g++
CC = avr-gcc
AS = avr-gcc

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(INCLUDE_DIRS) # -fno-jump-tables -
CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics 
#CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics -fexceptions

LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -u uart_init -u lok $(LIB_DIRS) 
#LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -lstdc++ -u uart_init -u lok -L $(LIB_DIR) 
AFLAGS = -g -Wall -mmcu=$(MCU_TARGET) $(DEFS) -c

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

all: hex ehex

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
dcc_receiver.o: uart.h 
uart.c: uart.h

clean:
	rm -rf *.o $(PRG).elf 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

lst: $(PRG).lst

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