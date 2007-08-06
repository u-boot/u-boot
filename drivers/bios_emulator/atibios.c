/****************************************************************************
*
*		     Video BOOT Graphics Card POST Module
*
*  ========================================================================
*   Copyright (C) 2007 Freescale Semiconductor, Inc. All rights reserved.
*   Jason Jin <Jason.jin@freescale.com>
*
*   Copyright (C) 1991-2004 SciTech Software, Inc. All rights reserved.
*
*   This file may be distributed and/or modified under the terms of the
*   GNU General Public License version 2.0 as published by the Free
*   Software Foundation and appearing in the file LICENSE.GPL included
*   in the packaging of this file.
*
*   Licensees holding a valid Commercial License for this product from
*   SciTech Software, Inc. may use this file in accordance with the
*   Commercial License Agreement provided with the Software.
*
*   This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
*   THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*   PURPOSE.
*
*   See http://www.scitechsoft.com/license/ for information about
*   the licensing options available and how to purchase a Commercial
*   License Agreement.
*
*   Contact license@scitechsoft.com if any conditions of this licensing
*   are not clear to you, or you have questions about licensing options.
*
*  ========================================================================
*
* Language:	ANSI C
* Environment:	Linux Kernel
* Developer:	Kendall Bennett
*
* Description:	Module to implement booting PCI/AGP controllers on the
*		bus. We use the x86 real mode emulator to run the BIOS on
*		graphics controllers to bring the cards up.
*
*		Note that at present this module does *not* support
*		multiple controllers.
*
*		The orignal name of this file is warmboot.c.
*		Jason ported this file to u-boot to run the ATI video card
*		BIOS in u-boot.
****************************************************************************/
#include <common.h>

#ifdef CONFIG_BIOSEMU

#include "biosemui.h"
#include <malloc.h>

/* Length of the BIOS image */
#define MAX_BIOSLEN	    (128 * 1024L)

/* Define some useful types and macros */
#define true		    1
#define false		    0

/* Place to save PCI BAR's that we change and later restore */
static u32 saveROMBaseAddress;
static u32 saveBaseAddress10;
static u32 saveBaseAddress14;
static u32 saveBaseAddress18;
static u32 saveBaseAddress20;

/****************************************************************************
PARAMETERS:
pcidev	- PCI device info for the video card on the bus to boot
VGAInfo - BIOS emulator VGA info structure

REMARKS:
This function executes the BIOS POST code on the controller. We assume that
at this stage the controller has its I/O and memory space enabled and
that all other controllers are in a disabled state.
****************************************************************************/
static void PCI_doBIOSPOST(pci_dev_t pcidev, BE_VGAInfo * VGAInfo)
{
	RMREGS regs;
	RMSREGS sregs;

	/* Determine the value to store in AX for BIOS POST. Per the PCI specs,
	 AH must contain the bus and AL must contain the devfn, encoded as
	 (dev << 3) | fn
	 */
	memset(&regs, 0, sizeof(regs));
	memset(&sregs, 0, sizeof(sregs));
	regs.x.ax = ((int)PCI_BUS(pcidev) << 8) |
	    ((int)PCI_DEV(pcidev) << 3) | (int)PCI_FUNC(pcidev);

	/*Setup the X86 emulator for the VGA BIOS*/
	BE_setVGA(VGAInfo);

	/*Execute the BIOS POST code*/
	BE_callRealMode(0xC000, 0x0003, &regs, &sregs);

	/*Cleanup and exit*/
	BE_getVGA(VGAInfo);
}

/****************************************************************************
PARAMETERS:
pcidev	- PCI device info for the video card on the bus
bar	- Place to return the base address register offset to use

RETURNS:
The address to use to map the secondary BIOS (AGP devices)

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

NOTE: This function leaves the original memory aperture disabled by leaving
      it programmed to all 1's. It must be restored to the correct value
      later.
****************************************************************************/
static u32 PCI_findBIOSAddr(pci_dev_t pcidev, int *bar)
{
	u32 base, size;

	for (*bar = 0x10; *bar <= 0x14; (*bar) += 4) {
		pci_read_config_dword(pcidev, *bar, &base);
		if (!(base & 0x1)) {
			pci_write_config_dword(pcidev, *bar, 0xFFFFFFFF);
			pci_read_config_dword(pcidev, *bar, &size);
			size = ~(size & ~0xFF) + 1;
			if (size >= MAX_BIOSLEN)
				return base & ~0xFF;
		}
	}
	return 0;
}

