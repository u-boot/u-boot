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
* Description:  Program to restore the console state state from a previously
*               saved state if the program crashed while the console
*               was in graphics mode.
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pmapi.h"

void setVideoMode(int mode)
{
    RMREGS r;

    r.x.ax = mode;
    PM_int86(0x10, &r, &r);
}

int main(void)
{
    PM_HWND hwndConsole;
    ulong   stateSize;
    void    *stateBuf;
    FILE    *f;

    /* Write the saved console state buffer to disk */
    if ((f = fopen("/etc/pmsave.dat","rb")) == NULL) {
	printf("Unable to open /etc/pmsave.dat for reading!\n");
	return -1;
	}
    fread(&stateSize,1,sizeof(stateSize),f);
    if (stateSize != PM_getConsoleStateSize()) {
	printf("Size mismatch in /etc/pmsave.dat!\n");
	return -1;
	}
    if ((stateBuf = PM_malloc(stateSize)) == NULL) {
	printf("Unable to allocate console state buffer!\n");
	return -1;
	}
    fread(stateBuf,1,stateSize,f);
    fclose(f);

    /* Open the console */
    hwndConsole = PM_openConsole(0,0,0,0,0,true);

    /* Forcibly set 80x25 text mode using the BIOS */
    setVideoMode(0x3);

    /* Restore the previous console state */
    PM_restoreConsoleState(stateBuf,0);
    PM_closeConsole(hwndConsole);
    PM_free(stateBuf);
    printf("Console state successfully restored from /etc/pmsave.dat\n");
    return 0;
}
