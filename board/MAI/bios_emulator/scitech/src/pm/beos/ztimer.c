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
* Environment:  *** TODO: ADD YOUR OS ENVIRONMENT NAME HERE ***
*
* Description:  OS specific implementation for the Zen Timer functions.
*
****************************************************************************/

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Initialise the Zen Timer module internals.
****************************************************************************/
void _ZTimerInit(void)
{
    /* TODO: Do any specific internal initialisation in here */
}

/****************************************************************************
REMARKS:
Start the Zen Timer counting.
****************************************************************************/
static void _LZTimerOn(
    LZTimerObject *tm)
{
    /* TODO: Start the Zen Timer counting. This should be a macro if */
    /*       possible. */
}

/****************************************************************************
REMARKS:
Compute the lap time since the timer was started.
****************************************************************************/
static ulong _LZTimerLap(
    LZTimerObject *tm)
{
    /* TODO: Compute the lap time between the current time and when the */
    /*       timer was started. */
    return 0;
}

/****************************************************************************
REMARKS:
Stop the Zen Timer counting.
****************************************************************************/
static void _LZTimerOff(
    LZTimerObject *tm)
{
    /* TODO: Stop the timer counting. Should be a macro if possible. */
}

/****************************************************************************
REMARKS:
Compute the elapsed time in microseconds between start and end timings.
****************************************************************************/
static ulong _LZTimerCount(
    LZTimerObject *tm)
{
    /* TODO: Compute the elapsed time and return it. Always microseconds. */
    return 0;
}

/****************************************************************************
REMARKS:
Define the resolution of the long period timer as microseconds per timer tick.
****************************************************************************/
#define ULZTIMER_RESOLUTION 1

/****************************************************************************
REMARKS:
Read the Long Period timer from the OS
****************************************************************************/
static ulong _ULZReadTime(void)
{
    /* TODO: Read the long period timer from the OS. The resolution of this */
    /*       timer should be around 1/20 of a second for timing long */
    /*       periods if possible. */
}

/****************************************************************************
REMARKS:
Compute the elapsed time from the BIOS timer tick. Note that we check to see
whether a midnight boundary has passed, and if so adjust the finish time to
account for this. We cannot detect if more that one midnight boundary has
passed, so if this happens we will be generating erronous results.
****************************************************************************/
ulong _ULZElapsedTime(ulong start,ulong finish)
{ return finish - start; }
