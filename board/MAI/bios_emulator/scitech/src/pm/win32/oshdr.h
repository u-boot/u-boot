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
* Environment:  Win32
*
* Description:  Include file to include all OS specific header files.
*
****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <float.h>
#define NONAMELESSUNION
#include "pm/ddraw.h"

/* Macros to save and restore the default control word. Windows 9x has
 * some bugs in it such that calls to load any DLL's which load 16-bit
 * DLL's cause the floating point control word to get trashed. We fix
 * this by saving and restoring the control word across problematic
 * calls.
 */

#if defined(__INTEL__)
#define GET_DEFAULT_CW()                    \
{                                           \
    if (_PM_cw_default == 0)                \
	_PM_cw_default = _control87(0,0);   \
}
#define RESET_DEFAULT_CW()                  \
    _control87(_PM_cw_default,0xFFFFFFFF)
#else
#define GET_DEFAULT_CW()
#define RESET_DEFAULT_CW()
#endif

/* Custom window messages */

#define	WM_DO_SUSPEND_APP			WM_USER
#define	WM_PM_LEAVE_FULLSCREEN	    0
#define	WM_PM_RESTORE_FULLSCREEN	1

/* Macro for disabling AutoPlay on a use system */

#define AUTOPLAY_DRIVE_CDROM    0x20

/*--------------------------- Global Variables ----------------------------*/

#ifdef  __INTEL__
extern uint     _PM_cw_default;         /* Default FPU control word     */
#endif
extern int      _PM_deskX,_PM_deskY;    /* Desktop dimensions           */
extern HWND     _PM_hwndConsole;        /* Window handle for console    */

/*-------------------------- Internal Functions ---------------------------*/

void _EVT_pumpMessages(void);
