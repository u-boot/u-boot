/****************************************************************************
*
*                        BIOS emulator and interface
*                      to Realmode X86 Emulator Library
*
*               Copyright (C) 1996-1999 SciTech Software, Inc.
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
* Developer:    Kendall Bennett
*
* Description:  Module to implement warm booting of all PCI/AGP controllers
*               on the bus. We use the x86 real mode emulator to run the
*               BIOS on the primary and secondary controllers to bring
*               the cards up.
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "biosemu.h"
#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

/*------------------------- Global Variables ------------------------------*/

static PCIDeviceInfo    PCI[MAX_PCI_DEVICES];
static int              NumPCI = -1;
static int              BridgeIndex[MAX_PCI_DEVICES] = {0};
static int              NumBridges;
static PCIBridgeInfo    *AGPBridge = NULL;
static int              DeviceIndex[MAX_PCI_DEVICES] = {0};
static int              NumDevices;
static u32              debugFlags = 0;
static BE_VGAInfo       VGAInfo[MAX_PCI_DEVICES] = {{0}};
static ibool            useV86 = false;
static ibool            forcePost = false;

/* Length of the BIOS image */

#define MAX_BIOSLEN         (64 * 1024L)
#define FINAL_BIOSLEN       (32 * 1024L)

/* Macro to determine if the VGA is enabled and responding */

#define VGA_NOT_ACTIVE()    (forcePost || (PM_inpb(0x3CC) == 0xFF) || ((PM_inpb(0x3CC) & 0x2) == 0))

#define ENABLE_DEVICE(device)   \
    PCI_writePCIRegB(0x4,PCI[DeviceIndex[device]].Command | 0x7,device)

#define DISABLE_DEVICE(device)  \
    PCI_writePCIRegB(0x4,0,device)

/* Macros to enable and disable AGP VGA resources */

#define ENABLE_AGP_VGA()    \
    PCI_accessReg(0x3E,AGPBridge->BridgeControl | 0x8,PCI_WRITE_WORD,(PCIDeviceInfo*)AGPBridge)

#define DISABLE_AGP_VGA()   \
    PCI_accessReg(0x3E,AGPBridge->BridgeControl & ~0x8,PCI_WRITE_WORD,(PCIDeviceInfo*)AGPBridge)

#define RESTORE_AGP_VGA()   \
    PCI_accessReg(0x3E,AGPBridge->BridgeControl,PCI_WRITE_WORD,(PCIDeviceInfo*)AGPBridge)

/*-------------------------- Implementation -------------------------------*/

/****************************************************************************
RETURNS:
The address to use to map the secondary BIOS (PCI/AGP devices)

REMARKS:
Searches all the PCI base address registers for the device looking for a
memory mapping that is large enough to hold our ROM BIOS. We usually end up
finding the framebuffer mapping (usually BAR 0x10), and we use this mapping
to map the BIOS for the device into. We use a mapping that is already
assigned to the device to ensure the memory range will be passed through
by any PCI->PCI or AGP->PCI bridge that may be present.

NOTE: Usually this function is only used for AGP devices, but it may be
      used for PCI devices that have already been POST'ed and the BIOS
      ROM base address has been zero'ed out.
****************************************************************************/
static ulong PCI_findBIOSAddr(
    int device)
{
    ulong   base,size;
    int     bar;

    for (bar = 0x10; bar <= 0x14; bar++) {
	base = PCI_readPCIRegL(bar,device) & ~0xFF;
	if (!(base & 0x1)) {
	    PCI_writePCIRegL(bar,0xFFFFFFFF,device);
	    size = PCI_readPCIRegL(bar,device) & ~0xFF;
	    size = ~size+1;
	    PCI_writePCIRegL(bar,0,device);
	    if (size >= MAX_BIOSLEN)
		return base;
	    }
	}
    return 0;
}

