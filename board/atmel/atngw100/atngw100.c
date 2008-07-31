/*
 * Copyright (C) 2006 Atmel Corporation
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
#include <asm/arch/gpio.h>
#include <asm/arch/hmatrix.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct sdram_config sdram_config = {
	.data_bits	= SDRAM_DATA_16BIT,
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

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	gpio_enable_ebi();
	gpio_enable_usart1();

#if defined(CONFIG_MACB)
	gpio_enable_macb0();
	gpio_enable_macb1();
#endif
#if defined(CONFIG_MMC)
	gpio_enable_mmci();
#endif
#if defined(CONFIG_ATMEL_SPI)
	gpio_enable_spi0(1 << 0);
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

void board_init_info(void)
{
	gd->bd->bi_phy_id[0] = 0x01;
	gd->bd->bi_phy_id[1] = 0x03;
}

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

#define ATNGW100_DATAFLASH_CS_PIN	GPIO_PIN_PA3

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio_set_value(ATNGW100_DATAFLASH_CS_PIN, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio_set_value(ATNGW100_DATAFLASH_CS_PIN, 1);
}
#endif /* CONFIG_ATMEL_SPI */
