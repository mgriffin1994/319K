; Print.s
; Student names: Michael Griffin and Rohan Kondetimmanahalli
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
	

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
	


;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
NUM	EQU	0

LCD_OutDec
		
		PUSH {R0, LR};allocate num
		MOV	R2, #10
		CMP	R0, R2
		BLO	BASE;is num < 10?
		LDR	R1, [SP, #NUM]
		MOV	R2, #10
		UDIV R0, R1, R2
		BL	LCD_OutDec; LCD_OutDec(num/10)
		LDR R1, [SP, #NUM]
		MOV	R2, #10
		UDIV R0, R1, R2
		MLS	R0, R2, R0, R1;num%10
BASE	ADD	R0, #0x30;convert to ascii
		BL	ST7735_OutChar;output single number less than 10
DONE	POP {R0, PC};deallocate and leave	
		
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
FIX EQU 0
STAR DCB "*.***",0

LCD_OutFix
		PUSH {R0, LR};allocate
		LDR	R1, =9999
		CMP	R0, R1
		BHI	INVALID;is fix > 9999?
		MOV	R2, #1000
		UDIV R0, R2
		BL	LCD_OutDec;output first number
		MOV	R0, #0x2E
		BL	ST7735_OutChar;output decimal
		MOV	R2, #1000
		MOV	R3, #100
		BL	OUTNUM;output tens place
		MOV	R2, #100
		MOV	R3, #10
		BL	OUTNUM;output hundreds place
		MOV	R2, #10
		MOV	R3, #1
		BL	OUTNUM;output thousands place
		POP	{R0, PC};deallocate and return to main
	
OUTNUM	LDR	R1, [SP, #FIX]
		UDIV R0, R1, R2
		MLS	R0, R2, R0, R1;fix%R2
		UDIV R0, R3;(fix%R2)/R3
		ADD	R0, #0x30;convert to ascii
		PUSH {R0, LR};save position
		BL	ST7735_OutChar;output character
		POP	{R0, PC};return to LCD_OutFix
		
INVALID	LDR	R0, =STAR
		BL	ST7735_OutString;output *.***
		POP	{R0, PC};return to main
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
