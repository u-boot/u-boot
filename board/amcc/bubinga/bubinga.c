/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

long int spd_sdram(void);

int board_early_init_f(void)
{
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000010);
	mtdcr(UIC0PR, 0xFFFF7FF0);	/* set int polarities */
	mtdcr(UIC0TR, 0x00000010);	/* set int trigger levels */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	/*
	 * Configure CPC0_PCI to enable PerWE as output
	 * and enable the internal PCI arbiter if selected
	 */
	if (in_8((void *)FPGA_REG1) & FPGA_REG1_PCI_INT_ARB)
		mtdcr(CPC0_PCI, CPC0_PCI_HOST_CFG_EN | CPC0_PCI_ARBIT_EN);
	else
		mtdcr(CPC0_PCI, CPC0_PCI_HOST_CFG_EN);

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	puts("Board: Bubinga - AMCC PPC405EP Evaluation Board");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

/* -------------------------------------------------------------------------
  dram_init() reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
   ------------------------------------------------------------------------- */
int dram_init(void)
{
	gd->ram_size = spd_sdram();

	return 0;
}
