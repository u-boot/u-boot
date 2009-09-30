/*
 * (C) Copyright 2001-2003
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2005-2009
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
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

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <command.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

extern void lxt971_no_sleep(void);

int board_early_init_f (void)
{
	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) SER0 ; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) SER1; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) FPGA 0; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) FPGA 1; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI INTA; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
	 */
	mtdcr(UIC0SR, 0xFFFFFFFF); /* clear all ints */
	mtdcr(UIC0ER, 0x00000000); /* disable all ints */
	mtdcr(UIC0CR, 0x00000000); /* set all to be non-critical*/
	mtdcr(UIC0PR, 0xFFFFFF81); /* set int polarities */
	mtdcr(UIC0TR, 0x10000000); /* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001); /* set vect base=0, INT0 highest priority */
	mtdcr(UIC0SR, 0xFFFFFFFF); /* clear all ints */

	/*
	 * EBC Configuration Register:
	 * set ready timeout to 512 ebc-clks -> ca. 15 us
	 */
	mtebc (EBC0_CFG, 0xa8400000);

	/*
	 * Setup GPIO pins
	 */
	mtdcr(CPC0_CR0, mfdcr(CPC0_CR0) | ((CONFIG_SYS_FPGA_INIT |
					CONFIG_SYS_FPGA_DONE |
					CONFIG_SYS_XEREADY |
					CONFIG_SYS_NONMONARCH |
					CONFIG_SYS_REV1_2) << 5));

	if (!(in_be32((void *)GPIO0_IR) & CONFIG_SYS_REV1_2)) {
		/* rev 1.2 boards */
		mtdcr(CPC0_CR0, mfdcr(CPC0_CR0) | ((CONFIG_SYS_INTA_FAKE |
						CONFIG_SYS_SELF_RST) << 5));
	}

	out_be32((void *)GPIO0_OR, CONFIG_SYS_VPEN);
	/* setup for output */
	out_be32((void *)GPIO0_TCR, CONFIG_SYS_FPGA_PRG | CONFIG_SYS_FPGA_CLK |
		 CONFIG_SYS_FPGA_DATA | CONFIG_SYS_XEREADY | CONFIG_SYS_VPEN);

	/*
	 * - check if rev1_2 is low, then:
	 * - set/reset CONFIG_SYS_INTA_FAKE/CONFIG_SYS_SELF_RST
	 *   in TCR to assert INTA# or SELFRST#
	 */
	return 0;
}

int misc_init_r (void)
{
	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/* deassert EREADY# */
	out_be32((void *)GPIO0_OR,
		 in_be32((void *)GPIO0_OR) | CONFIG_SYS_XEREADY);
	return (0);
}

ushort pmc405_pci_subsys_deviceid(void)
{
	ulong val;

	val = in_be32((void *)GPIO0_IR);
	if (!(val & CONFIG_SYS_REV1_2)) { /* low=rev1.2 */
		/* check monarch# signal */
		if (val & CONFIG_SYS_NONMONARCH)
			return CONFIG_SYS_PCI_SUBSYS_DEVICEID_NONMONARCH;
		return CONFIG_SYS_PCI_SUBSYS_DEVICEID_MONARCH;
	}
	return CONFIG_SYS_PCI_SUBSYS_DEVICEID_NONMONARCH;
}

/*
 * Check Board Identity
 */
int checkboard (void)
{
	ulong val;
	char str[64];
	int i = getenv_r("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1)
		puts ("### No HW ID - assuming PMC405");
	else
		puts(str);

	val = in_be32((void *)GPIO0_IR);
	if (!(val & CONFIG_SYS_REV1_2)) { /* low=rev1.2 */
		puts(" rev1.2 (");
		if (val & CONFIG_SYS_NONMONARCH) /* monarch# signal */
			puts("non-");
		puts("monarch)");
	} else
		puts(" <=rev1.1");

	putc ('\n');

	return 0;
}

void reset_phy(void)
{
#ifdef CONFIG_LXT971_NO_SLEEP

	/*
	 * Disable sleep mode in LXT971
	 */
	lxt971_no_sleep();
#endif
}
