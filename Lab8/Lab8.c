// Lab8.c
// Runs on LM4F120 or TM4C123
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
// Last Modified: 3/6/2015 

// Analog Input connected to PE2=ADC1
// displays on Sitronox ST7735
// PF3, PF2, PF1 are heartbeats

#include <stdint.h>

#include "ST7735.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "tm4c123gh6pm.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on Nokia
// main3 adds your convert function, position data is no Nokia

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value


#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
// Initialize Port F so PF1, PF2 and PF3 are heartbeats

uint16_t ADCMail;
uint8_t ADCStatus;

uint32_t wait;

void PortF_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate clock for Port F
	while((SYSCTL_PRGPIO_R&0x20) == 0){};  		// allow time for clock to start
	wait = SYSCTL_RCGCGPIO_R;
	wait = SYSCTL_RCGCGPIO_R;					//extra delay time
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
	GPIO_PORTF_CR_R = 0x0E;           // allow changes to PF4-0
	GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
	GPIO_PORTF_PCTL_R = 0x00;   // 4) PCTL GPIO on PF4-0
	GPIO_PORTF_DEN_R |= 0x0E;
	GPIO_PORTF_DIR_R |= 0x0E;
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

void SysTick_Handler(void){
	PF2 ^= 0x04;      // Heartbeat
	PF2 ^= 0x04;      // Heartbeat
	ADCMail = ADC_In();
	ADCStatus = 1;
	PF2 ^= 0x04;      // Heartbeat
}

uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
int main1(void){      // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 1
  }
}

int main2(void){
       // Bus clock is 80 MHz 
  ADC_Init();         // turn on ADC, set channel to 1
	DisableInterrupts();
  PortF_Init();
	ST7735_InitR(INITR_REDTAB); 
  while(1){           // use scope to measure execution time for ADC_In and LCD_OutDec           
    PF2 = 0x04;       // Profile ADC
    Data = ADC_In();  // sample 12-bit channel 1
    PF2 = 0x00;       // end of ADC Profile
    ST7735_SetCursor(0,0);
    PF1 = 0x02;       // Profile LCD
    LCD_OutDec(Data); 
    ST7735_OutString("    ");  // these spaces are used to coverup characters from last output
    PF1 = 0;          // end of LCD Profile
  }
}

uint32_t Convert(uint32_t input){
  return ((6619*input + 2675343)/16384);
}
int main3(void){ 
           // Bus clock is 80 MHz 
	ADC_Init();			// turn on ADC, set channel to 1
	DisableInterrupts();
  PortF_Init();     
	ST7735_InitR(INITR_REDTAB); 
  while(1){  
    PF2 ^= 0x04;      // Heartbeat
    Data = ADC_In();  // sample 12-bit channel 1
    PF3 = 0x08;       // Profile Convert
    Position = Convert(Data); 
    PF3 = 0;          // end of Convert Profile
    PF1 = 0x02;       // Profile LCD
    ST7735_SetCursor(0,0);
    LCD_OutDec(Data); ST7735_OutString("    "); 
    ST7735_SetCursor(6,0);
    LCD_OutFix(Position);
    PF1 = 0;          // end of LCD Profile
  }
}   
int main(void){
	TExaS_Init();
	//main3();
	PortF_Init(); 
	ADC_Init();			// turn on ADC, set channel to 1
	DisableInterrupts();    
	ST7735_InitR(INITR_REDTAB); 
	SysTick_Init(2000000);
	EnableInterrupts();
  while(1){
		if(ADCStatus == 1){
			ADCStatus = 0;
			ST7735_SetCursor(0,0);
			LCD_OutFix(Convert(ADCMail));
			ST7735_OutString("  cm"); 
		}
  }
}

