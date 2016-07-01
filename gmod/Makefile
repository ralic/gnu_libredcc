# comment the following line out for real.
#ccflags-y += -D DUMMY

BUILD=linux-e88b69aa792dbe2b289ff528c0d07303068aa0aa
#BUILD=build_new/linux/
#BUILD=linux-headers-4.4.0-1-rpi

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabi-

#AS		= $(CROSS_COMPILE)as
#LD		= $(CROSS_COMPILE)ld
#CC		= $(CROSS_COMPILE)gcc
#CPP		= $(CC) -E
#AR		= $(CROSS_COMPILE)ar
#NM		= $(CROSS_COMPILE)nm
#STRIP		= $(CROSS_COMPILE)strip
#OBJCOPY		= $(CROSS_COMPILE)objcopy
#OBJDUMP		= $(CROSS_COMPILE)objdump
#GENKSYMS	= scripts/genksyms/genksyms
#DEPMOD		= /sbin/depmod

#obj-m += dcc.o pwm-bcm2835.o pwm_init.o
obj-m += kdcc.o test_pwm.o
kdcc-objs += dcc.o gpio.o pwm.o dma.o buffer.o
test_pwm-objs+=t_pwm.o


all:
#	make -C /lib/modules/$(shell uname -r)/$(BUILD) M=$(PWD) modules
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -C ~/$(BUILD) M=$(PWD) modules


clean:
#	make -C /lib/modules/$(shell uname -r)/$(BUILD) M=$(PWD) clean
	make ARCH=arm -C ~/$(BUILD) M=$(PWD) clean

