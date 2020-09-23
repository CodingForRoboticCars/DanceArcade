// main.cpp
// Runs on LM4F120/TM4C123
// Thank you to Jonathan Valvano and Daniel Valvano for code on reading an SD card
// This is Viraj Wadhwa's Dance Arcade game main.cpp file

/* 
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "Buttons.h"
#include "diskio.h"
#include "ff.h"
#include "stdio.h"
#include "stdlib.h"

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C++" void Delay1ms(uint32_t n);
extern "C++" void Sound_Init(void);
extern "C++" void Sound_Play(uint32_t period);
/*
The below timers are not required for the game to run. They can be used if the functionality of reading data from the SD card
to buffers is moved from this main.cpp file to the timers file. 
*/
//extern "C++" void Timer3_Init(uint32_t period);
//extern "C++" void Timer2_Init(uint32_t period);
//extern "C++" void increase_size();
//extern "C++" void decrease_size();
//extern "C++" uint8_t return_size();
/*
We need two buffer flags to know if our buffers are filled with values we care about, 1 indicating it is 'ready' and 0 indicating
it is waiting for the next read from the SD card. We also declare two buffers each of 7000 char values. They are char because
that is how they are read from the SD card, but we have the SysTick Handler interrupt that parses the char and removes any info
that is not the data we care about, e.g. commas.
*/
extern "C++" unsigned char buffer_flag[1];
extern "C++" unsigned char buffer2_flag[1];
extern "C++" unsigned char buffer[7000];
extern "C++" unsigned char buffer2[7000];
uint32_t buffer_size = 7000;
extern "C++" unsigned char final_val;
extern "C++" uint8_t ready_flag;
extern int index;
extern uint8_t change_flag;
extern uint8_t end_flag;

extern const unsigned short dance_bmp[];
extern const unsigned short easy[];
extern const unsigned short easy_selected[];
extern const unsigned short medium[];
extern const unsigned short left_arrow_filler[], right_arrow_filler[], up_arrow_filler[], down_arrow_filler[];

/*
We declare our arrays that contain the sprite values for the moving arrows as well as the grayed-out filler arrows. 
We need to pass these to our ST7735 graphical libraries so we can fill the screen.
*/
extern const unsigned short up_arrow_0_version3[], up_arrow_1_version3[], up_arrow_2_version3[];
extern const unsigned short left_arrow_0_version3[], left_arrow_1_version3[], left_arrow_2_version3[];
extern const unsigned short right_arrow_0_version3[], right_arrow_1_version3[], right_arrow_2_version3[];
extern const unsigned short down_arrow_0_version3[], down_arrow_1_version3[], down_arrow_2_version3[];
extern const unsigned short right_filler_0[], right_filler_1[], right_filler_2[], right_filler_3[];
extern const unsigned short up_filler_0[], up_filler_1[], up_filler_2[], up_filler_3[];
extern const unsigned short left_filler_0[], left_filler_1[], left_filler_2[], left_filler_3[];
extern const unsigned short down_filler_0[], down_filler_1[], down_filler_2[], down_filler_3[];

const unsigned short* yes_fillers[7] = {right_filler_0, right_filler_1, right_filler_2, right_filler_1, right_filler_0, right_filler_3, right_arrow_filler};
const unsigned short* up_fillers[7] = {up_filler_0, up_filler_1, up_filler_2, up_filler_1, up_filler_0, up_filler_3, up_arrow_filler};
const unsigned short* left_fillers[7] = {left_filler_0, left_filler_1, left_filler_2, left_filler_1, left_filler_0, left_filler_3, left_arrow_filler};
const unsigned short* down_fillers[7] = {down_filler_0, down_filler_1, down_filler_2, down_filler_1, down_filler_0, down_filler_3, down_arrow_filler};

// variables that are used at the end of the program to indicate the score for the player
uint32_t static perfect;
uint32_t static great;
uint32_t static good;
uint32_t static ok;
uint32_t static miss;
uint32_t static total_arrows;
double static accuracy;
uint32_t static total_arrows_right;

uint8_t left_flag, up_flag, right_flag, down_flag;
uint8_t yes_pointer;

uint8_t rotation_flag;
uint16_t left_button;
uint16_t up_button;
uint16_t down_button;
uint16_t right_button;
uint32_t rotation_count;

uint8_t color_for_invert;
uint8_t red_1;
uint8_t green_1;
uint8_t blue_1;

uint8_t closest_distance;
int16_t left_arrows[4];
int16_t up_arrows[4];
int16_t down_arrows[4];
int16_t right_arrows[4];

//extern const unsigned short medium_selected[];
extern const unsigned short hard[];

uint8_t visualizer_flag;
extern int index_copy;
uint8_t filling_buffer_flag;
uint8_t start_flag;
uint8_t after_flag;
uint8_t checking_flag;
double  height1;
double  height2;
double  height3;
double  height4;
double height5;
double height6;
double height7;
double height8;
uint32_t static color;
uint32_t static arrow_count;
uint8_t arrow_count_multiplier;
double static near_val;
double static minimum;
uint8_t multiple_value = 11;

/*
These are our properties for our songs, e.g. number of songs we have stored (so the player can traverse through and pick a song they
like) and the text file names corresponding to the song they pick. This list is changeable.
*/
int16_t song_number;
uint8_t song_total = 7;
const char* song_list[] = {"l2y.txt", "otr.txt", "9la.txt", "del.txt", "spd.txt", "wat.txt", "lah.txt"};
uint8_t static num_arrows;
int8_t pixel_increment;
int16_t farthest_distance;
uint8_t farthest_flag;
int16_t smallest;
uint8_t which_array_holds_smallest;
uint8_t index_of_smallest;

uint32_t static count_down;
//extern const unsigned short down_arrow_0[], down_arrow_1[], down_arrow_2[];

uint8_t cc;
const char inFilename[] = "otr.txt";   // 8 characters or fewer test.txt

//const unsigned short *menu_buttons[20] = {menu_button_0,menu_button_1,menu_button_2,menu_button_3,menu_button_4,menu_button_5,menu_button_6,menu_button_7,menu_button_8,menu_button_9,
//menu_button_10,menu_button_11,menu_button_12,menu_button_13,menu_button_14,menu_button_15,menu_button_16,menu_button_17,menu_button_18,menu_button_19};

//static int8_t menu_button_index;
//static uint8_t menu_button_switch;

//FRESULT MountFresult;
/* variables we need for reading from the SD card */
static FATFS g_sFatFs;
FRESULT MountFresult = f_mount(&g_sFatFs, "", 0);
	
FIL Handle,Handle2;

FRESULT Fresult;
//#define MAXBLOCKS 100
//#define FILETESTSIZE 10000
UINT successfulreads = 1;

