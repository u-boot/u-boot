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
* Environment:  RTTarget-32
*
* Description:  Module to implement OS specific services to measure the
*               CPU frequency.
*
****************************************************************************/

/*---------------------------- Global variables ---------------------------*/

static ibool havePerformanceCounter;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
REMARKS:
Increase the thread priority to maximum, if possible.
****************************************************************************/
static int SetMaxThreadPriority(void)
{
    int     oldPriority;
    HANDLE  hThread = GetCurrentThread();

    oldPriority = GetThreadPriority(hThread);
    if (oldPriority != THREAD_PRIORITY_ERROR_RETURN)
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
    return oldPriority;
}

/****************************************************************************
REMARKS:
Restore the original thread priority.
****************************************************************************/
static void RestoreThreadPriority(
    int oldPriority)
{
    HANDLE  hThread = GetCurrentThread();

    if (oldPriority != THREAD_PRIORITY_ERROR_RETURN)
	SetThreadPriority(hThread, oldPriority);
}

/****************************************************************************
REMARKS:
Initialise the counter and return the frequency of the counter.
****************************************************************************/
static void GetCounterFrequency(
    CPU_largeInteger *freq)
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)freq)) {
	havePerformanceCounter = false;
	freq->low = 100000;
	freq->high = 0;
	}
    else
	havePerformanceCounter = true;
}

/****************************************************************************
REMARKS:
Read the counter and return the counter value.
****************************************************************************/
#define GetCounter(t)                                       \
{                                                           \
    if (havePerformanceCounter)                             \
	QueryPerformanceCounter((LARGE_INTEGER*)t);         \
    else {                                                  \
	(t)->low = timeGetTime() * 100;                     \
	(t)->high = 0;                                      \
	}                                                   \
}
