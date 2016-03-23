# Version: $Id: Makefile 1265 2013-05-04 21:45:24Z ag0015 $

# MCU_TARGET = attiny25
# DEVICE = /dev/ttyUSB0
# PROGRAMMER = buspirate
# F_CPU = 8000000
# CHIP = t25

MCU_TARGET = attiny45
DEVICE = /dev/ttyUSB0
PROGRAMMER = buspirate
F_CPU = 8000000
CHIP = t45



# default settings:
ARCH ?= avr
MCU_TARGET ?= atmega328p
PROGRAMMER ?= arduino
F_CPU ?= 16000000
DEVICE ?=/dev/ttyACM0
CHIP ?= m328p


PRG ?= decoder

OBJ = init_avr.o reset.o interrupts.o ../share/compose_packet.o eint0_isr.o ../share/$(PRG).o error_hw.o ../share/bitqueue.o ../share/port.o ../share/switch.o

ifeq ($(PRG), decoder)
OBJ += ../share/io.o io_hw.o eeprom_hw.o
endif	

# \todo this is not strictly necessary -- just a workaround -- remove these from sniffer, and make main so that it also works with sniffer
ifeq ($(PRG), sniffer)
#OBJ += ../share/io.o io_hw.o 
endif	



INCLUDE_DIR = -I . -I .. -I ../../include -I ../../../include
LIB_DIR = -L ../../lib/$(ARCH)/$(MCU_TARGET) -L ../../../lib/$(ARCH)/$(MCU_TARGET)
# could I get those automatically?
#LIBS = -ldcc -luart
SYMBOLS = -D F_CPU=$(F_CPU) #-D DEBUG=5 






OPTIMIZE = -O3
#OPTIMIZE = -O0        -fsee        -fsection-anchors        -mno-long-calls        -mno-longcall


DEFS =

CC = avr-gcc
AS = avr-gcc

CFLAGS = -std=c99 -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(INCLUDE_DIR) $(SYMBOLS) # -fno-jump-tables -
AFLAGS = -g -Wall -mmcu=$(MCU_TARGET) $(DEFS) $(SYMBOLS) -c


LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -ldcc  $(LIB_DIR) #$(LIBS) # -nostdlib $(DEFS) # -fnojum

ifeq ($(MCU_TARGET), atmega328p) 
LDFLAGS += -u uart_init -luart ../../../lib/avr/atmega328p/libuart.a ../../../lib/uart/uart_stdio.o ../../../lib/uart/uart_rx.o ../../../lib/uart/uart_tx.o
endif


OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

#all: hex ehex
all: $(PRG).elf

# compiling is done by an implicit rule.

# assembling:

%.o: %.S
	$(AS) $(AFLAGS) -o $@ $< 

#linking:
#$(LIB_DIR)/libs: 
#	make -C $(LIB_DIR)

$(PRG).elf: $(OBJ) #$(LIB_DIR)/libs
#	make -C $(LIB_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

#.PHONY: $(LIB_DIR)/%.a



#$(LIB_DIR)/lib%.a: 
#	$(MAKE) -C $@

# dependency:
dcc_receiver.o: $(INCLUDE_DIR)/uart.h $(INCLUDE_DIR)/dcc.h


clean:
	rm -rf $(OBJ) $(PRG).elf 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

lst: $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images
hex: $(PRG).hex

#%.hex: %.elf
#	$(OBJCOPY) -j .text -O ihex $< $@
#	$(OBJCOPY) -j .text -j .data -O ihex $< $@


# bin: $(PRG).bin

# %.bin: %.elf
	$(OBJCOPY) -j .text -O bin $< $@
# #	$(OBJCOPY) -j .text -j .data -O ihex $< $@


# Rules for building the .eeprom rom images
# ehex: $(PRG)_eeprom.hex

# %_eeprom.hex: %.elf
#	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ || { echo empty $@ not generated; exit 0; }

# Rules for Uploading to the Arduino board:
upload: all
#	avrdude -p $(CHIP) -c $(PROGRAMMER) -P $(DEVICE) -Ueeprom:w:$(PRG)_eeprom.hex
	avrdude -p $(CHIP) -c $(PROGRAMMER) -P $(DEVICE) -Uflash:w:$(PRG).elf  #-Uflash:w:$(PRG).elf 

fuse: all
	avrdude -p $(CHIP) -c $(PROGRAMMER) -P $(DEVICE) -Ulfuse:w:$(PRG).elf  #-Uflash:w:$(PRG).elf -Ueeprom:w:$(PRG).elf
