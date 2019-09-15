// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "DAC.h"

// **************DAC_Init*********************
// Initialize 8-bit DAC, called once 
// Input: none
// Output: none
// initialize the output
// initialize system clock for port b, wait, set DIR, set DEN
void DAC_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x02;	// port B
	volatile int nop;
	nop++; // wait 
	GPIO_PORTB_DIR_R |= 0xFF;	// OR (set) with 0000 0000 ... 1111 1111 (PB0-7 as outputs)
	GPIO_PORTB_DEN_R |= 0xFF;	// 0000 0000 ... 1111 1111 (PB0-7)	
	
}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 64 
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint8_t data){
	GPIO_PORTB_DATA_R = (data);
}


