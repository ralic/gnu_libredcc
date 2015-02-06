#include <linux/kernel.h>
//#include <linux/module.h> // do I need this here?

#include "rt_insert_irq.h"

/*
 credits to Andrew N Sloss, Chapter "Interrupt Handling" of ? for the examples of how interrupts are handled many of which have permeated this code.
 */


#define VECTORS_START_LOW 0x00000000
#define VECTORS_START_HIGH 0xFFFF0000

#define IRQ_VECTOR_OFFSET 0x18

#define BV(bit) (1 << (bit))
#define IRQ_BIT 7
#define FIQ_BIT 6

#define IRQ_RETURN_DISAGIO -4

#define BAL_OPC 0xEA000000

/**
   there must be a ready-made version of this somewhere in the kernel.
 */
/*
inline static void enableIRQ(void) {
  __asm {
    MRS  r1, CPSR
    BIC r1,r1, _BV(IRQ_BIT)
    MSR CPRS_c, r1
    }
}
inline static void enableIRQ() {
  __asm {
    MRS  r1, CPSR
    ORR r1,r1, _BV(IRQ_BIT)
    MSR CPRS_c, r1
    }
}

void rt_handler() __attribute__((naked));
void rt_handler() {

  // entry (we leave lr as is, as we will be chaining another IRS
  __asm {
    STMFD sp_irq!, {r0-r12, r14} // save all expect pc -- which of the register is used as sp (r13?)??
    // and r15 is pc and r14 lr?
  }

#warning check sp, pc and lr are handled correctly for the ABI
  // payload goes here

  // exit
  __asm {
    LDMFD sp_irq!, {r0-r12, r14} // and we are not restoring CPSR from SPRS -- left to next handler in chain.
  }

}

*/

bool is_branch_op_code(void* address) {

  unsigned* irq_vector_high = (unsigned*) VECTORS_START_HIGH + IRQ_VECTOR_OFFSET;
  unsigned* irq_vector_low = (unsigned*) VECTORS_START_LOW + IRQ_VECTOR_OFFSET;
  printk(KERN_INFO "Opcode for IRQ vector high is %u.\n", *irq_vector_high);
  printk(KERN_INFO "Opcode for IRQ vector low is %u.\n", *irq_vector_low);

  return false;

}
