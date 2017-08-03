/*
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_usba_udc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/sama5d4.h>
#include <atmel_hlcdc.h>
#include <atmel_mci.h>
#include <lcd.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <spi_flash.h>
#include <spl.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

static u8 boot_mode_sf;

#ifdef CONFIG_ATMEL_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTC, 3, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTC, 3, 1);
}

static void ma5d4evk_spi0_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* SPI0_MISO */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* SPI0_MOSI */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* SPI0_SPCK */

	at91_set_pio_output(AT91_PIO_PORTC, 3, 1);	/* SPI0_CS0 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_SPI0);
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_CMD_USB
static void ma5d4evk_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 11, 0);
	at91_set_pio_output(AT91_PIO_PORTE, 14, 0);
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col			= 800,
	.vl_row			= 480,
	.vl_clk			= 33500000,
	.vl_bpix		= LCD_BPP,
	.vl_tft			= 1,
	.vl_hsync_len		= 10,
	.vl_left_margin		= 89,
	.vl_right_margin	= 164,
	.vl_vsync_len		= 10,
	.vl_upper_margin	= 23,
	.vl_lower_margin	= 10,
	.mmio			= ATMEL_BASE_LCDC,
};

/* No power up/down pin for the LCD pannel */
void lcd_enable(void)	{ /* Empty! */ }
void lcd_disable(void)	{ /* Empty! */ }

unsigned int has_lcdc(void)
{
	return 1;
}

static void ma5d4evk_lcd_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 24, 1);	/* LCDPWM */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 25, 0);	/* LCDDISP */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 26, 0);	/* LCDVSYNC */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 27, 0);	/* LCDHSYNC */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 28, 0);	/* LCDDOTCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 29, 1);	/* LCDDEN */

	at91_pio3_set_a_periph(AT91_PIO_PORTA,  0, 0);	/* LCDD0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  1, 0);	/* LCDD1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  2, 0);	/* LCDD2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  3, 0);	/* LCDD3 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  4, 0);	/* LCDD4 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  5, 0);	/* LCDD5 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  6, 0);	/* LCDD6 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  7, 0);	/* LCDD7 */

	at91_pio3_set_a_periph(AT91_PIO_PORTA,  8, 0);	/* LCDD9 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  9, 0);	/* LCDD8 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 10, 0);	/* LCDD10 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* LCDD11 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* LCDD12 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* LCDD13 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 14, 0);	/* LCDD14 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 15, 0);	/* LCDD15 */

	at91_pio3_set_a_periph(AT91_PIO_PORTA, 16, 0);	/* LCDD16 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 17, 0);	/* LCDD17 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 18, 0);	/* LCDD18 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 19, 0);	/* LCDD19 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 20, 0);	/* LCDD20 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 21, 0);	/* LCDD21 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 22, 0);	/* LCDD22 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 23, 0);	/* LCDD23 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_LCDC);
}

#endif /* CONFIG_LCD */

#ifdef CONFIG_GENERIC_ATMEL_MCI
/* On-SoM eMMC */
void ma5d4evk_mci0_hw_init(void)
{
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 5, 1);	/* MCI1 CDA */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 6, 1);	/* MCI1 DA0 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 7, 1);	/* MCI1 DA1 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 8, 1);	/* MCI1 DA2 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 9, 1);	/* MCI1 DA3 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 10, 1);	/* MCI1 DA4 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 11, 1);	/* MCI1 DA5 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 12, 1);	/* MCI1 DA6 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 13, 1);	/* MCI1 DA7 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 4, 0);	/* MCI1 CLK */

	/*
	 * As the mci io internal pull down is too strong, so if the io needs
	 * external pull up, the pull up resistor will be very small, if so
	 * the power consumption will increase, so disable the internal pull
	 * down to save the power.
	 */
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 5, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 6, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 7, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 8, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 9, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 10, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 11, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 12, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 13, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 4, 0);

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI0);
}

