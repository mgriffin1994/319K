// Lab6.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// MOOC lab 13 or EE319K lab6 starter
// Program written by: put your names here
// Date Created: 1/24/2015 
// Last Modified: 3/6/2015 
// Section 1-2pm     TA: Wooseok Lee
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  //wait for interrupts
extern unsigned char Instrument;

//typedef struct song tune;
//unsigned char Instrument;
//unsigned char beat;
//unsigned short length;

int main(void){      
  TExaS_Init(SW_PIN_PE3210,DAC_PIN_PB3210,ScopeOn);    // bus clock at 80 MHz
  Piano_Init();
  Sound_Init(0, 0);  
	uint32_t input;
	uint32_t previous = 0;
	uint32_t prior = 0;
	uint32_t switchin;
	//Timer0A_Init(&UserTask, 0);
  while(1){    
		//WaitForInterrupt();	
		input = Piano_In();
		switchin = GPIO_PORTF_DATA_R & 0x01;
		if((input==0x01) && (previous == 0x00)){
			Sound_Play(C0);
		}
		else if((input==0x02) && (previous == 0x00)){
			Sound_Play(E);
		}
		else if((input==0x04) && (previous == 0x00)){
			Sound_Play(G);
		}
		else if((switchin == 0x01) && (prior == 0x00)){
			Instrument = (Instrument + 1)& 0x07;
		}
		else if(input==0x00){
			Sound_Play(0);
		}
	
		previous = input;
		prior = switchin;
		
		
		
		
		
  }         
}
//void GPIOPortE_Handler(void){    //for some reason I have two click the button twice to get it to produce a wave???
//	uint32_t input;
//	input = GPIO_PORTE_RIS_R + Piano_In();
//	
//	if(input == 0x02){ //interrupted on this bit and bit is high
//		Sound_Play(C0);						//play note
//		GPIO_PORTE_ICR_R = 0x01; //acknowledge interrupt
//	}
//	else if (input == 0x04){
//		Sound_Play(E);
//		GPIO_PORTE_ICR_R = 0x02;
//		}
//	else if(input == 0x08){
//		Sound_Play(G);
//		GPIO_PORTE_ICR_R = 0x04;
//		}
//	else{
//		Sound_Play(0);
//		GPIO_PORTE_ICR_R = 0x0F;
//	}
//}


//
//blank last line needed or errors thrown
