/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, <hs@denx.de>.
 *
 * (C) Copyright 2003
 * Juergen Beisert, EuroDesign embedded technologies, info@eurodsn.de
 * Derived from walnut.c
 *
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
 * $Log:$
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include "sc3.h"
#include <pci.h>
#include <i2c.h>
#include <malloc.h>
#include <netdev.h>

#undef writel
#undef writeb
#define writeb(b,addr) ((*(volatile u8 *) (addr)) = (b))
#define writel(b,addr) ((*(volatile u32 *) (addr)) = (b))

/* write only register to configure things in our CPLD */
#define CPLD_CONTROL_1	0x79000102
#define CPLD_VERSION	0x79000103

#define	IS_CAMERON ((*(unsigned char *)(CPLD_VERSION)== 0x32) ? 1 : 0)

static struct pci_controller hose={0,};

/************************************************************
 * Standard definition
 ************************************************************/

/* CPC0_CR0        Function                                 ISA bus
	-  GPIO0
	-  GPIO1      -> Output: NAND-Command Latch Enable
	-  GPIO2      -> Output: NAND Address Latch Enable
	-  GPIO3      -> IRQ input                               ISA-IRQ #5 (through CPLD)
	-  GPIO4      -> Output: NAND-Chip Enable
	-  GPIO5      -> IRQ input                               ISA-IRQ#7 (through CPLD)
	-  GPIO6      -> IRQ input                               ISA-IRQ#9 (through CPLD)
	-  GPIO7      -> IRQ input                               ISA-IRQ#10 (through CPLD)
	-  GPIO8      -> IRQ input                               ISA-IRQ#11 (through CPLD)
	-  GPIO9      -> IRQ input                               ISA-IRQ#12 (through CPLD)
	- GPIO10/CS1# -> CS1# NAND                               ISA-CS#0
	- GPIO11/CS2# -> CS2# ISA emulation                      ISA-CS#1
	- GPIO12/CS3# -> CS3# 2nd Flash-Bank                     ISA-CS#2 or ISA-CS#7
	- GPIO13/CS4# -> CS4# USB HC or ISA emulation            ISA-CS#3
	- GPIO14/CS5# -> CS5# Boosted IDE access                 ISA-CS#4
	- GPIO15/CS6# -> CS6# ISA emulation                      ISA-CS#5
	- GPIO16/CS7# -> CS7# ISA emulation                      ISA-CS#6
	- GPIO17/IRQ0 -> GPIO, in, NAND-Ready/Busy# line         ISA-IRQ#3
	- GPIO18/IRQ1 -> IRQ input                               ISA-IRQ#14
	- GPIO19/IRQ2 -> IRQ input or USB                        ISA-IRQ#4
	- GPIO20/IRQ3 -> IRQ input                               PCI-IRQ#D
	- GPIO21/IRQ4 -> IRQ input                               PCI-IRQ#C
	- GPIO22/IRQ5 -> IRQ input                               PCI-IRQ#B
	- GPIO23/IRQ6 -> IRQ input                               PCI-IRQ#A
	- GPIO24 -> if GPIO output: 0=JTAG CPLD activ, 1=JTAG CPLD inactiv
*/
/*
| CPLD register: io-space at offset 0x102 (write only)
| 0
| 1
| 2 0=CS#4 USB CS#, 1=ISA or GP bus
| 3
| 4
| 5
| 6 1=enable faster IDE access
| 7
*/
#define USB_CHIP_ENABLE 0x04
#define IDE_BOOSTING 0x40

/* --------------- USB stuff ------------------------------------- */
#ifdef CONFIG_ISP1161_PRESENT
/**
 * initUsbHost- Initialize the Philips isp1161 HC part if present
 * @cpldConfig: Pointer to value in write only CPLD register
 *
 * Initialize the USB host controller if present and fills the
 * scratch register to inform the driver about used resources
 */

