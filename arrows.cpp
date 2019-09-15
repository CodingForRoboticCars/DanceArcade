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

/*
void TIMER3A_Handler(void) {
	TIMER3_ICR_R = 0x00000001;	// acknowledge timer3 timeout
	//mode = 0;
	invert_flag = 1;
	//mode=58;
	if(mode==58) {
		
		//ST7735_DrawFastVLine(30,30,previous_variable,0);
		ST7735_DrawPixel(30,previous_variable, 0);
		uint8_t m = ((Random()>>24)%255)+1;
		m = m/2;
		//ST7735_DrawFastVLine(30,30,(final_val/2) + m, 0x079F);
		ST7735_DrawPixel(30, (final_val/2) + m, 0x079F);
		previous_variable = final_val/2 + m;
		m++;
		if(m>70) {
			ST7735_DrawBitmap(30,30,up_arrow_2, 26, 28);
			if(m>100) {
				m = 0;
				ST7735_DrawBitmap(30,30,up_arrow_0, 26, 28);
			}
		}
		
		else if(m>40) {
						ST7735_DrawBitmap(30,30,up_arrow_1, 26, 28);

		}
		else {
			ST7735_DrawBitmap(30,30,up_arrow_0, 26, 28);
		}
	}

	
	if(1==0) {
		if(mode == 0) {
		intro_check = intro_check + 3;
		if(!intro_check_flag && !intro_finish_flag && intro_check < 165) {
			ST7735_DrawBitmap(0, -80+intro_check, intro_pointer[intro_check_flag], 128, 80);
			if(intro_check >= 162) {
				intro_check = 0;
				intro_check_flag = 1;
			}
		}
		if(intro_check_flag && intro_check < 81) {
			ST7735_DrawBitmap(0, 240-intro_check, intro_pointer[intro_check_flag], 128, 80);
			if(intro_check >= 78) {
				intro_check = 0;
				intro_check_flag = 0;
				intro_finish_flag = 1;
			}
		}
		
		if(intro_finish_flag) {
			intro_wait++;
			if(intro_wait > 500) {
				mode = 1;
				ST7735_FillScreen(0x0000);	// set screen to black
			}
		}
	}
		
	else if(mode == 1) {
	if(intro_finish_flag && !exit_flag) {
		//ST7735_DrawBitmap(-90+check, 75, pointer[check_flag], 90, 71);
		check = check + 10;
		if(times >= 4 && check==120	&& check_flag == grade) {
			check = 0;
			exit_flag = 1;
			mode++;
		}
		if(check > 218) {
			times++;
			check = 0;
			check_flag++;
			if(check_flag > 4) {
				check_flag = 0;
			}
		}
	}
}
	
else if(mode == 2) {
	
	color_flag++;
	if(color_flag == 14) {
		if(red==255) {
			red=0;
			green=255;
			color_flag = 0;
		}
		else if(green==255) {
			green=200;
			blue=255;
			color_flag = 0;
		}
		else {
			blue = 0;
			green = 0;
			red = 255;
			color_flag = 0;
		}
	}
	// red, green, blue
	ST7735_DrawCharS(4, 80, 'P', ST7735_Color565(red, green, blue), 1, 1);
	ST7735_DrawCharS(13, 80, 'E', ST7735_Color565(blue, red, green), 1, 1);
	ST7735_DrawCharS(22, 80, 'R', ST7735_Color565(green, blue, red), 1, 1);
	ST7735_DrawCharS(31, 80, 'F', ST7735_Color565(red, green, blue), 1, 1);
	ST7735_DrawCharS(40, 80, 'E', ST7735_Color565(blue, red, green), 1, 1);
	ST7735_DrawCharS(49, 80, 'C', ST7735_Color565(green, blue, red), 1, 1);
	ST7735_DrawCharS(58, 80, 'T', ST7735_Color565(red, green, blue), 1, 1);
	ST7735_DrawCharS(67, 80, '!', ST7735_Color565(blue, red, green), 1, 1);

	if(!first_time_color_flag) {
		first_time_color_flag = 1;
		ST7735_DrawCharS(4, 92, 'G', ST7735_Color565(255, 255, 0), 1, 1);
		ST7735_DrawCharS(10, 92, 'R', ST7735_Color565(255, 255, 0), 1, 1);
		ST7735_DrawCharS(16, 92, 'E', ST7735_Color565(255, 255, 0), 1, 1);
		ST7735_DrawCharS(22, 92, 'A', ST7735_Color565(255, 255, 0), 1, 1);
		ST7735_DrawCharS(28, 92, 'T', ST7735_Color565(255, 255, 0), 1, 1);
		ST7735_DrawCharS(34, 92, '!', ST7735_Color565(255, 255, 0), 1, 1);
		
		ST7735_DrawCharS(4, 104, 'G', ST7735_Color565(0, 255, 0), 1, 1);
		ST7735_DrawCharS(10, 104, 'O', ST7735_Color565(0, 255, 0), 1, 1);
		ST7735_DrawCharS(16, 104, 'O', ST7735_Color565(0, 255, 0), 1, 1);
		ST7735_DrawCharS(22, 104, 'D', ST7735_Color565(0, 255, 0), 1, 1);
		ST7735_DrawCharS(28, 104, '!', ST7735_Color565(0, 255, 0), 1, 1);
		
		ST7735_DrawCharS(4, 116, 'O', ST7735_Color565(255, 255, 255), 1, 1);
		ST7735_DrawCharS(10, 116, 'K', ST7735_Color565(255, 255, 255), 1, 1);

		ST7735_DrawCharS(4, 128, 'M', ST7735_Color565(255, 0,0), 1, 1);
		ST7735_DrawCharS(10, 128, 'I', ST7735_Color565(255, 0,0), 1, 1);
		ST7735_DrawCharS(16, 128, 'S', ST7735_Color565(255, 0,0), 1, 1);
		ST7735_DrawCharS(22, 128, 'S', ST7735_Color565(255, 0,0), 1, 1);

	}

	}
}
		
	else if(invert_flag) {	// change to if
		if(invert_first_time == 1) {
				ST7735_DrawBitmap(4, 159-i, left_arrow_filler, 28, 26);	// 28
				ST7735_DrawBitmap(36, 159-i, up_arrow_filler, 26, 28);
				ST7735_DrawBitmap(66, 159-i, down_arrow_filler, 26, 28);
				ST7735_DrawBitmap(96, 159-i, right_arrow_filler, 28, 26);
				i--;
				if(i == 0) {
					invert_first_time++;
				}
		}
		else if(invert_first_time == 0) {
				Timer3_Init(296296);	// 888889
				i = 131;
				invert_first_time++;
		}
		else {
			if(invert_first_time == 2) {
				Timer3_Init(888889);
				invert_first_time++;
			}
			count_down++;
			count_up++;
			count_left++;
			count_right++;
		// ======================= RIGHT =======================
		if(count_right > 60) {
			ST7735_DrawBitmap(96, i+28, right_arrow_2, 28, 26);	// 28, 26
			if(count_right > 90) 
				count_right = 0;
		}
		else if(count_right > 30) {
				ST7735_DrawBitmap(96, i+28, right_arrow_1, 28, 26);	// 28, 26
		}
		else {
			ST7735_DrawBitmap(96, i+28, right_arrow_0, 28, 26);
		}
		// ======================= DOWN =======================
		if(count_down > 60) {
			ST7735_DrawBitmap(66, i+28, down_arrow_2, 26, 28);	// 26, 28
			if(count_down > 90) 
				count_down = 0;
		}
		else if(count_down > 30) {
				ST7735_DrawBitmap(66, i+28, down_arrow_1, 26, 28);	// 26, 28
		}
		else {
			ST7735_DrawBitmap(66, i+28, down_arrow_0, 26, 28);
		}
		// ======================= LEFT =======================
		if(count_left > 60) {
			ST7735_DrawBitmap(4, i+28, left_arrow_2, 28, 26);	// 28, 26
			if(count_left > 90) 
				count_left = 0;
		}
		else if(count_left > 30) {
				ST7735_DrawBitmap(4, i+28, left_arrow_1, 28, 26);	// 28, 26
		}
		else {
			ST7735_DrawBitmap(4, i+28, left_arrow_0, 28, 26);
		}
		// ======================= UP =======================
		if(count_up > 60) {
			ST7735_DrawBitmap(36, i+28, up_arrow_2, 26, 28);	// 26, 28
			if(count_up > 90)
				count_up = 0;
		}
		else if(count_up > 30) {
			ST7735_DrawBitmap(36, i+28, up_arrow_1, 26, 28);	// 26, 28
		}
		else {
			ST7735_DrawBitmap(36, i+28, up_arrow_0, 26, 28);	// 26, 28
		}
		i++;
		if(i == 187) {
			ST7735_DrawBitmap(4, 159, left_arrow_filler, 28, 26);	// 28
		//}
		//if(i == -27) {
			ST7735_DrawBitmap(36, 159, up_arrow_filler, 26, 28);
	//	}
			ST7735_DrawBitmap(66, 159, down_arrow_filler, 26, 28);
			ST7735_DrawBitmap(96, 159, right_arrow_filler, 28, 26);
			i = -28;
			invert_timer++;
			if(invert_timer > 5) {
				invert_flag = 0;
				i = 159;
				invert_first_time = 0;
				invert_timer = 0;
				timer++;
			}
		}
	}
		
		
		
		
	}
	else {
	count_left++;
	count_up++;
	count_down++;
	count_right++;
		
	if(timer == 9) {
		invert_flag = 1;
	}		
		
	if(timer > 6) {
		ST7735_InvertDisplay(0);
		timer_flag = 0;
	}
		else if(timer > 3) {
		ST7735_InvertDisplay(1);
		timer_flag = 1;
	}
		// ======================= RIGHT =======================
		if(count_right > 60) {
			ST7735_DrawBitmap(96, i+28, right_arrow_2, 28, 26);	// 28, 26
			if(count_right > 90) 
				count_right = 0;
		}
		else if(count_right > 30) {
				ST7735_DrawBitmap(96, i+28, right_arrow_1, 28, 26);	// 28, 26
		}
		else {
			ST7735_DrawBitmap(96, i+28, right_arrow_0, 28, 26);
		}
		// ======================= DOWN =======================
		if(count_down > 60) {
			ST7735_DrawBitmap(66, i+28, down_arrow_2, 26, 28);	// 26, 28
			if(count_down > 90) 
				count_down = 0;
		}
		else if(count_down > 30) {
				ST7735_DrawBitmap(66, i+28, down_arrow_1, 26, 28);	// 26, 28
		}
		else {
			ST7735_DrawBitmap(66, i+28, down_arrow_0, 26, 28);
		}
		// ======================= LEFT =======================
		if(count_left > 60) {
			ST7735_DrawBitmap(4, i+28, left_arrow_2, 28, 26);	// 28, 26
			if(count_left > 90) 
				count_left = 0;
		}
		else if(count_left > 30) {
				ST7735_DrawBitmap(4, i+28, left_arrow_1, 28, 26);	// 28, 26
		}
		else {
			ST7735_DrawBitmap(4, i+28, left_arrow_0, 28, 26);
		}
		// ======================= UP =======================
		if(count_up > 60) {
			ST7735_DrawBitmap(36, i+28, up_arrow_2, 26, 28);	// 26, 28
			if(count_up > 90)
				count_up = 0;
		}
		else if(count_up > 30) {
			ST7735_DrawBitmap(36, i+28, up_arrow_1, 26, 28);	// 26, 28
		}
		else {
			ST7735_DrawBitmap(36, i+28, up_arrow_0, 26, 28);	// 26, 28
		}
		
		
		*/
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		//ST7735_DrawBitmap(4, i+28, left_arrow_bmp_version2, 28, 26);	// 28, 26
		/*
		if(i+28 <= 53) { // 27
			ST7735_DrawBitmap(36, 53, up_arrows[up_arrow_count], 26, 53);
			up_arrow_count++;
			if(up_arrow_count > 23) {
				up_arrow_count = 0;
			}
		}
		*/
		/*
		else if(i+28 > 27){
			ST7735_DrawBitmap(36, i+28, up_arrow_bmp_version2, 26, 28);
		}
		*/
		//ST7735_DrawBitmap(36, i+28, up_arrow_bmp_version2, 26, 28);

		//ST7735_DrawBitmap(66, i+28, down_arrow_bmp_version2, 26, 28);
		//ST7735_DrawBitmap(96, i+28, right_arrow_bmp_version2, 28, 26); // 28, 26
		
		
		
		
		
		
		
		// THIS + THE PART ABOVE
		
		/*
	i--;
	if(i+28 == 28 && !(GPIO_PORTF_DATA_R & 0x01 ) && flag == 0) {
		
		ST7735_DrawBitmap(3, 120, star, 22, 22);
		ST7735_DrawBitmap(35, 120, star, 22, 22);
		flag = 1;
		
	}
	else if(i+28 <= 29 && i+28 >= 27 && !(GPIO_PORTF_DATA_R & 0x01) && flag == 0) {
		// GREAT
		ST7735_DrawBitmap(99, 120, star, 22, 22);
		flag = 1;
	}
	else if(i+28 <= 30 && i+28 >= 26) {
		// GOOD
	}
	if(i+28 <= 31 && i+28 >= 25) {
		// OKAY
	}
	
	if(i < 0 && (!invert_flag)) {
		//if(i < 26) {
			x++;
		if(i == -28) {
			ST7735_DrawBitmap(4, 28, left_arrow_filler, 28, 26);	// 28
		//}
		//if(i == -27) {
			ST7735_DrawBitmap(36, 28, up_arrow_filler, 26, 28);
	//	}
		ST7735_DrawBitmap(66, 28, down_arrow_filler, 26, 28);
		ST7735_DrawBitmap(96, 28, right_arrow_filler, 28, 26);
		}
		flag = 0;
		if(i == -28) {
			i = 159;
			x = 0;
			timer++;
	}
	
}

}
}
*/
