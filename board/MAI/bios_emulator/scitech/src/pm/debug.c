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
* Description:  Main module containing debug checking features.
*
****************************************************************************/

#include "pmapi.h"
#ifdef  __WIN32_VXD__
#include "vxdfile.h"
#elif defined(__NT_DRIVER__)
#include "ntdriver.h"
#elif defined(__OS2_VDD__)
#include "vddfile.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/*---------------------------- Global variables ---------------------------*/

/* {secret} */
void (*_CHK_fail)(int fatal,const char *msg,const char *cond,const char *file,int line) = _CHK_defaultFail;
static char logFile[256] = "";

/*----------------------------- Implementation ----------------------------*/

#ifdef  CHECKED
void _CHK_defaultFail(
    int fatal,
    const char *msg,
    const char *cond,
    const char *file,
    int line)
{
    FILE    *f;
    char    buf[256];

    if (logFile[0] == 0) {
	strcpy(logFile,PM_getNucleusPath());
	PM_backslash(logFile);
	strcat(logFile,"scitech.log");
	}
    if ((f = fopen(logFile,"a+")) != NULL) {
#if defined(__WIN32_VXD__) || defined(__OS2_VDD__) || defined(__NT_DRIVER__)
	sprintf(buf,msg,cond,file,line);
	fwrite(buf,1,strlen(buf),f);
#else
	fprintf(f,msg,cond,file,line);
#endif
	fclose(f);
	}
    if (fatal) {
	sprintf(buf,"Check failed: check '%s' for details", logFile);
	PM_fatalError(buf);
	}
}
#endif

/****************************************************************************
DESCRIPTION:
Sets the location of the debug log file.

HEADER:
pmapi.h

PARAMETERS:
logFilePath - Full file and path name to debug log file.

REMARKS:
Sets the name and location of the debug log file. The debug log file is
created and written to when runtime checks, warnings and failure conditions
are logged to disk when code is compiled in CHECKED mode. By default the
log file is called 'scitech.log' and goes into the current SciTech Nucleus
path for the application. You can use this function to set the filename
and location of the debug log file to your own application specific
directory.
****************************************************************************/
void PMAPI PM_setDebugLog(
    const char *logFilePath)
{
    strcpy(logFile,logFilePath);
}