static void initUsbHost (unsigned char *cpldConfig)
{
	int i;
	unsigned long usbBase;
	/*
	 * Read back where init.S has located the USB chip
	 */
	mtdcr (0x012, 0x04);
	usbBase = mfdcr (0x013);
	if (!(usbBase & 0x18000))	/* enabled? */
		return;
	usbBase &= 0xFFF00000;

	/*
	 * to test for the USB controller enable using of CS#4 and DMA 3 for USB access
	 */
	writeb (*cpldConfig | USB_CHIP_ENABLE,CPLD_CONTROL_1);

	/*
	 * first check: is the controller assembled?
	 */
	hcWriteWord (usbBase, 0x5555, HcScratch);
	if (hcReadWord (usbBase, HcScratch) == 0x5555) {
		hcWriteWord (usbBase, 0xAAAA, HcScratch);
		if (hcReadWord (usbBase, HcScratch) == 0xAAAA) {
			if ((hcReadWord (usbBase, HcChipID) & 0xFF00) != 0x6100)
				return;	/* this is not our controller */
		/*
		 * try a software reset. This needs up to 10 seconds (see datasheet)
		 */
			hcWriteDWord (usbBase, 0x00000001, HcCommandStatus);
			for (i = 1000; i > 0; i--) {	/* loop up to 10 seconds */
				udelay (10);
				if (!(hcReadDWord (usbBase, HcCommandStatus) & 0x01))
					break;
			}

			if (!i)
				return;  /* the controller doesn't responding. Broken? */
		/*
		 * OK. USB controller is ready. Initialize it in such way the later driver
		 * can us it (without any knowing about specific implementation)
		 */
			hcWriteDWord (usbBase, 0x00000000, HcControl);
		/*
		 * disable all interrupt sources. Because we
		 * don't know where we come from (hard reset, cold start, soft reset...)
		 */
			hcWriteDWord (usbBase, 0x8000007D, HcInterruptDisable);
		/*
		 * our current setup hardware configuration
		 * - every port power supply can switched indepently
		 * - every port can signal overcurrent
		 * - every port is "outside" and the devices are removeable
		 */
			hcWriteDWord (usbBase, 0x32000902, HcRhDescriptorA);
			hcWriteDWord (usbBase, 0x00060000, HcRhDescriptorB);
		/*
		 * don't forget to switch off power supply of each port
		 * The later running driver can reenable them to find and use
		 * the (maybe) connected devices.
		 *
		 */
			hcWriteDWord (usbBase, 0x00000200, HcRhPortStatus1);
			hcWriteDWord (usbBase, 0x00000200, HcRhPortStatus2);
			hcWriteWord (usbBase, 0x0428, HcHardwareConfiguration);
			hcWriteWord (usbBase, 0x0040, HcDMAConfiguration);
			hcWriteWord (usbBase, 0x0000, HcuPInterruptEnable);
			hcWriteWord (usbBase, 0xA000 | (0x03 << 8) | 27, HcScratch);
		/*
		 * controller is present and usable
		 */
			*cpldConfig |= USB_CHIP_ENABLE;
		}
	}
}
#endif

#if defined(CONFIG_START_IDE)
int board_start_ide(void)
{
	if (IS_CAMERON) {
		puts ("no IDE on cameron board.\n");
		return 0;
	}
	return 1;
}
#endif

static int sc3_cameron_init (void)
{
	/* Set up the Memory Controller for the CAMERON version */
	mtebc (pb4ap, 0x01805940);
	mtebc (pb4cr, 0x7401a000);
	mtebc (pb5ap, 0x01805940);
	mtebc (pb5cr, 0x7401a000);
	mtebc (pb6ap, 0x0);
	mtebc (pb6cr, 0x0);
	mtebc (pb7ap, 0x0);
	mtebc (pb7cr, 0x0);
	return 0;
}

void sc3_read_eeprom (void)
{
	uchar i2c_buffer[18];

	i2c_read (0x50, 0x03, 1, i2c_buffer, 9);
	i2c_buffer[9] = 0;
	setenv ("serial#", (char *)i2c_buffer);

	/* read mac-address from eeprom */
	i2c_read (0x50, 0x11, 1, i2c_buffer, 15);
	i2c_buffer[17] = 0;
	i2c_buffer[16] = i2c_buffer[14];
	i2c_buffer[15] = i2c_buffer[13];
	i2c_buffer[14] = ':';
	i2c_buffer[13] = i2c_buffer[12];
	i2c_buffer[12] = i2c_buffer[11];
	i2c_buffer[11] = ':';
	i2c_buffer[8] = ':';
	i2c_buffer[5] = ':';
	i2c_buffer[2] = ':';
	setenv ("ethaddr", (char *)i2c_buffer);
}

