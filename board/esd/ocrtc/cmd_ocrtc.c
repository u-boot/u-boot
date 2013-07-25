/*
 * (C) Copyright 2003
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/4xx_pci.h>


#if defined(CONFIG_CMD_BSP)

/*
 * Set device number on pci board
 */
int do_setdevice(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int idx = 1;	  /* start at 1 (skip device 0) */
	pci_dev_t bdf = 0;
	u32 addr;

	while (bdf >= 0) {
		if ((bdf = pci_find_device(PCI_VENDOR_ID_IBM, PCI_DEVICE_ID_IBM_405GP, idx++)) < 0) {
			break;
		}
		printf("Found device nr %d at %x!\n", idx-1, bdf);
		pci_read_config_dword(bdf, PCI_BASE_ADDRESS_1, &addr);
		addr &= ~0xf;
		*(u32 *)addr = (bdf & 0x0000f800) >> 11;
		printf("Wrote %x at %x!\n", (bdf & 0x0000f800) >> 11, addr);
	}

	return 0;
}
U_BOOT_CMD(
	setdevice,	1,	1,	do_setdevice,
	"Set device number on pci adapter boards",
	""
);


/*
 * Get device number on pci board
 */
int do_getdevice(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 device;
	char str[32];

	device = *(u32 *)0x0;
	device = 0x16 - device;      /* calculate vxworks bp slot id */
	sprintf(str, "%d", device);
	setenv("slot", str);
	printf("Variabel slot set to %x\n", device);

	return 0;
}
U_BOOT_CMD(
	getdevice,	1,	1,	do_getdevice,
	"Get device number and set slot env variable",
	""
);

#endif
