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

#if GA_MAX_DEVICES > 4
#error GA_MAX_DEVICES has changed!
#endif

static ibool            haveRDTSC;
static GA_largeInteger  countFreq;
static GA_loadDriver_t  ORG_GA_loadDriver;
extern HANDLE           _PM_hDevice;

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
DESCRIPTION:
Get the current graphics driver imports from the VxD

REMARKS:
This function returns a pointer to the common graphics driver loaded in the
helper VxD. The memory for the VxD is shared between all processes via
the VxD, so that the VxD, 16-bit code and 32-bit code all see the same
state when accessing the graphics binary portable driver.
****************************************************************************/
GA_sharedInfo * NAPI GA_getSharedInfo(
    int device)
{
    DWORD   inBuf[1];   /* Buffer to send data to VxD       */
    DWORD   outBuf[2];  /* Buffer to receive data from VxD  */
    DWORD   count;      /* Count of bytes returned from VxD */

    PM_init();
    inBuf[0] = device;
    if (DeviceIoControl(_PM_hDevice, PMHELP_GETSHAREDINFO32, inBuf, sizeof(inBuf),
	    outBuf, sizeof(outBuf), &count, NULL)) {
	return (GA_sharedInfo*)outBuf[0];
	}
    return NULL;
}

/****************************************************************************
REMARKS:
Nothing special for this OS.
****************************************************************************/
ibool NAPI GA_getSharedExports(
    GA_exports *gaExp)
{
    (void)gaExp;
    return false;
}

/****************************************************************************
REMARKS:
This function initialises the software stereo module by either calling
the Nucleus libraries directly, or calling into the VxD if we are running
on the shared Nucleus libraries loaded by the Windows VxD.
****************************************************************************/
static ibool NAPI _GA_softStereoInit(
    GA_devCtx *dc)
{
    if (_PM_hDevice) {
	DWORD   inBuf[1];   /* Buffer to send data to VxD       */
	DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
	DWORD   count;      /* Count of bytes returned from VxD */

	inBuf[0] = (ulong)dc;
	if (DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOINIT32, inBuf, sizeof(inBuf),
		outBuf, sizeof(outBuf), &count, NULL)) {
	    return outBuf[0];
	    }
	}
    return false;
}

/****************************************************************************
REMARKS:
This function turns on software stereo mode, either directly or via the VxD.
****************************************************************************/
static void NAPI _GA_softStereoOn(void)
{
    if (_PM_hDevice) {
	DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOON32, NULL, 0,
	    NULL, 0, NULL, NULL);
	}
}

/****************************************************************************
REMARKS:
This function schedules a software stereo mode page flip, either directly
or via the VxD.
****************************************************************************/
static void NAPI _GA_softStereoScheduleFlip(
    N_uint32 leftAddr,
    N_uint32 rightAddr)
{
    if (_PM_hDevice) {
	DWORD   inBuf[2];   /* Buffer to send data to VxD       */
	DWORD   count;      /* Count of bytes returned from VxD */

	inBuf[0] = (ulong)leftAddr;
	inBuf[1] = (ulong)rightAddr;
	DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOFLIP32, inBuf, sizeof(inBuf),
	    NULL, 0, &count, NULL);
	}
}

/****************************************************************************
REMARKS:
This function turns off software stereo mode, either directly or via the VxD.
****************************************************************************/
static N_int32 NAPI _GA_softStereoGetFlipStatus(void)
{
    if (_PM_hDevice) {
	DWORD   outBuf[1];  /* Buffer to receive data from VxD  */
	DWORD   count;      /* Count of bytes returned from VxD */

	if (DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOFLIPSTATUS32, NULL, 0,
		outBuf, sizeof(outBuf), &count, NULL)) {
	    return outBuf[0];
	    }
	}
    return 0;
}

/****************************************************************************
REMARKS:
This function turns off software stereo mode, either directly or via the VxD.
****************************************************************************/
static void NAPI _GA_softStereoWaitTillFlipped(void)
{
    while (!_GA_softStereoGetFlipStatus())
	;
}

/****************************************************************************
REMARKS:
This function turns off software stereo mode, either directly or via the VxD.
****************************************************************************/
static void NAPI _GA_softStereoOff(void)
{
    if (_PM_hDevice) {
	DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOOFF32, NULL, 0,
	    NULL, 0, NULL, NULL);
	}
}

/****************************************************************************
REMARKS:
This function disable the software stereo handler, either directly or via
the VxD.
****************************************************************************/
static void NAPI _GA_softStereoExit(void)
{
    if (_PM_hDevice) {
	DeviceIoControl(_PM_hDevice, PMHELP_GASTEREOEXIT32, NULL, 0,
	    NULL, 0, NULL, NULL);
	}
}

/****************************************************************************
REMARKS:
We hook this function in here so that we can avoid the memory detect and
other destructive sequences in the drivers if we are loading the driver
from a Win32 application (our display drivers in contrast load them inside
the VxD directly, but the control panel applets use this function).
****************************************************************************/
static GA_devCtx * NAPI _GA_loadDriver(
    N_int32 deviceIndex,
    N_int32 shared)
{
    GA_devCtx   *dc;
    DWORD       inBuf[1];
    DWORD       outBuf[1];
    N_int32     totalMemory = 0,oldIOPL;

    if (deviceIndex >= GA_MAX_DEVICES)
	PM_fatalError("DeviceIndex too large in GA_loadDriver!");
    PM_init();
    inBuf[0] = deviceIndex;
    if (DeviceIoControl(_PM_hDevice, PMHELP_GETMEMSIZE32,
	    inBuf, sizeof(inBuf), outBuf, sizeof(outBuf), NULL, NULL))
	totalMemory = outBuf[0];
    if (totalMemory == 0)
	totalMemory = 8192;
    _GA_exports.GA_forceMemSize(totalMemory,shared);
    oldIOPL = PM_setIOPL(3);
    dc = ORG_GA_loadDriver(deviceIndex,shared);
    PM_setIOPL(oldIOPL);
    return dc;
}

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
