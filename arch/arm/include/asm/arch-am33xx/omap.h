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

/*
 * Non-secure SRAM Addresses
 * Non-secure RAM starts at 0x40300000 for GP devices. But we keep SRAM_BASE
 * at 0x40304000(EMU base) so that our code works for both EMU and GP
 */
#ifdef CONFIG_AM33XX
#define NON_SECURE_SRAM_START	0x402F0400
#define NON_SECURE_SRAM_END	0x40310000
#define SRAM_SCRATCH_SPACE_ADDR	0x4030C000
#elif defined(CONFIG_TI814X)
#define NON_SECURE_SRAM_START	0x40300000
#define NON_SECURE_SRAM_END	0x40320000
#define SRAM_SCRATCH_SPACE_ADDR	0x4031B800
#endif
#endif
