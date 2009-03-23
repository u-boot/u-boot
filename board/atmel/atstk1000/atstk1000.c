/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>

#include <asm/io.h>
#include <asm/sdram.h>
#include <asm/arch/clk.h>
#include <asm/arch/hmatrix.h>
#include <asm/arch/portmux.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct sdram_config sdram_config = {
#if defined(CONFIG_ATSTK1006)
	/* Dual MT48LC16M16A2-7E (64 MB) on daughterboard */
	.data_bits	= SDRAM_DATA_32BIT,
	.row_bits	= 13,
	.col_bits	= 9,
	.bank_bits	= 2,
	.cas		= 2,
	.twr		= 2,
	.trc		= 7,
	.trp		= 2,
	.trcd		= 2,
	.tras		= 4,
	.txsr		= 7,
	/* 7.81 us */
	.refresh_period	= (781 * (SDRAMC_BUS_HZ / 1000)) / 100000,
#else
	/* MT48LC2M32B2P-5 (8 MB) on motherboard */
#ifdef CONFIG_ATSTK1004
	.data_bits	= SDRAM_DATA_16BIT,
#else
	.data_bits	= SDRAM_DATA_32BIT,
#endif
#ifdef CONFIG_ATSTK1000_16MB_SDRAM
	/* MT48LC4M32B2P-6 (16 MB) on mod'ed motherboard */
	.row_bits	= 12,
#else
	.row_bits	= 11,
#endif
	.col_bits	= 8,
	.bank_bits	= 2,
	.cas		= 3,
	.twr		= 2,
	.trc		= 7,
	.trp		= 2,
	.trcd		= 2,
	.tras		= 5,
	.txsr		= 5,
	/* 15.6 us */
	.refresh_period	= (156 * (SDRAMC_BUS_HZ / 1000)) / 10000,
#endif
};

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	portmux_enable_ebi(sdram_config.data_bits, 23, 0, PORTMUX_DRIVE_HIGH);
	portmux_enable_usart1(PORTMUX_DRIVE_MIN);
#if defined(CONFIG_MACB)
	portmux_enable_macb0(PORTMUX_MACB_MII, PORTMUX_DRIVE_LOW);
	portmux_enable_macb1(PORTMUX_MACB_MII, PORTMUX_DRIVE_LOW);
#endif
#if defined(CONFIG_MMC)
	portmux_enable_mmci(0, PORTMUX_MMCI_4BIT, PORTMUX_DRIVE_LOW);
#endif

	return 0;
}

phys_size_t initdram(int board_type)
{
	unsigned long expected_size;
	unsigned long actual_size;
	void *sdram_base;

	sdram_base = map_physmem(EBI_SDRAM_BASE, EBI_SDRAM_SIZE, MAP_NOCACHE);

	expected_size = sdram_init(sdram_base, &sdram_config);
	actual_size = get_ram_size(sdram_base, expected_size);

	unmap_physmem(sdram_base, EBI_SDRAM_SIZE);

	if (expected_size != actual_size)
		printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
				actual_size >> 20, expected_size >> 20);

	return actual_size;
}

int board_early_init_r(void)
{
	gd->bd->bi_phy_id[0] = 0x10;
	gd->bd->bi_phy_id[1] = 0x11;
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bi)
{
	macb_eth_initialize(0, (void *)MACB0_BASE, bi->bi_phy_id[0]);
	macb_eth_initialize(1, (void *)MACB1_BASE, bi->bi_phy_id[1]);
	return 0;
}
#endif
