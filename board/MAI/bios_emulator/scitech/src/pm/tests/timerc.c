/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Test program for the Zen Timer Library.
*
****************************************************************************/

#include <stdio.h>
#include "pmapi.h"
#include "ztimer.h"

#define DELAY_SECS  10

/*-------------------------- Implementation -------------------------------*/

/* The following routine takes a long count in microseconds and outputs
 * a string representing the count in seconds. It could be modified to
 * return a pointer to a static string representing the count rather
 * than printing it out.
 */

void ReportTime(ulong count)
{
    ulong   secs;

    secs = count / 1000000L;
    count = count - secs * 1000000L;
    printf("Time taken: %lu.%06lu seconds\n",secs,count);
}

int     i,j;                                /* NON register variables! */

int main(void)
{
#ifdef  LONG_TEST
    ulong   start,finish;
#endif

    printf("Processor type: %d %ld MHz\n", CPU_getProcessorType(), CPU_getProcessorSpeed(true));

    ZTimerInit();

    /* Test the long period Zen Timer (we don't check for overflow coz
     * it would take tooooo long!)
     */

    LZTimerOn();
    for (j = 0; j < 10; j++)
	for (i = 0; i < 20000; i++)
	    i = i;
    LZTimerOff();
    ReportTime(LZTimerCount());

    /* Test the ultra long period Zen Timer */
#ifdef LONG_TEST
    start = ULZReadTime();
    delay(DELAY_SECS * 1000);
    finish = ULZReadTime();
    printf("Delay of %d secs took %d 1/10ths of a second\n",
	DELAY_SECS,ULZElapsedTime(start,finish));
#endif

    return 0;
}
