// Piano.c
// This software configures the off-board piano keys
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
#include "tm4c123gh6pm.h"

// **************Piano_Init*********************
// Initialize piano key inputs, called once 
// Input: none 
// Output: none
void Piano_Init(void){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // activate port E
  delay = SYSCTL_RCGC2_R;    // allow time to finish activating
  GPIO_PORTE_AMSEL_R &= ~0x07;      // no analog
  GPIO_PORTE_PCTL_R &= ~0x00000FFF; // regular GPIO function
  GPIO_PORTE_DIR_R &= ~0x07;      // make PE3-2 in
  GPIO_PORTE_AFSEL_R &= ~0x07;   // disable alt funct on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;      // enable digital I/O on PE2-0
//	GPIO_PORTE_IS_R &= ~0x07;			//porte edge-sensitive
//	GPIO_PORTE_IBE_R |= 0x07;			//both edges		
//	GPIO_PORTE_ICR_R = 0x07;
//	GPIO_PORTE_IM_R |= 0x07;			//arm interrupt for PE0-2
//	NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF00)|0x00000040;	//priority 2
	
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // activate port F
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTF_LOCK_R = 0x4C4F434B;		//unlock Port F
	GPIO_PORTF_CR_R |= 0x15;					//allow changes in PF0,2,4
	GPIO_PORTF_AMSEL_R &= ~0x15;      // no analog
  GPIO_PORTF_PCTL_R &= ~0x00000FFF; // regular GPIO function
  GPIO_PORTF_DIR_R &= ~0x11;      // make PF0,4 in
	GPIO_PORTF_DIR_R |= 0x04;				//make PF2 out
  GPIO_PORTF_AFSEL_R &= ~0x15;   // disable alt funct on PF0,2,4
  GPIO_PORTF_DEN_R |= 0x15;      // enable digital I/O on PF0,2,4
	GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // PF4,0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,0 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    //     PF4,0 rising edge event
	GPIO_PORTF_ICR_R = 0x11;			//clear flag 0,4
	GPIO_PORTF_IM_R |= 0x11;      // arm interrupt on PF4,0
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;	//priority 5
//	NVIC_EN0_R = 0x40000010;		//enable interrupts on port e and f
	
} //

// **************Piano_In*********************
// Input from piano key inputs 
// Input: none 
// Output: 0 to 15 depending on keys
// 0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2, 0x08 is just Key3
uint32_t Piano_In(void){
  return(GPIO_PORTE_DATA_R);
}
