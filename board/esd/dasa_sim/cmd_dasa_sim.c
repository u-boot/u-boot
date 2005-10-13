/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
 *
 */

#include <common.h>
#include <command.h>
#include <pci.h>

#define OK 0
#define ERROR (-1)

#define TRUE 1
#define FALSE 0


extern u_long pci9054_iobase;


/***************************************************************************
 *
 * Routines for PLX PCI9054 eeprom access
 *
 */

static unsigned int PciEepromReadLongVPD (int offs)
{
	unsigned int value;
	unsigned int ret;
	int count;

	pci_write_config_dword (CFG_PCI9054_DEV_FN, 0x4c,
				(offs << 16) | 0x0003);
	count = 0;

	for (;;) {
		udelay (10 * 1000);
		pci_read_config_dword (CFG_PCI9054_DEV_FN, 0x4c, &ret);
		if ((ret & 0x80000000) != 0) {
			break;
		} else {
			count++;
			if (count > 10) {
				printf ("\nTimeout: ret=%08x - Please try again!\n", ret);
				break;
			}
		}
	}

	pci_read_config_dword (CFG_PCI9054_DEV_FN, 0x50, &value);

	return value;
}


static int PciEepromWriteLongVPD (int offs, unsigned int value)
{
	unsigned int ret;
	int count;

	pci_write_config_dword (CFG_PCI9054_DEV_FN, 0x50, value);
	pci_write_config_dword (CFG_PCI9054_DEV_FN, 0x4c,
				(offs << 16) | 0x80000003);
	count = 0;

	for (;;) {
		udelay (10 * 1000);
		pci_read_config_dword (CFG_PCI9054_DEV_FN, 0x4c, &ret);
		if ((ret & 0x80000000) == 0) {
			break;
		} else {
			count++;
			if (count > 10) {
				printf ("\nTimeout: ret=%08x - Please try again!\n", ret);
				break;
			}
		}
	}

	return TRUE;
}


static void showPci9054 (void)
{
	int val;
	int l, i;

	/* read 9054-values */
	for (l = 0; l < 6; l++) {
		printf ("%02x: ", l * 0x10);
		for (i = 0; i < 4; i++) {
			pci_read_config_dword (CFG_PCI9054_DEV_FN,
						l * 16 + i * 4,
						(unsigned int *)&val);
			printf ("%08x ", val);
		}
		printf ("\n");
	}
	printf ("\n");

	for (l = 0; l < 7; l++) {
		printf ("%02x: ", l * 0x10);
		for (i = 0; i < 4; i++)
			printf ("%08x ",
				PciEepromReadLongVPD ((i + l * 4) * 4));
		printf ("\n");
	}
	printf ("\n");
}


static void updatePci9054 (void)
{
	int val;

	/*
	 * Set EEPROM write-protect register to 0
	 */
	out32 (pci9054_iobase + 0x0c,
		   in32 (pci9054_iobase + 0x0c) & 0xffff00ff);

	/* Long Serial EEPROM Load Registers... */
	val = PciEepromWriteLongVPD (0x00, 0x905410b5);
	val = PciEepromWriteLongVPD (0x04, 0x09800001);	/* other input controller */
	val = PciEepromWriteLongVPD (0x08, 0x28140100);

	val = PciEepromWriteLongVPD (0x0c, 0x00000000);	/* MBOX0... */
	val = PciEepromWriteLongVPD (0x10, 0x00000000);

	/* las0: fpga access (0x0000.0000 ... 0x0003.ffff) */
	val = PciEepromWriteLongVPD (0x14, 0xfffc0000);	/* LAS0RR... */
	val = PciEepromWriteLongVPD (0x18, 0x00000001);	/* LAS0BA */

	val = PciEepromWriteLongVPD (0x1c, 0x00200000);	/* MARBR... */
	val = PciEepromWriteLongVPD (0x20, 0x00300500);	/* LMISC/BIGEND */

	val = PciEepromWriteLongVPD (0x24, 0x00000000);	/* EROMRR... */
	val = PciEepromWriteLongVPD (0x28, 0x00000000);	/* EROMBA */

	val = PciEepromWriteLongVPD (0x2c, 0x43030000);	/* LBRD0... */

	val = PciEepromWriteLongVPD (0x30, 0x00000000);	/* DMRR... */
	val = PciEepromWriteLongVPD (0x34, 0x00000000);
	val = PciEepromWriteLongVPD (0x38, 0x00000000);

	val = PciEepromWriteLongVPD (0x3c, 0x00000000);	/* DMPBAM... */
	val = PciEepromWriteLongVPD (0x40, 0x00000000);

	/* Extra Long Serial EEPROM Load Registers... */
	val = PciEepromWriteLongVPD (0x44, 0x010212fe);	/* PCISID... */

	/* las1: 505-sram access (0x0004.0000 ... 0x001f.ffff) */
	/* Offset to LAS1: Group 1: 0x00040000                 */
	/*                 Group 2: 0x00080000                 */
	/*                 Group 3: 0x000c0000                 */
	val = PciEepromWriteLongVPD (0x48, 0xffe00000);	/* LAS1RR */
	val = PciEepromWriteLongVPD (0x4c, 0x00040001);	/* LAS1BA */
	val = PciEepromWriteLongVPD (0x50, 0x00000208);	/* LBRD1 */ /* so wars bisher */

	val = PciEepromWriteLongVPD (0x54, 0x00004c06);	/* HotSwap... */

	printf ("Finished writing defaults into PLX PCI9054 EEPROM!\n");
}


static void clearPci9054 (void)
{
	int val;

	/*
	 * Set EEPROM write-protect register to 0
	 */
	out32 (pci9054_iobase + 0x0c,
		in32 (pci9054_iobase + 0x0c) & 0xffff00ff);

	/* Long Serial EEPROM Load Registers... */
	val = PciEepromWriteLongVPD (0x00, 0xffffffff);
	val = PciEepromWriteLongVPD (0x04, 0xffffffff);	/* other input controller */

	printf ("Finished clearing PLX PCI9054 EEPROM!\n");
}


/* ------------------------------------------------------------------------- */
int do_pci9054 (cmd_tbl_t * cmdtp, int flag, int argc,
				char *argv[])
{
	if (strcmp (argv[1], "info") == 0) {
		showPci9054 ();
		return 0;
	}

	if (strcmp (argv[1], "update") == 0) {
		updatePci9054 ();
		return 0;
	}

	if (strcmp (argv[1], "clear") == 0) {
		clearPci9054 ();
		return 0;
	}

	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;

}

U_BOOT_CMD(
	pci9054, 3, 1, do_pci9054,
	"pci9054 - PLX PCI9054 EEPROM access\n",
	"pci9054 info - print EEPROM values\n"
	"pci9054 update - updates EEPROM with default values\n"
);

/* ------------------------------------------------------------------------- */
