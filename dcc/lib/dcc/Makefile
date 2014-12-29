# Version: $Id: Makefile 1265 2013-05-04 21:45:24Z ag0015 $

LIB = dcc
INCLUDE_DIR = ../../include
LIB_DIR = ..

OBJ = xor_checksum.o get_ba_address.o idle_packet.o reset_packet.o

MCU_TARGET = atmega328p
OPTIMIZE = -O3

#DEFS = -D F_CPU=16000000 -D DEBUG=5 
#LIBS =

CC = avr-gcc
AS = avr-gcc
AR = avr-ar

CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -I $(INCLUDE_DIR) # -fno-jump-tables -
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

%.a: $(OBJ) # would be better to add all files individually... # what symbol do I use to $>
	$(AR) $(ARFLAGS) $@ $? 
# $(OBJ) all prerequisites that have changed?	


# dependencies (does this work for all OBJ files?
$(OBJ): $(INCLUDE_DIR)/$(LIB).h
#dcc_receiver.o: uart.h dcc.h

clean:
	rm -rf *.o #$(LIB).a
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

purge: clean
	rm lib$(LIB).a

lst: $(LIB).lst

%.lst: %.a # does this work? We will see...
	$(OBJDUMP) -h -S $< > $@