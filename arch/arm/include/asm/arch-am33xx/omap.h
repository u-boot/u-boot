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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _OMAP_H_
#define _OMAP_H_

#ifdef CONFIG_AM33XX
#define NON_SECURE_SRAM_START	0x402F0400
#define NON_SECURE_SRAM_END	0x40310000
#define SRAM_SCRATCH_SPACE_ADDR	0x4030B800
#elif defined(CONFIG_TI81XX)
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40320000
#define SRAM_SCRATCH_SPACE_ADDR	0x4031B800
#elif defined(CONFIG_AM43XX)
#define NON_SECURE_SRAM_START	0x402F0400
#define NON_SECURE_SRAM_END	0x40340000
#define SRAM_SCRATCH_SPACE_ADDR	0x40337C00
#define AM4372_BOARD_NAME_START	SRAM_SCRATCH_SPACE_ADDR
#define AM4372_BOARD_NAME_END	SRAM_SCRATCH_SPACE_ADDR + 0xC
#define AM4372_BOARD_VERSION_START	SRAM_SCRATCH_SPACE_ADDR + 0xD
#define AM4372_BOARD_VERSION_END	SRAM_SCRATCH_SPACE_ADDR + 0x14
#define QSPI_BASE              0x47900000
#endif

/* Boot parameters */
#ifndef __ASSEMBLY__
struct omap_boot_parameters {
	unsigned int reserved;
	unsigned int boot_device_descriptor;
	unsigned char boot_device;
	unsigned char reset_reason;
};
#endif

#endif
