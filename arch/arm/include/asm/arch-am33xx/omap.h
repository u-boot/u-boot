/*
 * omap.h
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Author:
 *	Chandan Nath <chandan.nath@ti.com>
 *
 * Derived from OMAP4 work by
 *	Aneesh V <aneesh@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _OMAP_H_
#define _OMAP_H_

/*
 * Non-secure SRAM Addresses
 * Non-secure RAM starts at 0x40300000 for GP devices. But we keep SRAM_BASE
 * at 0x40304000(EMU base) so that our code works for both EMU and GP
 */
#define NON_SECURE_SRAM_START	0x40304000
#define NON_SECURE_SRAM_END	0x4030E000

/* ROM code defines */
/* Boot device */
#define BOOT_DEVICE_MASK	0xFF
#define BOOT_DEVICE_OFFSET	0x8
#define DEV_DESC_PTR_OFFSET	0x4
#define DEV_DATA_PTR_OFFSET	0x18
#define BOOT_MODE_OFFSET	0x8
#define RESET_REASON_OFFSET	0x9
#define CH_FLAGS_OFFSET		0xA

#define CH_FLAGS_CHSETTINGS	(0x1 << 0)
#define CH_FLAGS_CHRAM		(0x1 << 1)
#define CH_FLAGS_CHFLASH	(0x1 << 2)
#define CH_FLAGS_CHMMCSD	(0x1 << 3)

#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	char *boot_message;
	unsigned int mem_boot_descriptor;
	unsigned char omap_bootdevice;
	unsigned char reset_reason;
	unsigned char ch_flags;
};
#endif
#endif