/****************************************************************************
REMARKS:
Some non-x86 Linux kernels map PCI relocateable I/O to values that
are above 64K, which will not work with the BIOS image that requires
the offset for the I/O ports to be a maximum of 16-bits. Ideally
someone should fix the kernel to map the I/O ports for VGA compatible
devices to a different location (or just all I/O ports since it is
unlikely you can have enough devices in the machine to use up all
64K of the I/O space - a total of more than 256 cards would be
necessary).

Anyway to fix this we change all I/O mapped base registers and
chop off the top bits.
****************************************************************************/
static void PCI_fixupIObase(pci_dev_t pcidev, int reg, u32 * base)
{
	if ((*base & 0x1) && (*base > 0xFFFE)) {
		*base &= 0xFFFF;
		pci_write_config_dword(pcidev, reg, *base);

	}
}

/****************************************************************************
PARAMETERS:
pcidev	- PCI device info for the video card on the bus

RETURNS:
Pointers to the mapped BIOS image

REMARKS:
Maps a pointer to the BIOS image on the graphics card on the PCI bus.
****************************************************************************/
void *PCI_mapBIOSImage(pci_dev_t pcidev)
{
	u32 BIOSImagePhys;
	int BIOSImageBAR;
	u8 *BIOSImage;

	/*Save PCI BAR registers that might get changed*/
	pci_read_config_dword(pcidev, PCI_ROM_ADDRESS, &saveROMBaseAddress);
	pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &saveBaseAddress10);
	pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_1, &saveBaseAddress14);
	pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_2, &saveBaseAddress18);
	pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_4, &saveBaseAddress20);

	/*Fix up I/O base registers to less than 64K */
	if(saveBaseAddress14 != 0)
		PCI_fixupIObase(pcidev, PCI_BASE_ADDRESS_1, &saveBaseAddress14);
	else
		PCI_fixupIObase(pcidev, PCI_BASE_ADDRESS_4, &saveBaseAddress20);

	/* Some cards have problems that stop us from being able to read the
	 BIOS image from the ROM BAR. To fix this we have to do some chipset
	 specific programming for different cards to solve this problem.
	*/

	if ((BIOSImagePhys = PCI_findBIOSAddr(pcidev, &BIOSImageBAR)) == 0) {
		printf("Find bios addr error\n");
		return NULL;
	}

	BIOSImage = (u8 *) BIOSImagePhys;

	/*Change the PCI BAR registers to map it onto the bus.*/
	pci_write_config_dword(pcidev, BIOSImageBAR, 0);
	pci_write_config_dword(pcidev, PCI_ROM_ADDRESS, BIOSImagePhys | 0x1);

	udelay(1);

	/*Check that the BIOS image is valid. If not fail, or return the
	 compiled in BIOS image if that option was enabled
	 */
	if (BIOSImage[0] != 0x55 || BIOSImage[1] != 0xAA || BIOSImage[2] == 0) {
		return NULL;
	}

	return BIOSImage;
}

/****************************************************************************
PARAMETERS:
pcidev	- PCI device info for the video card on the bus

REMARKS:
Unmaps the BIOS image for the device and restores framebuffer mappings
****************************************************************************/
void PCI_unmapBIOSImage(pci_dev_t pcidev, void *BIOSImage)
{
	pci_write_config_dword(pcidev, PCI_ROM_ADDRESS, saveROMBaseAddress);
	pci_write_config_dword(pcidev, PCI_BASE_ADDRESS_0, saveBaseAddress10);
	pci_write_config_dword(pcidev, PCI_BASE_ADDRESS_1, saveBaseAddress14);
	pci_write_config_dword(pcidev, PCI_BASE_ADDRESS_2, saveBaseAddress18);
	pci_write_config_dword(pcidev, PCI_BASE_ADDRESS_4, saveBaseAddress20);
}