/****************************************************************************
REMARKS:
Re-writes the PCI base address registers for the secondary PCI controller
with the values from our initial PCI bus enumeration. This fixes up the
values after we have POST'ed the secondary display controller BIOS, which
may have incorrectly re-programmed the base registers the same as the
primary display controller (the case for identical S3 cards).
****************************************************************************/
static void _PCI_fixupSecondaryBARs(void)
{
    int i;

    for (i = 0; i < NumDevices; i++) {
	PCI_writePCIRegL(0x10,PCI[DeviceIndex[i]].BaseAddress10,i);
	PCI_writePCIRegL(0x14,PCI[DeviceIndex[i]].BaseAddress14,i);
	PCI_writePCIRegL(0x18,PCI[DeviceIndex[i]].BaseAddress18,i);
	PCI_writePCIRegL(0x1C,PCI[DeviceIndex[i]].BaseAddress1C,i);
	PCI_writePCIRegL(0x20,PCI[DeviceIndex[i]].BaseAddress20,i);
	PCI_writePCIRegL(0x24,PCI[DeviceIndex[i]].BaseAddress24,i);
	}
}

/****************************************************************************
RETURNS:
True if successfully initialised, false if not.

REMARKS:
This function executes the BIOS POST code on the controller. We assume that
at this stage the controller has its I/O and memory space enabled and
that all other controllers are in a disabled state.
****************************************************************************/
static void PCI_doBIOSPOST(
    int device,
    ulong BIOSPhysAddr,
    void *mappedBIOS,
    ulong BIOSLen)
{
    RMREGS          regs;
    RMSREGS         sregs;

    /* Determine the value to store in AX for BIOS POST */
    regs.x.ax = (u16)(PCI[DeviceIndex[device]].slot.i >> 8);
    if (useV86) {
	/* Post the BIOS using the PM functions (ie: v86 mode on Linux) */
	if (!PM_doBIOSPOST(regs.x.ax,BIOSPhysAddr,mappedBIOS,BIOSLen)) {
	    /* If the PM function fails, this probably means are we are on */
	    /* DOS and can't re-map the real mode 0xC0000 region. In thise */
	    /* case if the device is the primary, we can use the real */
	    /* BIOS at 0xC0000 directly. */
	    if (device == 0)
		PM_doBIOSPOST(regs.x.ax,0xC0000,mappedBIOS,BIOSLen);
	    }
	}
    else {
	/* Setup the X86 emulator for the VGA BIOS */
	BE_setVGA(&VGAInfo[device]);

	/* Execute the BIOS POST code */
	BE_callRealMode(0xC000,0x0003,&regs,&sregs);

	/* Cleanup and exit */
	BE_getVGA(&VGAInfo[device]);
	}
}

