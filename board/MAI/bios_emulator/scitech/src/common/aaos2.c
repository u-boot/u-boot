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
* Environment:  OS/2 32-bit
*
* Description:  OS specific Nucleus Graphics Architecture services for
*               the OS/2 operating system environments.
*
****************************************************************************/

#include "pm_help.h"
#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_SUB
#define INCL_VIO
#define INCL_KBD
#include <os2.h>

/*---------------------------- Global Variables ---------------------------*/

static HFILE        hSDDHelp;
static ulong        outLen;         /* Must not cross 64Kb boundary!    */
static ulong        result;         /* Must not cross 64Kb boundary!    */
static ibool        haveRDTSC;

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
REMARKS:
This function returns a pointer to the common graphics driver loaded in the
helper VxD. The memory for the VxD is shared between all processes via
the VxD, so that the VxD, 16-bit code and 32-bit code all see the same
state when accessing the graphics binary portable driver.
****************************************************************************/
GA_sharedInfo * NAPI GA_getSharedInfo(
    int device)
{
    /* Initialise the PM library and connect to our runtime DLL's */
    PM_init();

    /* Open our helper device driver */
    if (DosOpen(PMHELP_NAME,&hSDDHelp,&result,0,0,
	    FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
	    NULL))
	PM_fatalError("Unable to open SDDHELP$ helper device driver!");
    outLen = sizeof(result);
    DosDevIOCtl(hSDDHelp,PMHELP_IOCTL,PMHELP_GETSHAREDINFO,
	NULL, 0, NULL,
	&result, outLen, &outLen);
    DosClose(hSDDHelp);
    if (result) {
	/* We have found the shared Nucleus packet. Because not all processes
	 * map to SDDPMI.DLL, we need to ensure that we connect to this
	 * DLL so that it gets mapped into our address space (that is
	 * where the shared Nucleus packet is located). Simply doing a
	 * DosLoadModule on it is enough for this.
	 */
	HMODULE hModSDDPMI;
	char    buf[80];
	DosLoadModule((PSZ)buf,sizeof(buf),(PSZ)"SDDPMI.DLL",&hModSDDPMI);
	}
    return (GA_sharedInfo*)result;
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
This function initialises the high precision timing functions for the
Nucleus loader library.
****************************************************************************/
ibool NAPI GA_TimerInit(void)
{
    if (_GA_haveCPUID() && (_GA_getCPUIDFeatures() & CPU_HaveRDTSC) != 0)
	haveRDTSC = true;
    return true;
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
	DosTmrQueryTime((QWORD*)value);
}
