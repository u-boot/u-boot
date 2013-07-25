/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* ide.c - ide support functions */


#include <common.h>
#if defined(CONFIG_CMD_IDE)
#include <ata.h>
#include <ide.h>
#include <pci.h>

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
