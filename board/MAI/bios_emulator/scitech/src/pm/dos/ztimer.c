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
* Environment:  MSDOS
*
* Description:  OS specific implementation for the Zen Timer functions.
*
****************************************************************************/


/*---------------------------- Global variables ---------------------------*/

uchar * _VARAPI _ZTimerBIOSPtr;

/*----------------------------- Implementation ----------------------------*/

/* External assembler functions */

void    _ASMAPI LZ_timerOn(void);
ulong   _ASMAPI LZ_timerLap(void);
void    _ASMAPI LZ_timerOff(void);
ulong   _ASMAPI LZ_timerCount(void);
void    _ASMAPI LZ_disable(void);
void    _ASMAPI LZ_enable(void);

/****************************************************************************
REMARKS:
Initialise the Zen Timer module internals.
****************************************************************************/
void __ZTimerInit(void)
{
    _ZTimerBIOSPtr = PM_getBIOSPointer();
}

/****************************************************************************
REMARKS:
Call the assembler Zen Timer functions to do the timing.
****************************************************************************/
#define __LZTimerOn(tm)     LZ_timerOn()

/****************************************************************************
REMARKS:
Call the assembler Zen Timer functions to do the timing.
****************************************************************************/
#define __LZTimerLap(tm)        LZ_timerLap()

/****************************************************************************
REMARKS:
Call the assembler Zen Timer functions to do the timing.
****************************************************************************/
#define __LZTimerOff(tm)        LZ_timerOff()

/****************************************************************************
REMARKS:
Call the assembler Zen Timer functions to do the timing.
****************************************************************************/
#define __LZTimerCount(tm)  LZ_timerCount()

/****************************************************************************
REMARKS:
Define the resolution of the long period timer as microseconds per timer tick.
****************************************************************************/
#define ULZTIMER_RESOLUTION     54925

/****************************************************************************
REMARKS:
Read the Long Period timer value from the BIOS timer tick.
****************************************************************************/
static ulong __ULZReadTime(void)
{
    ulong   ticks;
    LZ_disable();            /* Turn of interrupts               */
    ticks = PM_getLong(_ZTimerBIOSPtr+0x6C);
    LZ_enable();             /* Turn on interrupts again         */
    return ticks;
}

/****************************************************************************
REMARKS:
Compute the elapsed time from the BIOS timer tick. Note that we check to see
whether a midnight boundary has passed, and if so adjust the finish time to
account for this. We cannot detect if more that one midnight boundary has
passed, so if this happens we will be generating erronous results.
****************************************************************************/
ulong __ULZElapsedTime(ulong start,ulong finish)
{
    if (finish < start)
	finish += 1573040L;         /* Number of ticks in 24 hours      */
    return finish - start;
}
