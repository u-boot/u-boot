/*
 * (C) Copyright 2002-2004
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
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>
#include <asm/4xx_pci.h>
#include <asm/processor.h>

#include "pci405.h"


#if defined(CONFIG_CMD_BSP)

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);
extern int do_bootvx (cmd_tbl_t *, int, int, char *[]);
unsigned long get_dcr(unsigned short);


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

#if 0
		/*
		 * Boot image
		 */
		if (*ptr & 0x00000001) {
			/*
			 * Boot VxWorks image via bootvx
			 */
			addr[strlen(addr)-1] = '0';
			printf("\nBooting VxWorks-Image at addr 0x%s ...\n", addr);
			setenv("loadaddr", addr);

			local_args[0] = argv[0];
			local_args[1] = NULL;
			status = do_bootvx (cmdtp, 0, 1, local_args);
		} else {
			/*
			 * Boot image via bootm (normally Linux)
			 */
			printf("\nBooting Image at addr 0x%s ...\n", addr);
			setenv("loadaddr", addr);

			local_args[0] = argv[0];
			local_args[1] = NULL;
			status = do_bootm (cmdtp, 0, 1, local_args);
		}
#else
		/*
		 * Boot image via bootm
		 */
		printf("\nBooting Image at addr 0x%s ...\n", addr);
		setenv("loadaddr", addr);

		local_args[0] = argv[0];
		local_args[1] = NULL;
		status = do_bootm (cmdtp, 0, 1, local_args);
#endif
	}

	return 0;
}
U_BOOT_CMD(
	loadpci,	1,	1,	do_loadpci,
	"loadpci - Wait for pci-image and boot it\n",
	NULL
);

#endif

#if 1 /* test-only */
int do_getpci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int val;
	int i;

	printf("\nPCI Configuration Regs for PPC405GP:");
	for (i=0; i<0x64; i+=4) {
		pci_read_config_dword(PCIDEVID_405GP, i, &val);
		if (!(i % 0x10)) {
			printf("\n%02x: ", i);
		}
		printf("%08x ", val);
	}
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	getpci,	1,	1,	do_getpci,
	"getpci  - Print own pci configuration registers\n",
	NULL
);

int do_setpci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int val;

	addr = simple_strtol (argv[1], NULL, 16);
	val = simple_strtol (argv[2], NULL, 16);

	printf("\nWriting %08x to PCI reg %08x.\n", val, addr);
	pci_write_config_dword(PCIDEVID_405GP, addr, val);

	return 0;
}
U_BOOT_CMD(
	setpci,	3,	1,	do_setpci,
	"setpci  - Set one pci configuration lword\n",
	"<addr> <val>\n"
	"        - Write pci configuration lword <val> to <addr>.\n"
);

int do_dumpdcr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;

	printf("\nDevice Configuration Registers (DCR's) for PPC405GP:");
	for (i=0; i<=0x1e0; i++) {
		if (!(i % 0x8)) {
			printf("\n%04x ", i);
		}
		printf("%08lx ", get_dcr(i));
	}
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	dumpdcr,	1,	1,	do_dumpdcr,
	"dumpdcr - Dump all DCR registers\n",
	NULL
);


