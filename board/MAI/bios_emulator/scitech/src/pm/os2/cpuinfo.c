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
* Environment:  OS/2
*
* Description:  OS/2 specific code for the CPU detection module.
*
****************************************************************************/

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
TODO: This should be implemented for OS/2!
****************************************************************************/
#define SetMaxThreadPriority()  0

/****************************************************************************
REMARKS:
TODO: This should be implemented for OS/2!
****************************************************************************/
#define RestoreThreadPriority(i)

/****************************************************************************
REMARKS:
Initialise the counter and return the frequency of the counter.
****************************************************************************/
static void GetCounterFrequency(
    CPU_largeInteger *freq)
{
    freq->low = 100000;
    freq->high = 0;
}

/****************************************************************************
REMARKS:
Read the counter and return the counter value.
****************************************************************************/
#define GetCounter(t)                                                       \
{                                                                           \
    ULONG   count;                                                          \
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &count, sizeof(ULONG) );   \
    (t)->low = count * 100;                                                 \
    (t)->high = 0;                                                          \
}
