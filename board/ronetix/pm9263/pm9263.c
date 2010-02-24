/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * Copyright (C) 2008 Ronetix Ilko Iliev (www.ronetix.at)
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
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
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <asm/arch/hardware.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#include <dataflash.h>
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
static void pm9263_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBI0CSA);
	at91_sys_write(AT91_MATRIX_EBI0CSA,
		       csa | AT91_MATRIX_EBI0_CS3A_SMC_SMARTMEDIA);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		       AT91_SMC_NWESETUP_(1) | AT91_SMC_NCS_WRSETUP_(1) |
		       AT91_SMC_NRDSETUP_(1) | AT91_SMC_NCS_RDSETUP_(1));
	at91_sys_write(AT91_SMC_PULSE(3),
		       AT91_SMC_NWEPULSE_(3) | AT91_SMC_NCS_WRPULSE_(3) |
		       AT91_SMC_NRDPULSE_(3) | AT91_SMC_NCS_RDPULSE_(3));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(5) | AT91_SMC_NRDCYCLE_(5));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		       AT91_SMC_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		       AT91_SMC_DBW_8 |
#endif
		       AT91_SMC_TDF_(2));

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void pm9263_macb_hw_init(void)
{
	/*
	 * PB27 enables the 50MHz oscillator for Ethernet PHY
	 * 1 - enable
	 * 0 - disable
	 */
	at91_set_gpio_output(AT91_PIN_PB27, 1);
	at91_set_gpio_value(AT91_PIN_PB27, 1); /* 1- enable, 0 - disable */

	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *	RXDV (PC25) => PHY normal mode (not Test mode)
	 *	ERX0 (PE25) => PHY ADDR0
	 *	ERX1 (PE26) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PC25),
	       pin_to_controller(AT91_PIN_PC0) + PIO_PUDR);
	writel(pin_to_mask(AT91_PIN_PE25) |
	       pin_to_mask(AT91_PIN_PE26),
	       pin_to_controller(AT91_PIN_PE0) + PIO_PUDR);


	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PC25),
	       pin_to_controller(AT91_PIN_PC0) + PIO_PUER);
	writel(pin_to_mask(AT91_PIN_PE25) |
	       pin_to_mask(AT91_PIN_PE26),
	       pin_to_controller(AT91_PIN_PE0) + PIO_PUER);

	at91_macb_hw_init();
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
	at91_set_gpio_value(AT91_PIN_PA22, 1); /* power up */
}

void lcd_disable(void)
{
	at91_set_gpio_value(AT91_PIN_PA22, 0); /* power down */
}

#ifdef CONFIG_LCD_IN_PSRAM

#define PSRAM_CRE_PIN	AT91_PIN_PB29
#define PSRAM_CTRL_REG	(PHYS_PSRAM + PHYS_PSRAM_SIZE - 2)

