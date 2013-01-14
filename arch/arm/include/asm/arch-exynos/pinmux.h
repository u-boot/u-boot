/*
 * Copyright (C) 2012 Samsung Electronics
 * Abhilash Kesavan <a.kesavan@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ASM_ARM_ARCH_PINMUX_H
#define __ASM_ARM_ARCH_PINMUX_H

#include "periph.h"

/*
 * Flags for setting specific configarations of peripherals.
 * List will grow with support for more devices getting added.
 */
enum {
	PINMUX_FLAG_NONE	= 0x00000000,

	/* Flags for eMMC */
	PINMUX_FLAG_8BIT_MODE	= 1 << 0,       /* SDMMC 8-bit mode */

	/* Flags for SROM controller */
	PINMUX_FLAG_BANK	= 3 << 0,       /* bank number (0-3) */
	PINMUX_FLAG_16BIT	= 1 << 2,       /* 16-bit width */
};

/**
 * Configures the pinmux for a particular peripheral.
 *
 * Each gpio can be configured in many different ways (4 bits on exynos)
 * such as "input", "output", "special function", "external interrupt"
 * etc. This function will configure the peripheral pinmux along with
 * pull-up/down and drive strength.
 *
 * @param peripheral	peripheral to be configured
 * @param flags		configure flags
 * @return 0 if ok, -1 on error (e.g. unsupported peripheral)
 */
int exynos_pinmux_config(int peripheral, int flags);

/**
 * Decode the peripheral id using the interrpt numbers.
 *
 * @param blob  Device tree blob
 * @param node  FDT I2C node to find
 * @return peripheral id if ok, PERIPH_ID_NONE on error
 */
int pinmux_decode_periph_id(const void *blob, int node);
#endif
