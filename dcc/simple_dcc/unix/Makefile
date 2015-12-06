# Version: $Id: Makefile 1326 2014-12-27 16:21:01Z ag0015 $

ARCH ?= unix
#MCU_TARGET ?= i386

PRG = sprog2packet
OBJ = $(PRG).o dcc_encoder_hw.o ../share/r_io.o ../share/sprog.o ../share/service_mode.o ../share/dcc_encoder_core.o

#LIBS = -ldcc

OPTIMIZE = -O2

#DEFS =  -D DEBUG=5
INCLUDE_DIRS = -I ../../include -I ../../../include
LIB_DIRS = -L ../../lib/$(ARCH)/$(MCU_TARGET) -L ../../../lib/$(ARCH) #/$(MCU) should also be with uar

#CXX = avr-g++
#CC = avr-gcc
#AS = avr-gcc

CFLAGS = -g -Wall $(OPTIMIZE) $(DEFS) $(INCLUDE_DIRS) # -fno-jump-tables - #-mmcu=$(MCU_TARGET) 
#CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics 
#CXXFLAGS = $(CFLAGS) -fno-threadsafe-statics -fexceptions

LDFLAGS = -g -Wall $(OPTIMIZE)  #-u lok $(LIB_DIRS) -mmcu=$(MCU_TARGET)
#LDFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -lstdc++ -u uart_init -u lok -L $(LIB_DIR) 
# AFLAGS = -g -Wall -mmcu=$(MCU_TARGET) $(DEFS) -c

#OBJCOPY = avr-objcopy
#OBJDUMP = avr-objdump

#all: hex ehex

%.o: %.S
	$(AS) $(AFLAGS) -o $@ $< 

#linking:
$(PRG): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

#$(LIB_DIR)/lib%.a: 
#	$(MAKE) -C $@

# dependency:
#dcc_receiver.o: uart.h 
#uart.c: uart.h

sprog2packet: $(PRG)


clean:
	rm -rf $(OBJ) $(PRG).elf 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

#lst: $(PRG).lst

#%.lst: %.elf
#	$(OBJDUMP) -h -S $< > $@













