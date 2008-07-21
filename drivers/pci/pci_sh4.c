/*
 * SH4 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 *
 * u-boot/cpu/sh4/pci-sh4.c
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
#include <asm/pci.h>
#include <pci.h>

int pci_sh4_init(struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->region_count = 0;
	hose->last_busno = 0xff;

	/* PCI memory space */
	pci_set_region(hose->regions + 0,
		CONFIG_PCI_MEM_BUS,
		CONFIG_PCI_MEM_PHYS,
		CONFIG_PCI_MEM_SIZE,
		PCI_REGION_MEM);
	hose->region_count++;

	/* PCI IO space */
	pci_set_region(hose->regions + 1,
		CONFIG_PCI_IO_BUS,
		CONFIG_PCI_IO_PHYS,
		CONFIG_PCI_IO_SIZE,
		PCI_REGION_IO);
	hose->region_count++;

	udelay(1000);

	pci_set_ops(hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    pci_sh4_read_config_dword,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    pci_sh4_write_config_dword);

	pci_register_hose(hose);

	udelay(1000);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
	hose->last_busno = pci_hose_scan(hose);
	return 0;
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 0;
}

#ifdef CONFIG_PCI_SCAN_SHOW
int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	return 1;
}
#endif /* CONFIG_PCI_SCAN_SHOW */