/* Initialize the PSRAM memory */
static int pm9263_lcd_hw_psram_init(void)
{
	volatile uint16_t x;
	unsigned long csa;

	/* Enable CS3  3.3v, no pull-ups */
	csa = at91_sys_read(AT91_MATRIX_EBI1CSA);
	at91_sys_write(AT91_MATRIX_EBI1CSA,
		       csa | AT91_MATRIX_EBI1_DBPUC |
		       AT91_MATRIX_EBI1_VDDIOMSEL_3_3V);

	/* Configure SMC1 CS0 for PSRAM - 16-bit */
	at91_sys_write(AT91_SMC1_SETUP(0),
		       AT91_SMC_NWESETUP_(0) | AT91_SMC_NCS_WRSETUP_(0) |
		       AT91_SMC_NRDSETUP_(0) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC1_PULSE(0),
		       AT91_SMC_NWEPULSE_(7) | AT91_SMC_NCS_WRPULSE_(7) |
		       AT91_SMC_NRDPULSE_(2) | AT91_SMC_NCS_RDPULSE_(7));
	at91_sys_write(AT91_SMC1_CYCLE(0),
		       AT91_SMC_NWECYCLE_(8) | AT91_SMC_NRDCYCLE_(8));
	at91_sys_write(AT91_SMC1_MODE(0),
		       AT91_SMC_DBW_16 |
		       AT91_SMC_PMEN |
		       AT91_SMC_PS_32);

	/* setup PB29 as output */
	at91_set_gpio_output(PSRAM_CRE_PIN, 1);

	at91_set_gpio_value(PSRAM_CRE_PIN, 0);	/* set PSRAM_CRE_PIN to '0' */

	/* PSRAM: write BCR */
	x = readw(PSRAM_CTRL_REG);
	x = readw(PSRAM_CTRL_REG);
	writew(1, PSRAM_CTRL_REG);	/* 0 - RCR,1 - BCR */
	writew(0x9d4f, PSRAM_CTRL_REG);	/* write the BCR */

	/* write RCR of the PSRAM */
	x = readw(PSRAM_CTRL_REG);
	x = readw(PSRAM_CTRL_REG);
	writew(0, PSRAM_CTRL_REG);	/* 0 - RCR,1 - BCR */
	/* set RCR; 0x10-async mode,0x90-page mode */
	writew(0x90, PSRAM_CTRL_REG);

	/*
	 * test to see if the PSRAM is MT45W2M16A or MT45W2M16B
	 * MT45W2M16B - CRE must be 0
	 * MT45W2M16A - CRE must be 1
	 */
	writew(0x1234, PHYS_PSRAM);
	writew(0x5678, PHYS_PSRAM + 2);

	/* test if the chip is MT45W2M16B */
	if ((readw(PHYS_PSRAM) != 0x1234) || (readw(PHYS_PSRAM+2) != 0x5678)) {
		/* try with CRE=1 (MT45W2M16A) */
		at91_set_gpio_value(PSRAM_CRE_PIN, 1); /* set PSRAM_CRE_PIN to '1' */

		/* write RCR of the PSRAM */
		x = readw(PSRAM_CTRL_REG);
		x = readw(PSRAM_CTRL_REG);
		writew(0, PSRAM_CTRL_REG);	/* 0 - RCR,1 - BCR */
		/* set RCR;0x10-async mode,0x90-page mode */
		writew(0x90, PSRAM_CTRL_REG);


		writew(0x1234, PHYS_PSRAM);
		writew(0x5678, PHYS_PSRAM+2);
		if ((readw(PHYS_PSRAM) != 0x1234)
		   || (readw(PHYS_PSRAM + 2) != 0x5678))
			return 1;

	}

	/* Bus matrix */
	at91_sys_write( AT91_MATRIX_PRAS5, AT91_MATRIX_M5PR );
	at91_sys_write( AT91_MATRIX_SCFG5, AT91_MATRIX_ARBT_FIXED_PRIORITY |
				(AT91_MATRIX_FIXED_DEFMSTR & (5 << 18)) |
				AT91_MATRIX_DEFMSTR_TYPE_FIXED |
				(AT91_MATRIX_SLOT_CYCLE & (0xFF << 0)));

	return 0;
}
#endif

static void pm9263_lcd_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PC0, 0);	/* LCDVSYNC */
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

	/* Power Control */
	at91_set_gpio_output(AT91_PIN_PA22, 1);
	at91_set_gpio_value(AT91_PIN_PA22, 0);	/* power down */

#ifdef CONFIG_LCD_IN_PSRAM
	/* initialize te PSRAM */
	int stat = pm9263_lcd_hw_psram_init();

	gd->fb_base = (stat == 0) ? PHYS_PSRAM : AT91SAM9263_SRAM0_BASE;
#else
	gd->fb_base = AT91SAM9263_SRAM0_BASE;
#endif

}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

extern flash_info_t flash_info[];

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size, flash_size, dataflash_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2009 Ronetix GmbH\n");
	lcd_printf ("support@ronetix.at\n");
	lcd_printf ("%s CPU at %s MHz",
		CONFIG_SYS_AT91_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;

	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;

	flash_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++)
		flash_size += flash_info[i].size;

	dataflash_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_DATAFLASH_BANKS; i++)
		dataflash_size += (unsigned int) dataflash_info[i].Device.pages_number *
				dataflash_info[i].Device.pages_size;

	lcd_printf ("%ld MB SDRAM, %ld MB NAND\n%ld MB NOR Flash\n"
			"4 MB PSRAM, %ld MB DataFlash\n",
		dram_size >> 20,
		nand_size >> 20,
		flash_size >> 20,
		dataflash_size >> 20);
}
#endif /* CONFIG_LCD_INFO */

#endif /* CONFIG_LCD */

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	at91_sys_write(AT91_PMC_PCER,
					(1 << AT91SAM9263_ID_PIOA) |
					(1 << AT91SAM9263_ID_PIOCDE) |
					(1 << AT91SAM9263_ID_PIOB));

	/* arch number of AT91SAM9263EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_PM9263;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	pm9263_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	pm9263_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	pm9263_lcd_hw_init();
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

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)AT91SAM9263_BASE_EMAC, 0x01);
#endif
	return rc;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard (void)
{
	char *ss;

	printf ("Board : Ronetix PM9263\n");

	switch (gd->fb_base) {
	case PHYS_PSRAM:
		ss = "(PSRAM)";
		break;

	case AT91SAM9263_SRAM0_BASE:
		ss = "(Internal SRAM)";
		break;

	default:
		ss = "";
		break;
	}
	printf("Video memory : 0x%08lX %s\n", gd->fb_base, ss );

	printf ("\n");
	return 0;
}
#endif
