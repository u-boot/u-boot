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
* Description:  Test program to check the ability to install an assembly
*               language mouse interrupt handler. We use assembly language
*               as it must be a far function and should swap to a local
*               32 bit stack if it is going to call any C based code (which
*               we do in this example).
*
*               Functions tested:   PM_installMouseHandler()
*                                   PM_int86()
*
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

volatile long count = 0;

#pragma off (check_stack)           /* No stack checking under Watcom   */

void PMAPI mouseHandler(
    uint mask,
    uint butstate,
    int x,
    int y,
    int mickeyX,
    int mickeyY)
{
    mask = mask;                /* We dont use any of the parameters    */
    butstate = butstate;
    x = x;
    y = y;
    mickeyX = mickeyX;
    mickeyY = mickeyY;
    count++;
}

int main(void)
{
    RMREGS          regs;
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

    regs.x.ax = 33;     /* Mouse function 33 - Software reset       */
    PM_int86(0x33,&regs,&regs);
    if (regs.x.bx == 0) {
	printf("No mouse installed.\n");
	exit(1);
	}

    /* Install our mouse handler and lock handler pages in memory. It is
     * difficult to get the size of a function in C, but we know our
     * function is well less than 100 bytes (and an entire 4k page will
     * need to be locked by the server anyway).
     */
    PM_lockCodePages((__codePtr)mouseHandler,100,&lh);
    PM_lockDataPages((void*)&count,sizeof(count),&lh);
    if (!PM_setMouseHandler(0xFFFF, mouseHandler)) {
	printf("Unable to install mouse handler!\n");
	exit(1);
	}
    printf("Mouse handler installed - Hit any key to exit\n");
    PM_getch();

    PM_restoreMouseHandler();
    PM_unlockDataPages((void*)&count,sizeof(count),&lh);
    PM_unlockCodePages((__codePtr)mouseHandler,100,&lh);
    printf("Mouse handler was called %ld times\n", count);
    return 0;
}
