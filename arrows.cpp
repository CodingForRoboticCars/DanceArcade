#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
//#include "DDR_Images.h"
#include "ST7735.h"
#include "stdlib.h"
#include "random.h"

extern "C" void EnableInterrupts(void);
extern "C" void TIMER3A_Handler(void);
extern "C" void TIMER2A_Handler(void);

extern unsigned char buffer_flag[1];
extern unsigned char buffer2_flag[1];

/*
A, B, C, D, F are sprites that are not needed for the current game
*/
//extern const unsigned short A[];
//extern const unsigned short B[];
//extern const unsigned short C[];
//extern const unsigned short D[];
//extern const unsigned short F[];
extern const unsigned short STAGE[];
extern const unsigned short COMPLETE[];
extern const unsigned short left_arrow_filler[];
extern const unsigned short up_arrow_filler[];
extern const unsigned short down_arrow_filler[];
extern const unsigned short right_arrow_filler[];
extern const unsigned short star[];
extern unsigned char final_val;

static uint8_t size;
enum type {left, right, up, down};
struct arrow {enum type; int position;};

void Timer2_Init(uint32_t period) {
	SYSCTL_RCGCTIMER_R |= 0x0004;		// activate timer2
	volatile int nop;
	nop++;
	TIMER2_CTL_R = 0x00000000;
	TIMER2_CFG_R = 0x00000000;
	TIMER2_TAMR_R = 0x00000002;
	TIMER2_TAILR_R = (period);	// (period * 80) - 1
	TIMER2_TAPR_R = 0;
	TIMER2_ICR_R = 0x00000001;
	TIMER2_IMR_R |= 0x00000001;
	NVIC_PRI5_R = (NVIC_PRI5_R & 0x00FFFFFF) | 0x30000000;	 // priority 3 (was 8)	// was 3
	NVIC_EN0_R = 1<<23;
	TIMER2_CTL_R |= 0x00000001;
	EnableInterrupts();
	//arrow *array = (arrow*) calloc(30, sizeof(arrow));
	//size = 0;
}
#include "ff.h"
extern uint32_t value;
extern int index_copy;
extern FRESULT Fresult;
extern FIL Handle,Handle2;
extern "C++" unsigned char buffer[4000];
extern "C++" unsigned char buffer2[4000];
extern UINT successfulreads;

uint8_t height_stack[8];

void Timer3_Init(uint32_t period) {
	SYSCTL_RCGCTIMER_R |= 0x0008;		// activate timer3
	volatile int nop;
	nop++;
	TIMER3_CTL_R = 0x00000000;
	TIMER3_CFG_R = 0x00000000;
	TIMER3_TAMR_R = 0x00000002;
	TIMER3_TAILR_R = (period);	// (period * 80) - 1
	TIMER3_TAPR_R = 0;
	TIMER3_ICR_R = 0x00000001;
	TIMER3_IMR_R |= 0x00000001;
	NVIC_PRI8_R = (NVIC_PRI8_R & 0x00FFFFFF) | 0x40000000;	 // priority 3 (was 8)	// was 7
	NVIC_EN1_R = 1<<(35-32);
	TIMER3_CTL_R |= 0x00000001;
	EnableInterrupts();
	arrow *array = (arrow*) calloc(30, sizeof(arrow));
	size = 0;
	
}

void increase_size() {
	size++;
}

void decrease_size() {
	size--;
}

uint8_t return_size() {
	return size;
}

//const unsigned short* up_arrows[24] = {up0, up1, up2, up3, up4, up5, up6, up7, up8, up9, up10, up11,
//up12, up13, up14, up15, up16, up17, up18, up19, up20, up21, up22, up23};
//static uint8_t up_arrow_count;
static uint8_t count_left;
static uint8_t count_up;
static uint8_t count_down;
static uint8_t count_right;
static uint8_t timer;
static uint8_t timer_flag;
static uint8_t invert_flag;
static uint8_t invert_first_time;
static uint8_t invert_timer;
static uint8_t flag = 0;
static int16_t i = 159;
static uint8_t x;
// =======
static uint16_t check;
static uint16_t intro_check;
static uint8_t mode;
static uint8_t grade = 0;
static uint8_t check_flag = 0;
static uint8_t intro_check_flag = 0;
//const unsigned short *pointer[] = {A, B, C, D, F};
const unsigned short *intro_pointer[] = {STAGE, COMPLETE};
static uint8_t times = 0;
static uint8_t exit_flag = 0;
static uint8_t intro_finish_flag = 0;
static uint16_t intro_wait = 0;
static uint16_t color;
static uint16_t green;
static uint16_t red;
static uint16_t blue;
static uint8_t color_flag;
static uint8_t first_time_color_flag;
static uint16_t previous_variable;
static uint8_t m;

extern uint8_t checking_flag;
extern uint8_t filling_buffer_flag;

/*
TIMER2A_Handler is commented out because the functionality of reading and writing to the buffers (they store the music array values)
is handled already by code that runs in the main. You can use this handler if you like, but make sure to comment out or remove
the code in main that handles the buffers.
*/
void TIMER2A_Handler(void) {
	checking_flag = 1;
	/*
		if(buffer_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer, 4000, &successfulreads);
						buffer_flag[0] = 0;
				}
				if(buffer2_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer2, 4000, &successfulreads);
						buffer2_flag[0] = 0;
				}
	*/
}
