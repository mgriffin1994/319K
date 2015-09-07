// FIFO.c
// Runs on any LM3Sxxx
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Daniel Valvano
// June 16, 2011

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to the Arm Cortex M3",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2011
   Programs 3.7, 3.8., 3.9 and 3.10 in Section 3.7

 Copyright 2011 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

#include "FIFO.h"

// Two-index implementation of the transmit FIFO
// can hold 0 to FIFOSIZE elements
char volatile PutI;// put next
char volatile GetI;// get next
char static Fifo[FIFOSIZE];

// initialize index FIFO
void Fifo_Init(void){ long sr;
  sr = StartCritical(); // make atomic
  PutI = GetI = 0;  // Empty
  EndCritical(sr);
}
// add element to end of index FIFO
// return FIFOSUCCESS if successful
int Fifo_Put(char data){
  if(((PutI+1)%FIFOSIZE) == GetI){
    return(FIFOFAIL); // Failed, fifo full
  }
  Fifo[PutI] = data; // put
  PutI = (PutI+1)%FIFOSIZE;  // Success, update
  return(FIFOSUCCESS);
}
// remove element from front of index FIFO
// return FIFOSUCCESS if successful
int Fifo_Get(char *datapt){
  if(PutI == GetI ){
    return(FIFOFAIL); // Empty if PutI=GetI
  }
  *datapt = Fifo[GetI];
  GetI = (GetI+1)%FIFOSIZE;  // Success, update
  return(FIFOSUCCESS);
}
// number of elements in index FIFO
// 0 to FIFOSIZE-1
char Fifo_Size(void){
  if(PutI < GetI){
    return (PutI-GetI+FIFOSIZE);
  }
  return (PutI-GetI);
}


//// Two-pointer implementation of the receive FIFO
//// can hold 0 to FIFOSIZE-1 elements

//char volatile *PutPt; // put next
//char volatile *GetPt; // get next
//char static Fifo[FIFOSIZE];

//// initialize pointer FIFO
//void Fifo_Init(void){ long sr;
//  sr = StartCritical();      // make atomic
//  PutPt = GetPt = &Fifo[0]; // Empty
//  EndCritical(sr);
//}
//// add element to end of pointer FIFO
//// return RXFIFOSUCCESS if successful
//int Fifo_Put(char data){
//char volatile *nextPutPt;
//  nextPutPt = PutPt+1;
//  if(nextPutPt == &Fifo[FIFOSIZE]){
//    nextPutPt = &Fifo[0];  // wrap
//  }
//  if(nextPutPt == GetPt){
//    return(FIFOFAIL);      // Failed, fifo full
//  }
//  else{
//    *(PutPt) = data;       // Put
//    PutPt = nextPutPt;     // Success, update
//    return(FIFOSUCCESS);
//  }
//}
//// remove element from front of pointer FIFO
//// return RXFIFOSUCCESS if successful
//int Fifo_Get(char *datapt){
//  if(PutPt == GetPt ){
//    return(FIFOFAIL);      // Empty if PutPt=GetPt
//  }
//  *datapt = *(GetPt++);
//  if(GetPt == &Fifo[FIFOSIZE]){
//     GetPt = &Fifo[0];   // wrap
//  }
//  return(FIFOSUCCESS);
//}
//// number of elements in pointer FIFO
//// 0 to RXFIFOSIZE-1
//unsigned short Fifo_Size(void){
//  if(PutPt < GetPt){
//    return ((unsigned short)(PutPt-GetPt+FIFOSIZE));
//  }
//  return ((unsigned short)(PutPt-GetPt));
//}
