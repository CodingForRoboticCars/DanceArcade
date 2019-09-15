#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "Buttons.h"

void enablebuttons() {
		SYSCTL_RCGCGPIO_R |= 0x20;	// port F
		volatile int nop;
		nop++;
		GPIO_PORTF_LOCK_R = 0x4C4F434B;
		GPIO_PORTF_CR_R |= 0x1F;
		GPIO_PORTF_DIR_R &= 0xF0;	// OR (set) with 1111 1111 ... 1111 0000 (PF0-3 as inputs)
		GPIO_PORTF_DEN_R |= 0x0F;	// 0000 0000 ... 0000 1111 (PF0-3)	
		GPIO_PORTF_PUR_R |= 0x0F;	// 0000 0000 ... 0000 1111 (PF0-3 activated internal pullup resistors)
}
