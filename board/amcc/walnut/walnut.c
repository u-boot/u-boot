/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <spd_sdram.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*-------------------------------------------------------------------------+
	  | Interrupt controller setup for the Walnut/Sycamore board.
	  | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
	  |       IRQ 16    405GP internally generated; active low; level sensitive
	  |       IRQ 17-24 RESERVED
	  |       IRQ 25 (EXT IRQ 0) FPGA; active high; level sensitive
	  |       IRQ 26 (EXT IRQ 1) SMI; active high; level sensitive
	  |       IRQ 27 (EXT IRQ 2) Not Used
	  |       IRQ 28 (EXT IRQ 3) PCI SLOT 3; active low; level sensitive
	  |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
	  |       IRQ 30 (EXT IRQ 5) PCI SLOT 1; active low; level sensitive
	  |       IRQ 31 (EXT IRQ 6) PCI SLOT 0; active low; level sensitive
	  | Note for Walnut board:
	  |       An interrupt taken for the FPGA (IRQ 25) indicates that either
	  |       the Mouse, Keyboard, IRDA, or External Expansion caused the
	  |       interrupt. The FPGA must be read to determine which device
	  |       caused the interrupt. The default setting of the FPGA clears
	  |
	  +-------------------------------------------------------------------------*/

	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000020);	/* set all but FPGA SMI to be non-critical */
	mtdcr(UIC0PR, 0xFFFFFFE0);	/* set int polarities */
	mtdcr(UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	/* set UART1 control to select CTS/RTS */
#define FPGA_BRDC       0xF0300004
	*(volatile char *)(FPGA_BRDC) |= 0x1;

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));
	uint pvr = get_pvr();

	if (pvr == PVR_405GPR_RB) {
		puts("Board: Sycamore - AMCC PPC405GPr Evaluation Board");
	} else {
		puts("Board: Walnut - AMCC PPC405GP Evaluation Board");
	}

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

/*
 * dram_init() reads EEPROM via I2c. EEPROM contains all of
 * the necessary info for SDRAM controller configuration
 */
int dram_init(void)
{
	gd->ram_size = spd_sdram();

	return 0;
}
