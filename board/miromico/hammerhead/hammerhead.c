/*
 * Copyright (C) 2008 Miromico AG
 *
 * Mostly copied form atmel ATNGW100 sources
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

#include <asm/io.h>
#include <asm/sdram.h>
#include <asm/arch/clk.h>
#include <asm/arch/hmatrix.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mmu.h>
#include <asm/arch/portmux.h>

DECLARE_GLOBAL_DATA_PTR;

struct mmu_vm_range mmu_vmr_table[CONFIG_SYS_NR_VM_REGIONS] = {
	{
		.virt_pgno	= CONFIG_SYS_FLASH_BASE >> PAGE_SHIFT,
		.nr_pages	= CONFIG_SYS_FLASH_SIZE >> PAGE_SHIFT,
		.phys		= (CONFIG_SYS_FLASH_BASE >> PAGE_SHIFT)
					| MMU_VMR_CACHE_NONE,
	}, {
		.virt_pgno	= CONFIG_SYS_SDRAM_BASE >> PAGE_SHIFT,
		.nr_pages	= EBI_SDRAM_SIZE >> PAGE_SHIFT,
		.phys		= (CONFIG_SYS_SDRAM_BASE >> PAGE_SHIFT)
					| MMU_VMR_CACHE_WRBACK,
	},
};

static const struct sdram_config sdram_config = {
	.data_bits	= SDRAM_DATA_32BIT,
	.row_bits	= 13,
	.col_bits	= 9,
	.bank_bits	= 2,
	.cas		= 3,
	.twr		= 2,
	.trc		= 7,
	.trp		= 2,
	.trcd		= 2,
	.tras		= 5,
	.txsr		= 5,
	/* 7.81 us */
	.refresh_period	= (781 * (SDRAMC_BUS_HZ / 1000)) / 100000,
};

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	return macb_eth_initialize(0, (void *)ATMEL_BASE_MACB0,
		bis->bi_phy_id[0]);
}
#endif

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	portmux_enable_ebi(32, 23, 0, PORTMUX_DRIVE_HIGH);
	portmux_enable_usart1(PORTMUX_DRIVE_MIN);

#if defined(CONFIG_MACB)
	portmux_enable_macb0(PORTMUX_MACB_MII, PORTMUX_DRIVE_HIGH);
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

	sdram_base = uncached(EBI_SDRAM_BASE);

	expected_size = sdram_init(sdram_base, &sdram_config);
	actual_size = get_ram_size(sdram_base, expected_size);

	if (expected_size != actual_size)
		printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
		       actual_size >> 20, expected_size >> 20);

	return actual_size;
}

int board_early_init_r(void)
{
	gd->bd->bi_phy_id[0] = 0x01;
	return 0;
}

int board_postclk_init(void)
{
	/* Hammerhead boards uses GCLK3 as 25MHz output to ethernet PHY */
	gclk_enable_output(3, PORTMUX_DRIVE_LOW);
	gclk_set_rate(3, GCLK_PARENT_OSC0, 25000000);
	return 0;
}
