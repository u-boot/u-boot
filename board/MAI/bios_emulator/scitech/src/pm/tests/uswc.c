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
* Environment:  any
*
* Description:  Simple test program to test the write combine functions.
*
*               Note that this program should never be used in a production
*               environment, because write combining needs to be handled
*               with more intimate knowledge of the display hardware than
*               you can obtain by simply examining the PCI configuration
*               space.
*
****************************************************************************/

#include "pmapi.h"
#include "pcilib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*------------------------- Global Variables ------------------------------*/

static int              NumPCI = -1;
static PCIDeviceInfo    *PCI;
static int              *BridgeIndex;
static int              *DeviceIndex;
static int              NumBridges;
static PCIDeviceInfo    *AGPBridge = NULL;
static int              NumDevices;

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
RETURNS:
Number of display devices found.

REMARKS:
This function enumerates the number of available display devices on the
PCI bus, and returns the number found.
****************************************************************************/
static int PCI_enumerateDevices(void)
{
    int             i,j;
    PCIDeviceInfo   *info;

    /* If this is the first time we have been called, enumerate all */
    /* devices on the PCI bus. */
    if (NumPCI == -1) {
	if ((NumPCI = PCI_getNumDevices()) == 0)
	    return -1;
	PCI = malloc(NumPCI * sizeof(PCI[0]));
	BridgeIndex = malloc(NumPCI * sizeof(BridgeIndex[0]));
	DeviceIndex = malloc(NumPCI * sizeof(DeviceIndex[0]));
	if (!PCI || !BridgeIndex || !DeviceIndex)
	    return -1;
	for (i = 0; i < NumPCI; i++)
	    PCI[i].dwSize = sizeof(PCI[i]);
	if (PCI_enumerate(PCI) == 0)
	    return -1;

	/* Build a list of all PCI bridge devices */
	for (i = 0,NumBridges = 0,BridgeIndex[0] = -1; i < NumPCI; i++) {
	    if (PCI[i].BaseClass == PCI_BRIDGE_CLASS)
		BridgeIndex[NumBridges++] = i;
	    }

	/* Now build a list of all display class devices */
	for (i = 0,NumDevices = 1,DeviceIndex[0] = -1; i < NumPCI; i++) {
	    if (PCI_IS_DISPLAY_CLASS(&PCI[i])) {
		if ((PCI[i].Command & 0x3) == 0x3)
		    DeviceIndex[0] = i;
		else
		    DeviceIndex[NumDevices++] = i;
		if (PCI[i].slot.p.Bus != 0) {
		    /* This device is on a different bus than the primary */
		    /* PCI bus, so it is probably an AGP device. Find the */
		    /* AGP bus device that controls that bus so we can */
		    /* control it. */
		    for (j = 0; j < NumBridges; j++) {
			info = (PCIDeviceInfo*)&PCI[BridgeIndex[j]];
			if (info->u.type1.SecondayBusNumber == PCI[i].slot.p.Bus) {
			    AGPBridge = info;
			    break;
			    }
			}
		    }
		}
	    }
	}
    return NumDevices;
}

/****************************************************************************
REMARKS:
Enumerates useful information about attached display devices.
****************************************************************************/
static void ShowDisplayDevices(void)
{
    int i,index;

    printf("Displaying enumeration of %d PCI display devices\n", NumDevices);
    printf("\n");
    printf("DeviceID  SubSystem  Base10h  (length  )  Base14h  (length  )\n");
    for (index = 0; index < NumDevices; index++) {
	i = DeviceIndex[index];
	printf("%04X:%04X %04X:%04X  %08lX (%6ld KB) %08lX (%6ld KB)\n",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].u.type0.SubSystemVendorID,
	    PCI[i].u.type0.SubSystemID,
	    PCI[i].u.type0.BaseAddress10,
	    PCI[i].u.type0.BaseAddress10Len / 1024,
	    PCI[i].u.type0.BaseAddress14,
	    PCI[i].u.type0.BaseAddress14Len / 1024);
	}
    printf("\n");
}

/****************************************************************************
REMARKS:
Dumps the value for a write combine region to the display.
****************************************************************************/
static char *DecodeWCType(
    uint type)
{
    static char *names[] = {
	"UNCACHABLE",
	"WRCOMB",
	"UNKNOWN",
	"UNKNOWN",
	"WRTHROUGH",
	"WRPROT",
	"WRBACK",
	};
    if (type <= PM_MTRR_MAX)
	return names[type];
    return "UNKNOWN";
}

/****************************************************************************
REMARKS:
Dumps the value for a write combine region to the display.
****************************************************************************/
static void PMAPI EnumWriteCombine(
    ulong base,
    ulong length,
    uint type)
{
    printf("%08lX %-10ld %s\n", base, length / 1024, DecodeWCType(type));
}

