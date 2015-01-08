/*
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
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
#include <nand.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

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

static void sama5d4_xplained_spi0_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* SPI0_SPCK */

	at91_set_pio_output(AT91_PIO_PORTC, 3, 1);	/* SPI0_CS0 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_SPI0);
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_NAND_ATMEL
static void sama5d4_xplained_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS3 for NAND */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(2) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(2) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(2) | AT91_SMC_TIMINGS_TADL(7) |
	       AT91_SMC_TIMINGS_TAR(2)  | AT91_SMC_TIMINGS_TRR(3)   |
	       AT91_SMC_TIMINGS_TWB(7)  | AT91_SMC_TIMINGS_RBNSEL(3)|
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	at91_set_a_periph(AT91_PIO_PORTC, 5, 0);	/* D0 */
	at91_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* D1 */
	at91_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* D2 */
	at91_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* D3 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* D4 */
	at91_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* D5 */
	at91_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* D6 */
	at91_set_a_periph(AT91_PIO_PORTC, 12, 0);	/* D7 */
	at91_set_a_periph(AT91_PIO_PORTC, 13, 0);	/* RE */
	at91_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* WE */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 1);	/* NCS */
	at91_set_a_periph(AT91_PIO_PORTC, 16, 1);	/* RDY */
	at91_set_a_periph(AT91_PIO_PORTC, 17, 1);	/* ALE */
	at91_set_a_periph(AT91_PIO_PORTC, 18, 1);	/* CLE */
}
#endif

#ifdef CONFIG_CMD_USB
static void sama5d4_xplained_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 11, 1);
	at91_set_pio_output(AT91_PIO_PORTE, 14, 1);
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col = 480,
	.vl_row = 272,
	.vl_clk = 9000000,
	.vl_bpix = LCD_BPP,
	.vl_tft = 1,
	.vl_hsync_len = 41,
	.vl_left_margin = 2,
	.vl_right_margin = 2,
	.vl_vsync_len = 11,
	.vl_upper_margin = 2,
	.vl_lower_margin = 2,
	.mmio = ATMEL_BASE_LCDC,
};

/* No power up/down pin for the LCD pannel */
void lcd_enable(void)	{ /* Empty! */ }
void lcd_disable(void)	{ /* Empty! */ }

unsigned int has_lcdc(void)
{
	return 1;
}

static void sama5d4_xplained_lcd_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 24, 0);	/* LCDPWM */
	at91_set_a_periph(AT91_PIO_PORTA, 25, 0);	/* LCDDISP */
	at91_set_a_periph(AT91_PIO_PORTA, 26, 0);	/* LCDVSYNC */
	at91_set_a_periph(AT91_PIO_PORTA, 27, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTA, 28, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTA, 29, 0);	/* LCDDEN */

	at91_set_a_periph(AT91_PIO_PORTA,  0, 0);	/* LCDD0 */
	at91_set_a_periph(AT91_PIO_PORTA,  1, 0);	/* LCDD1 */
	at91_set_a_periph(AT91_PIO_PORTA,  2, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTA,  3, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTA,  4, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTA,  5, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTA,  6, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTA,  7, 0);	/* LCDD7 */

	at91_set_a_periph(AT91_PIO_PORTA,  8, 0);	/* LCDD9 */
	at91_set_a_periph(AT91_PIO_PORTA,  9, 0);	/* LCDD8 */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* LCDD12 */
	at91_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTA, 14, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTA, 15, 0);	/* LCDD15 */

	at91_set_a_periph(AT91_PIO_PORTA, 16, 0);	/* LCDD16 */
	at91_set_a_periph(AT91_PIO_PORTA, 17, 0);	/* LCDD17 */
	at91_set_a_periph(AT91_PIO_PORTA, 18, 0);	/* LCDD18 */
	at91_set_a_periph(AT91_PIO_PORTA, 19, 0);	/* LCDD19 */
	at91_set_a_periph(AT91_PIO_PORTA, 20, 0);	/* LCDD20 */
	at91_set_a_periph(AT91_PIO_PORTA, 21, 0);	/* LCDD21 */
	at91_set_a_periph(AT91_PIO_PORTA, 22, 0);	/* LCDD22 */
	at91_set_a_periph(AT91_PIO_PORTA, 23, 0);	/* LCDD23 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_LCDC);
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf("2014 ATMEL Corp\n");
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

#ifdef CONFIG_GENERIC_ATMEL_MCI
void sama5d4_xplained_mci1_hw_init(void)
{
	at91_set_c_periph(AT91_PIO_PORTE, 19, 1);	/* MCI1 CDA */
	at91_set_c_periph(AT91_PIO_PORTE, 20, 1);	/* MCI1 DA0 */
	at91_set_c_periph(AT91_PIO_PORTE, 21, 1);	/* MCI1 DA1 */
	at91_set_c_periph(AT91_PIO_PORTE, 22, 1);	/* MCI1 DA2 */
	at91_set_c_periph(AT91_PIO_PORTE, 23, 1);	/* MCI1 DA3 */
	at91_set_c_periph(AT91_PIO_PORTE, 18, 0);	/* MCI1 CLK */

	/*
	 * As the mci io internal pull down is too strong, so if the io needs
	 * external pull up, the pull up resistor will be very small, if so
	 * the power consumption will increase, so disable the interanl pull
	 * down to save the power.
	 */
	at91_set_pio_pulldown(AT91_PIO_PORTE, 18, 0);
	at91_set_pio_pulldown(AT91_PIO_PORTE, 19, 0);
	at91_set_pio_pulldown(AT91_PIO_PORTE, 20, 0);
	at91_set_pio_pulldown(AT91_PIO_PORTE, 21, 0);
	at91_set_pio_pulldown(AT91_PIO_PORTE, 22, 0);
	at91_set_pio_pulldown(AT91_PIO_PORTE, 23, 0);

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI1);
}

int board_mmc_init(bd_t *bis)
{
	return atmel_mci_init((void *)ATMEL_BASE_MCI1);
}
#endif /* CONFIG_GENERIC_ATMEL_MCI */

#ifdef CONFIG_MACB
void sama5d4_xplained_macb0_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ETXCK_EREFCK */
	at91_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* ERXDV */
	at91_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* ERX0 */
	at91_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ERX1 */
	at91_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ERXER */
	at91_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ETXEN */
	at91_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* ETX0 */
	at91_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* ETX1 */
	at91_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* EMDIO */
	at91_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* EMDC */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_GMAC0);
}
#endif

static void sama5d4_xplained_serial3_hw_init(void)
{
	at91_set_b_periph(AT91_PIO_PORTE, 17, 1);	/* TXD3 */
	at91_set_b_periph(AT91_PIO_PORTE, 16, 0);	/* RXD3 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART3);
}

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

	sama5d4_xplained_serial3_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_ATMEL_SPI
	sama5d4_xplained_spi0_hw_init();
#endif
#ifdef CONFIG_NAND_ATMEL
	sama5d4_xplained_nand_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	sama5d4_xplained_mci1_hw_init();
#endif
#ifdef CONFIG_MACB
	sama5d4_xplained_macb0_hw_init();
#endif
#ifdef CONFIG_LCD
	sama5d4_xplained_lcd_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	sama5d4_xplained_usb_hw_init();
#endif

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

	return rc;
}
