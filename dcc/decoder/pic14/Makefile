# Version: $Id: Makefile 1067 2012-05-09 11:50:15Z ag0015 $

ARCH ?= pic14
#CHIP = 12f683
CHIP ?= 16f690
PRG ?= decoder_$(CHIP)

SHARE = ../share

OBJ = $(SHARE)/compose_packet.o $(SHARE)/decoder.o $(SHARE)/io.o arch/io_hw.o arch/init.o arch/init_$(ARCH).o arch/interrupt.o arch/eeprom_hw.o $(SHARE)/switch.o $(SHARE)/port.o #pic$(CHIP)/io_hw.o 

INCLUDE_DIR = -I . -I .. -I ../../include -I ../../../include
LIB_DIRS = -L ../../lib/$(ARCH)/$(CHIP) -L ../../../lib/$(ARCH)/$(CHIP)

LIBS = -llibdcc.a -llibpicutil.a # -luart

SYMBOLS = \
        -D __$(CHIP) \
	-D F_CPU=8000000 \
	-D NO_LOCAL_STATICS=1 \
	-D NO_STRUCT_INIT=1 \
	-D HAS_NO_INIT=1 \
	-D inline=  \
	-D ARCH=$(ARCH)
# sdcc has a problem with the inline keyword	
# sdcc has a problem to init local statics
# sdcc has a problem to init structs

OPTIMIZE = -O3
DEFS =

CFLAGS = --use-non-free  -V -c -m$(ARCH) -p$(CHIP)  -opt-ext=o --std-sdcc99  $(INCLUDE_DIR)  $(SYMBOLS) 
LDFLAGS = --use-non-free -V -opt-ext=o -m$(ARCH) -Wl-m -Wl-smy_$(CHIP).lkr -p$(CHIP) $(LIB_DIRS) 
CC = sdcc
LD = sdcc 
AS = gpasm 
AFLAGS = -c -pp$(CHIP) 

all: hex 

# compiling is done by an implicit rule.

# assembling:

%.o: %.S 	# no such call -- default rule deleted.

.INTERMEDIATE: %.SS

%.SS: %.S 
	sdcpp -P -D__$(CHIP) -DPROCESSOR_INC="p$(CHIP).inc" -o$@ $<

%.o: %.SS # was the rule to make o from s
	$(AS) $(AFLAGS) -o $@ $< 

$(PRG).hex: $(OBJ) #$(LIB_DIR)/libs
#	make -C $(LIB_DIR)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(LIB_DIR)/lib%.a: 
	$(MAKE) -C $@

clean:
	rm -rf $(OBJ) $(PRG).hex $(PRG).cod $(OBJ).asm
	rm -rf $(OBJ).lst $(OBJ).map $(EXTRA_CLEAN_FILES)

lst: $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images
hex: $(PRG).hex

# Rules for Uploading via PIC KIT 2:
upload: all
	pk2cmd -B/usr/share/pk2/ -PPIC$(CHIP) -I -M -F $(PRG).hex -R -T

depend: 
	sdcc -M -MFdepend *.c #$(OBJ:.o=.c) if we had not assembler files
