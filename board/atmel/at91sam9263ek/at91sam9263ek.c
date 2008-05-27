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
#include <asm/sizes.h>
#include <asm/arch/at91sam9263.h>
#include <asm/arch/at91sam9263_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

static void at91sam9263ek_serial_hw_init(void)
{
#ifdef CONFIG_USART0
	at91_set_A_periph(AT91_PIN_PA26, 1);		/* TXD0 */
	at91_set_A_periph(AT91_PIN_PA27, 0);		/* RXD0 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US0);
#endif

#ifdef CONFIG_USART1
	at91_set_A_periph(AT91_PIN_PD0, 1);		/* TXD1 */
	at91_set_A_periph(AT91_PIN_PD1, 0);		/* RXD1 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US1);
#endif

#ifdef CONFIG_USART2
	at91_set_A_periph(AT91_PIN_PD2, 1);		/* TXD2 */
	at91_set_A_periph(AT91_PIN_PD3, 0);		/* RXD2 */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_US2);
#endif

#ifdef CONFIG_USART3	/* DBGU */
	at91_set_A_periph(AT91_PIN_PC30, 0);		/* DRXD */
	at91_set_A_periph(AT91_PIN_PC31, 1);		/* DTXD */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91_ID_SYS);
#endif
}

#ifdef CONFIG_CMD_NAND
static void at91sam9263ek_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBI0CSA);
	at91_sys_write(AT91_MATRIX_EBI0CSA,
		       csa | AT91_MATRIX_EBI0_CS3A_SMC_SMARTMEDIA);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		       AT91_SMC_NWESETUP_(1) | AT91_SMC_NCS_WRSETUP_(0) |
		       AT91_SMC_NRDSETUP_(1) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC_PULSE(3),
		       AT91_SMC_NWEPULSE_(3) | AT91_SMC_NCS_WRPULSE_(3) |
		       AT91_SMC_NRDPULSE_(3) | AT91_SMC_NCS_RDPULSE_(3));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(5) | AT91_SMC_NRDCYCLE_(5));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
#ifdef CFG_NAND_DBW_16
		       AT91_SMC_DBW_16 |
#else /* CFG_NAND_DBW_8 */
		       AT91_SMC_DBW_8 |
#endif
		       AT91_SMC_TDF_(2));

	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_PIOA |
				      1 << AT91SAM9263_ID_PIOCDE);

	/* Configure RDY/BSY */
	at91_set_gpio_input(AT91_PIN_PA22, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(AT91_PIN_PD15, 1);
}
#endif

#ifdef CONFIG_HAS_DATAFLASH
static void at91sam9263ek_spi_hw_init(void)
{
	at91_set_B_periph(AT91_PIN_PA5, 0);	/* SPI0_NPCS0 */

	at91_set_B_periph(AT91_PIN_PA0, 0);	/* SPI0_MISO */
	at91_set_B_periph(AT91_PIN_PA1, 0);	/* SPI0_MOSI */
	at91_set_B_periph(AT91_PIN_PA2, 0);	/* SPI0_SPCK */

	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_SPI0);
}
#endif

#ifdef CONFIG_MACB
static void at91sam9263ek_macb_hw_init(void)
{
	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *	RXDV (PC25) => PHY normal mode (not Test mode)
	 * 	ERX0 (PE25) => PHY ADDR0
	 *	ERX1 (PE26) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PC25),
	       pin_to_controller(AT91_PIN_PC0) + PIO_PUDR);
	writel(pin_to_mask(AT91_PIN_PE25) |
	       pin_to_mask(AT91_PIN_PE26),
	       pin_to_controller(AT91_PIN_PE0) + PIO_PUDR);

	/* Need to reset PHY -> 500ms reset */
	at91_sys_write(AT91_RSTC_MR, AT91_RSTC_KEY |
				     (AT91_RSTC_ERSTL & (0x0D << 8)) |
				     AT91_RSTC_URSTEN);

	at91_sys_write(AT91_RSTC_CR, AT91_RSTC_KEY | AT91_RSTC_EXTRST);

	/* Wait for end hardware reset */
	while (!(at91_sys_read(AT91_RSTC_SR) & AT91_RSTC_NRSTL));

	/* Restore NRST value */
	at91_sys_write(AT91_RSTC_MR, AT91_RSTC_KEY |
				     (AT91_RSTC_ERSTL & (0x0 << 8)) |
				     AT91_RSTC_URSTEN);

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PC25),
	       pin_to_controller(AT91_PIN_PC0) + PIO_PUER);
	writel(pin_to_mask(AT91_PIN_PE25) |
	       pin_to_mask(AT91_PIN_PE26),
	       pin_to_controller(AT91_PIN_PE0) + PIO_PUER);

	at91_set_A_periph(AT91_PIN_PE21, 0);	/* ETXCK_EREFCK */
	at91_set_B_periph(AT91_PIN_PC25, 0);	/* ERXDV */
	at91_set_A_periph(AT91_PIN_PE25, 0);	/* ERX0 */
	at91_set_A_periph(AT91_PIN_PE26, 0);	/* ERX1 */
	at91_set_A_periph(AT91_PIN_PE27, 0);	/* ERXER */
	at91_set_A_periph(AT91_PIN_PE28, 0);	/* ETXEN */
	at91_set_A_periph(AT91_PIN_PE23, 0);	/* ETX0 */
	at91_set_A_periph(AT91_PIN_PE24, 0);	/* ETX1 */
	at91_set_A_periph(AT91_PIN_PE30, 0);	/* EMDIO */
	at91_set_A_periph(AT91_PIN_PE29, 0);	/* EMDC */