/****************************************************************************
PARAMETERS:
err - Error to log

REMARKS:
Function to log an error message if the MTRR write combining attempt failed.
****************************************************************************/
static void LogMTRRError(
    int err)
{
    if (err == PM_MTRR_ERR_OK)
	return;
    switch (err) {
	case PM_MTRR_NOT_SUPPORTED:
	    printf("Failed: MTRR is not supported by host CPU\n");
	    break;
	case PM_MTRR_ERR_PARAMS:
	    printf("Failed: Invalid parameters passed to PM_enableWriteCombined!\n");
	    break;
	case PM_MTRR_ERR_NOT_4KB_ALIGNED:
	    printf("Failed: Address is not 4Kb aligned!\n");
	    break;
	case PM_MTRR_ERR_BELOW_1MB:
	    printf("Failed: Addresses below 1Mb cannot be write combined!\n");
	    break;
	case PM_MTRR_ERR_NOT_ALIGNED:
	    printf("Failed: Address is not correctly aligned for processor!\n");
	    break;
	case PM_MTRR_ERR_OVERLAP:
	    printf("Failed: Address overlaps an existing region!\n");
	    break;
	case PM_MTRR_ERR_TYPE_MISMATCH:
	    printf("Failed: Adress is contained with existing region, but type is different!\n");
	    break;
	case PM_MTRR_ERR_NONE_FREE:
	    printf("Failed: Out of MTRR registers!\n");
	    break;
	case PM_MTRR_ERR_NOWRCOMB:
	    printf("Failed: This processor does not support write combining!\n");
	    break;
	case PM_MTRR_ERR_NO_OS_SUPPORT:
	    printf("Failed: MTRR is not supported by host OS\n");
	    break;
	default:
	    printf("Failed: UNKNOWN ERROR!\n");
	    break;
	}
    exit(-1);
}

/****************************************************************************
REMARKS:
Shows all write combine regions.
****************************************************************************/
static void ShowWriteCombine(void)
{
    printf("Base     Length(KB) Type\n");
    LogMTRRError(PM_enumWriteCombine(EnumWriteCombine));
    printf("\n");
}

/****************************************************************************
REMARKS:
Dumps the value for a write combine region to the display.
****************************************************************************/
static void EnableWriteCombine(void)
{
    int i,index;

    for (index = 0; index < NumDevices; index++) {
	i = DeviceIndex[index];
	if (PCI[i].u.type0.BaseAddress10 & 0x8) {
	    LogMTRRError(PM_enableWriteCombine(
		PCI[i].u.type0.BaseAddress10 & 0xFFFFFFF0,
		PCI[i].u.type0.BaseAddress10Len,
		PM_MTRR_WRCOMB));
	    }
	if (PCI[i].u.type0.BaseAddress14 & 0x8) {
	    LogMTRRError(PM_enableWriteCombine(
		PCI[i].u.type0.BaseAddress14 & 0xFFFFFFF0,
		PCI[i].u.type0.BaseAddress14Len,
		PM_MTRR_WRCOMB));
	    }
	}
    printf("\n");
    ShowDisplayDevices();
    ShowWriteCombine();
}

/****************************************************************************
REMARKS:
Dumps the value for a write combine region to the display.
****************************************************************************/
static void DisableWriteCombine(void)
{
    int i,index;

    for (index = 0; index < NumDevices; index++) {
	i = DeviceIndex[index];
	if (PCI[i].u.type0.BaseAddress10 & 0x8) {
	    LogMTRRError(PM_enableWriteCombine(
		PCI[i].u.type0.BaseAddress10 & 0xFFFFFFF0,
		PCI[i].u.type0.BaseAddress10Len,
		PM_MTRR_UNCACHABLE));
	    }
	if (PCI[i].u.type0.BaseAddress14 & 0x8) {
	    LogMTRRError(PM_enableWriteCombine(
		PCI[i].u.type0.BaseAddress14 & 0xFFFFFFF0,
		PCI[i].u.type0.BaseAddress14Len,
		PM_MTRR_UNCACHABLE));
	    }
	}
    printf("\n");
    ShowDisplayDevices();
    ShowWriteCombine();
}

int main(int argc,char *argv[])
{
    PM_init();
    if (PCI_enumerateDevices() < 1) {
	printf("No PCI display devices found!\n");
	return -1;
	}
    if (argc < 2) {
	printf("usage: uswc [-show -on -off]\n\n");
	ShowDisplayDevices();
	return -1;
	}
    if (stricmp(argv[1],"-show") == 0)
	ShowWriteCombine();
    else if (stricmp(argv[1],"-on") == 0)
	EnableWriteCombine();
    else if (stricmp(argv[1],"-off") == 0)
	DisableWriteCombine();
    return 0;
}
