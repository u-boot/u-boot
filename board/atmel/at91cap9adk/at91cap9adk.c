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
#include <asm/arch/at91cap9.h>
#include <asm/arch/at91cap9_matrix.h>
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

#define MP_BLOCK_3_BASE	0xFDF00000

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

static void at91cap9_serial_hw_init(void)
{
#ifdef CONFIG_USART0
	at91_set_A_periph(AT91_PIN_PA22, 1);		/* TXD0 */
	at91_set_A_periph(AT91_PIN_PA23, 0);		/* RXD0 */
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

static void at91cap9_slowclock_hw_init(void)
{
	/*
	 * On AT91CAP9 revC CPUs, the slow clock can be based on an
	 * internal impreciseRC oscillator or an external 32kHz oscillator.
	 * Switch to the latter.
	 */
#define ARCH_ID_AT91CAP9_REVB	0x399
#define ARCH_ID_AT91CAP9_REVC	0x601
	if (at91_sys_read(AT91_PMC_VER) == ARCH_ID_AT91CAP9_REVC) {
		unsigned i, tmp = at91_sys_read(AT91_SCKCR);
		if ((tmp & AT91CAP9_SCKCR_OSCSEL) == AT91CAP9_SCKCR_OSCSEL_RC) {
			extern void timer_init(void);
			timer_init();
			tmp |= AT91CAP9_SCKCR_OSC32EN;
			at91_sys_write(AT91_SCKCR, tmp);
			for (i = 0; i < 1200; i++)
				udelay(1000);
			tmp |= AT91CAP9_SCKCR_OSCSEL_32;
			at91_sys_write(AT91_SCKCR, tmp);
			udelay(200);
			tmp &= ~AT91CAP9_SCKCR_RCEN;
			at91_sys_write(AT91_SCKCR, tmp);
		}
	}
}

static void at91cap9_nor_hw_init(void)
{
	unsigned long csa;

	/* Ensure EBI supply is 3.3V */
	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA,
		       csa | AT91_MATRIX_EBI_VDDIOMSEL_3_3V);
	/* Configure SMC CS0 for parallel flash */
	at91_sys_write(AT91_SMC_SETUP(0),
		       AT91_SMC_NWESETUP_(4) | AT91_SMC_NCS_WRSETUP_(2) |
		       AT91_SMC_NRDSETUP_(4) | AT91_SMC_NCS_RDSETUP_(2));
	at91_sys_write(AT91_SMC_PULSE(0),
		       AT91_SMC_NWEPULSE_(8) | AT91_SMC_NCS_WRPULSE_(10) |
		       AT91_SMC_NRDPULSE_(8) | AT91_SMC_NCS_RDPULSE_(10));
	at91_sys_write(AT91_SMC_CYCLE(0),
		       AT91_SMC_NWECYCLE_(16) | AT91_SMC_NRDCYCLE_(16));
	at91_sys_write(AT91_SMC_MODE(0),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_BAT_WRITE |
		       AT91_SMC_DBW_16 | AT91_SMC_TDF_(1));
}

#ifdef CONFIG_CMD_NAND
static void at91cap9_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA,
		       csa | AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA |
		       AT91_MATRIX_EBI_VDDIOMSEL_3_3V);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		       AT91_SMC_NWESETUP_(2) | AT91_SMC_NCS_WRSETUP_(1) |
		       AT91_SMC_NRDSETUP_(2) | AT91_SMC_NCS_RDSETUP_(1));
	at91_sys_write(AT91_SMC_PULSE(3),
		       AT91_SMC_NWEPULSE_(4) | AT91_SMC_NCS_WRPULSE_(6) |
		       AT91_SMC_NRDPULSE_(4) | AT91_SMC_NCS_RDPULSE_(6));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(8) | AT91_SMC_NRDCYCLE_(8));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
#ifdef CFG_NAND_DBW_16
		       AT91_SMC_DBW_16 |
#else /* CFG_NAND_DBW_8 */
		       AT91_SMC_DBW_8 |
#endif
		       AT91_SMC_TDF_(1));

	at91_sys_write(AT91_PMC_PCER, 1 << AT91CAP9_ID_PIOABCD);

	/* RDY/BSY is not connected */

	/* Enable NandFlash */
	at91_set_gpio_output(AT91_PIN_PD15, 1);
}
#endif

#ifdef CONFIG_HAS_DATAFLASH
static void at91cap9_spi_hw_init(void)
{
	at91_set_B_periph(AT91_PIN_PA5, 0);	/* SPI0_NPCS0 */

	at91_set_B_periph(AT91_PIN_PA0, 0);	/* SPI0_MISO */
	at91_set_B_periph(AT91_PIN_PA1, 0);	/* SPI0_MOSI */
	at91_set_B_periph(AT91_PIN_PA2, 0);	/* SPI0_SPCK */

	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91CAP9_ID_SPI0);
}
#endif