#ifndef CONFIG_RMII
	at91_set_A_periph(AT91_PIN_PE22, 0);	/* ECRS */
	at91_set_B_periph(AT91_PIN_PC26, 0);	/* ECOL */
	at91_set_B_periph(AT91_PIN_PC22, 0);	/* ERX2 */
	at91_set_B_periph(AT91_PIN_PC23, 0);	/* ERX3 */
	at91_set_B_periph(AT91_PIN_PC27, 0);	/* ERXCK */
	at91_set_B_periph(AT91_PIN_PC20, 0);	/* ETX2 */
	at91_set_B_periph(AT91_PIN_PC21, 0);	/* ETX3 */
	at91_set_B_periph(AT91_PIN_PC24, 0);	/* ETXER */
#endif

}
#endif

#ifdef CONFIG_USB_OHCI_NEW
static void at91sam9263ek_uhp_hw_init(void)
{
	/* Enable VBus on UHP ports */
	at91_set_gpio_output(AT91_PIN_PA21, 0);
	at91_set_gpio_output(AT91_PIN_PA24, 0);
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	vl_col:		240,
	vl_row:		320,
	vl_clk:		4965000,
	vl_sync:	ATMEL_LCDC_INVLINE_INVERTED |
			ATMEL_LCDC_INVFRAME_INVERTED,
	vl_bpix:	3,
	vl_tft:		1,
	vl_hsync_len:	5,
	vl_left_margin:	1,
	vl_right_margin:33,
	vl_vsync_len:	1,
	vl_upper_margin:1,
	vl_lower_margin:0,
	mmio:		AT91SAM9263_LCDC_BASE,
};

void lcd_enable(void)
{
	at91_set_gpio_value(AT91_PIN_PA30, 1);  /* power up */
}

void lcd_disable(void)
{
	at91_set_gpio_value(AT91_PIN_PA30, 0);  /* power down */
}

static void at91sam9263ek_lcd_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PC1, 0);	/* LCDHSYNC */
	at91_set_A_periph(AT91_PIN_PC2, 0);	/* LCDDOTCK */
	at91_set_A_periph(AT91_PIN_PC3, 0);	/* LCDDEN */
	at91_set_B_periph(AT91_PIN_PB9, 0);	/* LCDCC */
	at91_set_A_periph(AT91_PIN_PC6, 0);	/* LCDD2 */
	at91_set_A_periph(AT91_PIN_PC7, 0);	/* LCDD3 */
	at91_set_A_periph(AT91_PIN_PC8, 0);	/* LCDD4 */
	at91_set_A_periph(AT91_PIN_PC9, 0);	/* LCDD5 */
	at91_set_A_periph(AT91_PIN_PC10, 0);	/* LCDD6 */
	at91_set_A_periph(AT91_PIN_PC11, 0);	/* LCDD7 */
	at91_set_A_periph(AT91_PIN_PC14, 0);	/* LCDD10 */
	at91_set_A_periph(AT91_PIN_PC15, 0);	/* LCDD11 */
	at91_set_A_periph(AT91_PIN_PC16, 0);	/* LCDD12 */
	at91_set_B_periph(AT91_PIN_PC12, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PC18, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PC19, 0);	/* LCDD15 */
	at91_set_A_periph(AT91_PIN_PC22, 0);	/* LCDD18 */
	at91_set_A_periph(AT91_PIN_PC23, 0);	/* LCDD19 */
	at91_set_A_periph(AT91_PIN_PC24, 0);	/* LCDD20 */
	at91_set_B_periph(AT91_PIN_PC17, 0);	/* LCDD21 */
	at91_set_A_periph(AT91_PIN_PC26, 0);	/* LCDD22 */
	at91_set_A_periph(AT91_PIN_PC27, 0);	/* LCDD23 */

	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_LCDC);

	gd->fb_base = AT91SAM9263_SRAM0_BASE;
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91SAM9263EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9263EK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91sam9263ek_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	at91sam9263ek_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91sam9263ek_spi_hw_init();
#endif
#ifdef CONFIG_MACB
	at91sam9263ek_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91sam9263ek_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sam9263ek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_MACB
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init(gd->bd);
#endif
}
#endif
