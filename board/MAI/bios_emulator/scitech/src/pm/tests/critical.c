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
*               critical error handler.
*
*               Functions tested:   PM_installAltCriticalHandler()
*                                   PM_restoreCriticalHandler()
*
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

int main(void)
{
    FILE    *f;
    int     axcode,dicode;

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

    PM_installCriticalHandler();
    printf("Critical Error handler installed - trying to read from A: drive...\n");
    f = fopen("a:\bog.bog","rb");
    if (f) fclose(f);
    if (PM_criticalError(&axcode,&dicode,1)) {
	printf("Critical error occured on INT 21h function %02X!\n",
	    axcode >> 8);
	}
    else printf("Critical error was not caught!\n");
    PM_restoreCriticalHandler();
    return 0;
}
