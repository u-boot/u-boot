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
* Description:  Module to implement OS specific services to measure the
*               CPU frequency.
*
****************************************************************************/

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Increase the thread priority to maximum, if possible.
****************************************************************************/
static int SetMaxThreadPriority(void)
{
    /* TODO: If you have thread priorities, increase it to maximum for the */
    /*       thread for timing the CPU frequency. */
    return oldPriority;
}

/****************************************************************************
REMARKS:
Restore the original thread priority.
****************************************************************************/
static void RestoreThreadPriority(
    int priority)
{
    /* TODO: Restore the original thread priority on exit. */
}

/****************************************************************************
REMARKS:
Initialise the counter and return the frequency of the counter.
****************************************************************************/
static void GetCounterFrequency(
    CPU_largeInteger *freq)
{
    /* TODO: Return the frequency of the counter in here. You should try to */
    /*       normalise this value to be around 100,000 ticks per second. */
    freq->low = 0;
    freq->high = 0;
}

/****************************************************************************
REMARKS:
Read the counter and return the counter value.

TODO: Implement this to read the counter. It should be done as a macro
      for accuracy.
****************************************************************************/
#define GetCounter(t)               \
{                                   \
    (t)->low = 0;                   \
    (t)->high = 0;                  \
}
