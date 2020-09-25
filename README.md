# Dance Arcade - a TM4C123 80 Mhz Embedded System Video game

https://www.youtube.com/watch?v=YZSp_Ykef24

Here is a video game I developed for the TM4C123 80 Mhz microcontroller during the Summer of 2019. I had an absolute blast making it, though difficult, it taught me a lot
about leveraging hardware (like system timers) to make features a reality. It took the entire summer, working day and night (with time also spent on classes and other stuff),
debugging, and reaching out to some of my graduate student connections for help. Most of the code can be found in main.cpp

This project uses SD card libraries that have been provided to me by Professor Valvano at the University of Texas at Austin. I used this SD card code to store my music for
the game there. Other than that, all hardware and software interfaces have been developed from the ground up. Music is stored in data arrays, read from the SD card, 
sampled at 44.1 kHz, and outputted through a resistor ladder I made, to a 3.5 mm jack.

All the sprites have been self created with paint, and all the graphical images I used in the game are all non-copyrighted!
I am making this game for educational/project purposes only, no selling or commercial listings allowed. 

Here are some of the coolest behind the scenes innovation I had to do in my 3 months of working on this game!
* The audio you hear from this video, is made from the ground up. I created an 8 bit Digital to Analog converter using a resistor ladder. The audio was converted to 8-bit value arrays stored on an SD card, where I used interrupts to sample these values at 44.1 kHz! Then you could connect any headphones or speaker via 3.5 mm jack, and you have music! 44.1 kHz music run on an 80 Mhz microcontroller!
* Sampling at 44.1 kHz on the microcontroller was difficult, because the same pins that transmit data to the mini LCD screen are the same pins that transmit data from the SD card! This means if I wanted to read audio values for sound, I needed to stop transmitting screen data. The performance punch: I cannot read huge chunks from the SD card without having stutters on the screen. I fixed this problem by creating two small buffers, where once one buffer was loaded with chunk 1 of audio values, the other buffer was loaded with chunk 2 in the meantime. This meant I could switch between buffers, load audio whenever I wanted, and use the audio I needed without running out of audio and needing to read from the SD card!
* You may notice the audio visualizer bars in the song selection menu of my video game. I created an audio visualizer that uses the 8-bit values coming in from the sampler, and through various formulas, it scales the bars. That means that every song has a unique audio visualizer, and when the drum beats drop you can see the bars shift to the left!
* This game also uses sprite overlaying. The incoming arrows can pass over the gray framed arrows you try to fill in, and they don’t get taken off the screen!
* The project also required me to use my knowledge and understanding of the microcontroller hardware. For example, I used SysTick or the system timer to keep track of interrupts. I used GPIO data registers and bit-masking to read data from the gameplay buttons. I took full advantage of ST7735 graphical libraries, which ended up being features of the game (inverting the screen + reversing the colors on the screen)!

If you would like to make this a playable game for yourself, please reach out to me via my linkedin at https://www.linkedin.com/in/virajwadhwa/ and I can help you setup
the hardware you need to play this game!