/****************************************************************************
RETURNS:
True if successfully initialised, false if not.

REMARKS:
Loads and POST's the secondary controllers BIOS, directly from the BIOS
image we can extract over the PCI bus.
****************************************************************************/
static ibool PCI_postControllers(void)
{
    int     device;
    ulong   BIOSImageLen,mappedBIOSPhys;
    uchar   *mappedBIOS,*copyOfBIOS;
    char    filename[_MAX_PATH];
    FILE    *f;

    /* Disable the primary display controller and AGP VGA pass-through */
    DISABLE_DEVICE(0);
    if (AGPBridge)
	DISABLE_AGP_VGA();

    /* Now POST all the secondary controllers */
    for (device = 0; device < NumDevices; device++) {
	/* Skip the device if it is not enabled (probably an ISA device) */
	if (DeviceIndex[device] == -1)
	    continue;

	/* Enable secondary display controller. If the secondary controller */
	/* is on the AGP bus, then enable VGA resources for the AGP device. */
	ENABLE_DEVICE(device);
	if (AGPBridge && AGPBridge->SecondayBusNumber == PCI[DeviceIndex[device]].slot.p.Bus)
	    ENABLE_AGP_VGA();

	/* Check if the controller has already been POST'ed */
	if (VGA_NOT_ACTIVE()) {
	    /* Find a viable place to map the secondary PCI BIOS image and map it */
	    printk("Device %d not enabled, so attempting warm boot it\n", device);

	    /* For AGP devices (and PCI devices that do have the ROM base */
	    /* address zero'ed out) we have to map the BIOS to a location */
	    /* that is passed by the AGP bridge to the bus. Some AGP devices */
	    /* have the ROM base address already set up for us, and some */
	    /* do not (we map to one of the existing BAR locations in */
	    /* this case). */
	    mappedBIOS = NULL;
	    if (PCI[DeviceIndex[device]].ROMBaseAddress != 0)
		mappedBIOSPhys = PCI[DeviceIndex[device]].ROMBaseAddress & ~0xF;
	    else
		mappedBIOSPhys = PCI_findBIOSAddr(device);
	    printk("Mapping BIOS image to 0x%08X\n", mappedBIOSPhys);
	    mappedBIOS = PM_mapPhysicalAddr(mappedBIOSPhys,MAX_BIOSLEN-1,false);
	    PCI_writePCIRegL(0x30,mappedBIOSPhys | 0x1,device);
	    BIOSImageLen = mappedBIOS[2] * 512;
	    if ((copyOfBIOS = malloc(BIOSImageLen)) == NULL)
		return false;
	    memcpy(copyOfBIOS,mappedBIOS,BIOSImageLen);
	    PM_freePhysicalAddr(mappedBIOS,MAX_BIOSLEN-1);

	    /* Allocate memory to store copy of BIOS from secondary controllers */
	    VGAInfo[device].pciInfo = &PCI[DeviceIndex[device]];
	    VGAInfo[device].BIOSImage = copyOfBIOS;
	    VGAInfo[device].BIOSImageLen = BIOSImageLen;

	    /* Restore device mappings */
	    PCI_writePCIRegL(0x30,PCI[DeviceIndex[device]].ROMBaseAddress,device);
	    PCI_writePCIRegL(0x10,PCI[DeviceIndex[device]].BaseAddress10,device);
	    PCI_writePCIRegL(0x14,PCI[DeviceIndex[device]].BaseAddress14,device);

	    /* Now execute the BIOS POST for the device */
	    if (copyOfBIOS[0] == 0x55 && copyOfBIOS[1] == 0xAA) {
		printk("Executing BIOS POST for controller.\n");
		PCI_doBIOSPOST(device,mappedBIOSPhys,copyOfBIOS,BIOSImageLen);
		}

	    /* Reset the size of the BIOS image to the final size */
	    VGAInfo[device].BIOSImageLen = FINAL_BIOSLEN;

	    /* Save the BIOS and interrupt vector information to disk */
	    sprintf(filename,"%s/bios.%02d",PM_getNucleusConfigPath(),device);
	    if ((f = fopen(filename,"wb")) != NULL) {
		fwrite(copyOfBIOS,1,FINAL_BIOSLEN,f);
		fwrite(VGAInfo[device].LowMem,1,sizeof(VGAInfo[device].LowMem),f);
		fclose(f);
		}
	    }
	else {
	    /* Allocate memory to store copy of BIOS from secondary controllers */
	    if ((copyOfBIOS = malloc(FINAL_BIOSLEN)) == NULL)
		return false;
	    VGAInfo[device].pciInfo = &PCI[DeviceIndex[device]];
	    VGAInfo[device].BIOSImage = copyOfBIOS;
	    VGAInfo[device].BIOSImageLen = FINAL_BIOSLEN;

	    /* Load the BIOS and interrupt vector information from disk */
	    sprintf(filename,"%s/bios.%02d",PM_getNucleusConfigPath(),device);
	    if ((f = fopen(filename,"rb")) != NULL) {
		fread(copyOfBIOS,1,FINAL_BIOSLEN,f);
		fread(VGAInfo[device].LowMem,1,sizeof(VGAInfo[device].LowMem),f);
		fclose(f);
		}
	    }

	/* Fix up all the secondary PCI base address registers */
	/* (restores them all from the values we read previously) */
	_PCI_fixupSecondaryBARs();

	/* Disable the secondary controller and AGP VGA pass-through */
	DISABLE_DEVICE(device);
	if (AGPBridge)
	    DISABLE_AGP_VGA();
	}

    /* Reenable primary display controller and reset AGP bridge control */
    if (AGPBridge)
	RESTORE_AGP_VGA();
    ENABLE_DEVICE(0);

    /* Free physical BIOS image mapping */
    PM_freePhysicalAddr(mappedBIOS,MAX_BIOSLEN-1);

    /* Restore the X86 emulator BIOS info to primary controller */
    if (!useV86)
	BE_setVGA(&VGAInfo[0]);
    return true;
}

