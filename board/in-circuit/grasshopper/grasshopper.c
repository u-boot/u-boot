/*
 * Copyright (C) 2011
 * Corscience GmbH & Co.KG, Andreas Bießmann <biessmann@corscience.de>
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
#include <asm/arch/mmu.h>
#include <asm/arch/portmux.h>
#include <netdev.h>

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
	/* Dual MT48LC16M16A2-7E (or equal) */
	.data_bits		= SDRAM_DATA_32BIT,
	.row_bits		= 13,
	.col_bits		= 9,
	.bank_bits		= 2,
	.cas			= 2,
	.twr			= 2,
	.trc			= 7,
	.trp			= 2,
	.trcd			= 2,
	.tras			= 4,
	.txsr			= 7,
	/* 7.81 us */
	.refresh_period		= (781 * (SDRAMC_BUS_HZ / 1000)) / 100000,
};

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	portmux_enable_ebi(SDRAM_DATA_32BIT, 23, 0, PORTMUX_DRIVE_HIGH);
	portmux_enable_usart0(PORTMUX_DRIVE_MIN);
	portmux_enable_usart1(PORTMUX_DRIVE_MIN);
#if defined(CONFIG_MACB)
	/* set PHY reset and pwrdown to low */
	portmux_select_gpio(PORTMUX_PORT_B, (1 << 29) | (1 << 30),
		PORTMUX_DIR_OUTPUT | PORTMUX_INIT_LOW);
	udelay(100);
	/* release PHYs reset */
	gpio_set_value(GPIO_PIN_PB(29), 1);

	portmux_enable_macb0(PORTMUX_MACB_MII, PORTMUX_DRIVE_LOW);
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
	gd->bd->bi_phy_id[0] = 0x00;
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bi)
{
	macb_eth_initialize(0, (void *)ATMEL_BASE_MACB0, bi->bi_phy_id[0]);
	return 0;
}
#endif
/* vim: set noet ts=8: */
