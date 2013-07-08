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
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
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
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
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

	writel(1 << ATMEL_ID_PIOCD, &pmc->pcer);

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
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

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

		writel(1 << ATMEL_ID_LCDC, &pmc->pcer);
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
	at91_spi0_hw_init(1 << 0);
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