/****************************************************************************
REMARKS:
Enumerates the PCI bus and dumps the PCI configuration information to the
log file.
****************************************************************************/
static void EnumeratePCI(void)
{
    int             i,index;
    PCIBridgeInfo   *info;

    printk("Displaying enumeration of PCI bus (%d devices, %d display devices)\n",
	NumPCI, NumDevices);
    for (index = 0; index < NumDevices; index++)
	printk("  Display device %d is PCI device %d\n",index,DeviceIndex[index]);
    printk("\n");
    printk("Bus Slot Fnc DeviceID  SubSystem Rev Class IRQ Int Cmd\n");
    for (i = 0; i < NumPCI; i++) {
	printk("%2d   %2d  %2d  %04X:%04X %04X:%04X %02X  %02X:%02X %02X  %02X  %04X   ",
	    PCI[i].slot.p.Bus,
	    PCI[i].slot.p.Device,
	    PCI[i].slot.p.Function,
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].SubSystemVendorID,
	    PCI[i].SubSystemID,
	    PCI[i].RevID,
	    PCI[i].BaseClass,
	    PCI[i].SubClass,
	    PCI[i].InterruptLine,
	    PCI[i].InterruptPin,
	    PCI[i].Command);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printk("<- %d\n", index);
	else
	    printk("\n");
	}
    printk("\n");
    printk("DeviceID  Stat Ifc Cch Lat Hdr BIST\n");
    for (i = 0; i < NumPCI; i++) {
	printk("%04X:%04X %04X  %02X  %02X  %02X  %02X  %02X   ",
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
	    printk("<- %d\n", index);
	else
	    printk("\n");
	}
    printk("\n");
    printk("DeviceID  Base10h  Base14h  Base18h  Base1Ch  Base20h  Base24h  ROMBase\n");
    for (i = 0; i < NumPCI; i++) {
	printk("%04X:%04X %08X %08X %08X %08X %08X %08X %08X ",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].BaseAddress10,
	    PCI[i].BaseAddress14,
	    PCI[i].BaseAddress18,
	    PCI[i].BaseAddress1C,
	    PCI[i].BaseAddress20,
	    PCI[i].BaseAddress24,
	    PCI[i].ROMBaseAddress);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printk("<- %d\n", index);
	else
	    printk("\n");
	}
    printk("\n");
    printk("DeviceID  BAR10Len BAR14Len BAR18Len BAR1CLen BAR20Len BAR24Len ROMLen\n");
    for (i = 0; i < NumPCI; i++) {
	printk("%04X:%04X %08X %08X %08X %08X %08X %08X %08X ",
	    PCI[i].VendorID,
	    PCI[i].DeviceID,
	    PCI[i].BaseAddress10Len,
	    PCI[i].BaseAddress14Len,
	    PCI[i].BaseAddress18Len,
	    PCI[i].BaseAddress1CLen,
	    PCI[i].BaseAddress20Len,
	    PCI[i].BaseAddress24Len,
	    PCI[i].ROMBaseAddressLen);
	for (index = 0; index < NumDevices; index++) {
	    if (DeviceIndex[index] == i)
		break;
	    }
	if (index < NumDevices)
	    printk("<- %d\n", index);
	else
	    printk("\n");
	}
    printk("\n");
    printk("Displaying enumeration of %d bridge devices\n",NumBridges);
    printk("\n");
    printk("DeviceID  P# S# B# IOB  IOL  MemBase  MemLimit PreBase  PreLimit Ctrl\n");
    for (i = 0; i < NumBridges; i++) {
	info = (PCIBridgeInfo*)&PCI[BridgeIndex[i]];
	printk("%04X:%04X %02X %02X %02X %04X %04X %08X %08X %08X %08X %04X\n",
	    info->VendorID,
	    info->DeviceID,
	    info->PrimaryBusNumber,
	    info->SecondayBusNumber,
	    info->SubordinateBusNumber,
	    ((u16)info->IOBase << 8) & 0xF000,
	    info->IOLimit ?
		((u16)info->IOLimit << 8) | 0xFFF : 0,
	    ((u32)info->MemoryBase << 16) & 0xFFF00000,
	    info->MemoryLimit ?
		((u32)info->MemoryLimit << 16) | 0xFFFFF : 0,
	    ((u32)info->PrefetchableMemoryBase << 16) & 0xFFF00000,
	    info->PrefetchableMemoryLimit ?
		((u32)info->PrefetchableMemoryLimit << 16) | 0xFFFFF : 0,
	    info->BridgeControl);
	}
    printk("\n");
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
    PCIBridgeInfo   *info;

    /* If this is the first time we have been called, enumerate all */
    /* devices on the PCI bus. */
    if (NumPCI == -1) {
	for (i = 0; i < MAX_PCI_DEVICES; i++)
	    PCI[i].dwSize = sizeof(PCI[i]);
	if ((NumPCI = PCI_enumerate(PCI,MAX_PCI_DEVICES)) == 0)
	    return -1;

	/* Build a list of all PCI bridge devices */
	for (i = 0,NumBridges = 0,BridgeIndex[0] = -1; i < NumPCI; i++) {
	    if (PCI[i].BaseClass == PCI_BRIDGE_CLASS) {
		if (NumBridges < MAX_PCI_DEVICES)
		    BridgeIndex[NumBridges++] = i;
		}
	    }

	/* Now build a list of all display class devices */
	for (i = 0,NumDevices = 1,DeviceIndex[0] = -1; i < NumPCI; i++) {
	    if (PCI_IS_DISPLAY_CLASS(&PCI[i])) {
		if ((PCI[i].Command & 0x3) == 0x3) {
		    DeviceIndex[0] = i;
		    }
		else {
		    if (NumDevices < MAX_PCI_DEVICES)
			DeviceIndex[NumDevices++] = i;
		    }
		if (PCI[i].slot.p.Bus != 0) {
		    /* This device is on a different bus than the primary */
		    /* PCI bus, so it is probably an AGP device. Find the */
		    /* AGP bus device that controls that bus so we can */
		    /* control it. */
		    for (j = 0; j < NumBridges; j++) {
			info = (PCIBridgeInfo*)&PCI[BridgeIndex[j]];
			if (info->SecondayBusNumber == PCI[i].slot.p.Bus) {
			    AGPBridge = info;
			    break;
			    }
			}
		    }
		}
	    }

	/* Enumerate all PCI and bridge devices to log file */
	EnumeratePCI();
	}
    return NumDevices;
}

