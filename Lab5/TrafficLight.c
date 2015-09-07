// ***** 0. Documentation Section *****
// TrafficLight.c for Lab 5
// Runs on LM4F120/TM4C123
// Implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// Feb 24, 2015

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"


//#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
//#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
//#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
	NVIC_ST_RELOAD_R = 0x00FFFFFF;
	NVIC_ST_CURRENT_R = 0;
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 10000us equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

#define LIGHT                   (*((volatile unsigned long *)0x400050FC)) // bits 0-5 Port B
#define BOARD   								(*((volatile unsigned long *)0x40025028)) // bits 0-2 Port E
#define SENSOR                  (*((volatile unsigned long *)0x4002401C)) // bits 1 and 3 Port F

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined

extern int PLL_Init(void);

struct State {
  unsigned long Out; 
	unsigned long Out2;
  unsigned long Time;  
  unsigned long Next[8];}; 
typedef const struct State STyp;
	
#define goN   0
#define waitNE 1
#define waitNP 2
#define goE   3
#define waitEN 4
#define waitEP 5
#define goP 6
#define waitOnN 7
#define waitOffN 8
#define waitOn2N 9
#define waitOff2N 10
#define waitOnE 11
#define waitOffE 12
#define waitOn2E 13
#define waitOff2E 14
	
	

STyp FSM[15]={
 {0xA1,0x02, 100,{goN,waitNE,goN,waitNE,waitNP,waitNP,waitNP,waitNP}}, 
 {0xA2,0x02, 50,{goE,goE,goE,goE,goE,goE,goE,goE}},
 {0xA2,0x02, 50,{goP,goP,goP,goP,goP,goP,goP,goP}},
 {0x8C,0x02, 100,{goE,goE,waitEN,waitEN,waitEP,waitEP,waitEP,waitEN}},
 {0x94,0x02, 50,{goN,goN,goN,goN,goN,goN,goN,goN}},
 {0x94,0x02, 50,{goP,goP,goP,goP,goP,goP,goP,goP}},
 {0x64,0x08, 100,{goP,waitOnE,waitOnN,waitOnN,goP,waitOnE,waitOnN,waitOnE}},
 {0x24,0x02, 25,{waitOffN,waitOffN,waitOffN,waitOffN,waitOffN,waitOffN,waitOffN,waitOffN}},
 {0x24,0x00, 25,{waitOn2N,waitOn2N,waitOn2N,waitOn2N,waitOn2N,waitOn2N,waitOn2N,waitOn2N}},
 {0x24,0x02, 25,{waitOff2N,waitOff2N,waitOff2N,waitOff2N,waitOff2N,waitOff2N,waitOff2N,waitOff2N}},
 {0x24,0x00, 25,{goN,goN,goN,goN,goN,goN,goN,goN}},
 {0xA4,0x02, 25,{waitOffE,waitOffE,waitOffE,waitOffE,waitOffE,waitOffE,waitOffE,waitOffE}},
 {0xA4,0x00, 25,{waitOn2E,waitOn2E,waitOn2E,waitOn2E,waitOn2E,waitOn2E,waitOn2E,waitOn2E}},
 {0xA4,0x02, 25,{waitOff2E,waitOff2E,waitOff2E,waitOff2E,waitOff2E,waitOff2E,waitOff2E,waitOff2E}},
 {0xA4,0x00, 25,{goE,goE,goE,goE,goE,goE,goE,goE}}};

// ***** 3. Subroutines Section *****
 
volatile unsigned long delay;
unsigned long S;  // index to the current state 
unsigned long Input;
 
int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
	SysTick_Init();
	PLL_Init();
  SYSCTL_RCGC2_R |= 0x32;      // 1) B E F
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock
  GPIO_PORTE_DIR_R &= ~0x07;   // 5) inputs on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;    // 7) enable digital on PE2-0
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
  GPIO_PORTF_DIR_R |= 0x0A;    // 5) outputs on PBF1,3
  GPIO_PORTF_DEN_R |= 0x0A;    // 7) enable digital on PF1,3
	
  S = goP;  
 
  while(1){
		LIGHT = FSM[S].Out & 0x3F ;  // set lights on protoboard
		BOARD = FSM[S].Out2; // set lights on microcontroller
    SysTick_Wait10ms(FSM[S].Time);
    Input = SENSOR;     // read sensors
    S = FSM[S].Next[Input]; // switch state
  }
}

