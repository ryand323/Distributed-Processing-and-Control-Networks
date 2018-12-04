/*  sw_timer.c
 *
 *  Author: Richard Wall
 *  Date:   September 30, 2011
 *
 *  Software polling millisecond time delay using the core timer
 *
 *  Revised: October 26, 2014 - Added microsecond delay and period delay
*/

#include <plib.h>

#include "chipKIT_Pro_MX7.h"   // Provides core clock rate
#include "sw_timer.h"

/* DelayMs Function Description ***********************************************
SYNTAX:         void DelayMs(unsigned int msec);
PARAMETER1:     msec - the number of milliseconds to delay
KEYWORDS:       delay, ms, milliseconds, software delay, core timer
DESCRIPTION:    This is a millisecond delay function that will repeat a
                specified number of times.  It also has nodebug active to
                increase accuracy of measurement.
RETURN VALUE:   None
Notes:          None
END DESCRIPTION ************************************************************/

void DelayMs(unsigned int msec)
{
unsigned int tWait;

    tWait = ReadCoreTimer() + (CORE_TICKS_PER_MS * msec);
    while(ReadCoreTimer() < tWait); // wait for the time to pass
}/* End of DelayMs */

/* DelayUs Function Description ***********************************************
SYNTAX:         void DelayUs(unsigned int msec);
PARAMETER1:     usec - the number of microeconds to delay
KEYWORDS:       delay, us, microseconds, software delay, core timer
DESCRIPTION:    This is a microsecond delay function that will repeat a
                specified number of times.  It also has nodebug active to
                increase accuracy of measurement.
RETURN VALUE:   None
Notes:          Requires a system frequency greater than 1MHz
END DESCRIPTION ************************************************************/

void DelayUs(unsigned int Usec)
{
unsigned int tWait;

    tWait = ReadCoreTimer() + ((CORE_TICKS_PER_MS/1000) * Usec);
    while(ReadCoreTimer() < tWait); // wait for the time to pass
}/* End of DelayUs */

/* PeriodMs Function Description *********************************************
 * SYNTAX:          int PeriodMs(unsigned int msec);
 * KEYWORDS:        delay, ms, milliseconds, software delay, core timer,
 *                  non-blocking
 * DESCRIPTION:     This is a non-blocking millisecond delay function that
 *                  can be use for periodic activation of code.
 * PARAMETER1:      int msec - the number of milliseconds to delay
 * RETURN VALUE:    integer - delay in progress TURE / FALSE
 * Notes:           Calling this function with a zero time parameter can be
 *                  used to see if this function is currently in use.  A TRUE
 *                  is returned if a period timing is in progress.
 * END DESCRIPTION ************************************************************/

int PeriodMs(unsigned int msec)
{
static int delay_in_progross_flag = FALSE;
static unsigned int tWait, tStart;

    if(delay_in_progross_flag == 0)             /* New Period */
    {
	tStart = ReadCoreTimer();               /* Capture starting time */
        tWait = (CORE_TICKS_PER_MS * msec);     /* Compute delay period */
        delay_in_progross_flag = TRUE;          /* Set operation flag */
    }
    if((ReadCoreTimer()-tStart) >= tWait) /* Check if period is over */
        delay_in_progross_flag = FALSE;
    return delay_in_progross_flag;
}/* End of PeriodMs */

/* End od sw_timer.c */