/****************************************************************************
*
*                         Ultra Long Period Timer
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
* Environment:  Win32
*
* Description:  OS specific implementation for the Zen Timer functions.
*
****************************************************************************/

/*---------------------------- Global variables ---------------------------*/

static CPU_largeInteger countFreq;
static ibool            havePerformanceCounter;
static ulong            start,finish;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Initialise the Zen Timer module internals.
****************************************************************************/
void __ZTimerInit(void)
{
#ifdef  NO_ASSEMBLER
    havePerformanceCounter = false;
#else
    havePerformanceCounter = QueryPerformanceFrequency((LARGE_INTEGER*)&countFreq);
#endif
}

/****************************************************************************
REMARKS:
Start the Zen Timer counting.
****************************************************************************/
static void __LZTimerOn(
    LZTimerObject *tm)
{
    if (havePerformanceCounter)
	QueryPerformanceCounter((LARGE_INTEGER*)&tm->start);
    else
	tm->start.low = timeGetTime();
}

/****************************************************************************
REMARKS:
Compute the lap time since the timer was started.
****************************************************************************/
static ulong __LZTimerLap(
    LZTimerObject *tm)
{
    CPU_largeInteger    tmLap,tmCount;

    if (havePerformanceCounter) {
	QueryPerformanceCounter((LARGE_INTEGER*)&tmLap);
	_CPU_diffTime64(&tm->start,&tmLap,&tmCount);
	return _CPU_calcMicroSec(&tmCount,countFreq.low);
	}
    else {
	tmLap.low = timeGetTime();
	return (tmLap.low - tm->start.low) * 1000L;
	}
}

/****************************************************************************
REMARKS:
Stop the Zen Timer counting.
****************************************************************************/
static void __LZTimerOff(
    LZTimerObject *tm)
{
    if (havePerformanceCounter)
	QueryPerformanceCounter((LARGE_INTEGER*)&tm->end);
    else
	tm->end.low = timeGetTime();
}

/****************************************************************************
REMARKS:
Compute the elapsed time in microseconds between start and end timings.
****************************************************************************/
static ulong __LZTimerCount(
    LZTimerObject *tm)
{
    CPU_largeInteger    tmCount;

    if (havePerformanceCounter) {
	_CPU_diffTime64(&tm->start,&tm->end,&tmCount);
	return _CPU_calcMicroSec(&tmCount,countFreq.low);
	}
    else
	return (tm->end.low - tm->start.low) * 1000L;
}

/****************************************************************************
REMARKS:
Define the resolution of the long period timer as microseconds per timer tick.
****************************************************************************/
#define ULZTIMER_RESOLUTION     1000

/****************************************************************************
REMARKS:
Read the Long Period timer from the OS
****************************************************************************/
static ulong __ULZReadTime(void)
{ return timeGetTime(); }

/****************************************************************************
REMARKS:
Compute the elapsed time from the BIOS timer tick. Note that we check to see
whether a midnight boundary has passed, and if so adjust the finish time to
account for this. We cannot detect if more that one midnight boundary has
passed, so if this happens we will be generating erronous results.
****************************************************************************/
ulong __ULZElapsedTime(ulong start,ulong finish)
{ return finish - start; }
