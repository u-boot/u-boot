/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
/* ide.c - ide support functions */


#include <common.h>
#if defined(CONFIG_CMD_IDE)
#include <ata.h>
#include <ide.h>
#include <pci.h>

extern ulong ide_bus_offset[CONFIG_SYS_IDE_MAXBUS];
int cpci_hd_type;

int ata_device(int dev)
{
	int retval;

	retval = (dev & 1) << 4;
	if (cpci_hd_type == 2)
		retval ^= 1 << 4;
	return retval;
}


int ide_preinit (void)
{
	int status;
	pci_dev_t devbusfn;
	int l;

	status = 1;
	cpci_hd_type = 0;
	if (CPCI750_SLAVE_TEST != 0)
		return status;
	for (l = 0; l < CONFIG_SYS_IDE_MAXBUS; l++) {
		ide_bus_offset[l] = -ATA_STATUS;
	}
	devbusfn = pci_find_device (0x1103, 0x0004, 0);
	if (devbusfn != -1) {
		cpci_hd_type = 1;
	} else {
	        devbusfn = pci_find_device (0x1095, 0x3114, 0);
		if (devbusfn != -1) {
			cpci_hd_type = 2;
		}
	}
	if (devbusfn != -1) {
		ulong *ide_bus_offset_ptr;

		status = 0;

		ide_bus_offset_ptr = &ide_bus_offset[0];
		pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0,
				       (u32 *)ide_bus_offset_ptr);
		ide_bus_offset[0] &= 0xfffffffe;
		ide_bus_offset[0] += CONFIG_SYS_PCI0_IO_SPACE;
		ide_bus_offset_ptr = &ide_bus_offset[1];
		pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_2,
				       (u32 *)ide_bus_offset_ptr);
		ide_bus_offset[1] &= 0xfffffffe;
		ide_bus_offset[1] += CONFIG_SYS_PCI0_IO_SPACE;
	}
	return status;
}

void ide_set_reset (int flag) {
	return;
}

#endif /* of CONFIG_CMDS_IDE */