int board_early_init_f (void)
{
	/* write only register to configure things in our CPLD */
	unsigned char cpldConfig_1=0x00;

/*-------------------------------------------------------------------------+
| Interrupt controller setup for the SolidCard III CPU card (plus Evaluation board).
|
| Note: IRQ 0  UART 0, active high; level sensitive
|       IRQ 1  UART 1, active high; level sensitive
|       IRQ 2  IIC, active high; level sensitive
|       IRQ 3  Ext. master, rising edge, edge sensitive
|       IRQ 4  PCI, active high; level sensitive
|       IRQ 5  DMA Channel 0, active high; level sensitive
|       IRQ 6  DMA Channel 1, active high; level sensitive
|       IRQ 7  DMA Channel 2, active high; level sensitive
|       IRQ 8  DMA Channel 3, active high; level sensitive
|       IRQ 9  Ethernet Wakeup, active high; level sensitive
|       IRQ 10 MAL System Error (SERR), active high; level sensitive
|       IRQ 11 MAL Tx End of Buffer, active high; level sensitive
|       IRQ 12 MAL Rx End of Buffer, active high; level sensitive
|       IRQ 13 MAL Tx Descriptor Error, active high; level sensitive
|       IRQ 14 MAL Rx Descriptor Error, active high; level sensitive
|       IRQ 15 Ethernet, active high; level sensitive
|       IRQ 16 External PCI SERR, active high; level sensitive
|       IRQ 17 ECC Correctable Error, active high; level sensitive
|       IRQ 18 PCI Power Management, active high; level sensitive
|
|       IRQ 19 (EXT IRQ7 405GPr only)
|       IRQ 20 (EXT IRQ8 405GPr only)
|       IRQ 21 (EXT IRQ9 405GPr only)
|       IRQ 22 (EXT IRQ10 405GPr only)
|       IRQ 23 (EXT IRQ11 405GPr only)
|       IRQ 24 (EXT IRQ12 405GPr only)
|
|       IRQ 25 (EXT IRQ 0) NAND-Flash R/B# (raising edge means flash is ready)
|       IRQ 26 (EXT IRQ 1) IDE0 interrupt (x86 = IRQ14). Active high (edge sensitive)
|       IRQ 27 (EXT IRQ 2) USB controller
|       IRQ 28 (EXT IRQ 3) INT D, VGA; active low; level sensitive
|       IRQ 29 (EXT IRQ 4) INT C, Ethernet; active low; level sensitive
|       IRQ 30 (EXT IRQ 5) INT B, PC104+ SLOT; active low; level sensitive
|       IRQ 31 (EXT IRQ 6) INT A, PC104+ SLOT; active low; level sensitive
|
| Direct Memory Access Controller Signal Polarities
|       DRQ0 active high (like ISA)
|       ACK0 active low (like ISA)
|       EOT0 active high (like ISA)
|       DRQ1 active high (like ISA)
|       ACK1 active low (like ISA)
|       EOT1 active high (like ISA)
|       DRQ2 active high (like ISA)
|       ACK2 active low (like ISA)
|       EOT2 active high (like ISA)
|       DRQ3 active high (like ISA)
|       ACK3 active low (like ISA)
|       EOT3 active high (like ISA)
|
+-------------------------------------------------------------------------*/

	writeb (cpldConfig_1, CPLD_CONTROL_1);	/* disable everything in CPLD */

	mtdcr (uicsr, 0xFFFFFFFF);    /* clear all ints */
	mtdcr (uicer, 0x00000000);    /* disable all ints */
	mtdcr (uiccr, 0x00000000);    /* set all to be non-critical */

	if (IS_CAMERON) {
		sc3_cameron_init();
		mtdcr (0x0B6, 0x18000000);
		mtdcr (uicpr, 0xFFFFFFF0);
		mtdcr (uictr, 0x10001030);
	} else {
		mtdcr (0x0B6, 0x0000000);
		mtdcr (uicpr, 0xFFFFFFE0);
		mtdcr (uictr, 0x10000020);
	}
	mtdcr (uicvcr, 0x00000001);   /* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);    /* clear all ints */

	/* setup other implementation specific details */
	mtdcr (ecr, 0x60606000);

	mtdcr (cntrl1, 0x000042C0);

	if (IS_CAMERON) {
		mtdcr (cntrl0, 0x01380000);
		/* Setup the GPIOs */
		writel (0x08008000, 0xEF600700);	/* Output states */
		writel (0x00000000, 0xEF600718);	/* Open Drain control */
		writel (0x68098000, 0xEF600704);	/* Output control */
	} else {
		mtdcr (cntrl0,0x00080000);
		/* Setup the GPIOs */
		writel (0x08000000, 0xEF600700);	/* Output states */
		writel (0x14000000, 0xEF600718);	/* Open Drain control */
		writel (0x7C000000, 0xEF600704);	/* Output control */
	}

	/* Code decompression disabled */
	mtdcr (kiar, kconf);
	mtdcr (kidr, 0x2B);

	/* CPC0_ER: enable sleep mode of (currently) unused components */
	/* CPC0_FR: force unused components into sleep mode */
	mtdcr (cpmer, 0x3F800000);
	mtdcr (cpmfr, 0x14000000);

	/* set PLB priority */
	mtdcr (0x87, 0x08000000);

	/* --------------- DMA stuff ------------------------------------- */
	mtdcr (0x126, 0x49200000);

#ifndef IDE_USES_ISA_EMULATION
	cpldConfig_1 |= IDE_BOOSTING;	/* enable faster IDE */
	/* cpldConfig |= 0x01; */	/* enable 8.33MHz output, if *not* present on your baseboard */
	writeb (cpldConfig_1, CPLD_CONTROL_1);
#endif

#ifdef CONFIG_ISP1161_PRESENT
	initUsbHost (&cpldConfig_1);
	writeb (cpldConfig_1, CPLD_CONTROL_1);
#endif
	/* FIXME: for what must we do this */
	*(unsigned long *)0x79000080 = 0x0001;
	return(0);
}

