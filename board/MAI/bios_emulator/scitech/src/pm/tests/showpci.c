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
* Description:  Test program to test the PCI library functions.
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
REMARKS:
Enumerates the PCI bus and dumps the PCI configuration information to the
log file.
****************************************************************************/
static void EnumeratePCI(void)
{
    int             i,index;
    PCIDeviceInfo   *info;

    printf("Displaying enumeration of PCI bus (%d devices, %d display devices)\n",
	NumPCI, NumDevices);
    for (index = 0; index < NumDevices; index++)
	printf("  Display device %d is PCI device %d\n",index,DeviceIndex[index]);
    printf("\n");
    printf("Bus Slot Fnc DeviceID  SubSystem Rev Class IRQ Int Cmd\n");
    for (i = 0; i < NumPCI; i++) {
	printf("%2d   %2d  %2d  %04X:%04X %04X:%04X %02X  %02X:%02X %02X  %02X  %04X   ",
	    PCI[i].slot.p.Bus,
	    PCI[i].slot.p.Device,
	    PCI[i].slot.p.Function,
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].u.type0.SubSystemVendorID,
	    PCI[i].u.type0.SubSystemID,
	    PCI[i].RevID,
	    PCI[i].BaseClass,
	    PCI[i].SubClass,
	    PCI[i].u.type0.InterruptLine,
	    PCI[i].u.type0.InterruptPin,
	    PCI[i].Command);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printf("<- %d\n", index);
	else
	    printf("\n");
	}
    printf("\n");
    printf("DeviceID  Stat Ifc Cch Lat Hdr BIST\n");
    for (i = 0; i < NumPCI; i++) {
	printf("%04X:%04X %04X  %02X  %02X  %02X  %02X  %02X   ",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].Status,
	    PCI[i].Interface,
	    PCI[i].CacheLineSize,
	    PCI[i].LatencyTimer,
	    PCI[i].HeaderType,
	    PCI[i].BIST);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printf("<- %d\n", index);
	else
	    printf("\n");
	}
    printf("\n");
    printf("DeviceID  Base10h  Base14h  Base18h  Base1Ch  Base20h  Base24h  ROMBase\n");
    for (i = 0; i < NumPCI; i++) {
	printf("%04X:%04X %08lX %08lX %08lX %08lX %08lX %08lX %08lX ",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].u.type0.BaseAddress10,
	    PCI[i].u.type0.BaseAddress14,
	    PCI[i].u.type0.BaseAddress18,
	    PCI[i].u.type0.BaseAddress1C,
	    PCI[i].u.type0.BaseAddress20,
	    PCI[i].u.type0.BaseAddress24,
	    PCI[i].u.type0.ROMBaseAddress);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printf("<- %d\n", index);
	else
	    printf("\n");
	}
    printf("\n");
    printf("DeviceID  BAR10Len BAR14Len BAR18Len BAR1CLen BAR20Len BAR24Len ROMLen\n");
    for (i = 0; i < NumPCI; i++) {
	printf("%04X:%04X %08lX %08lX %08lX %08lX %08lX %08lX %08lX ",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].u.type0.BaseAddress10Len,
	    PCI[i].u.type0.BaseAddress14Len,
	    PCI[i].u.type0.BaseAddress18Len,
	    PCI[i].u.type0.BaseAddress1CLen,
	    PCI[i].u.type0.BaseAddress20Len,
	    PCI[i].u.type0.BaseAddress24Len,
	    PCI[i].u.type0.ROMBaseAddressLen);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printf("<- %d\n", index);
	else
	    printf("\n");
	}
    printf("\n");
    printf("Displaying enumeration of %d bridge devices\n",NumBridges);
    printf("\n");
    printf("DeviceID  P# S# B# IOB  IOL  MemBase  MemLimit PreBase  PreLimit Ctrl\n");
    for (i = 0; i < NumBridges; i++) {
	info = (PCIDeviceInfo*)&PCI[BridgeIndex[i]];
	printf("%04X:%04X %02X %02X %02X %04X %04X %08X %08X %08X %08X %04X\n",
	    info->VendorID,
	    info->DeviceID,
	    info->u.type1.PrimaryBusNumber,
	    info->u.type1.SecondayBusNumber,
	    info->u.type1.SubordinateBusNumber,
	    ((u16)info->u.type1.IOBase << 8) & 0xF000,
	    info->u.type1.IOLimit ?
		((u16)info->u.type1.IOLimit << 8) | 0xFFF : 0,
	    ((u32)info->u.type1.MemoryBase << 16) & 0xFFF00000,
	    info->u.type1.MemoryLimit ?
		((u32)info->u.type1.MemoryLimit << 16) | 0xFFFFF : 0,
	    ((u32)info->u.type1.PrefetchableMemoryBase << 16) & 0xFFF00000,
	    info->u.type1.PrefetchableMemoryLimit ?
		((u32)info->u.type1.PrefetchableMemoryLimit << 16) | 0xFFFFF : 0,
	    info->u.type1.BridgeControl);
	}
    printf("\n");
}

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

	/* Enumerate all PCI and bridge devices to standard output */
	EnumeratePCI();
	}
    return NumDevices;
}

int main(void)
{
    /* Enumerate all PCI devices */
    PM_init();
    if (PCI_enumerateDevices() < 1) {
	printf("No PCI display devices found!\n");
	return -1;
	}
    return 0;
}
