/*
 * Copyright (C) 1996-1999 SciTech Software, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _BIOS_EMUL_H
#define _BIOS_EMUL_H

/* Include the register header directly here */
#include "../drivers/bios_emulator/include/x86emu/regs.h"

/****************************************************************************
REMARKS:
Data structure used to describe the details for the BIOS emulator system
environment as used by the X86 emulator library.

HEADER:
biosemu.h

MEMBERS:
vgaInfo         - VGA BIOS information structure
biosmem_base    - Base of the BIOS image
biosmem_limit   - Limit of the BIOS image
busmem_base     - Base of the VGA bus memory
****************************************************************************/
typedef struct {
	int function;
	int device;
	int bus;
	u32 VendorID;
	u32 DeviceID;
	pci_dev_t pcidev;
	void *BIOSImage;
	u32 BIOSImageLen;
	u8 LowMem[1536];
} BE_VGAInfo;

struct vbe_mode_info;

int BootVideoCardBIOS(pci_dev_t pcidev, BE_VGAInfo **pVGAInfo, int cleanUp);

#endif
