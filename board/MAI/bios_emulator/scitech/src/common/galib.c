/****************************************************************************
*
*                   SciTech Nucleus Graphics Architecture
*
*               Copyright (C) 1991-1998 SciTech Software, Inc.
*                            All rights reserved.
*
*  ======================================================================
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  |                                                                    |
*  |This copyrighted computer code contains proprietary technology      |
*  |owned by SciTech Software, Inc., located at 505 Wall Street,        |
*  |Chico, CA 95928 USA (http://www.scitechsoft.com).                   |
*  |                                                                    |
*  |The contents of this file are subject to the SciTech Nucleus        |
*  |License; you may *not* use this file or related software except in  |
*  |compliance with the License. You may obtain a copy of the License   |
*  |at http://www.scitechsoft.com/nucleus-license.txt                   |
*  |                                                                    |
*  |Software distributed under the License is distributed on an         |
*  |"AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or      |
*  |implied. See the License for the specific language governing        |
*  |rights and limitations under the License.                           |
*  |                                                                    |
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  ======================================================================
*
* Language:     ANSI C
* Environment:  Any 32-bit protected mode environment
*
* Description:  C module for the Graphics Accelerator Driver API. Uses
*               the SciTech PM library for interfacing with DOS
*               extender specific functions.
*
****************************************************************************/

#include "nucleus/graphics.h"
#if defined(__WIN32_VXD__) || defined(__NT_DRIVER__)
#include "sdd/sddhelp.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/*---------------------------- Global Variables ---------------------------*/

#ifndef TEST_HARNESS
GA_exports  _VARAPI __GA_exports;
static int          loaded = false;
static PE_MODULE    *hModBPD = NULL;

static N_imports _N_imports = {
    sizeof(N_imports),
    _OS_delay,
    };

static GA_imports _GA_imports = {
    sizeof(GA_imports),
    GA_getSharedInfo,
    GA_TimerInit,
    GA_TimerRead,
    GA_TimerDifference,
    };
#endif

/*----------------------------- Implementation ----------------------------*/

#define DLL_NAME        "graphics.bpd"

/****************************************************************************
REMARKS:
This function is no longer used but we must implement it and return NULL
for compatibility with older binary drivers.
****************************************************************************/
GA_sharedInfo * NAPI GA_getSharedInfo(
    int device)
{
    return NULL;
}

#ifndef TEST_HARNESS
/****************************************************************************
REMARKS:
Fatal error handler for non-exported GA_exports.
****************************************************************************/
static void _GA_fatalErrorHandler(void)
{
    PM_fatalError("Unsupported Nucleus export function called! Please upgrade your copy of Nucleus!\n");
}

/****************************************************************************
PARAMETERS:
shared  - True to load the driver into shared memory.

REMARKS:
Loads the Nucleus binary portable DLL into memory and initilises it.
****************************************************************************/
static ibool LoadDriver(
    ibool shared)
{
    GA_initLibrary_t    GA_initLibrary;
    GA_exports          *gaExp;
    char                filename[PM_MAX_PATH];
    char                bpdpath[PM_MAX_PATH];
    int                 i,max;
    ulong               *p;

    /* Check if we have already loaded the driver */
    if (loaded)
	return true;
    PM_init();

    /* First try to see if we can find the system wide shared exports
     * if they are available. Under OS/2 this connects to our global
     * shared Nucleus loader in SDDPMI.DLL.
     */
    __GA_exports.dwSize = sizeof(__GA_exports);
    if (GA_getSharedExports(&__GA_exports,shared))
	return loaded = true;

    /* Open the BPD file */
    if (!PM_findBPD(DLL_NAME,bpdpath))
	return false;
    strcpy(filename,bpdpath);
    strcat(filename,DLL_NAME);
    if ((hModBPD = PE_loadLibrary(filename,shared)) == NULL)
	return false;
    if ((GA_initLibrary = (GA_initLibrary_t)PE_getProcAddress(hModBPD,"_GA_initLibrary")) == NULL)
	return false;
    bpdpath[strlen(bpdpath)-1] = 0;
    if (strcmp(bpdpath,PM_getNucleusPath()) == 0)
	strcpy(bpdpath,PM_getNucleusConfigPath());
    else {
	PM_backslash(bpdpath);
	strcat(bpdpath,"config");
	}
    if ((gaExp = GA_initLibrary(shared,bpdpath,filename,GA_getSystemPMImports(),&_N_imports,&_GA_imports)) == NULL)
	PM_fatalError("GA_initLibrary failed!\n");

    /* Initialize all default imports to point to fatal error handler
     * for upwards compatibility, and copy the exported functions.
     */
    max = sizeof(__GA_exports)/sizeof(GA_initLibrary_t);
    for (i = 0,p = (ulong*)&__GA_exports; i < max; i++)
	*p++ = (ulong)_GA_fatalErrorHandler;
    memcpy(&__GA_exports,gaExp,MIN(sizeof(__GA_exports),gaExp->dwSize));
    loaded = true;
    return true;
}

