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
* Environment:  Any
*
* Description:  Main module for building checked builds of products with
*               assertions and trace code.
*
****************************************************************************/

#include "scitech.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  __WINDOWS__
#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#endif

#ifdef  CHECKED

/*---------------------------- Global variables ---------------------------*/

#define LOGFILE "\\scitech.log"

void (*_CHK_fail)(int fatal,const char *msg,const char *cond,const char *file,int line) = _CHK_defaultFail;

/*---------------------------- Implementation -----------------------------*/

/****************************************************************************
DESCRIPTION:
Handles fatal error and warning conditions for checked builds.

HEADER:
scitech.h

REMARKS:
This function is called whenever an inline check or warning fails in any
of the SciTech runtime libraries. Warning conditions simply cause the
condition to be logged to the log file and send to the system debugger
under Window. Fatal error conditions do all of the above, and then
terminate the program with a fatal error conditions.

This handler may be overriden by the user code if necessary to replace it
with a different handler (the MGL for instance overrides this and replaces
it with a handler that does an MGL_exit() before terminating the application
so that it will clean up correctly.
****************************************************************************/
void _CHK_defaultFail(
    int fatal,
    const char *msg,
    const char *cond,
    const char *file,
    int line)
{
    char    buf[256];
    FILE    *log = fopen(LOGFILE, "at+");

    sprintf(buf,msg,cond,file,line);
    if (log) {
	fputs(buf,log);
	fflush(log);
	fclose(log);
#ifdef  __WINDOWS__
	OutputDebugStr(buf);
#endif
	}
    if (fatal) {
#ifdef  __WINDOWS__
	MessageBox(NULL, buf,"Fatal Error!",MB_ICONEXCLAMATION);
#else
	fputs(buf,stderr);
#endif
	exit(-1);
	}
}

#endif  /* CHECKED */