int main(void){
	/* First we need to setup our hardware initializations */
  	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  	Random_Init(1);
	ST7735_InitR(INITR_REDTAB);
  	ST7735_FillScreen(0);                 // set screen to black
  	//Output_Init();
  	//Timer0_Init(&background,1600000); // 50 Hz
  	//Timer1_Init(&clock,80000000); // 1 Hz
	EnableInterrupts();
	Sound_Init();	
	enablebuttons();
	ST7735_SetTextColor(ST7735_WHITE);
	
	/* Here we draw a sprite with our created game background/menu screen. It takes up the entire screen  */
	ST7735_DrawBitmap(0,159, dance_bmp, 128, 160);	// 128, 160	// ddr_menu_background
	
	// Fill the buffers with '0' data (char)
	for(int i = 0; i < buffer_size; i++) {
			buffer[i] = 0;
			buffer2[i] = 0;
		}

		if(MountFresult){
			while(1){};
		}
		const char start_music[] = "smf.txt";   // 8 characters or fewer test.txt
		Fresult = f_open(&Handle, start_music, FA_READ);	// inFilename
		Fresult = f_read(&Handle, &cc, 1, &successfulreads);

		while(cc != '{') {
			Fresult = f_read(&Handle, &cc, 1, &successfulreads);
		}
		uint8_t static flag1 = 0;
		start_flag = 1;
		buffer_flag[0] = 1;
		buffer2_flag[0] = 1;
		
	// READING VALUES INTO THE BUFFERS
	while((GPIO_PORTF_DATA_R & 0x01) && (GPIO_PORTF_DATA_R & 0x02) && (GPIO_PORTF_DATA_R & 0x04) && (GPIO_PORTF_DATA_R & 0x08)) {
				if(buffer_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
						buffer_flag[0] = 0;
				}
				if(buffer2_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
						buffer2_flag[0] = 0;
				}
				if(start_flag) {
					Sound_Play(1814);	// 1814
					start_flag = 0;
				}
	};
	for(int i = 0; i < buffer_size; i++) {
			buffer[i] = 0;
			buffer2[i] = 0;
		}
	Fresult = f_close(&Handle);
	ST7735_FillScreen(0x0000);	// set screen to black
	
	//MountFresult = f_mount(&g_sFatFs, "", 0);
	if(MountFresult){
			while(1){};
		}
		const char menu_music[] = "l2y.txt";   // 8 characters or fewer test.txt
		Fresult = f_open(&Handle, menu_music, FA_READ);	// inFilename
		Fresult = f_read(&Handle, &cc, 1, &successfulreads);

		while(cc != '{') {
			Fresult = f_read(&Handle, &cc, 1, &successfulreads);
		}
		flag1 = 0;
		start_flag = 1;
		buffer_flag[0] = 1;
		buffer2_flag[0] = 1;
		static uint32_t counter;
		static uint8_t done_flag;
	
		for(int i = 0; i < 1000000; i++);
		ST7735_SetRotation(2);
		
		uint8_t y_set = 70;	// due to Rotation of 2, lower the number, the lower the visualizer will be 
		
	while((GPIO_PORTF_DATA_R & 0x02) && (GPIO_PORTF_DATA_R & 0x04)) {
				if(buffer_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
						buffer_flag[0] = 0;
				}
				if(buffer2_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
						buffer2_flag[0] = 0;
				}
				if(color > 65535) {
					color = 1;
				}
				if(arrow_count > 32500) {
					arrow_count = 0;
					arrow_count_multiplier++;
					if(arrow_count_multiplier > 3) {
						arrow_count_multiplier = 0;
					}
				}
				/*
				We use these int counters to keep track of the speed of the game. The advantage: We don't necessarily
				want to use an entire timer for just one part of the game. Disadvantage: We don't get to use an interrupt,
				the timer is still useful. If desired we can use Handler2A or Handler3A timers.
				*/
				count_down++;
				arrow_count++;
				
				if(arrow_count_multiplier == 1 && arrow_count == 11000) {
						ST7735_DrawBitmap(100, 32, right_arrow_0_version3, 28, 28);	// ( __, 155, ___ ..)
						ST7735_DrawBitmap(2, 32, left_arrow_0_version3, 28, 28);
				}
				else if(arrow_count_multiplier == 2 && arrow_count == 11000) {
						ST7735_DrawBitmap(100, 32, right_arrow_1_version3, 28, 28);
						ST7735_DrawBitmap(2, 32, left_arrow_1_version3, 28, 28);
				}
				else if(arrow_count_multiplier == 3 && arrow_count == 11000) {
						ST7735_DrawBitmap(100, 32, right_arrow_2_version3, 28, 28);
						ST7735_DrawBitmap(2, 32, left_arrow_2_version3, 28, 28);
				}
					
				if(count_down == 9000) {
					count_down = 0;
				}

				if(count_down == 6000) {
					/*
					This entire segment of long code is doing the calculations necessary and calling graphical library
					functions to make sure our audio visualizer bars work properly when the player is picking a song (exciting!)
					The advantage here is that we are using graphical libraries that only need to update specific pixel indexes on 
					the screen, which means we don't need to refresh the screen with a new set of pixels everytime which is very costly!
					We are picking and choosing our indexes to fill in with bars, we fill in different amounts of pixels based on how
					tall we want the bars to scale. This audio visualizer runs fast so we can still sample music at 44.1 kHz
					*/
					color = color + 3;
					ST7735_DrawFastVLine(34, y_set, height1, 0);
					ST7735_DrawFastVLine(35, y_set, height1, 0);
					ST7735_DrawFastVLine(36, y_set, height1, 0);
					ST7735_DrawFastVLine(37, y_set, height1, 0);
					
					ST7735_DrawFastVLine(42, y_set, height2, 0);
					ST7735_DrawFastVLine(43, y_set, height2, 0);
					ST7735_DrawFastVLine(44, y_set, height2, 0);
					ST7735_DrawFastVLine(45, y_set, height2, 0);
					
					ST7735_DrawFastVLine(50, y_set, height3, 0);
					ST7735_DrawFastVLine(51, y_set, height3, 0);
					ST7735_DrawFastVLine(52, y_set, height3, 0);
					ST7735_DrawFastVLine(53, y_set, height3, 0);
					
					ST7735_DrawFastVLine(58, y_set, height4, 0);
					ST7735_DrawFastVLine(59, y_set, height4, 0);
					ST7735_DrawFastVLine(60, y_set, height4, 0);
					ST7735_DrawFastVLine(61, y_set, height4, 0);
					
					ST7735_DrawFastVLine(66, y_set, height5, 0);
					ST7735_DrawFastVLine(67, y_set, height5, 0);
					ST7735_DrawFastVLine(68, y_set, height5, 0);
					ST7735_DrawFastVLine(69, y_set, height5, 0);
					
					ST7735_DrawFastVLine(74, y_set, height6, 0);
					ST7735_DrawFastVLine(75, y_set, height6, 0);
					ST7735_DrawFastVLine(76, y_set, height6, 0);
					ST7735_DrawFastVLine(77, y_set, height6, 0);
					
					ST7735_DrawFastVLine(82, y_set, height7, 0);
					ST7735_DrawFastVLine(83, y_set, height7, 0);
					ST7735_DrawFastVLine(84, y_set, height7, 0);
					ST7735_DrawFastVLine(85, y_set, height7, 0);
					
					ST7735_DrawFastVLine(90, y_set, height8, 0);
					ST7735_DrawFastVLine(91, y_set, height8, 0);
					ST7735_DrawFastVLine(92, y_set, height8, 0);
					ST7735_DrawFastVLine(93, y_set, height8, 0);
					if(final_val < 31.875 && final_val > 1) {
						near_val = 31.875;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 63.75;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
					}
					else if(final_val < 63.75 && final_val > 31.875) {
						near_val = 63.75;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 95.625 && final_val > 63.75) {
						near_val = 95.625;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 127.5 && final_val > 95.625) {
						near_val = 127.5;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 159.375 && final_val > 127.5) {
						near_val = 159.375;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 191.25 && final_val > 159.375) {
						near_val = 191.25;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
					}
					else if(final_val < 223.125 && final_val > 191.25) {
						near_val = 223.125;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else {
						near_val = 255;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 223.125;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					} 
					ST7735_DrawFastVLine(34, y_set, height1, color);
					ST7735_DrawFastVLine(35, y_set, height1, color);
					ST7735_DrawFastVLine(36, y_set, height1, color);
					ST7735_DrawFastVLine(37, y_set, height1, color);
					
					ST7735_DrawFastVLine(42, y_set, height2, color);
					ST7735_DrawFastVLine(43, y_set, height2, color);
					ST7735_DrawFastVLine(44, y_set, height2, color);
					ST7735_DrawFastVLine(45, y_set, height2, color);
					
					ST7735_DrawFastVLine(50, y_set, height3, color);
					ST7735_DrawFastVLine(51, y_set, height3, color);
					ST7735_DrawFastVLine(52, y_set, height3, color);
					ST7735_DrawFastVLine(53, y_set, height3, color);
					
					ST7735_DrawFastVLine(58, y_set, height4, color);
					ST7735_DrawFastVLine(59, y_set, height4, color);
					ST7735_DrawFastVLine(60, y_set, height4, color);
					ST7735_DrawFastVLine(61, y_set, height4, color);
					
					ST7735_DrawFastVLine(66, y_set, height5, color);
					ST7735_DrawFastVLine(67, y_set, height5, color);
					ST7735_DrawFastVLine(68, y_set, height5, color);
					ST7735_DrawFastVLine(69, y_set, height5, color);
					
					ST7735_DrawFastVLine(74, y_set, height6, color);
					ST7735_DrawFastVLine(75, y_set, height6, color);
					ST7735_DrawFastVLine(76, y_set, height6, color);
					ST7735_DrawFastVLine(77, y_set, height6, color);
					
					ST7735_DrawFastVLine(82, y_set, height7, color);
					ST7735_DrawFastVLine(83, y_set, height7, color);
					ST7735_DrawFastVLine(84, y_set, height7, color);
					ST7735_DrawFastVLine(85, y_set, height7, color);
					
					ST7735_DrawFastVLine(90, y_set, height8, color);
					ST7735_DrawFastVLine(91, y_set, height8, color);
					ST7735_DrawFastVLine(92, y_set, height8, color);
					ST7735_DrawFastVLine(93, y_set, height8, color);
				}
				
				
				else if(count_down == 3000) {
					color = color + 3;
					ST7735_DrawFastVLine(34, y_set, height1, 0);
					ST7735_DrawFastVLine(35, y_set, height1, 0);
					ST7735_DrawFastVLine(36, y_set, height1, 0);
					ST7735_DrawFastVLine(37, y_set, height1, 0);
					
					ST7735_DrawFastVLine(42, y_set, height2, 0);
					ST7735_DrawFastVLine(43, y_set, height2, 0);
					ST7735_DrawFastVLine(44, y_set, height2, 0);
					ST7735_DrawFastVLine(45, y_set, height2, 0);
					
					ST7735_DrawFastVLine(50, y_set, height3, 0);
					ST7735_DrawFastVLine(51, y_set, height3, 0);
					ST7735_DrawFastVLine(52, y_set, height3, 0);
					ST7735_DrawFastVLine(53, y_set, height3, 0);
					
					ST7735_DrawFastVLine(58, y_set, height4, 0);
					ST7735_DrawFastVLine(59, y_set, height4, 0);
					ST7735_DrawFastVLine(60, y_set, height4, 0);
					ST7735_DrawFastVLine(61, y_set, height4, 0);
					
					ST7735_DrawFastVLine(66, y_set, height5, 0);
					ST7735_DrawFastVLine(67, y_set, height5, 0);
					ST7735_DrawFastVLine(68, y_set, height5, 0);
					ST7735_DrawFastVLine(69, y_set, height5, 0);
					
					ST7735_DrawFastVLine(74, y_set, height6, 0);
					ST7735_DrawFastVLine(75, y_set, height6, 0);
					ST7735_DrawFastVLine(76, y_set, height6, 0);
					ST7735_DrawFastVLine(77, y_set, height6, 0);
					
					ST7735_DrawFastVLine(82, y_set, height7, 0);
					ST7735_DrawFastVLine(83, y_set, height7, 0);
					ST7735_DrawFastVLine(84, y_set, height7, 0);
					ST7735_DrawFastVLine(85, y_set, height7, 0);
					
					ST7735_DrawFastVLine(90, y_set, height8, 0);
					ST7735_DrawFastVLine(91, y_set, height8, 0);
					ST7735_DrawFastVLine(92, y_set, height8, 0);
					ST7735_DrawFastVLine(93, y_set, height8, 0);
					if(final_val < 31.875 && final_val > 1) {
						near_val = 31.875;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 63.75;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
					}
					else if(final_val < 63.75 && final_val > 31.875) {
						near_val = 63.75;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 95.625 && final_val > 63.75) {
						near_val = 95.625;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 127.5 && final_val > 95.625) {
						near_val = 127.5;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 159.375 && final_val > 127.5) {
						near_val = 159.375;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 191.25 && final_val > 159.375) {
						near_val = 191.25;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
					}
					else if(final_val < 223.125 && final_val > 191.25) {
						near_val = 223.125;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else {
						near_val = 255;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 223.125;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					} 
					
					ST7735_DrawFastVLine(34, y_set, height1, color);
					ST7735_DrawFastVLine(35, y_set, height1, color);
					ST7735_DrawFastVLine(36, y_set, height1, color);
					ST7735_DrawFastVLine(37, y_set, height1, color);
					
					ST7735_DrawFastVLine(42, y_set, height2, color);
					ST7735_DrawFastVLine(43, y_set, height2, color);
					ST7735_DrawFastVLine(44, y_set, height2, color);
					ST7735_DrawFastVLine(45, y_set, height2, color);
					
					ST7735_DrawFastVLine(50, y_set, height3, color);
					ST7735_DrawFastVLine(51, y_set, height3, color);
					ST7735_DrawFastVLine(52, y_set, height3, color);
					ST7735_DrawFastVLine(53, y_set, height3, color);
					
					ST7735_DrawFastVLine(58, y_set, height4, color);
					ST7735_DrawFastVLine(59, y_set, height4, color);
					ST7735_DrawFastVLine(60, y_set, height4, color);
					ST7735_DrawFastVLine(61, y_set, height4, color);
					
					ST7735_DrawFastVLine(66, y_set, height5, color);
					ST7735_DrawFastVLine(67, y_set, height5, color);
					ST7735_DrawFastVLine(68, y_set, height5, color);
					ST7735_DrawFastVLine(69, y_set, height5, color);
					
					ST7735_DrawFastVLine(74, y_set, height6, color);
					ST7735_DrawFastVLine(75, y_set, height6, color);
					ST7735_DrawFastVLine(76, y_set, height6, color);
					ST7735_DrawFastVLine(77, y_set, height6, color);
					
					ST7735_DrawFastVLine(82, y_set, height7, color);
					ST7735_DrawFastVLine(83, y_set, height7, color);
					ST7735_DrawFastVLine(84, y_set, height7, color);
					ST7735_DrawFastVLine(85, y_set, height7, color);
					
					ST7735_DrawFastVLine(90, y_set, height8, color);
					ST7735_DrawFastVLine(91, y_set, height8, color);
					ST7735_DrawFastVLine(92, y_set, height8, color);
					ST7735_DrawFastVLine(93, y_set, height8, color);
					
					
				}
				else if(count_down == 0) {
					color = color + 3;
					ST7735_DrawFastVLine(34, y_set, height1, 0);
					ST7735_DrawFastVLine(35, y_set, height1, 0);
					ST7735_DrawFastVLine(36, y_set, height1, 0);
					ST7735_DrawFastVLine(37, y_set, height1, 0);
					
					ST7735_DrawFastVLine(42, y_set, height2, 0);
					ST7735_DrawFastVLine(43, y_set, height2, 0);
					ST7735_DrawFastVLine(44, y_set, height2, 0);
					ST7735_DrawFastVLine(45, y_set, height2, 0);
					
					ST7735_DrawFastVLine(50, y_set, height3, 0);
					ST7735_DrawFastVLine(51, y_set, height3, 0);
					ST7735_DrawFastVLine(52, y_set, height3, 0);
					ST7735_DrawFastVLine(53, y_set, height3, 0);
					
					ST7735_DrawFastVLine(58, y_set, height4, 0);
					ST7735_DrawFastVLine(59, y_set, height4, 0);
					ST7735_DrawFastVLine(60, y_set, height4, 0);
					ST7735_DrawFastVLine(61, y_set, height4, 0);
					
					ST7735_DrawFastVLine(66, y_set, height5, 0);
					ST7735_DrawFastVLine(67, y_set, height5, 0);
					ST7735_DrawFastVLine(68, y_set, height5, 0);
					ST7735_DrawFastVLine(69, y_set, height5, 0);
					
					ST7735_DrawFastVLine(74, y_set, height6, 0);
					ST7735_DrawFastVLine(75, y_set, height6, 0);
					ST7735_DrawFastVLine(76, y_set, height6, 0);
					ST7735_DrawFastVLine(77, y_set, height6, 0);
					
					ST7735_DrawFastVLine(82, y_set, height7, 0);
					ST7735_DrawFastVLine(83, y_set, height7, 0);
					ST7735_DrawFastVLine(84, y_set, height7, 0);
					ST7735_DrawFastVLine(85, y_set, height7, 0);
					
					ST7735_DrawFastVLine(90, y_set, height8, 0);
					ST7735_DrawFastVLine(91, y_set, height8, 0);
					ST7735_DrawFastVLine(92, y_set, height8, 0);
					ST7735_DrawFastVLine(93, y_set, height8, 0);
					
					/*
					This part uses specifically picked values to scale the bars. Our data values
					coming in from the SD card (the music data) is on range of 0-255. So we can
					break up the range into chunks, and give each chunk its own visualizer bar.
					That way, based on the value that we sample, we can scale the bars appropriately,
					e.g. a value of 55 means that values closer to 255 have shorter bars in height and
					values closer to 0 have taller bars in height. We don't visualize the audio at every sample
					because that would be costly to the hardware and is too fast for the player to even see.
					*/
					if(final_val < 31.875 && final_val > 1) {
						near_val = 31.875;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 63.75;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
					}
					else if(final_val < 63.75 && final_val > 31.875) {
						near_val = 63.75;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 95.625;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 95.625 && final_val > 63.75) {
						near_val = 95.625;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 127.5;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 127.5 && final_val > 95.625) {
						near_val = 127.5;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 159.375;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 159.375 && final_val > 127.5) {
						near_val = 159.375;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 191.25;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else if(final_val < 191.25 && final_val > 159.375) {
						near_val = 191.25;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 223.125;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
					}
					else if(final_val < 223.125 && final_val > 191.25) {
						near_val = 223.125;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						near_val = 255;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					else {
						near_val = 255;
						minimum = 1;
						height1 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 31.875;
						height2 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 63.75;
						height3 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 95.625;
						height4 = ((final_val - 1)/(near_val - minimum)) * multiple_value;		
						minimum = 127.5;
						height5 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 159.375;
						height6 = ((final_val - 1)/(near_val - minimum)) * multiple_value;	
						minimum = 191.25;
						height7 = ((final_val - 1)/(near_val - minimum)) * multiple_value;
						minimum = 223.125;
						height8 = ((final_val - 1)/(near_val - minimum)) * multiple_value;						
					}
					ST7735_DrawFastVLine(34, y_set, height1, color);
					ST7735_DrawFastVLine(35, y_set, height1, color);
					ST7735_DrawFastVLine(36, y_set, height1, color);
					ST7735_DrawFastVLine(37, y_set, height1, color);
					
					ST7735_DrawFastVLine(42, y_set, height2, color);
					ST7735_DrawFastVLine(43, y_set, height2, color);
					ST7735_DrawFastVLine(44, y_set, height2, color);
					ST7735_DrawFastVLine(45, y_set, height2, color);
					
					ST7735_DrawFastVLine(50, y_set, height3, color);
					ST7735_DrawFastVLine(51, y_set, height3, color);
					ST7735_DrawFastVLine(52, y_set, height3, color);
					ST7735_DrawFastVLine(53, y_set, height3, color);
					
					ST7735_DrawFastVLine(58, y_set, height4, color);
					ST7735_DrawFastVLine(59, y_set, height4, color);
					ST7735_DrawFastVLine(60, y_set, height4, color);
					ST7735_DrawFastVLine(61, y_set, height4, color);
					
					ST7735_DrawFastVLine(66, y_set, height5, color);
					ST7735_DrawFastVLine(67, y_set, height5, color);
					ST7735_DrawFastVLine(68, y_set, height5, color);
					ST7735_DrawFastVLine(69, y_set, height5, color);
					
					ST7735_DrawFastVLine(74, y_set, height6, color);
					ST7735_DrawFastVLine(75, y_set, height6, color);
					ST7735_DrawFastVLine(76, y_set, height6, color);
					ST7735_DrawFastVLine(77, y_set, height6, color);
					
					ST7735_DrawFastVLine(82, y_set, height7, color);
					ST7735_DrawFastVLine(83, y_set, height7, color);
					ST7735_DrawFastVLine(84, y_set, height7, color);
					ST7735_DrawFastVLine(85, y_set, height7, color);
					
					ST7735_DrawFastVLine(90, y_set, height8, color);
					ST7735_DrawFastVLine(91, y_set, height8, color);
					ST7735_DrawFastVLine(92, y_set, height8, color);
					ST7735_DrawFastVLine(93, y_set, height8, color);
					
				}
		
				/*
				Now we handle the player's presses on the buttons as they scroll
				through the list of songs and try to pick a song they like. The 
				important thing we need to do here is we need to make sure that
				when they scroll a certain way to pick a song, we need to load the buffers
				with that new song data. This allows them to hear the song they are currently
				on in the list, and we want the audio visualizer to work on that new song!
				*/
				if(!(GPIO_PORTF_DATA_R & 0x01)) {	// they scrolled left
					for(int i = 0; i < 100000; i++) {};	// register just one button press
					if(song_number-1 < 0) {	// reached the end, reset
						song_number = song_total-1;
					}
					else {
						song_number--;
					}
					for(int i = 0; i < buffer_size; i++) {
						buffer[i] = 0;
						buffer2[i] = 0;
					}
					Fresult = f_close(&Handle);
					ST7735_FillScreen(0x0000);	// set screen to black

					Fresult = f_open(&Handle, song_list[song_number], FA_READ);	// inFilename
					Fresult = f_read(&Handle, &cc, 1, &successfulreads);

					while(cc != '{') {
						Fresult = f_read(&Handle, &cc, 1, &successfulreads);
					}
					flag1 = 1;
					buffer_flag[0] = 1;
					buffer2_flag[0] = 1;
					count_down = 1;
					index = 0;
	
				}
				else if(!(GPIO_PORTF_DATA_R & 0x08)) {	// they scrolled right
					for(int i = 0; i < 100000; i++) {};	// register just one button press
					if(song_number+1 == song_total) {	// reached the end, reset
						song_number = 0;
					}
					else {
						song_number++;
					}
					for(int i = 0; i < buffer_size; i++) {
						buffer[i] = 0;
						buffer2[i] = 0;
					}
					Fresult = f_close(&Handle);
					ST7735_FillScreen(0x0000);	// set screen to black

					Fresult = f_open(&Handle, song_list[song_number], FA_READ);	// inFilename
					Fresult = f_read(&Handle, &cc, 1, &successfulreads);

					while(cc != '{') {
						Fresult = f_read(&Handle, &cc, 1, &successfulreads);
					}
					flag1 = 1;
					buffer_flag[0] = 1;
					buffer2_flag[0] = 1;
					count_down = 1;
					index = 0;
				}
				
				if(flag1 == 1) {
					ST7735_SetRotation(0);
					/*
					This chunk of code here is 'drawing out' the titles of our songs.
					We want to make sure that they are spaced properly and evenly, 
					which has been tested and calculated based on our mini LCD screen size
					and we want to make sure the characters look good too! Based on
					what song number we are on, we pick that song to draw onto the screen
					*/
					
					if(song_number==0) {	// Lie 2 You
						ST7735_DrawChar(20,100, 'L', 0x079F, 0, 2);
						ST7735_DrawChar(20+10,100, 'i', 0x079F, 0, 2);
						ST7735_DrawChar(20+20,100, 'e', 0x079F, 0, 2);
						ST7735_DrawChar(20+40,100, '2', 0x079F, 0, 2);		
						ST7735_DrawChar(20+60,100, 'Y', 0x079F, 0, 2);		
						ST7735_DrawChar(20+70,100, 'o', 0x079F, 0, 2);
						ST7735_DrawChar(20+80,100, 'u', 0x079F, 0, 2);
					}
					else if(song_number==1) {	// old town road
						ST7735_DrawChar(2,100, 'O', 0x079F, 0, 2);
						ST7735_DrawChar(2+10,100, 'l', 0x079F, 0, 2);
						ST7735_DrawChar(2+20,100, 'd', 0x079F, 0, 2);
						ST7735_DrawChar(2+35,100, 'T', 0x079F, 0, 2); 
						ST7735_DrawChar(2+45,100, 'o', 0x079F, 0, 2);
						ST7735_DrawChar(2+55,100, 'w', 0x079F, 0, 2);
						ST7735_DrawChar(2+65,100, 'n', 0x079F, 0, 2);
						ST7735_DrawChar(2+80,100, 'R', 0x079F, 0, 2);
						ST7735_DrawChar(2+90,100, 'o', 0x079F, 0, 2);
						ST7735_DrawChar(2+100,100, 'a', 0x079F, 0, 2);
						ST7735_DrawChar(2+110,100, 'd', 0x079F, 0, 2);
					}
					else if(song_number == 2) {
						ST7735_DrawChar(2,100, '9', 0x079F, 0, 2);
						ST7735_DrawChar(2+15,100, 'L', 0x079F, 0, 2);	
						ST7735_DrawChar(2+25,100, 'a', 0x079F, 0, 2);
						ST7735_DrawChar(2+35,100, 's', 0x079F, 0, 2); 
						ST7735_DrawChar(2+45,100, 'h', 0x079F, 0, 2);
						ST7735_DrawChar(2+55,100, 'e', 0x079F, 0, 2);
						ST7735_DrawChar(2+65,100, 's', 0x079F, 0, 2);
						ST7735_DrawChar(2+80,100, 'N', 0x079F, 0, 2);
						ST7735_DrawChar(2+95,100, 'B', 0x079F, 0, 2); 
						ST7735_DrawChar(2+110,100, 'D', 0x079F, 0, 2);
					}
					else if(song_number == 3) {
						ST7735_DrawChar(20,100, 'D', 0x079F, 0, 2);
						ST7735_DrawChar(20+12,100, 'e', 0x079F, 0, 2);
						ST7735_DrawChar(20+22,100, 'l', 0x079F, 0, 2);
						ST7735_DrawChar(20+32,100, 'u', 0x079F, 0, 2);		
						ST7735_DrawChar(20+44,100, 's', 0x079F, 0, 2);		
						ST7735_DrawChar(20+54,100, 'i', 0x079F, 0, 2);
						ST7735_DrawChar(20+64,100, 'o', 0x079F, 0, 2);	
						ST7735_DrawChar(20+76,100, 'n', 0x079F, 0, 2);						
					}
					else if(song_number == 4) {
						ST7735_DrawChar(5,100, 'D', 0x079F, 0, 2);
						ST7735_DrawChar(5+12,100, 'C', 0x079F, 0, 2);
						ST7735_DrawChar(5+24,100, 'L', 0x079F, 0, 2);
						ST7735_DrawChar(5+34,100, 'o', 0x079F, 0, 2); 
						ST7735_DrawChar(5+44,100, 'v', 0x079F, 0, 2);
						ST7735_DrawChar(5+54,100, 'e', 0x079F, 0, 2);
						ST7735_DrawChar(5+66,100, 'G', 0x079F, 0, 2);
						ST7735_DrawChar(5+78,100, 'o', 0x079F, 0, 2);
						ST7735_DrawChar(5+90,100, 'G', 0x079F, 0, 2);
						ST7735_DrawChar(5+102,100, 'o', 0x079F, 0, 2);
					}
					else if(song_number == 5) {
						ST7735_DrawChar(5,100, 'W', 0x079F, 0, 2);
						ST7735_DrawChar(5+12,100, 'o', 0x079F, 0, 2);
						ST7735_DrawChar(5+24,100, 'r', 0x079F, 0, 2);
						ST7735_DrawChar(5+36,100, 't', 0x079F, 0, 2); 
						ST7735_DrawChar(5+48,100, 'h', 0x079F, 0, 2);
						ST7735_DrawChar(5+62,100, 'a', 0x079F, 0, 2);
						ST7735_DrawChar(5+74,100, 'T', 0x079F, 0, 2);
						ST7735_DrawChar(5+86,100, 'r', 0x079F, 0, 2);
						ST7735_DrawChar(5+98,100, 'y', 0x079F, 0, 2);
					}
					else if(song_number == 6) {
						ST7735_DrawChar(5,100, 'L', 0x079F, 0, 2);
						ST7735_DrawChar(5+12,100, 'a', 0x079F, 0, 2);
						ST7735_DrawChar(5+24,100, 'h', 0x079F, 0, 2);
						ST7735_DrawChar(5+36,100, 'o', 0x079F, 0, 2); 
						ST7735_DrawChar(5+48,100, 'r', 0x079F, 0, 2);
						ST7735_DrawChar(5+62,100, 'e', 0x079F, 0, 2);
						ST7735_DrawChar(5+74,100, 'D', 0x079F, 0, 2);
						ST7735_DrawChar(5+86,100, 'i', 0x079F, 0, 2);
						ST7735_DrawChar(5+98,100, 'A', 0x079F, 0, 2);
					}
					
					// add more songs here! 
					flag1 = 0;
					ST7735_SetRotation(2);
				}
				
				if(start_flag) {
					/*
					When calculating what values to put inside the timers based on the 80 Mhz
					locked frequency of the microcontroller, make sure to think of the
					Nyquist Sampling theorem!
					*/ 
					//Timer3_Init(806248);	// 888889, should be 806248
					//Timer2_Init(7256235);	// half of Sound_Play		
					//Timer2_Init(806248);
					//Sound_Play(1814);	// 1814
					count_down = 1;
					flag1 = 1;
					index = 0;
					start_flag = 0;
				}
				
	};	
	
	ST7735_SetRotation(0);	// necesary
	
	for(int i = 0; i < buffer_size; i++) {
			buffer[i] = 0;
			buffer2[i] = 0;
		}
	/* 
	They've picked a song! But now we need to reset our file pointer so they can play the game
	with the song starting from the beginning, not starting from where they left off in the menu of
	sample songs!
	*/
	Fresult = f_close(&Handle);
	ST7735_FillScreen(0x0000);	// set screen to black

	/* open the song again */
	Fresult = f_open(&Handle, song_list[song_number], FA_READ);	// inFilename
	Fresult = f_read(&Handle, &cc, 1, &successfulreads);

	while(cc != '{') {
		Fresult = f_read(&Handle, &cc, 1, &successfulreads);
	}
	buffer_flag[0] = 1;
	buffer2_flag[0] = 1;
	
	static uint8_t count;
	num_arrows = 0;
	/* 
	We want our arrow sprites to be starting off the screen and coming from out the screen into 
	the game playground. 
	*/
	for(int i = 0; i < 4; i++) {
		left_arrows[i] = -200;
		up_arrows[i] = -200;
		down_arrows[i] = -200;
		right_arrows[i] = -200;
	}
	
	/* Here we are making sure to take into account the height of each arrow as they move along */
	closest_distance = 159+28;
	farthest_flag = 1;
	smallest = 159+28;
	
	left_flag = 0, down_flag = 0, up_flag = 0, right_flag = 0;
	yes_pointer = 0;
	total_arrows = 0;
	
	/*
	We draw the gray arrow fillers corresponding to their height (not all arrow fillers
	are made the same way! - some are slightly taller and some are slightly wider, e.g.
	the difference between a right facing arrow and an up facing arrow
	*/
	ST7735_DrawBitmap(4, 25, left_arrow_filler, 28, 26);	// 4, 28			
	ST7735_DrawBitmap(36, 27, up_arrow_filler, 26, 28);	// 36, 28			
	ST7735_DrawBitmap(66, 27, down_arrow_filler, 26, 28);	// 66, 28			
	ST7735_DrawBitmap(96, 25, right_arrow_filler, 28, 26);	// 96, 28	
	
	left_button = 0x01;
	up_button = 0x02;
	down_button = 0x04;
	right_button = 0x08;
	
	rotation_count = 0;
	rotation_flag = 0;
	
	uint32_t temp = 0;
	//	red, green, blue
	ST7735_DrawCharS(30, 60, '3', ST7735_Color565(255, 0, 0), 1, 12);
	Delay1ms(1000);
	ST7735_DrawCharS(30, 60, '3', ST7735_Color565(0, 0, 0), 1, 12);
	ST7735_DrawCharS(30, 60, '2', ST7735_Color565(255, 255, 0), 1, 12);
	Delay1ms(1000);
	ST7735_DrawCharS(30, 60, '2', ST7735_Color565(0, 0, 0), 1, 12);
	ST7735_DrawCharS(30, 60, '1', ST7735_Color565(255, 255, 255), 1, 12);
	Delay1ms(1000);
	ST7735_DrawCharS(30, 60, '1', ST7735_Color565(0,0,0), 1, 12);
	ST7735_DrawCharS(9, 60, 'G', ST7735_Color565(0, 255, 0), 1, 10);
	ST7735_DrawCharS(67, 60, 'O', ST7735_Color565(0, 255, 0), 1, 10);
	Delay1ms(1000); 
	ST7735_DrawCharS(9, 60, 'G', ST7735_Color565(0, 0, 0), 1, 10);
	ST7735_DrawCharS(67, 60, 'O', ST7735_Color565(0, 0, 0), 1, 10);

	index = 0;
	
	while(!end_flag) {	// while the song isn't over, play the game
				if(buffer_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
						buffer_flag[0] = 0;
				}
				if(buffer2_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
						buffer2_flag[0] = 0;
				}
				
				if(rotation_flag) {
					left_button = 0x08;
					right_button = 0x01;
					up_button = 0x04;
					down_button = 0x02;
				}
				else if(!rotation_flag) {
					left_button = 0x01;
					right_button = 0x08;
					up_button = 0x02;
					down_button = 0x04;
				}
			
				if((num_arrows < 4 && closest_distance < 100) || num_arrows == 0) {
						srand(NVIC_ST_CURRENT_R);
						uint8_t x = rand() % 4;
						rotation_count++;
	
						if(rotation_count == 145) {
							ST7735_InvertDisplay(1);
						}
						else if(rotation_count == 182) {
							ST7735_InvertDisplay(0);
						}
					
						if(rotation_count == 90) {	// this should be rotation_count == 180
							ST7735_SetRotation(0);
							rotation_flag = 0;
							ST7735_FillScreen(0);
							for(int i = 0; i < 100; i++) {
								if(buffer_flag[0] == 1) {
									Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
									buffer_flag[0] = 0;
								}
								if(buffer2_flag[0] == 1) {
									Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
									buffer2_flag[0] = 0;
								}
								
								color_for_invert++;
								if(color_for_invert == 14) {
									if(red_1==255) {
										red_1=0;
										green_1=255;
										color_for_invert = 0;
									}
									else if(green_1==255) {
										green_1=200;
										blue_1=255;
										color_for_invert = 0;
									}
									else {
										blue_1 = 0;
										green_1 = 0;
										red_1 = 255;
										color_for_invert = 0;
									}
								}
									// red, green, blue
	
								ST7735_DrawCharS(10, 80, 'I', ST7735_Color565(red_1, green_1, blue_1), 1, 3);
								ST7735_DrawCharS(28, 80, 'N', ST7735_Color565(blue_1, red_1, green_1), 1, 3);
								ST7735_DrawCharS(46, 80, 'V', ST7735_Color565(green_1, blue_1, red_1), 1, 3);
								ST7735_DrawCharS(64, 80, 'E', ST7735_Color565(red_1, green_1, blue_1), 1, 3);
								ST7735_DrawCharS(82, 80, 'R', ST7735_Color565(blue_1, red_1, green_1), 1, 3);
								ST7735_DrawCharS(100, 80, 'T', ST7735_Color565(green_1, blue_1, red_1), 1, 3);								
							}
							ST7735_DrawCharS(10, 80, 'I', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(28, 80, 'N', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(46, 80, 'V', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(64, 80, 'E', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(82, 80, 'R', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(100, 80, 'T', ST7735_Color565(0,0,0), 1, 3);
						}
						else if(rotation_count == 30) {	// this should be rotation_count == 90
							uint8_t i = 0;
							ST7735_FillScreen(0);
							for(int i = 0; i < 100; i++) {
								if(buffer_flag[0] == 1) {
									Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
									buffer_flag[0] = 0;
								}
								if(buffer2_flag[0] == 1) {
									Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
									buffer2_flag[0] = 0;
								}
								
								color_for_invert++;
								if(color_for_invert == 14) {
									if(red_1==255) {
										red_1=0;
										green_1=255;
										color_for_invert = 0;
									}
									else if(green_1==255) {
										green_1=200;
										blue_1=255;
										color_for_invert = 0;
									}
									else {
										blue_1 = 0;
										green_1 = 0;
										red_1 = 255;
										color_for_invert = 0;
									}
								}
									// red, green, blue
								
								ST7735_DrawCharS(10, 80, 'I', ST7735_Color565(red_1, green_1, blue_1), 1, 3);
								ST7735_DrawCharS(28, 80, 'N', ST7735_Color565(blue_1, red_1, green_1), 1, 3);
								ST7735_DrawCharS(46, 80, 'V', ST7735_Color565(green_1, blue_1, red_1), 1, 3);
								ST7735_DrawCharS(64, 80, 'E', ST7735_Color565(red_1, green_1, blue_1), 1, 3);
								ST7735_DrawCharS(82, 80, 'R', ST7735_Color565(blue_1, red_1, green_1), 1, 3);
								ST7735_DrawCharS(100, 80, 'T', ST7735_Color565(green_1, blue_1, red_1), 1, 3);								
							}
							ST7735_DrawCharS(10, 80, 'I', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(28, 80, 'N', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(46, 80, 'V', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(64, 80, 'E', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(82, 80, 'R', ST7735_Color565(0,0,0), 1, 3);
							ST7735_DrawCharS(100, 80, 'T', ST7735_Color565(0,0,0), 1, 3);	
							
								/*
								ST7735_DrawBitmap(4, 159, left_arrow_filler, 28, 26);	// 4, 28			
								ST7735_DrawBitmap(36, 157, up_arrow_filler, 26, 28);	// 36, 28			
								ST7735_DrawBitmap(66, 157, down_arrow_filler, 26, 28);	// 66, 28			
								ST7735_DrawBitmap(96, 159, right_arrow_filler, 28, 26);	// 96, 28	
								*/
								if(buffer_flag[0] == 1) {
										Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);		// Put these so the buffers can still refill
										buffer_flag[0] = 0;
								}
								if(buffer2_flag[0] == 1) {
										Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);		// Put these so the buffers can still refill
										buffer2_flag[0] = 0;
								}							
							
							ST7735_SetRotation(2);
							rotation_flag = 1;
						}
						
						if(x==0) {	// left arrow
							num_arrows++;
							total_arrows++;
							for(int i = 0; i < 4; i++) {
								if(left_arrows[i] == -200) {
									left_arrows[i] = 159+28;
									closest_distance = 159+28;
									break;
								}
							}
						}
						 if(x==1) {	// up arrow
							 num_arrows++;
							 total_arrows++;
							/*
							up.exist++;
							last_arrow = &up;
							*/
							for(int i = 0; i < 4; i++) {
								if(up_arrows[i] == -200) {
									up_arrows[i] = 159+28;
									closest_distance = 159+28;
									break;
								}
							}
						}
						
						else if(x==2) { // down arrow
							num_arrows++;							
							total_arrows++;
							for(int i = 0; i < 4; i++) {
								if(down_arrows[i] == -200) {
									down_arrows[i] = 159+28;
									closest_distance = 159+28;
									break;
								}
							}
						}
						
						else if(x==3) {	// right arrow
							num_arrows++;
							total_arrows++;
							/*
							right.exist++;
							last_arrow = &right;
							*/
							for(int i = 0; i < 4; i++) {
								if(right_arrows[i] == -200) {
									right_arrows[i] = 159+28;
									closest_distance = 159+28;
									break;
								}
							}
						}
					}
			
			pixel_increment = -num_arrows;
			count++;
			
		// ======================= RIGHT =======================
		if(count > 12) {
			for(int i = 0; i < 4; i++) {
				if(right_arrows[i] != -200) {
					ST7735_DrawBitmap(96, pixel_increment+right_arrows[i], right_arrow_2_version3, 28, 28);	// 28, 26
					right_arrows[i] = right_arrows[i] + pixel_increment;
				}
			}
			if(count > 18) 
				count = 0;
		}
		else if(count > 6) {
			for(int i = 0; i < 4; i++) {
				if(right_arrows[i] != -200) {
					ST7735_DrawBitmap(96, pixel_increment+right_arrows[i], right_arrow_1_version3, 28, 28);	// 28, 26
					right_arrows[i] = right_arrows[i] + pixel_increment;
				}
			}
		}
		else {
			for(int i = 0; i < 4; i++) {
				if(right_arrows[i] != -200) {
					ST7735_DrawBitmap(96, pixel_increment+right_arrows[i], right_arrow_0_version3, 28, 28);	// 28, 26
					right_arrows[i] = right_arrows[i] + pixel_increment;
				}
			}
		}		
		// ======================= DOWN =======================
		if(count > 12) {
			for(int i = 0; i < 4; i++) {
				if(down_arrows[i] != -200) {
					ST7735_DrawBitmap(66, pixel_increment+down_arrows[i], down_arrow_2_version3, 26, 29);	// 28, 26
					down_arrows[i] = down_arrows[i] + pixel_increment;
				}
			}
			if(count > 18) 
				count = 0;
		}
		else if(count > 6) {
			for(int i = 0; i < 4; i++) {
				if(down_arrows[i] != -200) {
					ST7735_DrawBitmap(66, pixel_increment+down_arrows[i], down_arrow_1_version3, 26, 29);	// 28, 26
					down_arrows[i] = down_arrows[i] + pixel_increment;
				}
			}
		}
		else {
			for(int i = 0; i < 4; i++) {
				if(down_arrows[i] != -200) {
					ST7735_DrawBitmap(66, pixel_increment+down_arrows[i], down_arrow_0_version3, 26, 29);	// 28, 26
					down_arrows[i] = down_arrows[i] + pixel_increment;
				}
			}
		}
				
		// ======================= LEFT =======================
		if(count > 12) {
			for(int i = 0; i < 4; i++) {
				if(left_arrows[i] != -200) {
					ST7735_DrawBitmap(4, pixel_increment+left_arrows[i], left_arrow_2_version3, 28, 28);	// 28, 26
					left_arrows[i] = left_arrows[i] + pixel_increment;
				}
			}
			if(count > 18) 
				count = 0;
		}
		else if(count > 6) {
			for(int i = 0; i < 4; i++) {
				if(left_arrows[i] != -200) {
					ST7735_DrawBitmap(4, pixel_increment+left_arrows[i], left_arrow_1_version3, 28, 28);	// 28, 26
					left_arrows[i] = left_arrows[i] + pixel_increment;
				}
			}
		}
		else {
			for(int i = 0; i < 4; i++) {
				if(left_arrows[i] != -200) {
					ST7735_DrawBitmap(4, pixel_increment+left_arrows[i], left_arrow_0_version3, 28, 28);	// 28, 26
					left_arrows[i] = left_arrows[i] + pixel_increment;
				}
			}
		}
		// ======================= UP =======================
		if(count > 12) {
			for(int i = 0; i < 4; i++) {
				if(up_arrows[i] != -200) {
					ST7735_DrawBitmap(36, pixel_increment+up_arrows[i], up_arrow_2_version3, 26, 29);	// 28, 26
					up_arrows[i] = up_arrows[i] + pixel_increment;
				}
			}
			if(count > 18)
				count = 0;
		}
		else if(count > 6) {
			for(int i = 0; i < 4; i++) {
				if(up_arrows[i] != -200) {
					ST7735_DrawBitmap(36, pixel_increment+up_arrows[i], up_arrow_1_version3, 26, 29);	// 28, 26
					up_arrows[i] = up_arrows[i] + pixel_increment;
				}
			}
		}
		else {
			for(int i = 0; i < 4; i++) {
				if(up_arrows[i] != -200) {
					ST7735_DrawBitmap(36, pixel_increment+up_arrows[i], up_arrow_0_version3, 26, 29);	// 28, 26
					up_arrows[i] = up_arrows[i] + pixel_increment;
				}
			}
		}
	// ==================================================================
		
		smallest = smallest + pixel_increment;	// we don't know which array the smallest value is in, but no matter what all arrows are drawn on the 
							// screen, so it has to move by pixel_increment
		closest_distance = closest_distance+pixel_increment;	// likewise
	
	if(farthest_flag) {
		smallest = 158+28;
		for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
		// ===================================================
	
		/////////////
	
		if((smallest <= 29 && smallest >= 23) && (which_array_holds_smallest == 0) && (!(GPIO_PORTF_DATA_R & left_button)) && (!left_flag)) {
			farthest_flag = 1;
			/*
			if(num_arrows != 0) {
				num_arrows--;
			}
			*/
			if(smallest == 29 || smallest == 23) 
				ok++;
			else if(smallest == 28 || smallest == 24)
				good++;
			else if(smallest == 27 || smallest==25) 
				great++;
			else if(smallest==26) 				
				perfect++;
			
			left_arrows[index_of_smallest] = -200;
			if(farthest_flag) {
				smallest = 158+28;
				for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
	left_flag = 1;
		}
		
		////////////
		if((smallest <= 29 && smallest >= 23) && (which_array_holds_smallest == 3) && (!(GPIO_PORTF_DATA_R & right_button)) && (!right_flag)) {
			farthest_flag = 1;
			/*
			if(num_arrows != 0) {
				num_arrows--;
			}
			*/
			if(smallest == 29 || smallest == 23) 
				ok++;
			else if(smallest == 28 || smallest == 24)
				good++;
			else if(smallest == 27 || smallest==25) 
				great++;
			else if(smallest==26) 				
				perfect++;
			
			right_arrows[index_of_smallest] = -200;
			if(farthest_flag) {
				smallest = 158+28;
				for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
	right_flag = 1;
		}
		///////
		
		else if((smallest <= 31 && smallest >= 25) && (which_array_holds_smallest == 1) && (!(GPIO_PORTF_DATA_R & up_button)) && (!up_flag)) {
			farthest_flag = 1;
		
			if(smallest == 31 || smallest == 25) 
				ok++;
			else if(smallest == 30 || smallest == 26)
				good++;
			else if(smallest == 29 || smallest==27) 
				great++;
			else if(smallest==28) 				
				perfect++;
			
			up_arrows[index_of_smallest] = -200;
			if(farthest_flag) {
				smallest = 158+28;
				for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
	up_flag = 1;
		}
		else if((smallest <= 31 && smallest >= 25) && (which_array_holds_smallest == 2) && (!(GPIO_PORTF_DATA_R & down_button)) && (!down_flag)) {
			farthest_flag = 1;
		
			if(smallest == 31 || smallest == 25) 
				ok++;
			else if(smallest == 30 || smallest == 26)
				good++;
			else if(smallest == 29 || smallest==27) 
				great++;
			else if(smallest==28) 				
				perfect++;
			
			down_arrows[index_of_smallest] = -200;
			if(farthest_flag) {
				smallest = 158+28;
				for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
	down_flag = 1;
		}
		
		if(right_flag) {
			if(yes_pointer != 7) {
				ST7735_DrawBitmap(96, 25, yes_fillers[yes_pointer], 28, 26);	// 36, 28
				yes_pointer++;
			}
			else {
				yes_pointer = 0;
				if(num_arrows != 0) {
				num_arrows--;
			}
				right_flag = 0;
			}
		}
		else if(up_flag) {
			if(yes_pointer != 7) {
				ST7735_DrawBitmap(36, 27, up_fillers[yes_pointer], 26, 28);	// 36, 28
				yes_pointer++;
			}
			else {
				yes_pointer = 0;
				if(num_arrows != 0) {
				num_arrows--;
			}
				up_flag = 0;
			}
		}
		else if(left_flag) {
			if(yes_pointer != 7) {
				ST7735_DrawBitmap(4, 25, left_fillers[yes_pointer], 28, 26);	// 36, 28
				yes_pointer++;
			}
			else {
				yes_pointer = 0;
				if(num_arrows != 0) {
				num_arrows--;
			}
				left_flag = 0;
			}
		}
		else if(down_flag) {
			if(yes_pointer != 7) {
				ST7735_DrawBitmap(66, 27, down_fillers[yes_pointer], 26, 28);	// 36, 28
				yes_pointer++;
			}
			else {
				yes_pointer = 0;
				if(num_arrows != 0) {
				num_arrows--;
			}
				down_flag = 0;
			}
		}
		
		if(smallest < 23) {
			if(which_array_holds_smallest == 0) {
				ST7735_DrawBitmap(4, 25, left_arrow_filler, 28, 26-smallest);	// 4, 28			
			}
			else if(which_array_holds_smallest == 1) {
				ST7735_DrawBitmap(36, 27, up_arrow_filler, 26, 28-smallest);	// 36, 28			
			}
			else if(which_array_holds_smallest == 2) {
				ST7735_DrawBitmap(66, 27, down_arrow_filler, 26, 28-smallest);	// 66, 28			
			}
			else if(which_array_holds_smallest == 3) {
				ST7735_DrawBitmap(96, 25, right_arrow_filler, 28, 26-smallest);	// 96, 28			
			}
		}
				
		if(smallest < 0) {		// > 187
			farthest_flag = 1;
			if(num_arrows != 0) {
				num_arrows--;
			}
			if(which_array_holds_smallest == 0) {
				left_arrows[index_of_smallest] = -200;
				//ST7735_DrawBitmap(4, 25, left_arrow_filler, 28, 26);	// 4, 28			
			}
			else if(which_array_holds_smallest == 1) {
				up_arrows[index_of_smallest] = -200;
				//ST7735_DrawBitmap(36, 27, up_arrow_filler, 26, 28);	// 36, 28			
			}
			else if(which_array_holds_smallest == 2) {
				down_arrows[index_of_smallest] = -200;
				//ST7735_DrawBitmap(66, 27, down_arrow_filler, 26, 28);	// 66, 28			
			}
			else if(which_array_holds_smallest == 3) {
				right_arrows[index_of_smallest] = -200;
				//ST7735_DrawBitmap(96, 25, right_arrow_filler, 28, 26);	// 96, 28			
			}
			
			if(farthest_flag) {
				smallest = 158+28;
				for(int i = 0; i < 4; i++) {
			if(left_arrows[i] <= smallest && left_arrows[i] != -200) {
				smallest = left_arrows[i];
				which_array_holds_smallest = 0;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(up_arrows[i] <= smallest && up_arrows[i] != -200) {
				smallest = up_arrows[i];
				which_array_holds_smallest = 1;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(down_arrows[i] <= smallest && down_arrows[i] != -200) {
				smallest = down_arrows[i];
				which_array_holds_smallest = 2;
				index_of_smallest = i;
			}
		}
		for(int i = 0; i < 4; i++) {
			if(right_arrows[i] <= smallest && right_arrows[i] != -200) {
				smallest = right_arrows[i];
				which_array_holds_smallest = 3;
				index_of_smallest = i;
			}
		}
		farthest_flag = 0;
	}
			
			/*
			invert_timer++;
			if(invert_timer > 5) {
				invert_flag = 0;
				i = 159;
				invert_first_time = 0;
				invert_timer = 0;
				timer++;
			}
			*/
		}
	}	// while(end_flag) ends here

	ST7735_FillScreen(0x0000);	// set screen to black
	
	Fresult = f_close(&Handle);
	ST7735_FillScreen(0x0000);	// set screen to black

	Fresult = f_open(&Handle, "hc.txt", FA_READ);	// inFilename
	Fresult = f_read(&Handle, &cc, 1, &successfulreads);

	while(cc != '{') {
			Fresult = f_read(&Handle, &cc, 1, &successfulreads);
	}
	buffer_flag[0] = 1;
	buffer2_flag[0] = 1;
	index = 0;
	end_flag = 0;

	uint8_t color_flag;
	uint8_t red, green, blue;
	red = 255;
	uint8_t first_time_color_flag = 0;
	accuracy = (((double)(perfect + great + good + ok))/(total_arrows)) * 100;
	total_arrows_right = perfect + great + good + ok;
	uint32_t accuracy_int = accuracy;
	uint32_t accuracy_saved = accuracy;
	uint8_t accuracy1 = (accuracy_int) % 10;
	char accuracy_1 = accuracy1 + '0';
	accuracy_int = accuracy_int/10;
	uint8_t accuracy2 = (accuracy_int) % 10;
	char accuracy_2 = accuracy2 + '0';
	accuracy_int = accuracy_int/10;
	uint8_t accuracy3 = (accuracy_int) % 10;
	char accuracy_3 = accuracy3 + '0';
	
	uint32_t total_arrows_right_saved = total_arrows_right;
		uint8_t total1 = total_arrows_right % 10;
		char total_1 = total1 + '0';
		total_arrows_right = total_arrows_right / 10;
		uint8_t total2 = (total_arrows_right) % 10;
		char total_2 = total2 + '0';
		total_arrows_right = total_arrows_right/10;
		uint8_t total3 = (total_arrows_right) % 10;
		char total_3 = total3 + '0';
	
	uint32_t perfect_saved = perfect;
		uint8_t per1 = perfect % 10;
		char per_1 = per1 + '0';
		perfect = perfect / 10;
		uint8_t per2 = (perfect) % 10;
		char per_2 = per2 + '0';
		perfect = perfect/10;
		uint8_t per3 = (perfect) % 10;
		char per_3 = per3 + '0';
	
	uint32_t great_saved = great;
	
		uint8_t gre1 = great % 10;
		char gre_1 = gre1 + '0';
		great = great / 10;
		uint8_t gre2 = (great) % 10;
		char gre_2 = gre2 + '0';
		great = great/10;
		uint8_t gre3 = (great) % 10;
		char gre_3 = gre3 + '0';
	
	uint32_t good_saved = good;
		uint8_t goo1 = good % 10;
		char goo_1 = goo1 + '0';
		good = good / 10;
		uint8_t goo2 = (good) % 10;
		char goo_2 = goo2 + '0';
		good = good/10;
		uint8_t goo3 = (good) % 10;
		char goo_3 = goo3 + '0';

	
	uint32_t ok_saved = ok;
		uint8_t ok1 = ok % 10;
		char ok_1 = ok1 + '0';
		ok = ok / 10;
		uint8_t ok2 = (ok) % 10;
		char ok_2 = ok2 + '0';
		ok = ok/10;
		uint8_t ok3 = (ok) % 10;
		char ok_3 = ok3 + '0';
	
	
	uint32_t miss_saved = miss;
		uint8_t mis1 = miss % 10;
		char mis_1 = mis1 + '0';
		miss = miss / 10;
		uint8_t mis2 = (miss) % 10;
		char mis_2 = mis2 + '0';
		miss = miss/10;
		uint8_t mis3 = (miss) % 10;
		char mis_3 = mis3 + '0';
		
	uint32_t total_arr_total = total_arrows;
		uint8_t tot1 = total_arrows % 10;
		char tot_1 = tot1 + '0';
		total_arrows = total_arrows / 10;
		uint8_t tot2 = total_arrows % 10;
		char tot_2 = tot2 + '0';
		total_arrows = total_arrows / 10;
		uint8_t tot3 = total_arrows % 10;
		char tot_3 = tot3 + '0';
	
	
	while(1) {
	
		if(buffer_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer, buffer_size, &successfulreads);
						buffer_flag[0] = 0;
				}
				if(buffer2_flag[0] == 1) {
						Fresult = f_read(&Handle, &buffer2, buffer_size, &successfulreads);
						buffer2_flag[0] = 0;
				}	
	
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
		if(accuracy_saved >= 100) {
			ST7735_DrawCharS(4, 10, accuracy_3, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(46, 10, accuracy_2, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(88, 10, accuracy_1, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(130, 10, '%', ST7735_Color565(255, 255, 255), 1, 7);						
		}
		else if(accuracy_saved >= 10) {
			ST7735_DrawCharS(4, 10, accuracy_2, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(46, 10, accuracy_1, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(88, 10, '%', ST7735_Color565(255, 255, 255), 1, 7);
		}
		else if(accuracy_saved < 10) {
			ST7735_DrawCharS(4, 10, accuracy_1, ST7735_Color565(255, 255, 255), 1, 7);
			ST7735_DrawCharS(46, 10, '%', ST7735_Color565(255, 255, 255), 1, 7);			
		}
		///
		if(perfect_saved >= 100) {
			ST7735_DrawCharS(85, 80, per_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(94, 80, per_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(103, 80, per_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(perfect_saved >= 10) {
			ST7735_DrawCharS(85, 80, per_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(94, 80, per_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(perfect_saved < 10) {
			ST7735_DrawCharS(85, 80, per_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		///
		if(great_saved >= 100) {
			ST7735_DrawCharS(46, 92, gre_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(52, 92, gre_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(58, 92, gre_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(great_saved >= 10) {
			ST7735_DrawCharS(46, 92, gre_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(52, 92, gre_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(great_saved < 10) {
			ST7735_DrawCharS(46, 92, gre_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		///
		if(good_saved >= 100) {
			ST7735_DrawCharS(40, 104, goo_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(46, 104, goo_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(52, 104, goo_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(good_saved >= 10) {
			ST7735_DrawCharS(40, 104, goo_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(46, 104, goo_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(good_saved < 10) {
			ST7735_DrawCharS(40, 104, goo_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		///
		if(ok_saved >= 100) {
			ST7735_DrawCharS(22, 116, ok_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(28, 116, ok_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(34, 116, ok_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(ok_saved >= 10) {
			ST7735_DrawCharS(22, 116, ok_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(28, 116, ok_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(ok_saved < 10) {
			ST7735_DrawCharS(22, 116, ok_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		///
		if(miss_saved >= 100) {
			ST7735_DrawCharS(34, 128, mis_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(40, 128, mis_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(46, 128, mis_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(miss_saved >= 10) {
			ST7735_DrawCharS(34, 128, mis_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(40, 128, mis_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		else if(miss_saved < 10) {
			ST7735_DrawCharS(34, 128, mis_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
		///
		if(total_arrows_right_saved >= 100) {
			ST7735_DrawCharS(82, 140, total_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(88, 140, total_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(94, 140, total_1, ST7735_Color565(255, 255, 255), 1, 1);		
		}
		else if(total_arrows_right_saved >= 10) {
			ST7735_DrawCharS(88, 140, total_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(94, 140, total_1, ST7735_Color565(255, 255, 255), 1, 1);			
		}
		else if(total_arrows_right_saved < 10) {
			ST7735_DrawCharS(94, 140, total_1, ST7735_Color565(255, 255, 255), 1, 1);						
		}
		///
		if(total_arr_total >= 100) {
			ST7735_DrawCharS(76, 152, tot_3, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(82, 152, tot_2, ST7735_Color565(255, 255, 255), 1, 1);
			ST7735_DrawCharS(88, 152, tot_1, ST7735_Color565(255, 255, 255), 1, 1);
		}
			
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
		
		ST7735_DrawCharS(4, 140, 'T', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(10, 140, 'O', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(16, 140, 'T', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(22, 140, 'A', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(28, 140, 'L', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(34, 140, 'C', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(40, 140, 'O', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(46, 140, 'R', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(52, 140, 'R', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(58, 140, 'E', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(64, 140, 'C', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(70, 140, 'T', ST7735_Color565(0, 199, 254), 1, 1);
		
		ST7735_DrawCharS(4, 152, 'T', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(10, 152, 'O', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(16, 152, 'T', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(22, 152, 'A', ST7735_Color565(0,199 , 254), 1, 1);
		ST7735_DrawCharS(28, 152, 'L', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(34, 152, 'A', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(40, 152, 'R', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(46, 152, 'R', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(52, 152, 'O', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(58, 152, 'W', ST7735_Color565(0, 199, 254), 1, 1);
		ST7735_DrawCharS(64, 152, 'S', ST7735_Color565(0, 199, 254), 1, 1);

	}
	
}
	
	ST7735_FillScreen(0x0000);	// set screen to black
	//Timer3_Init(888889);	// 888889
	
}