FILE *logfile;

void printk(const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(logfile, fmt, argptr);
    fflush(logfile);
    va_end(argptr);
}

int main(int argc,char *argv[])
{
    while (argc > 1) {
	if (stricmp(argv[1],"-usev86") == 0) {
	    useV86 = true;
	    }
	else if (stricmp(argv[1],"-force") == 0) {
	    forcePost = true;
	    }
#ifdef  DEBUG
	else if (stricmp(argv[1],"-decode") == 0) {
	    debugFlags |= DEBUG_DECODE_F;
	    }
	else if (stricmp(argv[1],"-iotrace") == 0) {
	    debugFlags |= DEBUG_IO_TRACE_F;
	    }
#endif
	else {
	    printf("Usage: warmboot [-usev86] [-force] [-decode] [-iotrace]\n");
	    exit(-1);
	    }
	argc--;
	argv++;
	}
    if ((logfile = fopen("warmboot.log","w")) == NULL)
	exit(1);

    PM_init();
    if (!useV86) {
	/* Initialise the x86 BIOS emulator */
	BE_init(false,debugFlags,65536,&VGAInfo[0]);
	}

    /* Enumerate all devices (which POST's them at the same time) */
    if (PCI_enumerateDevices() < 1) {
	printk("No PCI display devices found!\n");
	return -1;
	}

    /* Post all the display controller BIOS'es */
    PCI_postControllers();

    /* Cleanup and exit the emulator */
    if (!useV86)
	BE_exit();
    fclose(logfile);
    return 0;
}
