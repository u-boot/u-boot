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
* Environment:  Win32
*
* Description:  OS specific Nucleus Graphics Architecture services for
*               the Win32 operating system environments.
*
****************************************************************************/

#include "pm_help.h"
#include "pmapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*------------------------- Global Variables ------------------------------*/

#define DLL_NAME        "nga_w32.dll"

extern HANDLE           _PM_hDevice;
static HMODULE          hModDLL = NULL;
static ibool            useRing0Driver = false;
static ibool            haveRDTSC;
static GA_largeInteger  countFreq;

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
REMARKS:
Loads the shared "nga_w32.dll" library from disk and connects to it. This
library is *always* located in the same directory as the Nucleus
graphics.bpd file.
****************************************************************************/
static ibool LoadSharedDLL(void)
{
    char                filename[PM_MAX_PATH];
    char                bpdpath[PM_MAX_PATH];

    /* Check if we have already loaded the DLL */
    if (hModDLL)
	return true;
    PM_init();

    /* Open the DLL file */
    if (!PM_findBPD(DLL_NAME,bpdpath))
	return false;
    strcpy(filename,bpdpath);
    strcat(filename,DLL_NAME);
    if ((hModDLL = LoadLibrary(filename)) == NULL)
	return false;
    return true;
}

/****************************************************************************
PARAMETERS:
path    - Local path to the Nucleus driver files.

REMARKS:
This function is used by the application program to override the location
of the Nucleus driver files that are loaded. Normally the loader code
will look in the system Nucleus directories first, then in the 'drivers'
directory relative to the current working directory, and finally relative
to the MGL_ROOT environment variable.

Note that for Win32 we also call into the loaded PMHELP device driver
as necessary to change the local Nucleus path for system wide Nucleus
drivers.
****************************************************************************/
void NAPI GA_setLocalPath(
    const char *path)
{
    DWORD   inBuf[1];
    DWORD   outBuf[1],outCnt;

    PM_setLocalBPDPath(path);
    if (_PM_hDevice != INVALID_HANDLE_VALUE) {
	inBuf[0] = (DWORD)path;
	DeviceIoControl(_PM_hDevice, PMHELP_GASETLOCALPATH32,
	    inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), &outCnt, NULL);
	}
}

/****************************************************************************
RETURNS:
Pointer to the system wide PM library imports, or the internal version if none

REMARKS:
In order to support deploying new Nucleus drivers that may require updated
PM library functions, we check here to see if there is a system wide version
of the PM functions available. If so we return those functions for use with
the system wide Nucleus drivers, otherwise the compiled in version of the PM
library is used with the application local version of Nucleus.
****************************************************************************/
PM_imports * NAPI GA_getSystemPMImports(void)
{
    PM_imports * pmImp;
    PM_imports * (NAPIP _GA_getSystemPMImports)(void);

    if (LoadSharedDLL()) {
	/* Note that Visual C++ build DLL's with only a single underscore in front
	 * of the exported name while Watcom C provides two of them. We check for
	 * both to allow working with either compiled DLL.
	 */
	if ((_GA_getSystemPMImports = (void*)GetProcAddress(hModDLL,"_GA_getSystemPMImports")) != NULL) {
	    if ((_GA_getSystemPMImports = (void*)GetProcAddress(hModDLL,"__GA_getSystemPMImports")) != NULL) {
		pmImp = _GA_getSystemPMImports();
		memcpy(&_PM_imports,pmImp,MIN(_PM_imports.dwSize,pmImp->dwSize));
		return pmImp;
		}
	    }
	}
    return &_PM_imports;
}

