/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>
#include <asm/sdram.h>
#include <asm/arch/clk.h>
#include <asm/arch/hmatrix.h>
#include <asm/arch/mmu.h>
#include <asm/arch/portmux.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

struct mmu_vm_range mmu_vmr_table[CONFIG_SYS_NR_VM_REGIONS] = {
	{
		.virt_pgno	= CONFIG_SYS_FLASH_BASE >> MMU_PAGE_SHIFT,
		.nr_pages	= CONFIG_SYS_FLASH_SIZE >> MMU_PAGE_SHIFT,
		.phys		= (CONFIG_SYS_FLASH_BASE >> MMU_PAGE_SHIFT)
					| MMU_VMR_CACHE_NONE,
	}, {
		.virt_pgno	= CONFIG_SYS_SDRAM_BASE >> MMU_PAGE_SHIFT,
		.nr_pages	= EBI_SDRAM_SIZE >> MMU_PAGE_SHIFT,
		.phys		= (CONFIG_SYS_SDRAM_BASE >> MMU_PAGE_SHIFT)
					| MMU_VMR_CACHE_WRBACK,
	},
};

static const struct sdram_config sdram_config = {
	.data_bits	= SDRAM_DATA_32BIT,
#ifdef CONFIG_ATSTK1000_16MB_SDRAM
	/* MT48LC4M32B2P-6 (16 MB) on mod'ed motherboard */
	.row_bits	= 12,
#else
	/* MT48LC2M32B2P-5 (8 MB) on motherboard */
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
};

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	portmux_enable_ebi(sdram_config.data_bits, 23, 0, PORTMUX_DRIVE_HIGH);
	sdram_init(uncached(EBI_SDRAM_BASE), &sdram_config);

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

int board_early_init_r(void)
{
	gd->bd->bi_phy_id[0] = 0x10;
	gd->bd->bi_phy_id[1] = 0x11;
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bi)
{
	macb_eth_initialize(0, (void *)ATMEL_BASE_MACB0, bi->bi_phy_id[0]);
	macb_eth_initialize(1, (void *)ATMEL_BASE_MACB1, bi->bi_phy_id[1]);
	return 0;
}
#endif
