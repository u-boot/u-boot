/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <netdev.h>

#include <asm/io.h>
#include <asm/sdram.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hmatrix.h>
#include <asm/arch/mmu.h>
#include <asm/arch/portmux.h>
#include <atmel_lcdc.h>
#include <lcd.h>

#include "../../../arch/avr32/cpu/hsmc3.h"

struct mmu_vm_range mmu_vmr_table[CONFIG_SYS_NR_VM_REGIONS] = {
	{
		.virt_pgno	= CONFIG_SYS_FLASH_BASE >> MMU_PAGE_SHIFT,
		.nr_pages	= CONFIG_SYS_FLASH_SIZE >> MMU_PAGE_SHIFT,
		.phys		= (CONFIG_SYS_FLASH_BASE >> MMU_PAGE_SHIFT)
					| MMU_VMR_CACHE_NONE,
	}, {
		.virt_pgno	= EBI_SRAM_CS2_BASE >> MMU_PAGE_SHIFT,
		.nr_pages	= EBI_SRAM_CS2_SIZE >> MMU_PAGE_SHIFT,
		.phys		= (EBI_SRAM_CS2_BASE >> MMU_PAGE_SHIFT)
					| MMU_VMR_CACHE_NONE,
	}, {
		.virt_pgno	= CONFIG_SYS_SDRAM_BASE >> MMU_PAGE_SHIFT,
		.nr_pages	= EBI_SDRAM_SIZE >> MMU_PAGE_SHIFT,
		.phys		= (CONFIG_SYS_SDRAM_BASE >> MMU_PAGE_SHIFT)
					| MMU_VMR_CACHE_WRBACK,
	},
};

#if defined(CONFIG_LCD)
/* 480x272x16 @ 72 Hz */
vidinfo_t panel_info = {
	.vl_col			= 480,		/* Number of columns */
	.vl_row			= 272,		/* Number of rows */
	.vl_clk			= 5000000,	/* pixel clock in ps */
	.vl_sync		= ATMEL_LCDC_INVCLK_INVERTED |
				  ATMEL_LCDC_INVLINE_INVERTED |
				  ATMEL_LCDC_INVFRAME_INVERTED,
	.vl_bpix		= LCD_COLOR16,	/* Bits per pixel, BPP = 2^n */
	.vl_tft			= 1,		/* 0 = passive, 1 = TFT */
	.vl_hsync_len		= 42,		/* Length of horizontal sync */
	.vl_left_margin		= 1,		/* Time from sync to picture */
	.vl_right_margin	= 1,		/* Time from picture to sync */
	.vl_vsync_len		= 1,		/* Length of vertical sync */
	.vl_upper_margin	= 12,		/* Time from sync to picture */
	.vl_lower_margin	= 1,		/* Time from picture to sync */
	.mmio			= LCDC_BASE,	/* Memory mapped registers */
};

void lcd_enable(void)
{
}

void lcd_disable(void)
{
}
#endif

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

	/* Enable 26 address bits and NCS2 */
	portmux_enable_ebi(16, 26, PORTMUX_EBI_CS(2), PORTMUX_DRIVE_HIGH);
	sdram_init(uncached(EBI_SDRAM_BASE), &sdram_config);

	portmux_enable_usart1(PORTMUX_DRIVE_MIN);

	/* de-assert "force sys reset" pin */
	portmux_select_gpio(PORTMUX_PORT_D, 1 << 15,
			PORTMUX_DIR_OUTPUT | PORTMUX_INIT_HIGH);

	/* init custom i/o */
	/* cpu type inputs */
	portmux_select_gpio(PORTMUX_PORT_E, (1 << 19) | (1 << 20) | (1 << 23),
			PORTMUX_DIR_INPUT);
	/* main board type inputs */
	portmux_select_gpio(PORTMUX_PORT_B, (1 << 19) | (1 << 29),
			PORTMUX_DIR_INPUT);
	/* DEBUG input (use weak pullup) */
	portmux_select_gpio(PORTMUX_PORT_E, 1 << 21,
			PORTMUX_DIR_INPUT | PORTMUX_PULL_UP);

	/* are we suppressing the console ? */
	if (gpio_get_value(GPIO_PIN_PE(21)) == 1)
		gd->flags |= (GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE);

	/* reset phys */
	portmux_select_gpio(PORTMUX_PORT_E, 1 << 24, PORTMUX_DIR_INPUT);
	portmux_select_gpio(PORTMUX_PORT_C, 1 << 18,
			PORTMUX_DIR_OUTPUT | PORTMUX_INIT_HIGH);

	udelay(5000);

	/* release phys reset */
	gpio_set_value(GPIO_PIN_PC(18), 0);	/* PHY RESET (Release)	*/

	/* setup Data Flash chip select (NCS2) */
	hsmc3_writel(MODE2, 0x20121003);
	hsmc3_writel(CYCLE2, 0x000a0009);
	hsmc3_writel(PULSE2, 0x0a060806);
	hsmc3_writel(SETUP2, 0x00030102);

	/* setup FRAM chip select (NCS3) */
	hsmc3_writel(MODE3, 0x10120001);
	hsmc3_writel(CYCLE3, 0x001e001d);
	hsmc3_writel(PULSE3, 0x08040704);
	hsmc3_writel(SETUP3, 0x02050204);

#if defined(CONFIG_MACB)
	/* init macb0 pins */
	portmux_enable_macb0(PORTMUX_MACB_MII, PORTMUX_DRIVE_HIGH);
	portmux_enable_macb1(PORTMUX_MACB_MII, PORTMUX_DRIVE_HIGH);
#endif

#if defined(CONFIG_MMC)
	portmux_enable_mmci(0, PORTMUX_MMCI_4BIT, PORTMUX_DRIVE_LOW);
#endif

#if defined(CONFIG_LCD)
	portmux_enable_lcdc(1);
#endif

	return 0;
}

int board_early_init_r(void)
{
	gd->bd->bi_phy_id[0] = 0x01;
	gd->bd->bi_phy_id[1] = 0x03;
	return 0;
}

int board_postclk_init(void)
{
	/* Use GCLK0 as 10MHz output */
	gclk_enable_output(0, PORTMUX_DRIVE_LOW);
	gclk_set_rate(0, GCLK_PARENT_OSC0, 10000000);
	return 0;
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
	macb_eth_initialize(0, (void *)ATMEL_BASE_MACB0, bi->bi_phy_id[0]);
	macb_eth_initialize(1, (void *)ATMEL_BASE_MACB1, bi->bi_phy_id[1]);

	return 0;
}
#endif