/* On-board MicroSD slot */
void ma5d4evk_mci1_hw_init(void)
{
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 19, 1);	/* MCI1 CDA */
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 20, 1);	/* MCI1 DA0 */
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 21, 1);	/* MCI1 DA1 */
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 22, 1);	/* MCI1 DA2 */
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 23, 1);	/* MCI1 DA3 */
	at91_pio3_set_c_periph(AT91_PIO_PORTE, 18, 0);	/* MCI1 CLK */

	/*
	 * As the mci io internal pull down is too strong, so if the io needs
	 * external pull up, the pull up resistor will be very small, if so
	 * the power consumption will increase, so disable the internal pull
	 * down to save the power.
	 */
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 18, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 19, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 20, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 21, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 22, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 23, 0);

	/* Deal with WP pin on the microSD slot. */
	at91_set_pio_output(AT91_PIO_PORTE, 16, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 16, 1);

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI1);
}

int board_mmc_init(bd_t *bis)
{
	int ret;
	void *mci0 = (void *)ATMEL_BASE_MCI0;
	void *mci1 = (void *)ATMEL_BASE_MCI1;

	/* De-assert reset on On-SoM eMMC */
	at91_set_pio_output(AT91_PIO_PORTE, 15, 1);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 15, 0);

	ret = atmel_mci_init(boot_mode_sf ? mci0 : mci1);
	if (ret)	/* eMMC init failed, skip it. */
		at91_set_pio_output(AT91_PIO_PORTE, 15, 0);

	/* Enable the power supply to On-board MicroSD */
	at91_set_pio_output(AT91_PIO_PORTE, 17, 0);
	ret = atmel_mci_init(boot_mode_sf ? mci1 : mci0);
	if (ret)	/* uSD init failed, power it down. */
		at91_set_pio_output(AT91_PIO_PORTE, 17, 1);

	return 0;
}
#endif /* CONFIG_GENERIC_ATMEL_MCI */

#ifdef CONFIG_MACB
void ma5d4evk_macb0_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ETXCK_EREFCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* ERXDV */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* ERX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ERX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ERXER */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ETXEN */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* ETX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* ETX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* EMDIO */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* EMDC */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_GMAC0);
}
#endif

static void ma5d4evk_serial_hw_init(void)
{
	/* USART0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 13, 1);	/* TXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 12, 0);	/* RXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 11, 0);	/* RTS */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 10, 0);	/* CTS */
	at91_periph_clk_enable(ATMEL_ID_USART0);

	/* USART1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 17, 1);	/* TXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 16, 0);	/* RXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 15, 0);	/* RTS */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 14, 0);	/* CTS */
	at91_periph_clk_enable(ATMEL_ID_USART1);
}

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

	/* Configure LEDs as OFF */
	at91_set_pio_output(AT91_PIO_PORTD, 28, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 29, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 30, 0);

	ma5d4evk_serial_hw_init();

	return 0;
}

static void board_identify(void)
{
	struct spi_flash *sf;
	sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
			     CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	boot_mode_sf = (sf != NULL);
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_ATMEL_SPI
	ma5d4evk_spi0_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	ma5d4evk_mci0_hw_init();
	ma5d4evk_mci1_hw_init();
#endif
#ifdef CONFIG_MACB
	ma5d4evk_macb0_hw_init();
#endif
#ifdef CONFIG_LCD
	ma5d4evk_lcd_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	ma5d4evk_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
#endif

	board_identify();

	/* Reset CAN controllers */
	at91_set_pio_output(AT91_PIO_PORTB, 21, 0);
	udelay(100);
	at91_set_pio_output(AT91_PIO_PORTB, 21, 1);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTB, 21, 0);

	return 0;
}

int board_late_init(void)
{
	env_set("bootmode", boot_mode_sf ? "sf" : "emmc");
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_GMAC0, 0x00);
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	usba_udc_probe(&pdata);
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif

	return rc;
}

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
#ifdef CONFIG_ATMEL_SPI
	ma5d4evk_spi0_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	ma5d4evk_mci0_hw_init();
	ma5d4evk_mci1_hw_init();
#endif
	board_identify();
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();

	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case BOOT_DEVICE_SPI:
		break;
	case BOOT_DEVICE_USB:
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	}
}

static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_13 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_NDQS_DISABLED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

	ddr2->rtr = 0x2b0;

	ddr2->tpr0 = (8 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      10 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      25 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      23 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      3 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      8 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	/* enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	writel(AT91_PMC_DDR, &pmc->scer);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddr2);
}

void at91_pmc_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(87) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	writel(0x0 << 8, &pmc->pllicpr);

	tmp = AT91_PMC_MCKR_H32MXDIV |
	      AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
