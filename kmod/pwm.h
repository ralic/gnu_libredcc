// hardware definitions lacking from the kernel tree so far:

#define PWM_CTL 0x0 // offset from PWM_BASE

#define PWEN1 1 << 0  // enable
#define MODE1 1 << 1 // 0 is PWM mode, 1 is serial mode
#define RPTL1 1 << 2 // repeate last word in FIFO mode
#define SBIT1 1 << 3 // state of pwm output with no tranmission
#define POLA1 0 << 4// inversion of polariy?
#define USEF1 1 << 5// use fifo?
#define CLRF1 1 << 6 // clear fifo?
#define MSEN1 1 << 7 // do not use sophisticatted PWM algo

#define PWM_DMAC 0x8

#define ENAB 1 << 31
#define PANIC(__x) ((__x) << 8)
#define DREQ(__x) ((__x) << 0)


#define PWM_RNG1 0x10 // duration of one cycle
#define PWM_DAT1 0x14 // data for the cycle
#define PWM_FIF1 0x18 // data via FIFO