int misc_init_r (void)
{
	char *s1;
	int i, xilinx_val;
	volatile char *xilinx_adr;
	xilinx_adr = (char *)0x79000102;

	*xilinx_adr = 0x00;

/* customer settings ***************************************** */
/*
	s1 = getenv ("function");
	if (s1) {
		if (!strcmp (s1, "Rosho")) {
			printf ("function 'Rosho' activated\n");
			*xilinx_adr = 0x40;
		}
		else {
			printf (">>>>>>>>>> function %s not recognized\n",s1);
		}
	}
*/

/* individual settings ***************************************** */
	if ((s1 = getenv ("xilinx"))) {
		i=0;
		xilinx_val = 0;
		while (i < 3 && s1[i]) {
			if (s1[i] >= '0' && s1[i] <= '9')
				xilinx_val = (xilinx_val << 4) + s1[i] - '0';
			else
				if (s1[i] >= 'A' && s1[i] <= 'F')
					xilinx_val = (xilinx_val << 4) + s1[i] - 'A' + 10;
				else
					if (s1[i] >= 'a' && s1[i] <= 'f')
						xilinx_val = (xilinx_val << 4) + s1[i] - 'a' + 10;
					else {
						xilinx_val = -1;
						break;
					}
			i++;
		}
		if (xilinx_val >= 0 && xilinx_val <=255 && i < 3) {
			printf ("Xilinx: set to %s\n", s1);
			*xilinx_adr = (unsigned char) xilinx_val;
		} else
			printf ("Xilinx: rejected value %s\n", s1);
	}
	return 0;
}

/* -------------------------------------------------------------------------
 * printCSConfig
 *
 * Print some informations about chips select configurations
 * Only used while debugging.
 *
 * Params:
 * - No. of CS pin
 * - AP of this CS
 * - CR of this CS
 *
 * Returns
 * nothing
   ------------------------------------------------------------------------- */

