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
* Environment:  DOS
*
* Description:  MSDOS specific code for the CPU detection module.
*
****************************************************************************/

/*----------------------------- Implementation ----------------------------*/

/* External timing function */

void __ZTimerInit(void);

/****************************************************************************
REMARKS:
Do nothing for DOS because we don't have thread priorities.
****************************************************************************/
#define SetMaxThreadPriority()      0

/****************************************************************************
REMARKS:
Do nothing for DOS because we don't have thread priorities.
****************************************************************************/
#define RestoreThreadPriority(i)    (void)(i)

/****************************************************************************
REMARKS:
Initialise the counter and return the frequency of the counter.
****************************************************************************/
static void GetCounterFrequency(
    CPU_largeInteger *freq)
{
    ulong   resolution;

    __ZTimerInit();
    ULZTimerResolution(&resolution);
    freq->low = (ulong)(10000000000.0 / resolution);
    freq->high = 0;
}

/****************************************************************************
REMARKS:
Read the counter and return the counter value.
****************************************************************************/
#define GetCounter(t)                   \
{                                       \
    (t)->low = ULZReadTime() * 10000L;  \
    (t)->high = 0;                      \
}
