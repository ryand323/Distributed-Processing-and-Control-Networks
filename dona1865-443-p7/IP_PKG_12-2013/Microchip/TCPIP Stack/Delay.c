/*********************************************************************
 *
 *                  General Delay routines
 *
 *********************************************************************
 * FileName:        Delay.c
 * Dependencies:    Compiler.h
 * Processor:       PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip XC32 v1.20 or higher
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Richard Wall         12/4/13 Uses core timer
 ********************************************************************/
#define __DELAY_C

#include "TCPIP Stack/TCPIP.h"

void DelayMs(WORD ms)
{
volatile unsigned long tStart, tWait;

    tStart = ReadCoreTimer();
/* Core timer ticks per 10's of us */
    tWait = (GetInstructionClock() * ms ) / 1000;
    while(ReadCoreTimer()- tStart < tWait);
}

void Delay10us(DWORD dwCount)
{
volatile unsigned long tStart, tWait;

    tStart = ReadCoreTimer();
/* Core timer ticks per 10's of us */
    tWait = (GetInstructionClock() * dwCount ) / 100000;
    while(ReadCoreTimer() - tStart < tWait);
}
