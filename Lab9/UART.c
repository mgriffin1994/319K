// UART.c
// Runs on LM3S811, LM3S1968, LM3S8962, LM4F120, TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// May 30, 2014
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Program 4.12, Section 4.9.4, Figures 4.26 and 4.40

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
#include "UART.h"
#include "tm4c123gh6pm.h"
#include "FIFO.h"

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable

unsigned long Error;
unsigned long RxCounter;

void UART1_Handler(void){
	PF1 ^= 0x02;      // Heartbeat
	PF1 ^= 0x02;      // Heartbeat
	char data;
	int x;
	while((UART1_FR_R&0x10)==0){		//while H/W RxFIFO is not empty
		data = UART_InChar();					//read data from RxFIFO
		x = Fifo_Put(data);						//put data in S/W FIFO
		if (x == 0){									//FIFO full
			Error++;
		}
		RxCounter++;								
	}
		UART1_ICR_R = 0x10;						//acknowledge
		PF1 ^= 0x02;      // Heartbeat
}
//------------UART_Init------------
// Initialize the UART for 100,000 baud rate (assuming 80 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
	volatile uint32_t wait;
  SYSCTL_RCGCUART_R |= 0x02;            // activate UART1
	wait = SYSCTL_RCGCUART_R;
	wait = SYSCTL_RCGCUART_R;
	wait = SYSCTL_RCGCUART_R;
	wait = SYSCTL_RCGCUART_R;
  SYSCTL_RCGCGPIO_R |= 0x04;            // activate port C
  while((SYSCTL_PRGPIO_R&0x04) == 0){};
	wait = SYSCTL_RCGCGPIO_R;
	wait = SYSCTL_RCGCGPIO_R;
  UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART1_IBRD_R = 50;                    // IBRD = int(80,000,000 / (16 * 100,000)) = int(50)
  UART1_FBRD_R = 0;                     // FBRD = int(0.1267 * 64 + 0.5) = 0
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R |= (UART_LCRH_WLEN_8|UART_LCRH_FEN);									//8 bit words, H/W FIFO enabled
	UART1_IFLS_R |= 0x10;									//half full
	UART1_IM_R |= 0x10;										//enable interrupts
  UART1_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTC_AFSEL_R |= 0x30;           // enable alt funct on PC4-5
  GPIO_PORTC_DEN_R |= 0x30;             // enable digital I/O on PC4-5
                                        // configure PC4-5 as UART
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;          // disable analog functionality on PC
	NVIC_PRI1_R = (NVIC_PRI1_R&0xFF0FFFFF)|0x00400000;
	NVIC_EN0_R = 0x40;
	Error = 0;
	RxCounter = 0;
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0);  //wait for RxFIFO to have data in it
  return((char)(UART1_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UART1_FR_R&UART_FR_TXFF) != 0);				//wait for TxFIFO to be not full
  UART1_DR_R = data;
}