/****************************************************************************
PARAMETERS:
gaExp   - Place to store the exported functions
shared  - True if connecting to the shared, global Nucleus driver

REMARKS:
For Win32 if we are connecting to the shared, global Nucleus driver (loaded
at ring 0) then we need to load a special nga_w32.dll library which contains
thunks to call down into the Ring 0 device driver as necessary. If we are
connecting to the application local Nucleus drivers (ie: Nucleus on DirectDraw
emulation layer) then we do nothing here.
****************************************************************************/
ibool NAPI GA_getSharedExports(
    GA_exports *gaExp,
    ibool shared)
{
    GA_exports * exp;
    GA_exports * (NAPIP _GA_getSystemGAExports)(void);

    useRing0Driver = false;
    if (shared) {
	if (!LoadSharedDLL())
	    PM_fatalError("Unable to load " DLL_NAME "!");
	if ((_GA_getSystemGAExports = (void*)GetProcAddress(hModDLL,"_GA_getSystemGAExports")) == NULL)
	    if ((_GA_getSystemGAExports = (void*)GetProcAddress(hModDLL,"__GA_getSystemGAExports")) == NULL)
		PM_fatalError("Unable to load " DLL_NAME "!");
	exp = _GA_getSystemGAExports();
	memcpy(gaExp,exp,MIN(gaExp->dwSize,exp->dwSize));
	useRing0Driver = true;
	return true;
	}
    return false;
}

#ifndef TEST_HARNESS
/****************************************************************************
REMARKS:
Nothing special for this OS
****************************************************************************/
ibool NAPI GA_queryFunctions(
    GA_devCtx *dc,
    N_uint32 id,
    void _FAR_ *funcs)
{
    static ibool (NAPIP _GA_queryFunctions)(GA_devCtx *dc,N_uint32 id,void _FAR_ *funcs) = NULL;

    if (useRing0Driver) {
	/* Call the version in nga_w32.dll if it is loaded */
	if (!_GA_queryFunctions) {
	    if ((_GA_queryFunctions = (void*)GetProcAddress(hModDLL,"_GA_queryFunctions")) == NULL)
		if ((_GA_queryFunctions = (void*)GetProcAddress(hModDLL,"__GA_queryFunctions")) == NULL)
		    PM_fatalError("Unable to get exports from " DLL_NAME "!");
	    }
	return _GA_queryFunctions(dc,id,funcs);
	}
    return __GA_exports.GA_queryFunctions(dc,id,funcs);
}

/****************************************************************************
REMARKS:
Nothing special for this OS
****************************************************************************/
ibool NAPI REF2D_queryFunctions(
    REF2D_driver *ref2d,
    N_uint32 id,
    void _FAR_ *funcs)
{
    static ibool (NAPIP _REF2D_queryFunctions)(REF2D_driver *ref2d,N_uint32 id,void _FAR_ *funcs) = NULL;

    if (useRing0Driver) {
	/* Call the version in nga_w32.dll if it is loaded */
	if (!_REF2D_queryFunctions) {
	    if ((_REF2D_queryFunctions = (void*)GetProcAddress(hModDLL,"_REF2D_queryFunctions")) == NULL)
		if ((_REF2D_queryFunctions = (void*)GetProcAddress(hModDLL,"__REF2D_queryFunctions")) == NULL)
		    PM_fatalError("Unable to get exports from " DLL_NAME "!");
	    }
	return _REF2D_queryFunctions(ref2d,id,funcs);
	}
    return __GA_exports.REF2D_queryFunctions(ref2d,id,funcs);
}
#endif

/****************************************************************************
REMARKS:
This function initialises the high precision timing functions for the
Nucleus loader library.
****************************************************************************/
ibool NAPI GA_TimerInit(void)
{
    if (_GA_haveCPUID() && (_GA_getCPUIDFeatures() & CPU_HaveRDTSC) != 0) {
	haveRDTSC = true;
	return true;
	}
    else if (QueryPerformanceFrequency((LARGE_INTEGER*)&countFreq)) {
	haveRDTSC = false;
	return true;
	}
    return false;
}

/****************************************************************************
REMARKS:
This function reads the high resolution timer.
****************************************************************************/
void NAPI GA_TimerRead(
    GA_largeInteger *value)
{
    if (haveRDTSC)
	_GA_readTimeStamp(value);
    else
	QueryPerformanceCounter((LARGE_INTEGER*)value);
}
