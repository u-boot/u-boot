/****************************************************************************
*
*                   SciTech Multi-platform Graphics Library
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
* Environment:  Linux
*
* Description:  Include all the OS specific header files.
*
****************************************************************************/

#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fs.h>
#ifdef USE_OS_JOYSTICK
#include <linux/joystick.h>
#endif
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

/* Internal global variables */

extern int _PM_console_fd,_PM_leds,_PM_modifiers;

/* Internal function prototypes */

void _PM_restore_kb_mode(void);
void _PM_keyboard_rawmode(void);

/* Linux needs the generic joystick scaling code */

#define NEED_SCALE_JOY_AXIS
