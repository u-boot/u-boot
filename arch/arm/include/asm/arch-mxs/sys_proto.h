/*
 * Freescale i.MX23/i.MX28 specific functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __SYS_PROTO_H__
#define __SYS_PROTO_H__

int mxs_reset_block(struct mxs_register_32 *reg);
int mxs_wait_mask_set(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);
int mxs_wait_mask_clr(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);

int mxsmmc_initialize(bd_t *bis, int id, int (*wp)(int));

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/iomux-mx28.h>
void mxs_common_spl_init(const iomux_cfg_t *iomux_setup,
			const unsigned int iomux_size);
#endif

struct mxs_pair {
	uint8_t	boot_pads;
	uint8_t boot_mask;
	const char *mode;
};

static const struct mxs_pair mxs_boot_modes[] = {
	{ 0x00, 0x0f, "USB #0" },
	{ 0x01, 0x1f, "I2C #0, master, 3V3" },
	{ 0x11, 0x1f, "I2C #0, master, 1V8" },
	{ 0x02, 0x1f, "SSP SPI #2, master, 3V3 NOR" },
	{ 0x12, 0x1f, "SSP SPI #2, master, 1V8 NOR" },
	{ 0x03, 0x1f, "SSP SPI #3, master, 3V3 NOR" },
	{ 0x13, 0x1f, "SSP SPI #3, master, 1V8 NOR" },
	{ 0x04, 0x1f, "NAND, 3V3" },
	{ 0x14, 0x1f, "NAND, 1V8" },
	{ 0x08, 0x1f, "SSP SPI #3, master, 3V3 EEPROM" },
	{ 0x18, 0x1f, "SSP SPI #3, master, 1V8 EEPROM" },
	{ 0x09, 0x1f, "SSP SD/MMC #0, 3V3" },
	{ 0x19, 0x1f, "SSP SD/MMC #0, 1V8" },
	{ 0x0a, 0x1f, "SSP SD/MMC #1, 3V3" },
	{ 0x1a, 0x1f, "SSP SD/MMC #1, 1V8" },
	{ 0x00, 0x00, "Reserved/Unknown/Wrong" },
};

struct mxs_spl_data {
	uint8_t		boot_mode_idx;
	uint32_t	mem_dram_size;
};

int mxs_dram_init(void);

#endif	/* __SYS_PROTO_H__ */
