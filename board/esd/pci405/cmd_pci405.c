/*
 * (C) Copyright 2002
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
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>
#include <405gp_pci.h>
#include <cmd_bsp.h>

#include "pci405.h"


#if (CONFIG_COMMANDS & CFG_CMD_BSP)

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

#if 0 /* test-only */
#include "../common/fpga.c"
void error_print(void)
{
	int i;
	volatile unsigned char *ptr;
	volatile unsigned long *ptr2;
	
	printf("\n 2nd SJA1000:\n");
	ptr = 0xf0000100;
	for (i=0; i<0x20; i++) {
		printf("%02x ", *ptr++);
	}

	ptr2 = 0xf0400008;
	printf("\nTimestamp = %x\n", *ptr2);
	udelay(1000);
	printf("Timestamp = %x\n", *ptr2);
	udelay(1000);
	printf("Timestamp = %x\n", *ptr2);

#if 0 /* test-only */
	/*
	 * Reset FPGA via FPGA_DATA pin
	 */
	printf("Resetting FPGA...\n");
	SET_FPGA(FPGA_PRG | FPGA_CLK);
	udelay(1000); /* wait 1ms */
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);
	udelay(1000); /* wait 1ms */

	do_loadpci(NULL, 0,0, NULL);
#endif
}

void read_loop(void)
{
	int i;
	volatile unsigned char *ptr;
	volatile unsigned char val;
	volatile unsigned long *ptr2;

	printf("\nread loop on 1st sja1000...");
	while (1) {
		ptr = 0xf0000000;
/*		printf("\n1st SJA1000:\n");*/
		for (i=0; i<0x20; i++) {
			i = i;
			val = *ptr++;
/*			printf("%02x ", val);*/
		}

		/* Abort if ctrl-c was pressed */
		if (ctrlc()) {
			puts("\nAbort\n");
			return 0;
		}
	}
}
#endif
/*
 * Command loadpci: wait for signal from host and boot image.
 */
int do_loadpci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int *ptr = 0;
	int count = 0;
	int count2 = 0;
	int status;
	int i;
	char addr[16];
	char str[] = "\\|/-";
	char *local_args[2];

#if 0 /* test-only */
	puts("\nStarting sja1000 test...");
	{
		int count;
		volatile unsigned char *ptr;
		volatile unsigned char val;
		volatile unsigned char val2;

#if 1 /* write test */
		ptr = 0xf0000014;
		for (i=1; i<11; i++)
			*ptr++ = i;
#endif
		
		count = 0;
		while (1) {
			count++;
#if 0 /* write test */
			ptr = 0xf0000014;
			for (i=1; i<11; i++)
				*ptr++ = i;
#endif
#if 1 /* read test */
			ptr = 0xf0000014;
			for (i=1; i<11; i++) {
				val = *ptr++;
#if 1
				if (val != i) {
					ptr = 0xf0000100;
					val = *ptr; /* trigger las */

					ptr = 0xf0000014;
					val2 = *ptr;

					printf("\nERROR: count=%d: soll=%x ist=%x -> staring read loop on 1st sja1000...\n",	count, i, val);

					printf("soll=%x ist=%x -> staring read loop on 1st sja1000...\n", 1, val2);

					return 0; /* test-only */
					udelay(1000);
					error_print();
					read_loop();
					return 0;
				}
#endif
			}
#endif

			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			if (!(count % 100000)) {
				printf(".");
			}
		}
	}
#endif
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
		*ptr = crc32(0, (char *)PCI_REGS_ADDR+4, PCI_REGS_LEN-4);

		printf("\nStoring PCI Configuration Regs...\n");
	} else {
		sprintf(addr, "%08x", *ptr);
		
		/*
		 * Boot image
		 */
		printf("\nBooting image at addr 0x%s ...\n", addr);
		setenv("loadaddr", addr);
		
		local_args[0] = argv[0];
		local_args[1] = NULL;
		status = do_bootm (cmdtp, 0, 1, local_args);
	}

	return 0;
}

#endif
