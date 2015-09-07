// Sound.c
// Runs on any computer
// Sound assets based off the original Space Invaders 
// Import these constants into your SpaceInvaders.c for sounds!
// Jonathan Valvano
// November 17, 2014

#include "DAC.h"
#include "tm4c123gh6pm.h"
#include "SpaceInvaders.h"

unsigned long Index;
unsigned long counter;
const unsigned char* Array;
void Timer0_Init(void(*task)(Game *game), unsigned long period);


void player(Game *game){
	DAC_Out(Array[Index]);
	Index++;
	counter--;
	if(counter == 0){
		TIMER0_CTL_R = 0x00000000;
		game->soundPlaying = 0;
	}
}
void Sound_Init(void){	
	DAC_Init();
	Index = 0;
	counter = 0;
	Array = 0;
	Timer0_Init(&player, 7273); 
};
void Sound_Play(const unsigned char *pt, unsigned long count, Game *game){
	game->soundPlaying = 1;
	Array = pt;
	counter = count;
	Index = 0;
	TIMER0_CTL_R = 0x00000001;    // enable TIMER0A
};