int do_dumpspr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("\nSpecial Purpose Registers (SPR's) for PPC405GP:");
	printf("\n%04x %08x ", 947, mfspr(947));
	printf("\n%04x %08x ", 9, mfspr(9));
	printf("\n%04x %08x ", 1014, mfspr(1014));
	printf("\n%04x %08x ", 1015, mfspr(1015));
	printf("\n%04x %08x ", 1010, mfspr(1010));
	printf("\n%04x %08x ", 957, mfspr(957));
	printf("\n%04x %08x ", 1008, mfspr(1008));
	printf("\n%04x %08x ", 1018, mfspr(1018));
	printf("\n%04x %08x ", 954, mfspr(954));
	printf("\n%04x %08x ", 950, mfspr(950));
	printf("\n%04x %08x ", 951, mfspr(951));
	printf("\n%04x %08x ", 981, mfspr(981));
	printf("\n%04x %08x ", 980, mfspr(980));
	printf("\n%04x %08x ", 982, mfspr(982));
	printf("\n%04x %08x ", 1012, mfspr(1012));
	printf("\n%04x %08x ", 1013, mfspr(1013));
	printf("\n%04x %08x ", 948, mfspr(948));
	printf("\n%04x %08x ", 949, mfspr(949));
	printf("\n%04x %08x ", 1019, mfspr(1019));
	printf("\n%04x %08x ", 979, mfspr(979));
	printf("\n%04x %08x ", 8, mfspr(8));
	printf("\n%04x %08x ", 945, mfspr(945));
	printf("\n%04x %08x ", 987, mfspr(987));
	printf("\n%04x %08x ", 287, mfspr(287));
	printf("\n%04x %08x ", 953, mfspr(953));
	printf("\n%04x %08x ", 955, mfspr(955));
	printf("\n%04x %08x ", 272, mfspr(272));
	printf("\n%04x %08x ", 273, mfspr(273));
	printf("\n%04x %08x ", 274, mfspr(274));
	printf("\n%04x %08x ", 275, mfspr(275));
	printf("\n%04x %08x ", 260, mfspr(260));
	printf("\n%04x %08x ", 276, mfspr(276));
	printf("\n%04x %08x ", 261, mfspr(261));
	printf("\n%04x %08x ", 277, mfspr(277));
	printf("\n%04x %08x ", 262, mfspr(262));
	printf("\n%04x %08x ", 278, mfspr(278));
	printf("\n%04x %08x ", 263, mfspr(263));
	printf("\n%04x %08x ", 279, mfspr(279));
	printf("\n%04x %08x ", 26, mfspr(26));
	printf("\n%04x %08x ", 27, mfspr(27));
	printf("\n%04x %08x ", 990, mfspr(990));
	printf("\n%04x %08x ", 991, mfspr(991));
	printf("\n%04x %08x ", 956, mfspr(956));
	printf("\n%04x %08x ", 284, mfspr(284));
	printf("\n%04x %08x ", 285, mfspr(285));
	printf("\n%04x %08x ", 986, mfspr(986));
	printf("\n%04x %08x ", 984, mfspr(984));
	printf("\n%04x %08x ", 256, mfspr(256));
	printf("\n%04x %08x ", 1, mfspr(1));
	printf("\n%04x %08x ", 944, mfspr(944));
	printf("\n");

	return 0;
}
U_BOOT_CMD(
	dumpspr,	1,	1,	do_dumpspr,
	"dumpspr - Dump all SPR registers\n",
	NULL
);


#define PCI0_BRDGOPT1 0x4a
#define plb0_acr      0x87

int do_getplb(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned short val;

	printf("PLB0_ACR=%08lx\n", get_dcr(0x87));
	pci_read_config_word(PCIDEVID_405GP, PCI0_BRDGOPT1, &val);
	printf("PCI0_BRDGOPT1=%04x\n", val);
	printf("CCR0=%08x\n", mfspr(ccr0));

	return 0;
}
U_BOOT_CMD(
	getplb,	1,	1,	do_getplb,
	"getplb  - Dump all plb arbiter registers\n",
	NULL
);

int do_setplb(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int my_acr;
	unsigned int my_brdgopt1;
	unsigned int my_ccr0;

	my_acr = simple_strtol (argv[1], NULL, 16);
	my_brdgopt1 = simple_strtol (argv[2], NULL, 16);
	my_ccr0 = simple_strtol (argv[3], NULL, 16);

	mtdcr(plb0_acr, my_acr);
	pci_write_config_word(PCIDEVID_405GP, PCI0_BRDGOPT1, my_brdgopt1);
	mtspr(ccr0, my_ccr0);

	return 0;
}
U_BOOT_CMD(
	setplb,	4,	1,	do_setplb,
	"setplb  - Set all plb arbiter registers\n",
	"PLB0_ACR PCI0_BRDGOPT1 CCR0\n"
	"        - Set all plb arbiter registers\n"
);


/***********************************************************************
 *
 * The following code is only for test purposes!!!!
 * Please ignore this ugly stuff!!!!!!!!!!!!!!!!!!!
 *
 ***********************************************************************/

#define PCI_ADDR 0xc0000000

