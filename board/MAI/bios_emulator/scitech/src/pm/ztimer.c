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
* Description:  Module to implement high precision timing on each OS.
*
****************************************************************************/

#include "ztimer.h"
#include "pmapi.h"
#include "oshdr.h"

/*---------------------------- Global variables ---------------------------*/

static LZTimerObject    LZTimer;
static ulong            start,finish;
#ifdef  __INTEL__
static long             cpuSpeed = -1;
static ibool            haveRDTSC = false;
#endif

/*----------------------------- Implementation ----------------------------*/

/* External Intel assembler functions */
#ifdef  __INTEL__
/* {secret} */
void  _ASMAPI _CPU_readTimeStamp(CPU_largeInteger *time);
/* {secret} */
ulong _ASMAPI _CPU_diffTime64(CPU_largeInteger *t1,CPU_largeInteger *t2,CPU_largeInteger *t);
/* {secret} */
ulong _ASMAPI _CPU_calcMicroSec(CPU_largeInteger *count,ulong freq);
#endif

#if     defined(__SMX32__)
#include "smx/ztimer.c"
#elif   defined(__RTTARGET__)
#include "rttarget/ztimer.c"
#elif   defined(__REALDOS__)
#include "dos/ztimer.c"
#elif   defined(__NT_DRIVER__)
#include "ntdrv/ztimer.c"
#elif   defined(__WIN32_VXD__)
#include "vxd/ztimer.c"
#elif   defined(__WINDOWS32__)
#include "win32/ztimer.c"
#elif   defined(__OS2_VDD__)
#include "vdd/ztimer.c"
#elif   defined(__OS2__)
#include "os2/ztimer.c"
#elif   defined(__LINUX__)
#include "linux/ztimer.c"
#elif   defined(__QNX__)
#include "qnx/ztimer.c"
#elif   defined(__BEOS__)
#include "beos/ztimer.c"
#else
#error  Timer library not ported to this platform yet!
#endif

/*------------------------ Public interface routines ----------------------*/

/****************************************************************************
DESCRIPTION:
Initializes the Zen Timer library (extended)

PARAMETERS:
accurate    - True of the speed should be measured accurately

HEADER:
ztimer.h

REMARKS:
This function initializes the Zen Timer library, and /must/ be called before
any of the remaining Zen Timer library functions are called. The accurate
parameter is used to determine whether highly accurate timing should be
used or not. If high accuracy is needed, more time is spent profiling the
actual speed of the CPU so that we can obtain highly accurate timing
results, but the time spent in the initialisation routine will be
significantly longer (on the order of 5 seconds).
****************************************************************************/
void ZAPI ZTimerInitExt(
    ibool accurate)
{
    if (cpuSpeed == -1) {
	__ZTimerInit();
#ifdef  __INTEL__
	cpuSpeed = CPU_getProcessorSpeedInHZ(accurate);
	haveRDTSC = CPU_haveRDTSC() && (cpuSpeed > 0);
#endif
	}
}

/****************************************************************************
DESCRIPTION:
Initializes the Zen Timer library.

HEADER:
ztimer.h

REMARKS:
This function initializes the Zen Timer library, and /must/ be called before
any of the remaining Zen Timer library functions are called.
****************************************************************************/
void ZAPI ZTimerInit(void)
{
    ZTimerInitExt(false);
}

/****************************************************************************
DESCRIPTION:
Starts the Long Period Zen Timer counting.

HEADER:
ztimer.h

PARAMETERS:
tm  - Timer object to start timing with

REMARKS:
Starts the Long Period Zen Timer counting. Once you have started the timer,
you can stop it with LZTimerOff or you can latch the current count with
LZTimerLap.

The Long Period Zen Timer uses a number of different high precision timing
mechanisms to obtain microsecond accurate timings results whenever possible.
The following different techniques are used depending on the operating
system, runtime environment and CPU on the target machine. If the target
system has a Pentium CPU installed which supports the Read Time Stamp
Counter instruction (RDTSC), the Zen Timer library will use this to
obtain the maximum timing precision available.

Under 32-bit Windows, if the Pentium RDTSC instruction is not available, we
first try to use the Win32 QueryPerformanceCounter API, and if that is not
available we fall back on the timeGetTime API which is always supported.

Under 32-bit DOS, if the Pentium RDTSC instruction is not available, we
then do all timing using the old style 8253 timer chip. The 8253 timer
routines provide highly accurate timings results in pure DOS mode, however
in a DOS box under Windows or other Operating Systems the virtualization
of the timer can produce inaccurate results.

Note: Because the Long Period Zen Timer stores the results in a 32-bit
      unsigned integer, you can only time periods of up to 2^32 microseconds,
      or about 1hr 20mins. For timing longer periods use the Ultra Long
      Period Zen Timer.

SEE ALSO:
LZTimerOff, LZTimerLap, LZTimerCount
****************************************************************************/
void ZAPI LZTimerOnExt(
    LZTimerObject *tm)
{
#ifdef  __INTEL__
    if (haveRDTSC) {
	_CPU_readTimeStamp(&tm->start);
	}
    else
#endif
	__LZTimerOn(tm);
}

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer and keeps it
running.

