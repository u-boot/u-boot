/*
 * (C) Copyright 2017 Heiko Stuebner <heiko@sntech.de>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ASM_ARCH_BOOTROM_H
#define _ASM_ARCH_BOOTROM_H

/*
 * Saved Stack pointer address.
 * Access might be needed in some special cases.
 */
extern u32 SAVE_SP_ADDR;

/**
 * Hand control back to the bootrom to load another
 * boot stage.
 */
void back_to_bootrom(void);

/**
 * Assembler component for the above (do not call this directly)
 */
void _back_to_bootrom_s(void);

/**
 * Boot-device identifiers as used by the BROM
 */
enum {
	BROM_BOOTSOURCE_NAND = 1,
	BROM_BOOTSOURCE_EMMC = 2,
	BROM_BOOTSOURCE_SPINOR = 3,
	BROM_BOOTSOURCE_SPINAND = 4,
	BROM_BOOTSOURCE_SD = 5,
	BROM_BOOTSOURCE_USB = 10,
	BROM_LAST_BOOTSOURCE = BROM_BOOTSOURCE_USB
};

/**
 * Locations of the boot-device identifier in SRAM
 */
#define RK3399_BROM_BOOTSOURCE_ID_ADDR   0xff8c0010

#endif