#ifdef CONFIG_MACB
static void at91cap9_macb_hw_init(void)
{
	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91CAP9_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *	RXDV (PB22) => PHY normal mode (not Test mode)
	 *	ERX0 (PB25) => PHY ADDR0
	 *	ERX1 (PB26) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PB22) |
	       pin_to_mask(AT91_PIN_PB25) |
	       pin_to_mask(AT91_PIN_PB26),
	       pin_to_controller(AT91_PIN_PA0) + PIO_PUDR);

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
	writel(pin_to_mask(AT91_PIN_PB22) |
	       pin_to_mask(AT91_PIN_PB25) |
	       pin_to_mask(AT91_PIN_PB26),
	       pin_to_controller(AT91_PIN_PA0) + PIO_PUER);

	at91_set_A_periph(AT91_PIN_PB21, 0);	/* ETXCK_EREFCK */
	at91_set_A_periph(AT91_PIN_PB22, 0);	/* ERXDV */
	at91_set_A_periph(AT91_PIN_PB25, 0);	/* ERX0 */
	at91_set_A_periph(AT91_PIN_PB26, 0);	/* ERX1 */
	at91_set_A_periph(AT91_PIN_PB27, 0);	/* ERXER */
	at91_set_A_periph(AT91_PIN_PB28, 0);	/* ETXEN */
	at91_set_A_periph(AT91_PIN_PB23, 0);	/* ETX0 */
	at91_set_A_periph(AT91_PIN_PB24, 0);	/* ETX1 */
	at91_set_A_periph(AT91_PIN_PB30, 0);	/* EMDIO */
	at91_set_A_periph(AT91_PIN_PB29, 0);	/* EMDC */

#ifndef CONFIG_RMII
	at91_set_B_periph(AT91_PIN_PC25, 0);	/* ECRS */
	at91_set_B_periph(AT91_PIN_PC26, 0);	/* ECOL */
	at91_set_B_periph(AT91_PIN_PC22, 0);	/* ERX2 */
	at91_set_B_periph(AT91_PIN_PC23, 0);	/* ERX3 */
	at91_set_B_periph(AT91_PIN_PC27, 0);	/* ERXCK */
	at91_set_B_periph(AT91_PIN_PC20, 0);	/* ETX2 */
	at91_set_B_periph(AT91_PIN_PC21, 0);	/* ETX3 */
	at91_set_B_periph(AT91_PIN_PC24, 0);	/* ETXER */
#endif
	/* Unlock EMAC, 3 0 2 1 sequence */
#define MP_MAC_KEY0	0x5969cb2a
#define MP_MAC_KEY1	0xb4a1872e
#define MP_MAC_KEY2	0x05683fbc
#define MP_MAC_KEY3	0x3634fba4
#define UNLOCK_MAC	0x00000008
	writel(MP_MAC_KEY3, MP_BLOCK_3_BASE + 0x3c);
	writel(MP_MAC_KEY0, MP_BLOCK_3_BASE + 0x30);
	writel(MP_MAC_KEY2, MP_BLOCK_3_BASE + 0x38);
	writel(MP_MAC_KEY1, MP_BLOCK_3_BASE + 0x34);
	writel(UNLOCK_MAC, MP_BLOCK_3_BASE + 0x40);
}
#endif

#ifdef CONFIG_USB_OHCI_NEW
static void at91cap9_uhp_hw_init(void)
{
	/* Unlock USB OHCI, 3 2 0 1 sequence */
#define MP_OHCI_KEY0	0x896c11ca
#define MP_OHCI_KEY1	0x68ebca21
#define MP_OHCI_KEY2	0x4823efbc
#define MP_OHCI_KEY3	0x8651aae4
#define UNLOCK_OHCI	0x00000010
	writel(MP_OHCI_KEY3, MP_BLOCK_3_BASE + 0x3c);
	writel(MP_OHCI_KEY2, MP_BLOCK_3_BASE + 0x38);
	writel(MP_OHCI_KEY0, MP_BLOCK_3_BASE + 0x30);
	writel(MP_OHCI_KEY1, MP_BLOCK_3_BASE + 0x34);
	writel(UNLOCK_OHCI, MP_BLOCK_3_BASE + 0x40);
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
	mmio:		AT91CAP9_LCDC_BASE,
};

void lcd_enable(void)
{
	at91_set_gpio_value(AT91_PIN_PC0, 0);  /* power up */
}

void lcd_disable(void)
{
	at91_set_gpio_value(AT91_PIN_PC0, 1);  /* power down */
}

static void at91cap9_lcd_hw_init(void)
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
	at91_set_A_periph(AT91_PIN_PC17, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PC18, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PC19, 0);	/* LCDD15 */
	at91_set_A_periph(AT91_PIN_PC22, 0);	/* LCDD18 */
	at91_set_A_periph(AT91_PIN_PC23, 0);	/* LCDD19 */
	at91_set_A_periph(AT91_PIN_PC24, 0);	/* LCDD20 */
	at91_set_A_periph(AT91_PIN_PC25, 0);	/* LCDD21 */
	at91_set_A_periph(AT91_PIN_PC26, 0);	/* LCDD22 */
	at91_set_A_periph(AT91_PIN_PC27, 0);	/* LCDD23 */

	at91_sys_write(AT91_PMC_PCER, 1 << AT91CAP9_ID_LCDC);

	gd->fb_base = 0;
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91CAP9ADK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91CAP9ADK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91cap9_serial_hw_init();
	at91cap9_slowclock_hw_init();
	at91cap9_nor_hw_init();
#ifdef CONFIG_CMD_NAND
	at91cap9_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91cap9_spi_hw_init();
#endif
#ifdef CONFIG_MACB
	at91cap9_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91cap9_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	at91cap9_lcd_hw_init();
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
