/*
 * (C) Copyright 2002-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>
#include <asm/4xx_pci.h>
#include <asm/processor.h>

#include "pci405.h"

#if defined(CONFIG_CMD_BSP)

/*
 * Command loadpci: wait for signal from host and boot image.
 */
int do_loadpci(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int *ptr = 0;
	int count = 0;
	int count2 = 0;
	int i;
	char addr[16];
	char str[] = "\\|/-";
	char *local_args[2];

	/*
	 * Mark sync address
	 */
	ptr = 0;
	*ptr = 0xffffffff;
	puts("\nWaiting for image from pci host -");

	/*
	 * Wait for host to write the start address
	 */
	while (*ptr == 0xffffffff) {
		count++;
		if (!(count % 100)) {
			count2++;
			putc(0x08); /* backspace */
			putc(str[count2 % 4]);
		}

		/* Abort if ctrl-c was pressed */
		if (ctrlc()) {
			puts("\nAbort\n");
			return 0;
		}

		udelay(1000);
	}

	if (*ptr == PCI_RECONFIG_MAGIC) {
		/*
		 * Save own pci configuration in PRAM
		 */
		memset((char *)PCI_REGS_ADDR, 0, PCI_REGS_LEN);
		ptr = (unsigned int *)PCI_REGS_ADDR + 1;
		for (i=0; i<0x40; i+=4) {
			pci_read_config_dword(PCIDEVID_405GP, i, ptr++);
		}
		ptr = (unsigned int *)PCI_REGS_ADDR;
		*ptr = crc32(0, (uchar *)PCI_REGS_ADDR+4, PCI_REGS_LEN-4);

		printf("\nStoring PCI Configuration Regs...\n");
	} else {
		sprintf(addr, "%08x", *ptr);

		/*
		 * Boot image via bootm
		 */
		printf("\nBooting Image at addr 0x%s ...\n", addr);
		setenv("loadaddr", addr);

		local_args[0] = argv[0];
		local_args[1] = NULL;
		do_bootm (cmdtp, 0, 1, local_args);
	}

	return 0;
}
U_BOOT_CMD(
	loadpci,	1,	1,	do_loadpci,
	"Wait for pci-image and boot it",
	""
);
#endif
