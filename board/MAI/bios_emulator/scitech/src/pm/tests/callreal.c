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
* Description:  Test program to check the ability to call a real mode
*               procedure. We simply copy a terribly simple assembly
*               language routine into a real mode block that we allocate,
*               and then attempt to call the routine and verify that it
*               was successful.
*
*               Functions tested:   PM_allocRealSeg()
*                                   PM_freeRealSeg()
*                                   PM_callRealMode()
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pmapi.h"

/* Block of real mode code we will eventually call */

static unsigned char realModeCode[] = {
    0x93,           /*  xchg    ax,bx   */
    0x87, 0xCA,     /*  xchg    cx,dx   */
    0xCB            /*  retf            */
    };

int main(void)
{
    RMREGS          regs;
    RMSREGS         sregs;
    uchar           *p;
    unsigned        r_seg,r_off;

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

    /* Allocate a the block of real mode memory */
    if ((p = PM_allocRealSeg(sizeof(realModeCode), &r_seg, &r_off)) == NULL) {
	printf("Unable to allocate real mode memory!\n");
	exit(1);
	}

    /* Copy the real mode code */
    memcpy(p,realModeCode,sizeof(realModeCode));

    /* Now call the real mode code */
    regs.x.ax = 1;
    regs.x.bx = 2;
    regs.x.cx = 3;
    regs.x.dx = 4;
    regs.x.si = 5;
    regs.x.di = 6;
    sregs.es = 7;
    sregs.ds = 8;
    PM_callRealMode(r_seg,r_off,&regs,&sregs);
    if (regs.x.ax != 2 || regs.x.bx != 1 || regs.x.cx != 4 || regs.x.dx != 3
	    || regs.x.si != 5 || regs.x.di != 6 || sregs.es != 7
	    || sregs.ds != 8) {
	printf("Real mode call failed!\n");
	printf("\n");
	printf("ax = %04X, bx = %04X, cx = %04X, dx = %04X\n",
	    regs.x.ax,regs.x.bx,regs.x.cx,regs.x.dx);
	printf("si = %04X, di = %04X, es = %04X, ds = %04X\n",
	    regs.x.si,regs.x.di,sregs.es,sregs.ds);
	}
    else
	printf("Real mode call succeeded!\n");

    /* Free the memory we allocated */
    PM_freeRealSeg(p);
    return 0;
}