HEADER:
ztimer.h

PARAMETERS:
tm  - Timer object to do lap timing with

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Returns the current count that has elapsed since the last call to
LZTimerOn in microseconds. The time continues to run after this function is
called so you can call this function repeatedly.

SEE ALSO:
LZTimerOn, LZTimerOff, LZTimerCount
****************************************************************************/
ulong ZAPI LZTimerLapExt(
    LZTimerObject *tm)
{
#ifdef  __INTEL__
    CPU_largeInteger    tmLap,tmCount;

    if (haveRDTSC) {
	_CPU_readTimeStamp(&tmLap);
	_CPU_diffTime64(&tm->start,&tmLap,&tmCount);
	return _CPU_calcMicroSec(&tmCount,cpuSpeed);
	}
    else
#endif
	return __LZTimerLap(tm);
}

/****************************************************************************
DESCRIPTION:
Stops the Long Period Zen Timer counting.

HEADER:
ztimer.h

PARAMETERS:
tm  - Timer object to stop timing with

REMARKS:
Stops the Long Period Zen Timer counting and latches the count. Once you
have stopped the timer you can read the count with LZTimerCount. If you need
highly accurate timing, you should use the on and off functions rather than
the lap function since the lap function does not subtract the overhead of
the function calls from the timed count.

SEE ALSO:
LZTimerOn, LZTimerLap, LZTimerCount
****************************************************************************/
void ZAPI LZTimerOffExt(
    LZTimerObject *tm)
{
#ifdef  __INTEL__
    if (haveRDTSC) {
	_CPU_readTimeStamp(&tm->end);
	}
    else
#endif
	__LZTimerOff(tm);
}

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer.

HEADER:
ztimer.h

PARAMETERS:
tm  - Timer object to compute the elapsed time with.

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Returns the current count that has elapsed between calls to
LZTimerOn and LZTimerOff in microseconds.

SEE ALSO:
LZTimerOn, LZTimerOff, LZTimerLap
****************************************************************************/
ulong ZAPI LZTimerCountExt(
    LZTimerObject *tm)
{
#ifdef  __INTEL__
    CPU_largeInteger    tmCount;

    if (haveRDTSC) {
	_CPU_diffTime64(&tm->start,&tm->end,&tmCount);
	return _CPU_calcMicroSec(&tmCount,cpuSpeed);
	}
    else
#endif
	return __LZTimerCount(tm);
}