#ifdef SC3_DEBUGOUT
static void printCSConfig(int reg,unsigned long ap,unsigned long cr)
{
	const char *bsize[4] = {"8","16","32","?"};
	const unsigned char banks[8] = {1, 2, 4, 8, 16, 32, 64, 128};
	const char *bankaccess[4] = {"disabled", "RO", "WO", "RW"};

#define CYCLE 30  /* time of one clock (based on 33MHz) */

	printf("\nCS#%d",reg);
	if (!(cr & 0x00018000))
		puts(" unused");
	else {
		if (((cr&0xFFF00000U) & ((banks[(cr & 0x000E0000) >> 17]-1) << 20)))
			puts(" Address is not multiple of bank size!");

		printf("\n -%s bit device",
			bsize[(cr & 0x00006000) >> 13]);
		printf(" at 0x%08lX", cr & 0xFFF00000U);
		printf(" size: %u MB", banks[(cr & 0x000E0000) >> 17]);
		printf(" rights: %s", bankaccess[(cr & 0x00018000) >> 15]);
		if (ap & 0x80000000) {
			printf("\n -Burst device (%luns/%luns)",
				(((ap & 0x7C000000) >> 26) + 1) * CYCLE,
				(((ap & 0x03800000) >> 23) + 1) * CYCLE);
		} else {
			printf("\n -Non burst device, active cycle %luns",
				(((ap & 0x7F800000) >> 23) + 1) * CYCLE);
			printf("\n -Address setup %luns",
				((ap & 0xC0000) >> 18) * CYCLE);
			printf("\n -CS active to RD %luns/WR %luns",
				((ap & 0x30000) >> 16) * CYCLE,
				((ap & 0xC000) >> 14) * CYCLE);
			printf("\n -WR to CS inactive %luns",
				((ap & 0x3000) >> 12) * CYCLE);
			printf("\n -Hold after access %luns",
				((ap & 0xE00) >> 9) * CYCLE);
			printf("\n -Ready is %sabled",
				ap & 0x100 ? "en" : "dis");
		}
	}
}
#endif

#ifdef SC3_DEBUGOUT

static unsigned int ap[] = {pb0ap, pb1ap, pb2ap, pb3ap, pb4ap,
				pb5ap, pb6ap, pb7ap};
static unsigned int cr[] = {pb0cr, pb1cr, pb2cr, pb3cr, pb4cr,
				pb5cr, pb6cr, pb7cr};

static int show_reg (int nr)
{
	unsigned long ul1, ul2;

	mtdcr (ebccfga, ap[nr]);
	ul1 = mfdcr (ebccfgd);
	mtdcr (ebccfga, cr[nr]);
	ul2 = mfdcr(ebccfgd);
	printCSConfig(nr, ul1, ul2);
	return 0;
}
#endif

int checkboard (void)
{
#ifdef SC3_DEBUGOUT
	unsigned long ul1;
	int	i;

	for (i = 0; i < 8; i++) {
		show_reg (i);
	}

	mtdcr (ebccfga, epcr);
	ul1 = mfdcr (ebccfgd);

	puts ("\nGeneral configuration:\n");

	if (ul1 & 0x80000000)
		printf(" -External Bus is always driven\n");

	if (ul1 & 0x400000)
		printf(" -CS signals are always driven\n");

	if (ul1 & 0x20000)
		printf(" -PowerDown after %lu clocks\n",
			(ul1 & 0x1F000) >> 7);

	switch (ul1 & 0xC0000)
	{
	case 0xC0000:
		printf(" -No external master present\n");
		break;
	case 0x00000:
		printf(" -8 bit external master present\n");
		break;
	case 0x40000:
		printf(" -16 bit external master present\n");
		break;
	case 0x80000:
		printf(" -32 bit external master present\n");
		break;
	}

	switch (ul1 & 0x300000)
	{
	case 0x300000:
		printf(" -Prefetch: Illegal setting!\n");
		break;
	case 0x000000:
		printf(" -1 doubleword prefetch\n");
		break;
	case 0x100000:
		printf(" -2 doublewords prefetch\n");
		break;
	case 0x200000:
		printf(" -4 doublewords prefetch\n");
		break;
	}
	putc ('\n');
#endif
	printf("Board: SolidCard III %s %s version.\n",
		(IS_CAMERON ? "Cameron" : "Eurodesign"), CONFIG_SC3_VERSION);
	return 0;
}

