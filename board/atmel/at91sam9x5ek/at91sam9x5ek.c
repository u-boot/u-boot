/*
 * Copyright (C) 2012 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <lcd.h>
#include <atmel_hlcdc.h>
#include <atmel_mci.h>
#ifdef CONFIG_MACB
#include <net.h>
#endif
#include <netdev.h>
#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>
#endif
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
#ifdef CONFIG_CMD_NAND
static void at91sam9x5ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	/* NAND flash on D16 */
	csa |= AT91_MATRIX_NFD0_ON_D16;

	/* Configure IO drive */
	csa &= ~AT91_MATRIX_EBI_EBI_IOSR_NORMAL;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
		AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(6),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(6),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(1),
		&smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOCD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);
	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	at91_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* NAND ALE */
	at91_set_a_periph(AT91_PIO_PORTD, 3, 1);	/* NAND CLE */
	at91_set_a_periph(AT91_PIO_PORTD, 6, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 7, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 8, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 9, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 10, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 11, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 12, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 13, 1);
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_MACB
	if (has_emac0())
		rc = macb_eth_initialize(0,
			(void *)ATMEL_BASE_EMAC0, 0x00);
	if (has_emac1())
		rc = macb_eth_initialize(1,
			(void *)ATMEL_BASE_EMAC1, 0x00);
#endif
	return rc;
}

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col	= 800,
	.vl_row = 480,
	.vl_clk = 24000000,
	.vl_sync = LCDC_LCDCFG5_HSPOL | LCDC_LCDCFG5_VSPOL,
	.vl_bpix = LCD_BPP,
	.vl_tft = 1,
	.vl_clk_pol = 1,
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
	if (has_lcdc())
		at91_set_a_periph(AT91_PIO_PORTC, 29, 1);	/* power up */
}

void lcd_disable(void)
{
	if (has_lcdc())
		at91_set_a_periph(AT91_PIO_PORTC, 29, 0);	/* power down */
}

static void at91sam9x5ek_lcd_hw_init(void)
{
	if (has_lcdc()) {
		at91_set_a_periph(AT91_PIO_PORTC, 26, 0);	/* LCDPWM */
		at91_set_a_periph(AT91_PIO_PORTC, 27, 0);	/* LCDVSYNC */
		at91_set_a_periph(AT91_PIO_PORTC, 28, 0);	/* LCDHSYNC */
		at91_set_a_periph(AT91_PIO_PORTC, 24, 0);	/* LCDDISP */
		at91_set_a_periph(AT91_PIO_PORTC, 29, 0);	/* LCDDEN */
		at91_set_a_periph(AT91_PIO_PORTC, 30, 0);	/* LCDPCK */

		at91_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* LCDD0 */
		at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* LCDD1 */
		at91_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* LCDD2 */
		at91_set_a_periph(AT91_PIO_PORTC, 3, 0);	/* LCDD3 */
		at91_set_a_periph(AT91_PIO_PORTC, 4, 0);	/* LCDD4 */
		at91_set_a_periph(AT91_PIO_PORTC, 5, 0);	/* LCDD5 */
		at91_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* LCDD6 */
		at91_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* LCDD7 */
		at91_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* LCDD8 */
		at91_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* LCDD9 */
		at91_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD10 */
		at91_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD11 */
		at91_set_a_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD12 */
		at91_set_a_periph(AT91_PIO_PORTC, 13, 0);	/* LCDD13 */
		at91_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD14 */
		at91_set_a_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD15 */
		at91_set_a_periph(AT91_PIO_PORTC, 16, 0);	/* LCDD16 */
		at91_set_a_periph(AT91_PIO_PORTC, 17, 0);	/* LCDD17 */
		at91_set_a_periph(AT91_PIO_PORTC, 18, 0);	/* LCDD18 */
		at91_set_a_periph(AT91_PIO_PORTC, 19, 0);	/* LCDD19 */
		at91_set_a_periph(AT91_PIO_PORTC, 20, 0);	/* LCDD20 */
		at91_set_a_periph(AT91_PIO_PORTC, 21, 0);	/* LCDD21 */
		at91_set_a_periph(AT91_PIO_PORTC, 22, 0);	/* LCDD22 */
		at91_set_a_periph(AT91_PIO_PORTC, 23, 0);	/* LCDD23 */

		at91_periph_clk_enable(ATMEL_ID_LCDC);
	}
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	if (has_lcdc()) {
		lcd_printf("%s\n", U_BOOT_VERSION);
		lcd_printf("(C) 2012 ATMEL Corp\n");
		lcd_printf("at91support@atmel.com\n");
		lcd_printf("%s CPU at %s MHz\n",
			get_cpu_name(),
			strmhz(temp, get_cpu_clk_rate()));

		dram_size = 0;
		for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
			dram_size += gd->bd->bi_dram[i].size;
		nand_size = 0;
		for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
			nand_size += nand_info[i].size;
		lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
			dram_size >> 20,
			nand_size >> 20);
	}
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
		at91_set_pio_output(AT91_PIO_PORTA, 7, 0);
		break;
	case 0:
	default:
		at91_set_pio_output(AT91_PIO_PORTA, 14, 0);
		break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
		at91_set_pio_output(AT91_PIO_PORTA, 7, 1);
		break;
	case 0:
	default:
		at91_set_pio_output(AT91_PIO_PORTA, 14, 1);
		break;
	}
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bd)
{
	at91_mci_hw_init();

	return atmel_mci_init((void *)ATMEL_BASE_HSMCI0);
}
#endif

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	/* arch number of AT91SAM9X5EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9X5EK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sam9x5ek_nand_hw_init();
#endif

#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif

#ifdef CONFIG_MACB
	at91_macb_hw_init();
#endif

#if defined(CONFIG_USB_OHCI_NEW) || defined(CONFIG_USB_EHCI)
	at91_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sam9x5ek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
					CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>

void at91_spl_board_init(void)
{
#ifdef CONFIG_SYS_USE_MMC
	at91_mci_hw_init();
#elif CONFIG_SYS_USE_NANDFLASH
	at91sam9x5ek_nand_hw_init();
#elif CONFIG_SYS_USE_SPIFLASH
	at91_spi0_hw_init(1 << 4);
#endif
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_13 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED);

	ddr2->rtr = 0x411;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      19 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      18 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      3 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	/* enable DDR2 clock */
	writel(AT91_PMC_DDR, &pmc->scer);

	/* Chip select 1 is for DDR2/SDRAM */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS1A_SDRAMC;
	csa &= ~AT91_MATRIX_EBI_DBPU_OFF;
	csa |= AT91_MATRIX_EBI_DBPD_OFF;
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;
	writel(csa, &matrix->ebicsa);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC, ATMEL_BASE_CS1, &ddr2);
}
#endif
