# comment the following line out for real.
#ccflags-y += -D DUMMY


BUILD=build_new/linux/
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
obj-m += kdcc.o
kdcc-objs += dcc.o gpio.o pwm.o dma.o buffer.o



all:
#	make -C /lib/modules/$(shell uname -r)/$(BUILD) M=$(PWD) modules
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -C ~/$(BUILD) M=$(PWD) modules


clean:
#	make -C /lib/modules/$(shell uname -r)/$(BUILD) M=$(PWD) clean
	make ARCH=arm -C ~/$(BUILD) M=$(PWD) clean

