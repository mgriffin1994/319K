// UARTIntsTestMain.c
// Runs on LM4F120/TM4C123
// Tests the UART0 to implement bidirectional data transfer to and from a
// computer running HyperTerminal.  This time, interrupts and FIFOs
// are used.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 5.11 Section 5.6, Program 3.10

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include <stdint.h>
#include "PLL.h"
#include "UART.h"
#include "ADC.h"
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "LCD.h"
#include "FIFO.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
// Initialize Port F so PF1, PF2 and PF3 are heartbeats

uint32_t TxCounter;

void PortF_Init(void){
	volatile uint32_t wait;
	SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate clock for Port F
	while((SYSCTL_PRGPIO_R&0x20) == 0){};  		// allow time for clock to start
	wait = SYSCTL_RCGCGPIO_R;
	wait = SYSCTL_RCGCGPIO_R;					//extra delay time
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
	GPIO_PORTF_CR_R = 0x0E;           // allow changes to PF4-0
	GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
	GPIO_PORTF_PCTL_R = 0x00;   			// 4) PCTL GPIO on PF4-0
	GPIO_PORTF_DEN_R |= 0x0E;					//enable pins 
	GPIO_PORTF_DIR_R |= 0x0E;					//output pins
	GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
}

void SysTick_Init(uint32_t period){
	long sr;
	sr = StartCritical();
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2          
	NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
  // enable interrupts after all initialization is finished
	EndCritical(sr);
}

uint32_t ConvertFix(uint32_t input){
  return ((6619*input + 2675343)/16384);	//return fixed point number e.g. 1500
}

void SysTick_Handler(void){
	uint32_t voltage;
	uint32_t fixed;
	PF2 ^= 0x04;      // Heartbeat
	voltage = ADC_In();		//read ADC
	PF2 ^= 0x04;      // Heartbeat
	fixed = ConvertFix(voltage);
	char int1 = (char)(((fixed/1000)+0x30)&0xFF);		//convert to form 1.500, where each number is a char
	char d1 = (char)((((fixed%1000)/100)+0x30)&0xFF);
	char d2 = (char)((((fixed%100)/10+0x30))&0xFF);
	char d3 = (char)(((fixed%10)+0x30)&0xFF);  //changed to char type, and added &0xFF
	UART_OutChar(STX);			//output to H/W TxFIFO STX, 1.500, CR, ETX
	UART_OutChar(int1);
	UART_OutChar('.');
	UART_OutChar(d1);
	UART_OutChar(d2);
	UART_OutChar(d3);
	UART_OutChar(CR);
	UART_OutChar(ETX);
	TxCounter++;
	PF2 ^= 0x04;      // Heartbeat
}
	

//int main2(void){
//	DisableInterrupts();
//	PLL_Init();
//	SysTick_Init(1250000);
//	ADC_Init();
//	UART_Init();
//	PortF_Init();
//	ST7735_InitR(INITR_REDTAB); 
//	EnableInterrupts();
//	while(1){
//	}
//}
int main(void){
		char FifoSize;
		char Message[8];
		int i = 0;
		DisableInterrupts();  
		PLL_Init();												//initialize to 80 MHz
		PortF_Init(); 										//setup heartbeat pins
		ADC_Init();												// turn on ADC, set channel to 1    
		ST7735_InitR(INITR_REDTAB); 			//turn on screen
		SysTick_Init(2000000);						//systick operates at 40 Hz
		UART_Init();											//turn on UART1 with interrupts when RxFIFO half full
		Fifo_Init();											//setup S/W FIFO
		EnableInterrupts();
		while(1){
			FifoSize = Fifo_Size();
			ST7735_SetCursor(0,0);
			if(FifoSize != 0){							//S/W FIFO not empty
				for(i = 0; i < 8; i++){				//read RxFIFO 8 times and output to Message array
					Fifo_Get(&Message[i]);
				} 
				if (Message[0] == STX){  //check for valid message format, added
					for(i = 0; i < 8; i++){
						if (((Message[i] >= '0') && (Message[i] <= '9')) || (Message[i] == '.')){  //changed from not STX and not CR and not ETX
							ST7735_OutChar(Message[i]);
						}
					}					
					ST7735_OutString(" cm");																														//output units to screen
				}
			}
		}
}
//int Status[20];
//char GetData[10];

//int main3(void){
//	Fifo_Init();
//	for(;;){
//		Status[0]  = Fifo_Get(&GetData[0]);  // should fail,    empty
//    Status[1]  = Fifo_Put(1);            // should succeed, 1 
//    Status[2]  = Fifo_Put(2);            // should succeed, 1 2
//    Status[3]  = Fifo_Put(3);            // should succeed, 1 2 3
//    Status[4]  = Fifo_Put(4);            // should succeed, 1 2 3 4
//    Status[5]  = Fifo_Put(5);            // should succeed, 1 2 3 4 5
//    Status[6]  = Fifo_Put(6);            // should succeed, 1 2 3 4 5 6
//    Status[7]  = Fifo_Put(7);            // should fail,    1 2 3 4 5 6 
//    Status[8]  = Fifo_Get(&GetData[1]);  // should succeed, 2 3 4 5 6
//    Status[9]  = Fifo_Get(&GetData[2]);  // should succeed, 3 4 5 6
//    Status[10] = Fifo_Put(7);            // should succeed, 3 4 5 6 7
//    Status[11] = Fifo_Put(8);            // should succeed, 3 4 5 6 7 8
//    Status[12] = Fifo_Put(9);            // should fail,    3 4 5 6 7 8 
//    Status[13] = Fifo_Get(&GetData[3]);  // should succeed, 4 5 6 7 8
//    Status[14] = Fifo_Get(&GetData[4]);  // should succeed, 5 6 7 8
//    Status[15] = Fifo_Get(&GetData[5]);  // should succeed, 6 7 8
//    Status[16] = Fifo_Get(&GetData[6]);  // should succeed, 7 8
//    Status[17] = Fifo_Get(&GetData[7]);  // should succeed, 8
//    Status[18] = Fifo_Get(&GetData[8]);  // should succeed, empty
//    Status[19] = Fifo_Get(&GetData[9]);  // should fail,    empty
//	}
//}

