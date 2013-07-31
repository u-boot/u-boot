/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Sricharan R <r.sricharan@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

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
	unsigned long omap_bootmode;
};
#endif
