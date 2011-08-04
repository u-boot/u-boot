/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
void at91sam9m10g45ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(2),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(4),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_CMD_USB
static void at91sam9m10g45ek_usb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(1 << ATMEL_ID_PIODE, &pmc->pcer);

	at91_set_gpio_output(AT91_PIN_PD1, 0);
	at91_set_gpio_output(AT91_PIN_PD3, 0);
}
#endif

#ifdef CONFIG_MACB
static void at91sam9m10g45ek_macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;
	struct at91_rstc *rstc = (struct at91_rstc *)ATMEL_BASE_RSTC;
	unsigned long erstl;

	/* Enable clock */
	writel(1 << ATMEL_ID_EMAC, &pmc->pcer);

	/*
	 * Disable pull-up on:
	 *      RXDV (PA15) => PHY normal mode (not Test mode)
	 *      ERX0 (PA12) => PHY ADDR0
	 *      ERX1 (PA13) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA12) |
	       pin_to_mask(AT91_PIN_PA13),
	       &pioa->pudr);

	erstl = readl(&rstc->mr) & AT91_RSTC_MR_ERSTL_MASK;

	/* Need to reset PHY -> 500ms reset */
	writel(AT91_RSTC_KEY | AT91_RSTC_MR_ERSTL(13) |
		AT91_RSTC_MR_URSTEN, &rstc->mr);

	writel(AT91_RSTC_KEY | AT91_RSTC_CR_EXTRST, &rstc->cr);

	/* Wait for end hardware reset */
	while (!(readl(&rstc->sr) & AT91_RSTC_SR_NRSTL))
		;

	/* Restore NRST value */
	writel(AT91_RSTC_KEY | erstl | AT91_RSTC_MR_URSTEN,
		&rstc->mr);

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA12) |
	       pin_to_mask(AT91_PIN_PA13),
	       &pioa->puer);

	/* And the pins. */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_LCD

vidinfo_t panel_info = {
	vl_col:		480,
	vl_row:		272,
	vl_clk:		9000000,
	vl_sync:	ATMEL_LCDC_INVLINE_NORMAL |
			ATMEL_LCDC_INVFRAME_NORMAL,
	vl_bpix:	3,
	vl_tft:		1,
	vl_hsync_len:	45,
	vl_left_margin:	1,
	vl_right_margin:1,
	vl_vsync_len:	1,
	vl_upper_margin:40,
	vl_lower_margin:1,
	mmio :		 ATMEL_BASE_LCDC,
};


void lcd_enable(void)
{
	at91_set_A_periph(AT91_PIN_PE6, 1);	/* power up */
}

void lcd_disable(void)
{
	at91_set_A_periph(AT91_PIN_PE6, 0);	/* power down */
}

static void at91sam9m10g45ek_lcd_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	at91_set_A_periph(AT91_PIN_PE0, 0);	/* LCDDPWR */
	at91_set_A_periph(AT91_PIN_PE2, 0);	/* LCDCC */
	at91_set_A_periph(AT91_PIN_PE3, 0);	/* LCDVSYNC */
	at91_set_A_periph(AT91_PIN_PE4, 0);	/* LCDHSYNC */
	at91_set_A_periph(AT91_PIN_PE5, 0);	/* LCDDOTCK */

	at91_set_A_periph(AT91_PIN_PE7, 0);	/* LCDD0 */
	at91_set_A_periph(AT91_PIN_PE8, 0);	/* LCDD1 */
	at91_set_A_periph(AT91_PIN_PE9, 0);	/* LCDD2 */
	at91_set_A_periph(AT91_PIN_PE10, 0);	/* LCDD3 */
	at91_set_A_periph(AT91_PIN_PE11, 0);	/* LCDD4 */
	at91_set_A_periph(AT91_PIN_PE12, 0);	/* LCDD5 */
	at91_set_A_periph(AT91_PIN_PE13, 0);	/* LCDD6 */
	at91_set_A_periph(AT91_PIN_PE14, 0);	/* LCDD7 */
	at91_set_A_periph(AT91_PIN_PE15, 0);	/* LCDD8 */
	at91_set_A_periph(AT91_PIN_PE16, 0);	/* LCDD9 */
	at91_set_A_periph(AT91_PIN_PE17, 0);	/* LCDD10 */
	at91_set_A_periph(AT91_PIN_PE18, 0);	/* LCDD11 */
	at91_set_A_periph(AT91_PIN_PE19, 0);	/* LCDD12 */
	at91_set_B_periph(AT91_PIN_PE20, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PE21, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PE22, 0);	/* LCDD15 */
	at91_set_A_periph(AT91_PIN_PE23, 0);	/* LCDD16 */
	at91_set_A_periph(AT91_PIN_PE24, 0);	/* LCDD17 */
	at91_set_A_periph(AT91_PIN_PE25, 0);	/* LCDD18 */
	at91_set_A_periph(AT91_PIN_PE26, 0);	/* LCDD19 */
	at91_set_A_periph(AT91_PIN_PE27, 0);	/* LCDD20 */
	at91_set_B_periph(AT91_PIN_PE28, 0);	/* LCDD21 */
	at91_set_A_periph(AT91_PIN_PE29, 0);	/* LCDD22 */
	at91_set_A_periph(AT91_PIN_PE30, 0);	/* LCDD23 */

	writel(1 << ATMEL_ID_LCDC, &pmc->pcer);

	gd->fb_base = CONFIG_AT91SAM9G45_LCD_BASE;
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2008 ATMEL Corp\n");
	lcd_printf ("at91support@atmel.com\n");
	lcd_printf ("%s CPU at %s MHz\n",
		ATMEL_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;
	lcd_printf ("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20 );
}
#endif /* CONFIG_LCD_INFO */
#endif

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91SAM9M10G45EK-Board */
#ifdef CONFIG_AT91SAM9M10G45EK
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9M10G45EK;
#elif defined CONFIG_AT91SAM9G45EKES
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9G45EKES;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sam9m10g45ek_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	at91sam9m10g45ek_usb_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_MACB
	at91sam9m10g45ek_macb_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sam9m10g45ek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
	return rc;
}

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch(slave->cs) {
		case 1:
			at91_set_gpio_output(AT91_PIN_PB18, 0);
			break;
		case 0:
		default:
			at91_set_gpio_output(AT91_PIN_PB3, 0);
			break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch(slave->cs) {
		case 1:
			at91_set_gpio_output(AT91_PIN_PB18, 1);
			break;
		case 0:
		default:
			at91_set_gpio_output(AT91_PIN_PB3, 1);
		break;
	}
}
#endif /* CONFIG_ATMEL_SPI */
