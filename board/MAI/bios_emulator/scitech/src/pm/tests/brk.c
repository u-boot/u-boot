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
*               control C/break interrupt handler.
*
*               Functions tested:   PM_installBreakHandler()
*                                   PM_ctrlCHit()
*                                   PM_ctrlBreakHit()
*                                   PM_restoreBreakHandler()
*
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

int main(void)
{
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

    PM_installBreakHandler();
    printf("Control C/Break interrupt handler installed\n");
    while (1) {
	if (PM_ctrlCHit(1)) {
	    printf("Code termimated with Ctrl-C.\n");
	    break;
	    }
	if (PM_ctrlBreakHit(1)) {
	    printf("Code termimated with Ctrl-Break.\n");
	    break;
	    }
	if (PM_kbhit() && PM_getch() == 0x1B) {
	    printf("No break code detected!\n");
	    break;
	    }
	printf("Hit Ctrl-C or Ctrl-Break to exit!\n");
	}

    PM_restoreBreakHandler();
    return 0;
}