/* The following are stub entry points that the application calls to
 * initialise the Nucleus loader library, and we use this to load our
 * driver DLL from disk and initialise the library using it.
 */

/* {secret} */
int NAPI GA_status(void)
{
    if (!loaded)
	return nDriverNotFound;
    return __GA_exports.GA_status();
}

/* {secret} */
const char * NAPI GA_errorMsg(
    N_int32 status)
{
    if (!loaded)
	return "Unable to load Nucleus device driver!";
    return __GA_exports.GA_errorMsg(status);
}

/* {secret} */
int NAPI GA_getDaysLeft(N_int32 shared)
{
    if (!LoadDriver(shared))
	return -1;
    return __GA_exports.GA_getDaysLeft(shared);
}

/* {secret} */
int NAPI GA_registerLicense(uchar *license,N_int32 shared)
{
    if (!LoadDriver(shared))
	return 0;
    return __GA_exports.GA_registerLicense(license,shared);
}

/* {secret} */
ibool NAPI GA_loadInGUI(N_int32 shared)
{
    if (!LoadDriver(shared))
	return false;
    return __GA_exports.GA_loadInGUI(shared);
}

/* {secret} */
int NAPI GA_enumerateDevices(N_int32 shared)
{
    if (!LoadDriver(shared))
	return 0;
    return __GA_exports.GA_enumerateDevices(shared);
}

/* {secret} */
GA_devCtx * NAPI GA_loadDriver(N_int32 deviceIndex,N_int32 shared)
{
    if (!LoadDriver(shared))
	return NULL;
    return __GA_exports.GA_loadDriver(deviceIndex,shared);
}

/* {secret} */
void NAPI GA_getGlobalOptions(
    GA_globalOptions *options,
    ibool shared)
{
    if (LoadDriver(shared))
	__GA_exports.GA_getGlobalOptions(options,shared);
}

/* {secret} */
PE_MODULE * NAPI GA_loadLibrary(
    const char *szBPDName,
    ulong *size,
    ibool shared)
{
    if (!LoadDriver(shared))
	return NULL;
    return __GA_exports.GA_loadLibrary(szBPDName,size,shared);
}

/* {secret} */
GA_devCtx * NAPI GA_getCurrentDriver(
    N_int32 deviceIndex)
{
    /* Bail for older drivers that didn't export this function! */
    if (!__GA_exports.GA_getCurrentDriver)
	return NULL;
    return __GA_exports.GA_getCurrentDriver(deviceIndex);
}

/* {secret} */
REF2D_driver * NAPI GA_getCurrentRef2d(
    N_int32 deviceIndex)
{
    /* Bail for older drivers that didn't export this function! */
    if (!__GA_exports.GA_getCurrentRef2d)
	return NULL;
    return __GA_exports.GA_getCurrentRef2d(deviceIndex);
}

/* {secret} */
int NAPI GA_isOEMVersion(ibool shared)
{
    if (!LoadDriver(shared))
	return 0;
    return __GA_exports.GA_isOEMVersion(shared);
}

/* {secret} */
N_uint32 * NAPI GA_getLicensedDevices(ibool shared)
{
    if (!LoadDriver(shared))
	return 0;
    return __GA_exports.GA_getLicensedDevices(shared);
}
#endif
