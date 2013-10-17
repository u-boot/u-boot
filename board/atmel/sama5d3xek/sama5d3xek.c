/*
 * Copyright (C) 2012 - 2013 Atmel Corporation
 * Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#include <atmel_mci.h>
#include <micrel.h>
#include <net.h>
#include <netdev.h>

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#include <asm/arch/atmel_usba_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_NAND_ATMEL
void sama5d3xek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(5),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(8),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(3) | AT91_SMC_TIMINGS_TADL(10) |
	       AT91_SMC_TIMINGS_TAR(3)  | AT91_SMC_TIMINGS_TRR(4)   |
	       AT91_SMC_TIMINGS_TWB(5)  | AT91_SMC_TIMINGS_RBNSEL(3)|
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);
}
#endif

#ifdef CONFIG_CMD_USB
static void sama5d3xek_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTD, 25, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 26, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 27, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
static void sama5d3xek_mci_hw_init(void)
{
	at91_mci_hw_init();

	at91_set_pio_output(AT91_PIO_PORTB, 10, 0);	/* MCI0 Power */
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col = 800,
	.vl_row = 480,
	.vl_clk = 24000000,
	.vl_sync = ATMEL_LCDC_INVLINE_NORMAL | ATMEL_LCDC_INVFRAME_NORMAL,
	.vl_bpix = LCD_BPP,
	.vl_tft = 1,
	.vl_hsync_len = 128,
	.vl_left_margin = 64,
	.vl_right_margin = 64,
	.vl_vsync_len = 2,
	.vl_upper_margin = 22,
	.vl_lower_margin = 21,
	.mmio = ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
}

void lcd_disable(void)
{
}

static void sama5d3xek_lcd_hw_init(void)
{
	gd->fb_base = CONFIG_SAMA5D3_LCD_BASE;

	/* The higher 8 bit of LCD is board related */
	at91_set_c_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD16 */
	at91_set_c_periph(AT91_PIO_PORTC, 13, 0);	/* LCDD17 */
	at91_set_c_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD18 */
	at91_set_c_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD19 */
	at91_set_c_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD20 */
	at91_set_c_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD21 */
	at91_set_c_periph(AT91_PIO_PORTE, 27, 0);	/* LCDD22 */
	at91_set_c_periph(AT91_PIO_PORTE, 28, 0);	/* LCDD23 */

	/* Configure lower 16 bit of LCD and enable clock */
	at91_lcd_hw_init();
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_printf("(C) 2013 ATMEL Corp\n");
	lcd_printf("at91@atmel.com\n");
	lcd_printf("%s CPU at %s MHz\n", get_cpu_name(),
		   strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;

	nand_size = 0;
#ifdef CONFIG_NAND_ATMEL
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;
#endif
	lcd_printf("%ld MB SDRAM, %ld MB NAND\n",
		   dram_size >> 20, nand_size >> 20);
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

int board_early_init_f(void)
{
	at91_seriald_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	sama5d3xek_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	sama5d3xek_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	sama5d3xek_mci_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	if (has_emac())
		at91_macb_hw_init();
	if (has_gmac())
		at91_gmac_hw_init();
#endif
#ifdef CONFIG_LCD
	if (has_lcdc())
		sama5d3xek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	/* rx data delay */
	ksz9021_phy_extended_write(phydev,
				   MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0x2222);
	/* tx data delay */
	ksz9021_phy_extended_write(phydev,
				   MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0x2222);
	/* rx/tx clock delay */
	ksz9021_phy_extended_write(phydev,
				   MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0xf2f4);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_MACB
	if (has_emac())
		rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
	if (has_gmac())
		rc = macb_eth_initialize(0, (void *)ATMEL_BASE_GMAC, 0x00);
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	usba_udc_probe(&pdata);
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif

	return rc;
}

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bis)
{
	int rc = 0;

	rc = atmel_mci_init((void *)ATMEL_BASE_MCI0);

	return rc;
}
#endif

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 4;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 0:
		at91_set_pio_output(AT91_PIO_PORTD, 13, 0);
	case 1:
		at91_set_pio_output(AT91_PIO_PORTD, 14, 0);
	case 2:
		at91_set_pio_output(AT91_PIO_PORTD, 15, 0);
	case 3:
		at91_set_pio_output(AT91_PIO_PORTD, 16, 0);
	default:
		break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 0:
		at91_set_pio_output(AT91_PIO_PORTD, 13, 1);
	case 1:
		at91_set_pio_output(AT91_PIO_PORTD, 14, 1);
	case 2:
		at91_set_pio_output(AT91_PIO_PORTD, 15, 1);
	case 3:
		at91_set_pio_output(AT91_PIO_PORTD, 16, 1);
	default:
		break;
	}
}
#endif /* CONFIG_ATMEL_SPI */
