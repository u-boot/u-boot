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
* Environment:  Linux/QNX
*
* Description:  Program to save the console state state so that it can
*               be later restored if the program crashed while the console
*               was in graphics mode.
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

int main(void)
{
    PM_HWND hwndConsole;
    ulong   stateSize;
    void    *stateBuf;
    FILE    *f;

    /* Allocate a buffer to save console state and save the state */
    hwndConsole = PM_openConsole(0,0,0,0,0,true);
    stateSize = PM_getConsoleStateSize();
    if ((stateBuf = PM_malloc(stateSize)) == NULL) {
	PM_closeConsole(hwndConsole);
	printf("Unable to allocate console state buffer!\n");
	return -1;
	}
    PM_saveConsoleState(stateBuf,0);

    /* Restore the console state on exit */
    PM_restoreConsoleState(stateBuf,0);
    PM_closeConsole(hwndConsole);

    /* Write the saved console state buffer to disk */
    if ((f = fopen("/etc/pmsave.dat","wb")) == NULL)
	printf("Unable to open /etc/pmsave/dat for writing!\n");
    else {
	fwrite(&stateSize,1,sizeof(stateSize),f);
	fwrite(stateBuf,1,stateSize,f);
	fclose(f);
	printf("Console state successfully saved to /etc/pmsave.dat\n");
	}
    PM_free(stateBuf);
    return 0;
}
