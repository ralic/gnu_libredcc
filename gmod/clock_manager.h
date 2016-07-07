// offset for peripherial clocks:
#define CM_PWM 0xA0

// offset within each clock:
#define CM_CTL 0x0 // control register
#define CM_DIV 0x4 // divisor register

// values to write into CM_CTL:
#define CLK_PASSWD (0x5A << 24)
#define CLK_MASH(__stage) ((__stage & 0x3) << 9) // stage can be 0,1,2,3 -- see manual
#define CLK_BUSY (1 << 7)
#define CLK_ENAB (1 << 4)
#define CLK_SRC(__source) (__source & 0xF) // source can be one of the following below:

#define SRC_GND 0
#define SRC_XTAL 1 // 19.2 MHz
#define SRC_PLLA 4
#define SRC_PLLC 5
#define SRC_PLLD 6
#define SRC_HDMIAUX 7

// values to write into CM_DIV:
#define CLK_DIVI(__divi) ((__divi & ((1 << 12) -1)) << 12)
#define CLK_DIVF(__divf) ((__divf & ((1 << 12) -1)))

// \todo get the F_XTAL frequency from elsewhere in the raspi sources
#define F_XTAL 19200000 // 19.2 MHz

#define DCC_DIVI(_period) ((F_XTAL * (_period)) / 1000000)


int init_clockmanager (void);
void set_clock(void);
