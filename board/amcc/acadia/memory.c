/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/gpio.h>

extern void board_pll_init_f(void);

#if !defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
static void cram_bcr_write(u32 wr_val)
{
	wr_val <<= 2;

	/* set CRAM_CRE to 1 */
	gpio_write_bit(CONFIG_SYS_GPIO_CRAM_CRE, 1);

	/* Write BCR to CRAM on CS1 */
	out32(wr_val + 0x00200000, 0);
	debug("CRAM VAL: %08x for CS1 ", wr_val + 0x00200000);

	/* Write BCR to CRAM on CS2 */
	out32(wr_val + 0x02200000, 0);
	debug("CRAM VAL: %08x for CS2\n", wr_val + 0x02200000);

	sync();
	eieio();

	/* set CRAM_CRE back to 0 (normal operation) */
	gpio_write_bit(CONFIG_SYS_GPIO_CRAM_CRE, 0);

	return;
}
#endif

phys_size_t initdram(int board_type)
{
#if defined(CONFIG_NAND_SPL)
	u32 reg;

	/* don't reinit PLL when booting via I2C bootstrap option */
	mfsdr(SDR0_PINSTP, reg);
	if (reg != 0xf0000000)
		board_pll_init_f();
#endif

#if !defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	int i;
	u32 val;

	/* 1. EBC need to program READY, CLK, ADV for ASync mode */
	gpio_config(CONFIG_SYS_GPIO_CRAM_CLK, GPIO_OUT, GPIO_SEL, GPIO_OUT_0);
	gpio_config(CONFIG_SYS_GPIO_CRAM_ADV, GPIO_OUT, GPIO_SEL, GPIO_OUT_0);
	gpio_config(CONFIG_SYS_GPIO_CRAM_CRE, GPIO_OUT, GPIO_SEL, GPIO_OUT_0);
	gpio_config(CONFIG_SYS_GPIO_CRAM_WAIT, GPIO_IN, GPIO_SEL, GPIO_OUT_NO_CHG);

	/* 2. EBC in Async mode */
	mtebc(PB1AP, 0x078F1EC0);
	mtebc(PB2AP, 0x078F1EC0);
	mtebc(PB1CR, 0x000BC000);
	mtebc(PB2CR, 0x020BC000);

	/* 3. Set CRAM in Sync mode */
	cram_bcr_write(0x7012);		/* CRAM burst setting */

	/* 4. EBC in Sync mode */
	mtebc(PB1AP, 0x9C0201C0);
	mtebc(PB2AP, 0x9C0201C0);

	/* Set GPIO pins back to alternate function */
	gpio_config(CONFIG_SYS_GPIO_CRAM_CLK, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG);
	gpio_config(CONFIG_SYS_GPIO_CRAM_ADV, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG);

	/* Config EBC to use RDY */
	mfsdr(SDR0_ULTRA0, val);
	mtsdr(SDR0_ULTRA0, val | SDR_ULTRA0_EBCRDYEN);

	/* Wait a short while, since for NAND booting this is too fast */
	for (i=0; i<200000; i++)
		;
#endif

	return (CONFIG_SYS_MBYTES_RAM << 20);
}
