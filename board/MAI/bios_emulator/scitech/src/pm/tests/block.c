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
* Description:  Test program for the PM_blockUntilTimeout function.
*
****************************************************************************/

#include <stdio.h>
#include "pmapi.h"

#define DELAY_MSECS 1100
#define LOOPS       5

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

int main(void)
{
    int i;

    printf("Detecting processor information ...");
    fflush(stdout);
    printf("\n\n%s\n", CPU_getProcessorName());
    ZTimerInit();
    LZTimerOn();
    for (i = 0; i < LOOPS; i++) {
	PM_blockUntilTimeout(DELAY_MSECS);
	ReportTime(LZTimerLap());
	}
    LZTimerOff();
    return 0;
}
