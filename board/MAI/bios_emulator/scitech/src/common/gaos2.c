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

/*--------------------------- Global variables ----------------------------*/

static ibool    haveRDTSC = false;
static ulong    parms[3];       /* Must not cross 64Kb boundary!    */
static ulong    result[4];      /* Must not cross 64Kb boundary!    */

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
PARAMETERS:
func        - Helper device driver function to call

RETURNS:
First return value from the device driver in parmsOut[0]

REMARKS:
Function to open our helper device driver, call it and close the file
handle. Note that we have to open the device driver for every call because
of two problems:

 1. We cannot open a single file handle in a DLL that is shared amongst
    programs, since every process must have it's own open file handle.

 2. For some reason there appears to be a limit of about 12 open file
    handles on a device driver in the system. Hence when we open more
    than about 12 file handles things start to go very strange.

Hence we simply open the file handle every time that we need to call the
device driver to work around these problems.
****************************************************************************/
static ulong CallSDDHelp(
    int func)
{
    static ulong    inLen;          /* Must not cross 64Kb boundary!    */
    static ulong    outLen;         /* Must not cross 64Kb boundary!    */
    HFILE           hSDDHelp;

    /* If this code in here fails, we are screwed! Many of our drivers
     * use this code and don't have a C library, so we simply assume we
     * can't fail here.
     */
    DosOpen(PMHELP_NAME,&hSDDHelp,&result[0],0,0,
	    FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
	    NULL);
    DosDevIOCtl(hSDDHelp,PMHELP_IOCTL,func,
	     &parms, inLen = sizeof(parms), &inLen,
	    &result, outLen = sizeof(result), &outLen);
    DosClose(hSDDHelp);
    return result[0];
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
****************************************************************************/
void NAPI GA_setLocalPath(
    const char *path)
{
    PM_setLocalBPDPath(path);
}

/****************************************************************************
RETURNS:
Pointer to the system wide PM library imports, or the internal version if none

REMARKS:
For OS/2 we don't need to do anything special because Nucleus is always
loaded via the shared SDDPMI driver when SDD is loaded so we don't need
a system wide PM library imports function.
****************************************************************************/
PM_imports * NAPI GA_getSystemPMImports(void)
{
    return &_PM_imports;
}

/****************************************************************************
PARAMETERS:
gaExp   - Place to store the exported functions
shared  - True if connecting to the shared, global Nucleus driver

REMARKS:
For OS/2 if SDD is loaded we *always* connect to the shared Nucleus functions
contained within the SDDPMI driver. This allows the Nucleus functions contained
within this driver to be utilised by all Nucleus apps in the system and
maintains a consistent state between versions.
****************************************************************************/
ibool NAPI GA_getSharedExports(
    GA_exports *gaExp,
    ibool shared)
{
    /* In test harness mode, we need to load a local copy of Nucleus */
#if !defined (TEST_HARNESS) || defined (DEBUG_SDDPMI)
    HMODULE     hModSDDPMI;
    char        buf[80];
    GA_exports  *exp;

    /* Initialise the PM library and connect to our runtime DLL's */
    PM_init();
    if (CallSDDHelp(PMHELP_GETSHAREDEXP) != 0) {
	/* We have found the shared Nucleus exports. Because not all processes
	 * map to SDDPMI.DLL, we need to ensure that we connect to this
	 * DLL so that it gets mapped into our address space (that is
	 * where the shared Nucleus loader code is located). Simply doing a
	 * DosLoadModule on it is enough for this.
	 */
	DosLoadModule((PSZ)buf,sizeof(buf),(PSZ)"SDDPMI.DLL",&hModSDDPMI);
	exp = (GA_exports*)result[0];
	memcpy(gaExp,exp,MIN(gaExp->dwSize,exp->dwSize));
	return true;
	}
#endif
    (void)shared;
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

/****************************************************************************
REMARKS:
On OS/2, we need special memory allocation functions if we build SDDPMI in
test harness mode. But if we build GATest etc. in test mode, we want to use
the normal C runtime functions, so route them back here.
****************************************************************************/

#if defined (TEST_HARNESS) && !defined (DEBUG_SDDPMI)

/* Undefine these macros first or we'll recurse to hell! */
#undef malloc
#undef calloc
#undef realloc
#undef free

void *SDDPMI_malloc(size_t size) {
    return malloc(size);
}

void *SDDPMI_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

void SDDPMI_free(void *ptr) {
    free(ptr);
}

void *SDDPMI_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

#endif