/****************************************************************************
PARAMETERS:
pcidev	- PCI device info for the video card on the bus to boot
VGAInfo - BIOS emulator VGA info structure

RETURNS:
True if successfully initialised, false if not.

REMARKS:
Loads and POST's the display controllers BIOS, directly from the BIOS
image we can extract over the PCI bus.
****************************************************************************/
static int PCI_postController(pci_dev_t pcidev, BE_VGAInfo * VGAInfo)
{
	u32 BIOSImageLen;
	uchar *mappedBIOS;
	uchar *copyOfBIOS;

	/*Allocate memory to store copy of BIOS from display controller*/
	if ((mappedBIOS = PCI_mapBIOSImage(pcidev)) == NULL) {
		printf("videoboot: Video ROM failed to map!\n");
		return false;
	}

	BIOSImageLen = mappedBIOS[2] * 512;

	if ((copyOfBIOS = malloc(BIOSImageLen)) == NULL) {
		printf("videoboot: Out of memory!\n");
		return false;
	}
	memcpy(copyOfBIOS, mappedBIOS, BIOSImageLen);

	PCI_unmapBIOSImage(pcidev, mappedBIOS);

	/*Save information in VGAInfo structure*/
	VGAInfo->function = PCI_FUNC(pcidev);
	VGAInfo->device = PCI_DEV(pcidev);
	VGAInfo->bus = PCI_BUS(pcidev);
	VGAInfo->pcidev = pcidev;
	VGAInfo->BIOSImage = copyOfBIOS;
	VGAInfo->BIOSImageLen = BIOSImageLen;

	/*Now execute the BIOS POST for the device*/
	if (copyOfBIOS[0] != 0x55 || copyOfBIOS[1] != 0xAA) {
		printf("videoboot: Video ROM image is invalid!\n");
		return false;
	}

	PCI_doBIOSPOST(pcidev, VGAInfo);

	/*Reset the size of the BIOS image to the final size*/
	VGAInfo->BIOSImageLen = copyOfBIOS[2] * 512;
	return true;
}

/****************************************************************************
PARAMETERS:
pcidev	    - PCI device info for the video card on the bus to boot
pVGAInfo    - Place to return VGA info structure is requested
cleanUp	    - True to clean up on exit, false to leave emulator active

REMARKS:
Boots the PCI/AGP video card on the bus using the Video ROM BIOS image
and the X86 BIOS emulator module.
****************************************************************************/
int BootVideoCardBIOS(pci_dev_t pcidev, BE_VGAInfo ** pVGAInfo, int cleanUp)
{
	BE_VGAInfo *VGAInfo;

	printf("videoboot: Booting PCI video card bus %d, function %d, device %d\n",
	     PCI_BUS(pcidev), PCI_FUNC(pcidev), PCI_DEV(pcidev));

	/*Initialise the x86 BIOS emulator*/
	if ((VGAInfo = malloc(sizeof(*VGAInfo))) == NULL) {
		printf("videoboot: Out of memory!\n");
		return false;
	}
	memset(VGAInfo, 0, sizeof(*VGAInfo));
	BE_init(0, 65536, VGAInfo, 0);

	/*Post all the display controller BIOS'es*/
	PCI_postController(pcidev, VGAInfo);

	/*Cleanup and exit the emulator if requested. If the BIOS emulator
	is needed after booting the card, we will not call BE_exit and
	leave it enabled for further use (ie: VESA driver etc).
	*/
	if (cleanUp) {
		BE_exit();
		if (VGAInfo->BIOSImage)
			free(VGAInfo->BIOSImage);
		free(VGAInfo);
		VGAInfo = NULL;
	}
	/*Return VGA info pointer if the caller requested it*/
	if (pVGAInfo)
		*pVGAInfo = VGAInfo;
	return true;
}

#endif
