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
* Environment:  QNX
*
* Description:  Include file to include all OS specific header files.
*
****************************************************************************/

#include <sys/stat.h>
#include <fcntl.h>
#include <process.h>
#include <time.h>
#ifndef __QNXNTO__
#include <sys/mouse.h>
#include <sys/keyboard.h>
#include <sys/fd.h>
#include <conio.h>
#else
#include <sys/dcmd_input.h>

/* Things 'borrowed' from photon/keycodes.h */

/*
 * Keyboard modifiers
 */
#define KEYMODBIT_SHIFT                                         0
#define KEYMODBIT_CTRL                                          1
#define KEYMODBIT_ALT                                           2
#define KEYMODBIT_ALTGR                                         3
#define KEYMODBIT_SHL3                                          4
#define KEYMODBIT_MOD6                                          5
#define KEYMODBIT_MOD7                                          6
#define KEYMODBIT_MOD8                                          7

#define KEYMODBIT_SHIFT_LOCK                                    8
#define KEYMODBIT_CTRL_LOCK                                     9
#define KEYMODBIT_ALT_LOCK                                      10
#define KEYMODBIT_ALTGR_LOCK                                    11
#define KEYMODBIT_SHL3_LOCK                                     12
#define KEYMODBIT_MOD6_LOCK                                     13
#define KEYMODBIT_MOD7_LOCK                                     14
#define KEYMODBIT_MOD8_LOCK                                     15

#define KEYMODBIT_CAPS_LOCK                                     16
#define KEYMODBIT_NUM_LOCK                                      17
#define KEYMODBIT_SCROLL_LOCK                                   18

#define KEYMOD_SHIFT                                            (1 << KEYMODBIT_SHIFT)
#define KEYMOD_CTRL                                             (1 << KEYMODBIT_CTRL)
#define KEYMOD_ALT                                              (1 << KEYMODBIT_ALT)
#define KEYMOD_ALTGR                                            (1 << KEYMODBIT_ALTGR)
#define KEYMOD_SHL3                                             (1 << KEYMODBIT_SHL3)
#define KEYMOD_MOD6                                             (1 << KEYMODBIT_MOD6)
#define KEYMOD_MOD7                                             (1 << KEYMODBIT_MOD7)
#define KEYMOD_MOD8                                             (1 << KEYMODBIT_MOD8)

#define KEYMOD_SHIFT_LOCK                                       (1 << KEYMODBIT_SHIFT_LOCK)
#define KEYMOD_CTRL_LOCK                                        (1 << KEYMODBIT_CTRL_LOCK)
#define KEYMOD_ALT_LOCK                                         (1 << KEYMODBIT_ALT_LOCK)
#define KEYMOD_ALTGR_LOCK                                       (1 << KEYMODBIT_ALTGR_LOCK)
#define KEYMOD_SHL3_LOCK                                        (1 << KEYMODBIT_SHL3_LOCK)
#define KEYMOD_MOD6_LOCK                                        (1 << KEYMODBIT_MOD6_LOCK)
#define KEYMOD_MOD7_LOCK                                        (1 << KEYMODBIT_MOD7_LOCK)
#define KEYMOD_MOD8_LOCK                                        (1 << KEYMODBIT_MOD8_LOCK)

#define KEYMOD_CAPS_LOCK                                        (1 << KEYMODBIT_CAPS_LOCK)
#define KEYMOD_NUM_LOCK                                         (1 << KEYMODBIT_NUM_LOCK)
#define KEYMOD_SCROLL_LOCK                                      (1 << KEYMODBIT_SCROLL_LOCK)

/*
 * Keyboard flags
 */
#define KEY_DOWN                                                0x00000001      /* Key was pressed down */
#define KEY_REPEAT                                              0x00000002      /* Key was repeated */
#define KEY_SCAN_VALID                                          0x00000020      /* Scancode is valid */
#define KEY_SYM_VALID                                           0x00000040      /* Key symbol is valid */
#define KEY_CAP_VALID                                           0x00000080      /* Key cap is valid */
#define KEY_DEAD                                                0x40000000      /* Key symbol is a DEAD key */
#define KEY_OEM_CAP                                             0x80000000      /* Key cap is an OEM scan code from keyboard */

#endif  /* __QNXNTO__ */
