// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on LM4F120 or TM4C123
// Program written by: put your names here
// Date Created: 8/25/2014 
// Last Modified: 3/6/2015 
// Section 1-2pm     TA: Wooseok Lee
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "dac.h"
#include "tm4c123gh6pm.h"
#include "sound.h"

void (*PeriodicTask)(void);   // user function
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts



unsigned char Index;           // Index varies from 0 to 63
unsigned char Instrument;

#define SineWave 0
#define Guitar 1
#define Flute 2
#define Trumpet 3
#define Horn 4
#define Bassoon 5
#define SquareWave 6
#define SawtoothWave 7

//***need to actually write song here***


const uint8_t Instruments[8][64]= {
	{ 32,35,38,41,44,47,49,52,
		54,56,58,59,61,62,62,63,63,
		63,62,62,61,59,58,56,54,52,
		49,47,44,41,38,35,32,29,26,
		23,20,17,15,12,10,8,6,5,3,2,
		2,1,1,1,2,2,3,5,6,8,10,12,
		15,17,20,23,26,29},
	{ 5,5,5,5,4,3,2,1,0,1,2,	
		4,7,9,10,11,12,11,10,8,6,3,	
		2,2,4,6,9,12,14,15,14,12,	
		10,8,7,6,6,5,5,5,6,7,	
		7,8,8,8,7,6,5,3,3,2,	
		3,3,3,3,3,3,4,4,5,5,5,5},
	{ 6,7,9,9,10,11,12,13,13,14,15,	
		15,15,15,15,14,13,13,12,11,10,9,	
		8,7,7,6,6,5,5,5,5,4,	
		4,4,4,4,4,3,3,3,3,3,	
		2,2,1,1,1,1,0,0,0,0,	
		0,0,1,1,1,2,2,3,3,4,4,5},
	{ 10,10,11,11,11,12,12,11,10,9,8,
		5,3,2,1,3,8,11,14,15,15,13,
		12,11,10,10,10,10,11,11,10,10,
		10,10,10,10,10,10,10,10,10,10,
		10,10,10,11,11,11,11,11,11,11,
		11,11,11,11,11,10,10,10,10,10,10,10},
	{ 7,8,8,8,8,9,10,12,15,15,15,	
	  13,10,7,4,3,3,3,3,3,3,3,	
    4,4,4,4,5,6,7,8,8,9,	
    9,10,11,11,12,13,13,12,12,13,	
	  14,12,11,9,8,6,3,2,1,1,	
	  0,1,1,1,2,2,3,4,4,6,7,7},  	
	{	7,8,8,8,8,8,8,8,7,7,6,	
		7,8,10,14,15,12,7,2,0,0,2,	
		3,5,7,9,11,11,10,9,7,5,	
		3,2,2,3,5,7,9,10,9,7,	
		6,5,5,5,5,5,6,6,7,7,	
		7,8,8,9,9,8,8,8,8,8,7,7},
		{63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63}
};
// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Called once, with sound initially off
// Input: interrupt period
//           Units to be determined by YOU
//           Maximum to be determined by YOU
//           Minimum to be determined by YOU
// Output: none
void Sound_Init(uint32_t period, uint32_t device){
	DAC_Init();          // Port B is DAC
  Index = 0;
	Instrument = device;
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
	NVIC_EN0_R |= 0x8000;
  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
}
//**************Timer0A_Init*********************
//Initialize Timer0A
//Called once
//Input: none
//Output: none
//void Timer0A_Init(void(*task)(void), uint32_t delay){
//	DisableInterrupts(); // Disable interrupts
//  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
//	PeriodicTask = task;          // user function
//  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
//  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
//  TIMER0_TAMR_R = 0x00000001;   // 3) configure for delay mode
//  TIMER0_TAILR_R = delay;    // 4) reload value
//  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
//  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
//  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
//  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
//// interrupts enabled in the main program after all devices initialized
//// vector number 35, interrupt number 19
//  NVIC_EN0_R |= 1<<19;           // 9) enable IRQ 19 in NVIC
//  TIMER0_CTL_R |= 0x00000001;    // 10) enable TIMER0A
//	EnableInterrupts();  // Enable interrupts
//}

// **************Sound_Play*********************
// Start sound output, and set Systick interrupt period 
// Input: interrupt period
//           Units to be determined by YOU
//           Maximum to be determined by YOU
//           Minimum to be determined by YOU
//         input of zero disable sound output
// Output: none
void Sound_Play(uint32_t period){
	NVIC_ST_CTRL_R &= ~0x0004;  // disable SysTick during setup
	NVIC_ST_RELOAD_R = period;
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_ST_CTRL_R |= 0x0004;   // enable SysTick
}
//void Play_Song(void){
//	

//}
void SysTick_Handler(void){
	GPIO_PORTF_DATA_R ^= 1<<2;
  Index = (Index+1)&0x3F;      // 32,35,38,41,...
  DAC_Out(Instruments[Instrument][Index]);    // output one value each interrupt
}
//void Timer0A_Handler(void){
//	TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
//	(*PeriodicTask)();
//}
//
