; IO.s
; Student names: Michael Griffin and Rohan Kondetimmanahalli
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120/TM4C123

; EE319K lab 7 device driver for the switch and LED.
; You are allowed to use any switch and any LED,
; although the Lab suggests the SW1 switch PF4 and Red LED PF1

; As part of Lab 7, students need to implement these three functions

; negative logic SW2 connected to PF0 on the Launchpad
; red LED connected to PF1 on the Launchpad
; blue LED connected to PF2 on the Launchpad
; green LED connected to PF3 on the Launchpad
; negative logic SW1 connected to PF4 on the Launchpad

        EXPORT   IO_Init
        EXPORT   IO_Touch
        EXPORT   IO_HeartBeat

GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_PUR_R   EQU 0x40025510
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_LOCK_R  EQU 0x40025520
GPIO_PORTF_CR_R    EQU 0x40025524
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
GPIO_LOCK_KEY      EQU 0x4C4F434B  ; Unlocks the GPIO_CR register
PF0       EQU 0x40025004
PF1       EQU 0x40025008
PF2       EQU 0x40025010
PF3       EQU 0x40025020
PF4       EQU 0x40025040
LEDS      EQU 0x40025038
RED       EQU 0x02
BLUE      EQU 0x04
GREEN     EQU 0x08
SWITCHES  EQU 0x40025044
SW1       EQU 0x10                 ; on the left side of the Launchpad board
SW2       EQU 0x01                 ; on the right side of the Launchpad board
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
    
        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB



;------------IO_Init------------
; Initialize GPIO Port for a switch and an LED
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_Init
    LDR	R1, =SYSCTL_RCGCGPIO_R
	LDR R0, [R1]
	ORR	R0, R0, #0x20
	STR	R0, [R1];turn on clock for port f
	NOP
	NOP;let clock stabilize
	LDR	R1, =GPIO_PORTF_DIR_R
	LDR	R0, [R1]
	ORR	R0, R0, #0x02
	BIC	R0, R0, #0x10
	STR	R0, [R1];PF1 is output, PF4 is input
	LDR	R1, =GPIO_PORTF_DEN_R
	LDR	R0, [R1]
	ORR	R0, R0, #0x12
	STR	R0, [R1];enable PF0,4
	LDR	R1, =GPIO_PORTF_LOCK_R
	LDR	R0, =GPIO_LOCK_KEY
	STR	R0, [R1];unlock port f
	LDR	R1, =GPIO_PORTF_CR_R
	LDR	R0, [R1]
	ORR	R0, R0, #0x12
	STR	R0, [R1];enable commit to port f
	LDR	R1, =GPIO_PORTF_PUR_R
	LDR	R0, [R1]
	ORR	R0, R0, #0x10
	STR	R0, [R1];enable pullup resistor for negative logic switch PF4
	BX  LR
;* * * * * * * * End of IO_Init * * * * * * * *

;------------IO_HeartBeat------------
; Toggle the output state of the LED.
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_HeartBeat
    LDR	R1, =PF1
	LDR	R0, [R1]
	EOR	R0, R0, #0x02;toggle PF1
	STR	R0, [R1]
    BX  LR                          ; return
;* * * * * * * * End of IO_HeartBeat * * * * * * * *

;------------IO_Touch------------
; First: wait for the of the release of the switch
; and then: wait for the touch of the switch
; Input: none
; Input: none
; Output: none
; This is a public function
; Invariables: This function must not permanently modify registers R4 to R11
IO_Touch
		LDR	R1, =PF4
		MOV	R2, #0x10
LOOP1	LDR	R0, [R1]
		CMP	R0, R2
		BNE	LOOP1;wait for switch to be off
		LDR	R0, =400000;branch takes 1 cycle
		;LDR R0, =266666; branch takes 2 cycles
LOOP2	SUBS R0, R0, #0x01
		BNE	LOOP2;wait 10ms between checks
		LDR	R0, [R1]
		CMP	R0, R2;wait for switch to be on
		BEQ	LOOP2
		BX  LR                          ; return
;* * * * * * * * End of IO_Touch * * * * * * * *

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file