static int printSDRAMConfig(char reg, unsigned long cr)
{
	const int bisize[8]={4, 8, 16, 32, 64, 128, 256, 0};
#ifdef SC3_DEBUGOUT
	const char *basize[8]=
		{"4", "8", "16", "32", "64", "128", "256", "Reserved"};

	printf("SDRAM bank %d",reg);

	if (!(cr & 0x01))
		puts(" disabled\n");
	else {
		printf(" at 0x%08lX, size %s MB",cr & 0xFFC00000,basize[(cr&0xE0000)>>17]);
		printf(" mode %lu\n",((cr & 0xE000)>>13)+1);
	}
#endif

	if (cr & 0x01)
		return(bisize[(cr & 0xE0000) >> 17]);

	return 0;
}

#ifdef SC3_DEBUGOUT
static unsigned int mbcf[] = {mem_mb0cf, mem_mb1cf, mem_mb2cf, mem_mb3cf};
#endif

phys_size_t initdram (int board_type)
{
	unsigned int mems=0;
	unsigned long ul1;

#ifdef SC3_DEBUGOUT
	unsigned long ul2;
	int	i;

	puts("\nSDRAM configuration:\n");

	mtdcr (memcfga, mem_mcopt1);
	ul1 = mfdcr(memcfgd);

	if (!(ul1 & 0x80000000)) {
		puts(" Controller disabled\n");
		return 0;
	}
	for (i = 0; i < 4; i++) {
		mtdcr (memcfga, mbcf[i]);
		ul1 = mfdcr (memcfgd);
		mems += printSDRAMConfig (i, ul1);
	}

	mtdcr (memcfga, mem_sdtr1);
	ul1 = mfdcr(memcfgd);

	printf ("Timing:\n -CAS latency %lu\n", ((ul1 & 0x1800000) >> 23)+1);
	printf (" -Precharge %lu (PTA) \n", ((ul1 & 0xC0000) >> 18) + 1);
	printf (" -R/W to Precharge %lu (CTP)\n", ((ul1 & 0x30000) >> 16) + 1);
	printf (" -Leadoff %lu\n", ((ul1 & 0xC000) >> 14) + 1);
	printf (" -CAS to RAS %lu\n", ((ul1 & 0x1C) >> 2) + 4);
	printf (" -RAS to CAS %lu\n", ((ul1 & 0x3) + 1));
	puts ("Misc:\n");
	mtdcr (memcfga, mem_rtr);
	ul1 = mfdcr(memcfgd);
	printf (" -Refresh rate: %luns\n", (ul1 >> 16) * 7);

	mtdcr(memcfga,mem_pmit);
	ul2=mfdcr(memcfgd);

	mtdcr(memcfga,mem_mcopt1);
	ul1=mfdcr(memcfgd);

	if (ul1 & 0x20000000)
		printf(" -Power Down after: %luns\n",
			((ul2 & 0xFFC00000) >> 22) * 7);
	else
		puts(" -Power Down disabled\n");

	if (ul1 & 0x40000000)
		printf(" -Self refresh feature active\n");
	else
		puts(" -Self refresh disabled\n");

	if (ul1 & 0x10000000)
		puts(" -ECC enabled\n");
	else
		puts(" -ECC disabled\n");

	if (ul1 & 0x8000000)
		puts(" -Using registered SDRAM\n");

	if (!(ul1 & 0x6000000))
		puts(" -Using 32 bit data width\n");
	else
		puts(" -Illegal data width!\n");

	if (ul1 & 0x400000)
		puts(" -ECC drivers inactive\n");
	else
		puts(" -ECC drivers active\n");

	if (ul1 & 0x200000)
		puts(" -Memory lines always active outputs\n");
	else
		puts(" -Memory lines only at write cycles active outputs\n");

	mtdcr (memcfga, mem_status);
	ul1 = mfdcr (memcfgd);
	if (ul1 & 0x80000000)
		puts(" -SDRAM Controller ready\n");
	else
		puts(" -SDRAM Controller not ready\n");

	if (ul1 & 0x4000000)
		puts(" -SDRAM in self refresh mode!\n");

	return (mems * 1024 * 1024);
#else
	mtdcr (memcfga, mem_mb0cf);
	ul1 = mfdcr (memcfgd);
	mems = printSDRAMConfig (0, ul1);

	mtdcr (memcfga, mem_mb1cf);
	ul1 = mfdcr (memcfgd);
	mems += printSDRAMConfig (1, ul1);

	mtdcr (memcfga, mem_mb2cf);
	ul1 = mfdcr(memcfgd);
	mems += printSDRAMConfig (2, ul1);

	mtdcr (memcfga, mem_mb3cf);
	ul1 = mfdcr(memcfgd);
	mems += printSDRAMConfig (3, ul1);

	return (mems * 1024 * 1024);
#endif
}

