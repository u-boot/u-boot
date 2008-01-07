/*
 * (C) Copyright 2007
 * Gary Jennejohn, DENX Software Engineering, garyj@denx.de.
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
/* sil680.c - ide support functions for the Sil0680A controller */

/*
 * The following parameters must be defined in the configuration file
 * of the target board:
 *
 * #define CFG_IDE_SIL680
 *
 * #define CONFIG_PCI_PNP
 * NOTE it may also be necessary to define this if the default of 8 is
 * incorrect for the target board (e.g. the sequoia board requires 0).
 * #define CFG_PCI_CACHE_LINE_SIZE	0
 *
 * #define CONFIG_CMD_IDE
 * #undef  CONFIG_IDE_8xx_DIRECT
 * #undef  CONFIG_IDE_LED
 * #undef  CONFIG_IDE_RESET
 * #define CONFIG_IDE_PREINIT
 * #define CFG_IDE_MAXBUS		2 - modify to suit
 * #define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*2) - modify to suit
 * #define CFG_ATA_BASE_ADDR	0
 * #define CFG_ATA_IDE0_OFFSET	0
 * #define CFG_ATA_IDE1_OFFSET	0
 * #define CFG_ATA_DATA_OFFSET	0
 * #define CFG_ATA_REG_OFFSET	0
 * #define CFG_ATA_ALT_OFFSET	0x0004
 *
 * The mapping for PCI IO-space.
 * NOTE this is the value for the sequoia board. Modify to suit.
 * #define CFG_PCI0_IO_SPACE   0xE8000000
 */

#include <common.h>
#if defined(CFG_IDE_SIL680)
#include <ata.h>
#include <ide.h>
#include <pci.h>

extern ulong ide_bus_offset[CFG_IDE_MAXBUS];

int ide_preinit (void)
{
	int status;
	pci_dev_t devbusfn;
	int l;

	status = 1;
	for (l = 0; l < CFG_IDE_MAXBUS; l++) {
		ide_bus_offset[l] = -ATA_STATUS;
	}
	devbusfn = pci_find_device (0x1095, 0x0680, 0);
	if (devbusfn != -1) {
		status = 0;

		pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0,
				       (u32 *) &ide_bus_offset[0]);
		ide_bus_offset[0] &= 0xfffffff8;
		ide_bus_offset[0] += CFG_PCI0_IO_SPACE;
		pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_2,
				       (u32 *) &ide_bus_offset[1]);
		ide_bus_offset[1] &= 0xfffffff8;
		ide_bus_offset[1] += CFG_PCI0_IO_SPACE;
		/* init various things - taken from the Linux driver */
		/* set PIO mode */
		pci_write_config_byte(devbusfn, 0x80, 0x00);
		pci_write_config_byte(devbusfn, 0x84, 0x00);
		/* IDE0 */
		pci_write_config_byte(devbusfn,  0xA1, 0x02);
		pci_write_config_word(devbusfn,  0xA2, 0x328A);
		pci_write_config_dword(devbusfn, 0xA4, 0x62DD62DD);
		pci_write_config_dword(devbusfn, 0xA8, 0x43924392);
		pci_write_config_dword(devbusfn, 0xAC, 0x40094009);
		/* IDE1 */
		pci_write_config_byte(devbusfn,  0xB1, 0x02);
		pci_write_config_word(devbusfn,  0xB2, 0x328A);
		pci_write_config_dword(devbusfn, 0xB4, 0x62DD62DD);
		pci_write_config_dword(devbusfn, 0xB8, 0x43924392);
		pci_write_config_dword(devbusfn, 0xBC, 0x40094009);
	}
	return (status);
}

void ide_set_reset (int flag) {
	return;
}

#endif /* CFG_IDE_SIL680 */