int do_writepci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int size;
	unsigned int countmax;
	int i;
	int max;
	volatile unsigned long *ptr;
	volatile unsigned long val;
	int loopcount = 0;
	int test_pci_read = 0;
	int test_pci_cfg_write = 0;
	int test_sync = 0;
	int test_pci_pre_read = 0;

	addr = simple_strtol (argv[1], NULL, 16);
	size = simple_strtol (argv[2], NULL, 16);
	countmax = simple_strtol (argv[3], NULL, 16);
	if (countmax == 0)
		countmax = 1000;

	do_getplb(NULL, 0, 0, NULL);

#if 0
	out32r(PMM0LA, 0);
	out32r(PMM0PCILA, 0);
	out32r(PMM0PCIHA, 0);
	out32r(PMM0MA, 0);
	out32r(PMM1LA, PCI_ADDR);
	out32r(PMM1PCILA, addr & 0xff000000);
	out32r(PMM1PCIHA, 0x00000000);
	out32r(PMM1MA, 0xff000001);
#endif

	printf("PMM1LA    =%08lx\n", in32r(PMM1LA));
	printf("PMM1MA    =%08lx\n", in32r(PMM1MA));
	printf("PMM1PCILA =%08lx\n", in32r(PMM1PCILA));
	printf("PMM1PCIHA =%08lx\n", in32r(PMM1PCIHA));

	addr = PCI_ADDR | (addr & 0x00ffffff);
	printf("\nWriting at addr %08x, size %08x (countmax=%x)\n", addr, size, countmax);

	max = size >> 2;

	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	val = *(ulong *)0x00000000;
	if (val & 0x00000008) {
		test_pci_pre_read = 1;
		printf("Running test with pre pci-memory-read access!\n");
	}
	if (val & 0x00000004) {
		test_sync = 1;
		printf("Running test with sync instruction!\n");
	}
	if (val & 0x00000001) {
		test_pci_read = 1;
		printf("Running test with pci-memory-read access!\n");
	}
	if (val & 0x00000002) {
		test_pci_cfg_write = 1;
		printf("Running test with pci-config-write access!\n");
	}

	while (1) {

		if (test_pci_pre_read) {
			/*
			 * Read one value back
			 */
			ptr = (volatile unsigned long *)addr;
			val = *ptr;
		}

		/*
		 * Write some values to host via pci busmastering
		 */
		ptr = (volatile unsigned long *)addr;
		for (i=0; i<max; i++) {
			*ptr++ = i;
		}

		if (test_sync) {
			/*
			 * Sync previous writes
			 */
			ppcSync();
		}

		if (test_pci_read) {
			/*
			 * Read one value back
			 */
			ptr = (volatile unsigned long *)addr;
			val = *ptr;
		}

		if (test_pci_cfg_write) {
			/*
			 * Generate IRQ to host via config regs
			 */
			pci_write_config_byte(PCIDEVID_405GP, 0x44, 0x00);
		}

		if (loopcount++ > countmax) {
			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			putc('.');

			loopcount = 0;
		}
	}

	return 0;
}
U_BOOT_CMD(
	writepci,	4,	1,	do_writepci,
	"writepci - Write some data to pcibus\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

#define PCI_CFGADDR        0xeec00000
#define PCI_CFGDATA        0xeec00004

int ibmPciConfigWrite
(
	int offset,     /* offset into the configuration space */
	int width,      /* data width                          */
	unsigned int data       /* data to be written                  */
	)
{
	/*
	 * Write config register address to the PCI config address register
	 * bit 31 must be 1 and bits 1:0 must be 0 (note LE bit notation)
	 */
	out32r(PCI_CFGADDR, 0x80000000 | (offset & 0xFFFFFFFC));

#if 0 /* test-only */
	ppcSync();
#endif

	/*
	 * Write value to be written to the PCI config data register
	 */
	switch ( width ) {
	case 1: out32r(PCI_CFGDATA | (offset & 0x3), (unsigned char)(data & 0xFF));
		break;
	case 2: out32r(PCI_CFGDATA | (offset & 0x3), (unsigned short)(data & 0xFFFF));
		break;
	case 4:	out32r(PCI_CFGDATA | (offset & 0x3), data);
		break;
	}

	return (0);
}

int do_writepci2(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int size;
	unsigned int countmax;
	int max;
	volatile unsigned long *ptr;
	volatile unsigned long val;
	int loopcount = 0;

	addr = simple_strtol (argv[1], NULL, 16);
	size = simple_strtol (argv[2], NULL, 16);
	countmax = simple_strtol (argv[3], NULL, 16);
	if (countmax == 0)
		countmax = 1000;

	do_getplb(NULL, 0, 0, NULL);

#if 0
	out32r(PMM0LA, 0);
	out32r(PMM0PCILA, 0);
	out32r(PMM0PCIHA, 0);
	out32r(PMM0MA, 0);
	out32r(PMM1LA, PCI_ADDR);
	out32r(PMM1PCILA, addr & 0xff000000);
	out32r(PMM1PCIHA, 0x00000000);
	out32r(PMM1MA, 0xff000001);
#endif

	printf("PMM1LA    =%08lx\n", in32r(PMM1LA));
	printf("PMM1MA    =%08lx\n", in32r(PMM1MA));
	printf("PMM1PCILA =%08lx\n", in32r(PMM1PCILA));
	printf("PMM1PCIHA =%08lx\n", in32r(PMM1PCIHA));

	addr = PCI_ADDR | (addr & 0x00ffffff);
	printf("\nWriting at addr %08x, size %08x (countmax=%x)\n", addr, size, countmax);

	max = size >> 2;

	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	while (1) {

		/*
		 * Write one values to host via pci busmastering
		 */
		ptr = (volatile unsigned long *)addr;
		*ptr = 0x01234567;

		/*
		 * Read one value back
		 */
		ptr = (volatile unsigned long *)addr;
		val = *ptr;

		/*
		 * One pci config write
		 */
/*		pci_write_config_byte(PCIDEVID_405GP, 0x44, 0x00); */
/*		ibmPciConfigWrite(0x44, 1, 0x00); */
		ibmPciConfigWrite(0x2e, 2, 0x1234); /* subsystem id */

		if (loopcount++ > countmax) {
			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			putc('.');

			loopcount = 0;
		}
	}

	return 0;
}
U_BOOT_CMD(
	writepci2,	4,	1,	do_writepci2,
	"writepci2- Write some data to pcibus\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_writepci22(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int size;
	unsigned int countmax = 0;
	volatile unsigned long *ptr;
	volatile unsigned long val;

	addr = simple_strtol (argv[1], NULL, 16);
	size = simple_strtol (argv[2], NULL, 16);

	addr = PCI_ADDR | (addr & 0x00ffffff);
	printf("\nWriting at addr %08x, size %08x (countmax=%x)\n", addr, size, countmax);
	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	while (1) {

		/*
		 * Write one values to host via pci busmastering
		 */
		ptr = (volatile unsigned long *)addr;
		*ptr = 0x01234567;

		/*
		 * Read one value back
		 */
		ptr = (volatile unsigned long *)addr;
		val = *ptr;

		/*
		 * One pci config write
		 */
		ibmPciConfigWrite(0x2e, 2, 0x1234); /* subsystem id */
	}

	return 0;
}
U_BOOT_CMD(
	writepci22,	4,	1,	do_writepci22,
	"writepci22- Write some data to pcibus\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int ibmPciConfigWrite3
(
	int offset,     /* offset into the configuration space */
	int width,      /* data width                          */
	unsigned int data       /* data to be written                  */
	)
{
	/*
	 * Write config register address to the PCI config address register
	 * bit 31 must be 1 and bits 1:0 must be 0 (note LE bit notation)
	 */
	out32r(PCI_CFGADDR, 0x80000000 | (offset & 0xFFFFFFFC));

#if 1 /* test-only */
	ppcSync();
#endif

	/*
	 * Write value to be written to the PCI config data register
	 */
	switch ( width ) {
	case 1: out32r(PCI_CFGDATA | (offset & 0x3), (unsigned char)(data & 0xFF));
		break;
	case 2: out32r(PCI_CFGDATA | (offset & 0x3), (unsigned short)(data & 0xFFFF));
		break;
	case 4:	out32r(PCI_CFGDATA | (offset & 0x3), data);
		break;
	}

	return (0);
}

int do_writepci3(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int size;
	unsigned int countmax;
	int max;
	volatile unsigned long *ptr;
	volatile unsigned long val;
	int loopcount = 0;

	addr = simple_strtol (argv[1], NULL, 16);
	size = simple_strtol (argv[2], NULL, 16);
	countmax = simple_strtol (argv[3], NULL, 16);
	if (countmax == 0)
		countmax = 1000;

	do_getplb(NULL, 0, 0, NULL);

#if 0
	out32r(PMM0LA, 0);
	out32r(PMM0PCILA, 0);
	out32r(PMM0PCIHA, 0);
	out32r(PMM0MA, 0);
	out32r(PMM1LA, PCI_ADDR);
	out32r(PMM1PCILA, addr & 0xff000000);
	out32r(PMM1PCIHA, 0x00000000);
	out32r(PMM1MA, 0xff000001);
#endif

	printf("PMM1LA    =%08lx\n", in32r(PMM1LA));
	printf("PMM1MA    =%08lx\n", in32r(PMM1MA));
	printf("PMM1PCILA =%08lx\n", in32r(PMM1PCILA));
	printf("PMM1PCIHA =%08lx\n", in32r(PMM1PCIHA));

	addr = PCI_ADDR | (addr & 0x00ffffff);
	printf("\nWriting at addr %08x, size %08x (countmax=%x)\n", addr, size, countmax);

	max = size >> 2;

	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	while (1) {

		/*
		 * Write one values to host via pci busmastering
		 */
		ptr = (volatile unsigned long *)addr;
		*ptr = 0x01234567;

		/*
		 * Read one value back
		 */
		ptr = (volatile unsigned long *)addr;
		val = *ptr;

		/*
		 * One pci config write
		 */
/*		pci_write_config_byte(PCIDEVID_405GP, 0x44, 0x00); */
/*		ibmPciConfigWrite(0x44, 1, 0x00); */
		ibmPciConfigWrite3(0x2e, 2, 0x1234); /* subsystem id */

		if (loopcount++ > countmax) {
			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			putc('.');

			loopcount = 0;
		}
	}

	return 0;
}
U_BOOT_CMD(
	writepci3,	4,	1,	do_writepci3,
	"writepci3- Write some data to pcibus\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);


#define SECTOR_SIZE 	32		/* 32 byte cache line */
#define SECTOR_MASK	0x1F

void my_flush_dcache(ulong lcl_addr, ulong count)
{
  unsigned int lcl_target;

  /* promote to nearest cache sector */
  lcl_target =  (lcl_addr + count + SECTOR_SIZE - 1) & ~SECTOR_MASK;
  lcl_addr &= ~SECTOR_MASK;
  while (lcl_addr != lcl_target)
    {
      /*      ppcDcbf((void *)lcl_addr);*/
      __asm__("dcbf 0,%0": :"r" (lcl_addr));
      lcl_addr += SECTOR_SIZE;
    }
  __asm__("sync");		/* Always flush prefetch queue in any case */
}

int do_writepci_cache(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int addr;
	unsigned int size;
	unsigned int countmax;
	int i;
	volatile unsigned long *ptr;
	volatile unsigned long val;
	int loopcount = 0;

	addr = simple_strtol (argv[1], NULL, 16);
	size = simple_strtol (argv[2], NULL, 16);
	countmax = simple_strtol (argv[3], NULL, 16);
	if (countmax == 0)
		countmax = 1000;

	do_getplb(NULL, 0, 0, NULL);

#if 0
	out32r(PMM0LA, 0);
	out32r(PMM0PCILA, 0);
	out32r(PMM0PCIHA, 0);
	out32r(PMM0MA, 0);
	out32r(PMM1LA, PCI_ADDR);
	out32r(PMM1PCILA, addr & 0xff000000);
	out32r(PMM1PCIHA, 0x00000000);
	out32r(PMM1MA, 0xff000001);
#endif

	printf("PMM1LA    =%08lx\n", in32r(PMM1LA));
	printf("PMM1MA    =%08lx\n", in32r(PMM1MA));
	printf("PMM1PCILA =%08lx\n", in32r(PMM1PCILA));
	printf("PMM1PCIHA =%08lx\n", in32r(PMM1PCIHA));

	addr = PCI_ADDR | (addr & 0x00ffffff);
	printf("\nWriting at addr %08x, size %08x (countmax=%x)\n", addr, size, countmax);

	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	i = 0;

	/*
	 * Set pci region as cachable
	 */
	ppcSync();
	__asm__ volatile ("	addis	4,0,0x0000 ");
	__asm__ volatile ("	addi	4,4,0x0080 ");
	__asm__ volatile ("	mtdccr	4 ");
	ppcSync();

	while (1) {

		/*
		 * Write one values to host via pci busmastering
		 */
		ptr = (volatile unsigned long *)addr;
		printf("A\n"); /* test-only */
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		*ptr++ = i++;
		printf("B\n"); /* test-only */
		my_flush_dcache(addr, 32);
		printf("C\n"); /* test-only */

		/*
		 * Read one value back
		 */
		ptr = (volatile unsigned long *)addr;
		val = *ptr;
		printf("D\n"); /* test-only */

		/*
		 * One pci config write
		 */
/*		pci_write_config_byte(PCIDEVID_405GP, 0x44, 0x00); */
/*		ibmPciConfigWrite(0x44, 1, 0x00); */
		ibmPciConfigWrite3(0x2e, 2, 0x1234); /* subsystem id */
		printf("E\n"); /* test-only */

		if (loopcount++ > countmax) {
			/* Abort if ctrl-c was pressed */
			if (ctrlc()) {
				puts("\nAbort\n");
				return 0;
			}

			putc('.');

			loopcount = 0;
		}
	}

	return 0;
}
U_BOOT_CMD(
	writepci_cache,	4,	1,	do_writepci_cache,
	"writepci_cache - Write some data to pcibus\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_savepci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int *ptr;
	int i;

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

	return 0;
}
U_BOOT_CMD(
	savepci,	4,	1,	do_savepci,
	"savepci  - Save all pci regs\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_restorepci(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int *ptr;
	int i;

	/*
	 * Rewrite pci config regs (only after soft-reset with magic set)
	 */
	ptr = (unsigned int *)PCI_REGS_ADDR;
	if (crc32(0, (uchar *)PCI_REGS_ADDR+4, PCI_REGS_LEN-4) == *ptr) {
		puts("Restoring PCI Configurations Regs!\n");
		ptr = (unsigned int *)PCI_REGS_ADDR + 1;
		for (i=0; i<0x40; i+=4) {
			pci_write_config_dword(PCIDEVID_405GP, i, *ptr++);
		}
	}
	mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */

	return 0;
}
U_BOOT_CMD(
	restorepci,	4,	1,	do_restorepci,
	"restorepci  - Restore all pci regs\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);


extern void write_without_sync(void);
extern void write_with_sync(void);
extern void write_with_less_sync(void);
extern void write_with_more_sync(void);

/*
 * code from IBM-PPCSUPP
 */
int do_writeibm1(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	write_without_sync();

	return 0;
}
U_BOOT_CMD(
	writeibm1,	4,	1,	do_writeibm1,
	"writeibm1- Write some data to pcibus (without sync)\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_writeibm2(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	write_with_sync();

	return 0;
}
U_BOOT_CMD(
	writeibm2,	4,	1,	do_writeibm2,
	"writeibm2- Write some data to pcibus (with sync)\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_writeibm22(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	write_with_less_sync();

	return 0;
}
U_BOOT_CMD(
	writeibm22,	4,	1,	do_writeibm22,
	"writeibm22- Write some data to pcibus (with less sync)\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);

int do_writeibm3(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	pci_write_config_word(PCIDEVID_405GP, 0x04, 0x0106); /* write command reg */

	write_with_more_sync();

	return 0;
}
U_BOOT_CMD(
	writeibm3,	4,	1,	do_writeibm3,
	"writeibm3- Write some data to pcibus (with more sync)\n",
	"<addr> <size>\n"
	"        - Write some data to pcibus.\n"
);
#endif
