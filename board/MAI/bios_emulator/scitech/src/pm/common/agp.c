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
* Environment:  32-bit Ring 0 device driver
*
* Description:  Generic module to implement AGP support functions using the
*               SciTech Nucleus AGP support drivers. If the OS provides
*               native AGP support, this module should *NOT* be used. Instead
*               wrappers should be placed around the OS support functions
*               to implement this functionality.
*
****************************************************************************/

#include "pmapi.h"
#ifndef REALMODE
#include "nucleus/agp.h"

/*--------------------------- Global variables ----------------------------*/

static AGP_devCtx       *agp;
static AGP_driverFuncs  driver;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
RETURNS:
Size of AGP aperture in MB on success, 0 on failure.

REMARKS:
This function initialises the AGP driver in the system and returns the
size of the available AGP aperture in megabytes.
****************************************************************************/
ulong PMAPI PM_agpInit(void)
{
    if ((agp = AGP_loadDriver(0)) == NULL)
	return 0;
    driver.dwSize = sizeof(driver);
    if (!agp->QueryFunctions(AGP_GET_DRIVERFUNCS,&driver))
	return 0;
    switch (driver.GetApertureSize()) {
	case agpSize4MB:    return 4;
	case agpSize8MB:    return 8;
	case agpSize16MB:   return 16;
	case agpSize32MB:   return 32;
	case agpSize64MB:   return 64;
	case agpSize128MB:  return 128;
	case agpSize256MB:  return 256;
	case agpSize512MB:  return 512;
	case agpSize1GB:    return 1024;
	case agpSize2GB:    return 2048;
	}
    return 0;
}

/****************************************************************************
REMARKS:
This function closes down the loaded AGP driver.
****************************************************************************/
void PMAPI PM_agpExit(void)
{
    AGP_unloadDriver(agp);
}

/****************************************************************************
PARAMETERS:
numPages    - Number of memory pages that should be reserved
type        - Type of memory to allocate
physContext - Returns the physical context handle for the mapping
physAddr    - Returns the physical address for the mapping

RETURNS:
True on success, false on failure.

REMARKS:
This function reserves a range of physical memory addresses on the system
bus which the AGP controller will respond to. If this function succeeds,
the AGP controller can respond to the reserved physical address range on
the bus. However you must first call AGP_commitPhysical to cause this memory
to actually be committed for use before it can be accessed.
****************************************************************************/
ibool PMAPI PM_agpReservePhysical(
    ulong numPages,
    int type,
    void **physContext,
    PM_physAddr *physAddr)
{
    switch (type) {
	case PM_agpUncached:
	    type = agpUncached;
	    break;
	case PM_agpWriteCombine:
	    type = agpWriteCombine;
	    break;
	case PM_agpIntelDCACHE:
	    type = agpIntelDCACHE;
	    break;
	default:
	    return false;
	}
    return driver.ReservePhysical(numPages,type,physContext,physAddr) == nOK;
}

/****************************************************************************
PARAMETERS:
physContext - Physical AGP context to release

RETURNS:
True on success, false on failure.

REMARKS:
This function releases a range of physical memory addresses on the system
bus which the AGP controller will respond to. All committed memory for
the physical address range covered by the context will be released.
****************************************************************************/
ibool PMAPI PM_agpReleasePhysical(
    void *physContext)
{
    return driver.ReleasePhysical(physContext) == nOK;
}

/****************************************************************************
PARAMETERS:
physContext - Physical AGP context to commit memory for
numPages    - Number of pages to be committed
startOffset - Offset in pages into the reserved physical context
physAddr    - Returns the physical address of the committed memory

RETURNS:
True on success, false on failure.

REMARKS:
This function commits into the specified physical context that was previously
reserved by a call to ReservePhysical. You can use the startOffset and
numPages parameters to only commit portions of the reserved memory range at
a time.
****************************************************************************/
ibool PMAPI PM_agpCommitPhysical(
    void *physContext,
    ulong numPages,
    ulong startOffset,
    PM_physAddr *physAddr)
{
    return driver.CommitPhysical(physContext,numPages,startOffset,physAddr) == nOK;
}

/****************************************************************************
PARAMETERS:
physContext - Physical AGP context to free memory for
numPages    - Number of pages to be freed
startOffset - Offset in pages into the reserved physical context

RETURNS:
True on success, false on failure.

REMARKS:
This function frees memory previously committed by the CommitPhysical
function. Note that you can free a portion of a memory range that was
previously committed if you wish.
****************************************************************************/
ibool PMAPI PM_agpFreePhysical(
    void *physContext,
    ulong numPages,
    ulong startOffset)
{
    return driver.FreePhysical(physContext,numPages,startOffset) == nOK;
}

#endif  /* !REALMODE */
