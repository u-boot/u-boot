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
* Environment:  any
*
* Description:  Test program to check the ability to manipulate the
*               BIOS data area from protected mode using the PM
*               library. Compile and link with the appropriate command
*               line for your DOS extender.
*
*               Functions tested:   PM_getBIOSSelector()
*                                   PM_getLong()
*                                   PM_getByte()
*                                   PM_getWord()
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

/* Macros to obtain values from the BIOS data area */

#define TICKS()     PM_getLong(bios+0x6C)
#define KB_STAT     PM_getByte(bios+0x17)
#define KB_HEAD     PM_getWord(bios+0x1A)
#define KB_TAIL     PM_getWord(bios+0x1C)

/* Macros for working with the keyboard buffer */

#define KB_HIT()    (KB_HEAD != KB_TAIL)
#define CTRL()      (KB_STAT & 4)
#define SHIFT()     (KB_STAT & 2)
#define ESC         0x1B

/* Selector for BIOS data area */

uchar *bios;

int main(void)
{
    int c,done = 0;

    printf("Program running in ");
    switch (PM_getModeType()) {
	case PM_realMode:
	    printf("real mode.\n\n");
	    break;
	case PM_286:
	    printf("16 bit protected mode.\n\n");
	    break;
	case PM_386:
	    printf("32 bit protected mode.\n\n");
	    break;
	}

    bios = PM_getBIOSPointer();
    printf("Hit any key to test, Ctrl-Shift-Esc to quit\n");
    while (!done) {
	if (KB_HIT()) {
	    c = PM_getch();
	    if (c == 0) PM_getch();
	    printf("TIME=%-8lX ST=%02X CHAR=%02X ", TICKS(), KB_STAT, c);
	    printf("\n");
	    if ((c == ESC) && SHIFT() && CTRL())/* Ctrl-Shift-Esc */
		break;
	    }
	}

    return 0;
}
