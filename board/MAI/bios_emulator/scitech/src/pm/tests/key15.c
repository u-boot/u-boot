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
*
* Language:     ANSI C
* Environment:  any
*
* Description:  Test program to check the ability to install a C based
*               keyboard Int 15h interrupt handler. This is an alternate
*               way to intercept scancodes from the keyboard by hooking
*               the Int 15h keyboard intercept callout.
*
*               Functions tested:   PM_setKey15Handler()
*                                   PM_restoreKey15Handler()
*
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

volatile long count = 0;
volatile short lastScanCode = 0;

#pragma off (check_stack)           /* No stack checking under Watcom   */

short PMAPI keyHandler(short scanCode)
{
    count++;
    lastScanCode = scanCode;
    return scanCode;            /* Let BIOS process as normal */
}

int main(void)
{
    int             ch;
    PM_lockHandle   lh;

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

    /* Install our timer handler and lock handler pages in memory. It is
     * difficult to get the size of a function in C, but we know our
     * function is well less than 100 bytes (and an entire 4k page will
     * need to be locked by the server anyway).
     */
    PM_lockCodePages((__codePtr)keyHandler,100,&lh);
    PM_lockDataPages((void*)&count,sizeof(count),&lh);
    PM_installBreakHandler();       /* We *DONT* want Ctrl-Break's! */
    PM_setKey15Handler(keyHandler);
    printf("Keyboard interrupt handler installed - Type some characters and\n");
    printf("hit ESC to exit\n");
    while ((ch = PM_getch()) != 0x1B) {
	printf("%c", ch);
	fflush(stdout);
	}

    PM_restoreKey15Handler();
    PM_restoreBreakHandler();
    PM_unlockDataPages((void*)&count,sizeof(count),&lh);
    PM_unlockCodePages((__codePtr)keyHandler,100,&lh);
    printf("\n\nKeyboard handler was called %ld times\n", count);
    printf("Last scan code %04X\n", lastScanCode);
    return 0;
}
