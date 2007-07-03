/*
 * Copyright (C) Procsys. All rights reserved.
 * Author: Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
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
 * with the reference to ata_piix driver in kernel 2.4.32
 */

/*
 * This file contains SATA controller and SATA drive initialization functions
 */

#include <common.h>
#include <pci.h>
#include <command.h>
#include <config.h>
#include <asm/byteorder.h>
#include <ide.h>
#include <ata.h>

#ifdef CFG_ATA_PIIX		/*ata_piix driver */

#define DEBUG_SATA 0		/*For debug prints set DEBUG_SATA to 1 */

#define DRV_DECL		/*For file specific declarations */
#include <sata.h>
#undef DRV_DECL

/*Macros realted to PCI*/
#define PCI_SATA_BUS	0x00
#define PCI_SATA_DEV	0x1f
#define PCI_SATA_FUNC	0x02

#define PCI_SATA_BASE1 0x10
#define PCI_SATA_BASE2 0x14
#define PCI_SATA_BASE3 0x18
#define PCI_SATA_BASE4 0x1c
#define PCI_SATA_BASE5 0x20
#define PCI_PMR         0x90
#define PCI_PI          0x09
#define PCI_PCS         0x92
#define PCI_DMA_CTL     0x48

#define PORT_PRESENT (1<<0)
#define PORT_ENABLED (1<<4)

u32 bdf;
u32 iobase1 = 0;		/*Primary cmd block */
u32 iobase2 = 0;		/*Primary ctl block */
u32 iobase3 = 0;		/*Sec cmd block */
u32 iobase4 = 0;		/*sec ctl block */
u32 iobase5 = 0;		/*BMDMA*/
int
pci_sata_init (void)
{
	u32 bus = PCI_SATA_BUS;
	u32 dev = PCI_SATA_DEV;
	u32 fun = PCI_SATA_FUNC;
	u16 cmd = 0;
	u8 lat = 0, pcibios_max_latency = 0xff;
	u8 pmr;			/*Port mapping reg */
	u8 pi;			/*Prgming Interface reg */

	bdf = PCI_BDF (bus, dev, fun);
	pci_read_config_dword (bdf, PCI_SATA_BASE1, &iobase1);
	pci_read_config_dword (bdf, PCI_SATA_BASE2, &iobase2);
	pci_read_config_dword (bdf, PCI_SATA_BASE3, &iobase3);
	pci_read_config_dword (bdf, PCI_SATA_BASE4, &iobase4);
	pci_read_config_dword (bdf, PCI_SATA_BASE5, &iobase5);

	if ((iobase1 == 0xFFFFFFFF) || (iobase2 == 0xFFFFFFFF) ||
	    (iobase3 == 0xFFFFFFFF) || (iobase4 == 0xFFFFFFFF) ||
	    (iobase5 == 0xFFFFFFFF)) {
		printf ("error no base addr for SATA controller\n");
		return 1;
	 /*ERROR*/}

	iobase1 &= 0xFFFFFFFE;
	iobase2 &= 0xFFFFFFFE;
	iobase3 &= 0xFFFFFFFE;
	iobase4 &= 0xFFFFFFFE;
	iobase5 &= 0xFFFFFFFE;

	/*check for mode */
	pci_read_config_byte (bdf, PCI_PMR, &pmr);
	if (pmr > 1) {
		printf ("combined mode not supported\n");
		return 1;
	}

	pci_read_config_byte (bdf, PCI_PI, &pi);
	if ((pi & 0x05) != 0x05) {
		printf ("Sata is in Legacy mode\n");
		return 1;
	} else {
		printf ("sata is in Native mode\n");
	}

	/*MASTER CFG AND IO CFG */
	pci_read_config_word (bdf, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_IO;
	pci_write_config_word (bdf, PCI_COMMAND, cmd);
	pci_read_config_byte (dev, PCI_LATENCY_TIMER, &lat);

	if (lat < 16)
		lat = (64 <= pcibios_max_latency) ? 64 : pcibios_max_latency;
	else if (lat > pcibios_max_latency)
		lat = pcibios_max_latency;
	pci_write_config_byte (dev, PCI_LATENCY_TIMER, lat);

	return 0;
}

int
sata_bus_probe (int port_no)
{
	int orig_mask, mask;
	u16 pcs;

	mask = (PORT_PRESENT << port_no);
	pci_read_config_word (bdf, PCI_PCS, &pcs);
	orig_mask = (int) pcs & 0xff;
	if ((orig_mask & mask) != mask)
		return 0;
	else
		return 1;
}

int
init_sata (void)
{
	u8 i, rv = 0;

	for (i = 0; i < CFG_SATA_MAXDEVICES; i++) {
		sata_dev_desc[i].type = DEV_TYPE_UNKNOWN;
		sata_dev_desc[i].if_type = IF_TYPE_IDE;
		sata_dev_desc[i].dev = i;
		sata_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		sata_dev_desc[i].blksz = 0;
		sata_dev_desc[i].lba = 0;
		sata_dev_desc[i].block_read = sata_read;
	}

	rv = pci_sata_init ();
	if (rv == 1) {
		printf ("pci initialization failed\n");
		return 1;
	}

	port[0].port_no = 0;
	port[0].ioaddr.cmd_addr = iobase1;
	port[0].ioaddr.altstatus_addr = port[0].ioaddr.ctl_addr =
	    iobase2 | ATA_PCI_CTL_OFS;
	port[0].ioaddr.bmdma_addr = iobase5;

	port[1].port_no = 1;
	port[1].ioaddr.cmd_addr = iobase3;
	port[1].ioaddr.altstatus_addr = port[1].ioaddr.ctl_addr =
	    iobase4 | ATA_PCI_CTL_OFS;
	port[1].ioaddr.bmdma_addr = iobase5 + 0x8;

	for (i = 0; i < CFG_SATA_MAXBUS; i++)
		sata_port (&port[i].ioaddr);

	for (i = 0; i < CFG_SATA_MAXBUS; i++) {
		if (!(sata_bus_probe (i))) {
			port[i].port_state = 0;
			printf ("SATA#%d port is not present \n", i);
		} else {
			printf ("SATA#%d port is present\n", i);
			if (sata_bus_softreset (i)) {
				port[i].port_state = 0;
			} else {
				port[i].port_state = 1;
			}
		}
	}

	for (i = 0; i < CFG_SATA_MAXBUS; i++) {
		u8 j, devno;

		if (port[i].port_state == 0)
			continue;
		for (j = 0; j < CFG_SATA_DEVS_PER_BUS; j++) {
			sata_identify (i, j);
			set_Feature_cmd (i, j);
			devno = i * CFG_SATA_DEVS_PER_BUS + j;
			if ((sata_dev_desc[devno].lba > 0) &&
			    (sata_dev_desc[devno].blksz > 0)) {
				dev_print (&sata_dev_desc[devno]);
				/* initialize partition type */
				init_part (&sata_dev_desc[devno]);
				if (curr_dev < 0)
					curr_dev =
					    i * CFG_SATA_DEVS_PER_BUS + j;
			}
		}
	}
	return 0;
}
#endif
