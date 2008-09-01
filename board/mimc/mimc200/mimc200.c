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
#include <netdev.h>

#include <asm/io.h>
#include <asm/sdram.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hmatrix.h>
#include <lcd.h>

#define SM_PM_GCCTRL				0x0060

DECLARE_GLOBAL_DATA_PTR;

static const struct sdram_config sdram_config = {
	.data_bits	= SDRAM_DATA_16BIT,
	.row_bits	= 13,
	.col_bits	= 9,
	.bank_bits	= 2,
	.cas		= 3,
	.twr		= 2,
	.trc		= 6,
	.trp		= 2,
	.trcd		= 2,
	.tras		= 6,
	.txsr		= 6,
	/* 15.6 us */
	.refresh_period	= (156 * (SDRAMC_BUS_HZ / 1000)) / 10000,
};

int board_early_init_f(void)
{
	/* Enable SDRAM in the EBI mux */
	hmatrix_slave_write(EBI, SFR, HMATRIX_BIT(EBI_SDRAM_ENABLE));

	gpio_enable_ebi();
	gpio_enable_usart1();

	/* enable higher address lines for larger flash devices */
	gpio_select_periph_A(GPIO_PIN_PE16, 0);	/* ADDR23 */
	gpio_select_periph_A(GPIO_PIN_PE17, 0);	/* ADDR24 */
	gpio_select_periph_A(GPIO_PIN_PE18, 0);	/* ADDR25 */

	/* enable data flash chip select */
	gpio_select_periph_A(GPIO_PIN_PE25, 0);	/* NCS2 */

	/* de-assert "force sys reset" pin */
	gpio_set_value(GPIO_PIN_PD15, 1);	/* FORCE RESET	*/
	gpio_select_pio(GPIO_PIN_PD15, GPIOF_OUTPUT);

	/* init custom i/o */
	/* cpu type inputs */
	gpio_select_pio(GPIO_PIN_PE19, 0);
	gpio_select_pio(GPIO_PIN_PE20, 0);
	gpio_select_pio(GPIO_PIN_PE23, 0);
	/* main board type inputs */
	gpio_select_pio(GPIO_PIN_PB19, 0);
	gpio_select_pio(GPIO_PIN_PB29, 0);
	/* DEBUG input (use weak pullup) */
	gpio_select_pio(GPIO_PIN_PE21, GPIOF_PULLUP);

	/* are we suppressing the console ? */
	if (gpio_get_value(GPIO_PIN_PE21) == 1)
		gd->flags |= GD_FLG_SILENT;

	/* reset phys */
	gpio_select_pio(GPIO_PIN_PE24, 0);
	gpio_set_value(GPIO_PIN_PC18, 1);	/* PHY RESET	*/
	gpio_select_pio(GPIO_PIN_PC18, GPIOF_OUTPUT);

	/* GCLK0 - 10MHz clock */
	writel(0x00000004, (void *)SM_BASE + SM_PM_GCCTRL);
	gpio_select_periph_A(GPIO_PIN_PA30, 0);

	udelay(5000);

	/* release phys reset */
	gpio_set_value(GPIO_PIN_PC18, 0);	/* PHY RESET (Release)	*/

#if defined(CONFIG_MACB)
	/* init macb0 pins */
	gpio_select_periph_A(GPIO_PIN_PC3,  0);	/* TXD0	*/
	gpio_select_periph_A(GPIO_PIN_PC4,  0);	/* TXD1	*/
	gpio_select_periph_A(GPIO_PIN_PC7,  0);	/* TXEN	*/
	gpio_select_periph_A(GPIO_PIN_PC8,  0);	/* TXCK */
	gpio_select_periph_A(GPIO_PIN_PC9,  0);	/* RXD0	*/
	gpio_select_periph_A(GPIO_PIN_PC10, 0);	/* RXD1	*/
	gpio_select_periph_A(GPIO_PIN_PC13, 0);	/* RXER	*/
	gpio_select_periph_A(GPIO_PIN_PC15, 0);	/* RXDV	*/
	gpio_select_periph_A(GPIO_PIN_PC16, 0);	/* MDC	*/
	gpio_select_periph_A(GPIO_PIN_PC17, 0);	/* MDIO	*/
#if !defined(CONFIG_RMII)
	gpio_select_periph_A(GPIO_PIN_PC0,  0);	/* COL	*/
	gpio_select_periph_A(GPIO_PIN_PC1,  0);	/* CRS	*/
	gpio_select_periph_A(GPIO_PIN_PC2,  0);	/* TXER	*/
	gpio_select_periph_A(GPIO_PIN_PC5,  0);	/* TXD2	*/
	gpio_select_periph_A(GPIO_PIN_PC6,  0);	/* TXD3 */
	gpio_select_periph_A(GPIO_PIN_PC11, 0);	/* RXD2	*/
	gpio_select_periph_A(GPIO_PIN_PC12, 0);	/* RXD3	*/
	gpio_select_periph_A(GPIO_PIN_PC14, 0);	/* RXCK	*/
#endif

	/* init macb1 pins */
	gpio_select_periph_B(GPIO_PIN_PD13, 0);	/* TXD0	*/
	gpio_select_periph_B(GPIO_PIN_PD14, 0);	/* TXD1	*/
	gpio_select_periph_B(GPIO_PIN_PD11, 0);	/* TXEN	*/
	gpio_select_periph_B(GPIO_PIN_PD12, 0);	/* TXCK */
	gpio_select_periph_B(GPIO_PIN_PD10, 0);	/* RXD0	*/
	gpio_select_periph_B(GPIO_PIN_PD6,  0);	/* RXD1	*/
	gpio_select_periph_B(GPIO_PIN_PD5,  0);	/* RXER	*/
	gpio_select_periph_B(GPIO_PIN_PD4,  0);	/* RXDV	*/
	gpio_select_periph_B(GPIO_PIN_PD3,  0);	/* MDC	*/
	gpio_select_periph_B(GPIO_PIN_PD2,  0);	/* MDIO	*/
#if !defined(CONFIG_RMII)
	gpio_select_periph_B(GPIO_PIN_PC19, 0);	/* COL	*/
	gpio_select_periph_B(GPIO_PIN_PC23, 0);	/* CRS	*/
	gpio_select_periph_B(GPIO_PIN_PC26, 0);	/* TXER	*/
	gpio_select_periph_B(GPIO_PIN_PC27, 0);	/* TXD2	*/
	gpio_select_periph_B(GPIO_PIN_PC28, 0);	/* TXD3 */
	gpio_select_periph_B(GPIO_PIN_PC29, 0);	/* RXD2	*/
	gpio_select_periph_B(GPIO_PIN_PC30, 0);	/* RXD3	*/
	gpio_select_periph_B(GPIO_PIN_PC24, 0);	/* RXCK	*/
#endif
#endif

#if defined(CONFIG_MMC)
	gpio_enable_mmci();
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

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return (bus == 0) && (cs == 0);
}

void spi_cs_activate(struct spi_slave *slave)
{
}

void spi_cs_deactivate(struct spi_slave *slave)
{
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bi)
{
	macb_eth_initialize(0, (void *)MACB0_BASE, bi->bi_phy_id[0]);
	macb_eth_initialize(1, (void *)MACB1_BASE, bi->bi_phy_id[1]);

	return 0;
}
#endif
