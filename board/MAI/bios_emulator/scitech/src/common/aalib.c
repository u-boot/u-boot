/****************************************************************************
*
*                     SciTech Nucleus Audio Architecture
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

#include "nucleus/audio.h"
#ifdef __WIN32_VXD__
#include "sdd/sddhelp.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/*---------------------------- Global Variables ---------------------------*/

#ifdef  TEST_HARNESS
extern PM_imports   _VARAPI _PM_imports;
#else
AA_exports  _VARAPI _AA_exports;
static int          loaded = false;
static PE_MODULE    *hModBPD = NULL;

#ifdef  __DRIVER__
extern PM_imports _PM_imports;
#else
#include "pmimp.h"
#endif

static N_imports _N_imports = {
    sizeof(N_imports),
    _OS_delay,
    };

#ifdef  __DRIVER__
extern AA_imports _AA_imports;
#else
static AA_imports _AA_imports = {
    sizeof(AA_imports),
    };
#endif
#endif

/*----------------------------- Implementation ----------------------------*/

#define DLL_NAME        "audio.bpd"

#ifndef TEST_HARNESS
/****************************************************************************
REMARKS:
Fatal error handler for non-exported AA_exports.
****************************************************************************/
static void _AA_fatalErrorHandler(void)
{
    PM_fatalError("Unsupported Nucleus export function called! Please upgrade your copy of Nucleus!\n");
}

/****************************************************************************
REMARKS:
Loads the Nucleus binary portable DLL into memory and initilises it.
****************************************************************************/
static ibool LoadDriver(void)
{
    AA_initLibrary_t    AA_initLibrary;
    AA_exports          *aaExp;
    char                filename[PM_MAX_PATH];
    char                bpdpath[PM_MAX_PATH];
    int                 i,max;
    ulong               *p;

    /* Check if we have already loaded the driver */
    if (loaded)
	return true;
    PM_init();
    _AA_exports.dwSize = sizeof(_AA_exports);

    /* Open the BPD file */
    if (!PM_findBPD(DLL_NAME,bpdpath))
	return false;
    strcpy(filename,bpdpath);
    strcat(filename,DLL_NAME);
    if ((hModBPD = PE_loadLibrary(filename,false)) == NULL)
	return false;
    if ((AA_initLibrary = (AA_initLibrary_t)PE_getProcAddress(hModBPD,"_AA_initLibrary")) == NULL)
	return false;
    bpdpath[strlen(bpdpath)-1] = 0;
    if (strcmp(bpdpath,PM_getNucleusPath()) == 0)
	strcpy(bpdpath,PM_getNucleusConfigPath());
    else {
	PM_backslash(bpdpath);
	strcat(bpdpath,"config");
	}
    if ((aaExp = AA_initLibrary(bpdpath,filename,&_PM_imports,&_N_imports,&_AA_imports)) == NULL)
	PM_fatalError("AA_initLibrary failed!\n");

    /* Initialize all default imports to point to fatal error handler
     * for upwards compatibility, and copy the exported functions.
     */
    max = sizeof(_AA_exports)/sizeof(AA_initLibrary_t);
    for (i = 0,p = (ulong*)&_AA_exports; i < max; i++)
	*p++ = (ulong)_AA_fatalErrorHandler;
    memcpy(&_AA_exports,aaExp,MIN(sizeof(_AA_exports),aaExp->dwSize));
    loaded = true;
    return true;
}

/* The following are stub entry points that the application calls to
 * initialise the Nucleus loader library, and we use this to load our
 * driver DLL from disk and initialise the library using it.
 */

/* {secret} */
int NAPI AA_status(void)
{
    if (!loaded)
	return nDriverNotFound;
    return _AA_exports.AA_status();
}

/* {secret} */
const char * NAPI AA_errorMsg(
    N_int32 status)
{
    if (!loaded)
	return "Unable to load Nucleus device driver!";
    return _AA_exports.AA_errorMsg(status);
}

/* {secret} */
int NAPI AA_getDaysLeft(void)
{
    if (!LoadDriver())
	return -1;
    return _AA_exports.AA_getDaysLeft();
}

/* {secret} */
int NAPI AA_registerLicense(uchar *license)
{
    if (!LoadDriver())
	return 0;
    return _AA_exports.AA_registerLicense(license);
}

/* {secret} */
int NAPI AA_enumerateDevices(void)
{
    if (!LoadDriver())
	return 0;
    return _AA_exports.AA_enumerateDevices();
}

/* {secret} */
AA_devCtx * NAPI AA_loadDriver(N_int32 deviceIndex)
{
    if (!LoadDriver())
	return NULL;
    return _AA_exports.AA_loadDriver(deviceIndex);
}
#endif

typedef struct {
    N_uint32    low;
    N_uint32    high;
    } AA_largeInteger;

void    NAPI _OS_delay8253(N_uint32 microSeconds);
ibool   NAPI _GA_haveCPUID(void);
uint    NAPI _GA_getCPUIDFeatures(void);
void    NAPI _GA_readTimeStamp(AA_largeInteger *time);
#define CPU_HaveRDTSC   0x00000010

/****************************************************************************
REMARKS:
This function delays for the specified number of microseconds
****************************************************************************/
void NAPI _OS_delay(
    N_uint32 microSeconds)
{
    static ibool    inited = false;
    LZTimerObject   tm;

    if (_GA_haveCPUID() && (_GA_getCPUIDFeatures() & CPU_HaveRDTSC) != 0) {
	if (!inited) {
	    ZTimerInit();
	    inited = true;
	    }
	LZTimerOnExt(&tm);
	while (LZTimerLapExt(&tm) < microSeconds)
	    ;
	LZTimerOnExt(&tm);
	}
    else
	_OS_delay8253(microSeconds);
}