/****************************************************************************
DESCRIPTION:
Starts the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Obsolete function. You should use the LZTimerOnExt function instead
which allows for multiple timers running at the same time.
****************************************************************************/
void ZAPI LZTimerOn(void)
{ LZTimerOnExt(&LZTimer); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer and keeps it
running.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Obsolete function. You should use the LZTimerLapExt function instead
which allows for multiple timers running at the same time.
****************************************************************************/
ulong ZAPI LZTimerLap(void)
{ return LZTimerLapExt(&LZTimer); }

/****************************************************************************
DESCRIPTION:
Stops the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Obsolete function. You should use the LZTimerOffExt function instead
which allows for multiple timers running at the same time.
****************************************************************************/
void ZAPI LZTimerOff(void)
{ LZTimerOffExt(&LZTimer); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Obsolete function. You should use the LZTimerCountExt function instead
which allows for multiple timers running at the same time.
****************************************************************************/
ulong ZAPI LZTimerCount(void)
{ return LZTimerCountExt(&LZTimer); }

/****************************************************************************
DESCRIPTION:
Starts the Ultra Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Starts the Ultra Long Period Zen Timer counting. Once you have started the
timer, you can stop it with ULZTimerOff or you can latch the current count
with ULZTimerLap.

The Ultra Long Period Zen Timer uses the available operating system services
to obtain accurate timings results with as much precision as the operating
system provides, but with enough granularity to time longer periods of
time than the Long Period Zen Timer. Note that the resolution of the timer
ticks is not constant between different platforms, and you should use the
ULZTimerResolution function to determine the number of seconds in a single
tick of the timer, and use this to convert the timer counts to seconds.

Under 32-bit Windows, we use the timeGetTime function which provides a
resolution of 1 millisecond (0.001 of a second). Given that the timer
count is returned as an unsigned 32-bit integer, this we can time intervals
that are a maximum of 2^32 milliseconds in length (or about 1,200 hours or
50 days!).

Under 32-bit DOS, we use the system timer tick which runs at 18.2 times per
second. Given that the timer count is returned as an unsigned 32-bit integer,
this we can time intervals that are a maximum of 2^32 * (1/18.2) in length
(or about 65,550 hours or 2731 days!).

SEE ALSO:
ULZTimerOff, ULZTimerLap, ULZTimerCount, ULZElapsedTime, ULZReadTime
****************************************************************************/
void ZAPI ULZTimerOn(void)
{ start = __ULZReadTime(); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Ultra Long Period Zen Timer and keeps it
running.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in resolution counts.

REMARKS:
Returns the current count that has elapsed since the last call to
ULZTimerOn in microseconds. The time continues to run after this function is
called so you can call this function repeatedly.

SEE ALSO:
ULZTimerOn, ULZTimerOff, ULZTimerCount
****************************************************************************/
ulong ZAPI ULZTimerLap(void)
{ return (__ULZReadTime() - start); }

/****************************************************************************
DESCRIPTION:
Stops the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Stops the Ultra Long Period Zen Timer counting and latches the count. Once
you have stopped the timer you can read the count with ULZTimerCount.

SEE ALSO:
ULZTimerOn, ULZTimerLap, ULZTimerCount
****************************************************************************/
void ZAPI ULZTimerOff(void)
{ finish = __ULZReadTime(); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in resolution counts.

REMARKS:
Returns the current count that has elapsed between calls to
ULZTimerOn and ULZTimerOff in resolution counts.

SEE ALSO:
ULZTimerOn, ULZTimerOff, ULZTimerLap, ULZTimerResolution
****************************************************************************/
ulong ZAPI ULZTimerCount(void)
{ return (finish - start); }

/****************************************************************************
DESCRIPTION:
Reads the current time from the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Current timer value in resolution counts.

REMARKS:
Reads the current Ultra Long Period Zen Timer and returns it’s current
count. You can use the ULZElapsedTime function to find the elapsed time
between two timer count readings.

SEE ALSO:
ULZElapsedTime, ULZTimerResolution
****************************************************************************/
ulong ZAPI ULZReadTime(void)
{ return __ULZReadTime(); }

/****************************************************************************
DESCRIPTION:
Compute the elapsed time between two timer counts.

HEADER:
ztimer.h

PARAMETERS:
start   - Starting time for elapsed count
finish  - Ending time for elapsed count

RETURNS:
Elapsed timer in resolution counts.

REMARKS:
Returns the elapsed time for the Ultra Long Period Zen Timer in units of the
timers resolution (1/18th of a second under DOS). This function correctly
computes the difference even if a midnight boundary has been crossed
during the timing period.

SEE ALSO:
ULZReadTime, ULZTimerResolution
****************************************************************************/
ulong ZAPI ULZElapsedTime(
    ulong start,
    ulong finish)
{ return __ULZElapsedTime(start,finish); }

/****************************************************************************
DESCRIPTION:
Returns the resolution of the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

PARAMETERS:
resolution   - Place to store the timer in microseconds per timer count.

REMARKS:
Returns the resolution of the Ultra Long Period Zen Timer as a 32-bit
integer value measured in microseconds per timer count.

SEE ALSO:
ULZReadTime, ULZElapsedTime, ULZTimerCount
****************************************************************************/
void ZAPI ULZTimerResolution(
	ulong *resolution)
{ *resolution = ULZTIMER_RESOLUTION; }
