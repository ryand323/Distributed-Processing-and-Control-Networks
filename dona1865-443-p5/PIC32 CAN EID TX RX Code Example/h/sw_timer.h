/* 	sw_timer.h - Software time include file
**
** Author:  Richard Wall
** Date:    September 30, 2011
** Revised: October 26, 2014 - Added delayUs and PeriodMs functions
*/
#ifndef __SW_TIMER__
    #define __SW_TIMER__
    #include <plib.h>

//core timer definitions
    #define TOGGLES_PER_MSEC	1000   	/* for milliseconds */
    #define CORE_TICKS_PER_MS	(GetSystemClock()/2/TOGGLES_PER_MSEC)


#endif

/* Function prototypes */

void DelayMs(unsigned int msec);
void DelayUs(unsigned int Usec);
int PeriodMs(unsigned int msec);

/* End of sw_timer.h */
