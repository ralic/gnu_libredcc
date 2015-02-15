/** \file
  provide intfrastructure for dealing with brown-out resets, the watchdog, and with determing the reset sources
*/

#include<avr/wdt.h>
#include "reset.h"

// don´t zero while initialising variables
uint8_t MCUSR_copy __attribute__((section(".noinit"))); 

/** save the source of reset and disable watchdog 
    zero_reg and stack are initialised in init2, so we go after that.
 */
void init_reset() __attribute__((naked)) __attribute__((section(".init3"))); 
void init_reset() {
  MCUSR_copy = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

#if 0
void print_reset_source(FILE* f) {
  if(MCUSR_copy & _BV(PORF))
    INFO("Power On Reset\n");
  if(MCUSR_copy & _BV(EXTRF))
    INFO("External Reset\n");
  if(MCUSR_copy & _BV(BORF))
    INFO("Brown Out Reset\n");
  if(MCUSR_copy & _BV(WDRF))
    INFO("Watchdog Timeout Reset\n");
}         
#endif 