static void pci_solidcard3_fixup_irq (struct pci_controller *hose, pci_dev_t dev)
{
/*-------------------------------------------------------------------------+
 |             ,-.     ,-.        ,-.        ,-.        ,-.
 |   INTD# ----|B|-----|P|-.    ,-|P|-.    ,-| |-.    ,-|G|
 |             |R|     |C|  \  /  |C|  \  /  |E|  \  /  |r|
 |   INTC# ----|I|-----|1|-. `/---|1|-. `/---|t|-. `/---|a|
 |             |D|     |0|  \/    |0|  \/    |h|  \/    |f|
 |   INTB# ----|G|-----|4|-./`----|4|-./`----|e|-./`----|i|
 |             |E|     |+| /\     |+| /\     |r| /\     |k|
 |   INTA# ----| |-----| |-  `----| |-  `----| |-  `----| |
 |             `-'     `-'        `-'        `-'        `-'
 |   Slot      0       10         11         12         13
 |   REQ#              0          1          2          *
 |   GNT#              0          1          2          *
 +-------------------------------------------------------------------------*/
	unsigned char int_line = 0xff;

	switch (PCI_DEV(dev)) {
	case 10:
		int_line = 31; /* INT A */
		POST_OUT(0x42);
		break;

	case 11:
		int_line = 30; /* INT B */
		POST_OUT(0x43);
		break;

	case 12:
		int_line = 29; /* INT C */
		POST_OUT(0x44);
		break;

	case 13:
		int_line = 28; /* INT D */
		POST_OUT(0x45);
		break;
	}
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, int_line);
}

extern void pci_405gp_init(struct pci_controller *hose);
extern void pci_405gp_fixup_irq(struct pci_controller *hose, pci_dev_t dev);
extern void pci_405gp_setup_bridge(struct pci_controller *hose, pci_dev_t dev,struct pci_config_table *entry);
/*
 * The following table is used when there is a special need to setup a PCI device.
 * For every PCI device found in this table is called the given init function with given
 * parameters. So never let all IDs at PCI_ANY_ID. In this case any found device gets the same
 * parameters!
 *
*/
static struct pci_config_table pci_solidcard3_config_table[] =
{
/* Host to PCI Bridge device (405GP) */
	{
		vendor: 0x1014,
		device: 0x0156,
		class: PCI_CLASS_BRIDGE_HOST,
		bus: 0,
		dev: 0,
		func: 0,
		config_device: pci_405gp_setup_bridge
	},
	{ }
};

/*-------------------------------------------------------------------------+
 | pci_init_board (Called from pci_init() in drivers/pci/pci.c)
 |
 | Init the PCI part of the SolidCard III
 |
 | Params:
 * - Pointer to current PCI hose
 * - Current Device
 *
 * Returns
 * nothing
 +-------------------------------------------------------------------------*/

void pci_init_board(void)
{
	POST_OUT(0x41);
/*
 * we want the ptrs to RAM not flash (ie don't use init list)
 */
	hose.fixup_irq    = pci_solidcard3_fixup_irq;
	hose.config_table = pci_solidcard3_config_table;
	pci_405gp_init(&hose);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
