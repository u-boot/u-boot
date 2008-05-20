/* Plx9030.c - system configuration module for PLX9030 PCI to Local Bus Bridge */
/*
 * (C) Copyright 2002-2003
 * Josef Wagner, MicroSys GmbH, wagner@microsys.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *   Date       Modification                                      by
 * -------      ----------------------------------------------    ---
 * 30sep02      converted from VxWorks to LINUX                   wa
*/


/*
DESCRIPTION

This is the configuration module for the PLX9030 PCI to Local Bus Bridge.
It configures the Chip select lines for SRAM (CS0), ST16C552 (CS1,CS2), Display and local
registers (CS3) on CPC45.
*/

/* includes */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <pci.h>

/* imports */


/* defines */
#define	PLX9030_VENDOR_ID	0x10B5
#define	PLX9030_DEVICE_ID	0x9030

#undef PLX_DEBUG

/* PLX9030 register offsets  */
#define	P9030_LAS0RR	0x00
#define	P9030_LAS1RR	0x04
#define	P9030_LAS2RR	0x08
#define	P9030_LAS3RR	0x0c
#define	P9030_EROMRR	0x10
#define	P9030_LAS0BA	0x14
#define	P9030_LAS1BA	0x18
#define	P9030_LAS2BA	0x1c
#define	P9030_LAS3BA	0x20
#define	P9030_EROMBA	0x24
#define	P9030_LAS0BRD	0x28
#define	P9030_LAS1BRD	0x2c
#define	P9030_LAS2BRD	0x30
#define	P9030_LAS3BRD	0x34
#define	P9030_EROMBRD	0x38
#define	P9030_CS0BASE	0x3C
#define	P9030_CS1BASE	0x40
#define	P9030_CS2BASE	0x44
#define	P9030_CS3BASE	0x48
#define	P9030_INTCSR	0x4c
#define	P9030_CNTRL	0x50
#define	P9030_GPIOC	0x54

/* typedefs */


/* locals */

static struct pci_device_id supported[] = {
	{ PLX9030_VENDOR_ID, PLX9030_DEVICE_ID },
	{ }
};

/* forward declarations */
void sysOutLong(ulong address, ulong value);


/***************************************************************************
*
* Plx9030Init - init CS0..CS3 for CPC45
*
*
* RETURNS: N/A
*/

void Plx9030Init (void)
{
    pci_dev_t   devno;
    ulong	membaseCsr;	  /* base address of device memory space */
    int		idx = 0;	  /* general index */


    /* find plx9030 device */

    if ((devno = pci_find_devices(supported, idx++)) < 0)
    {
	printf("No PLX9030 device found !!\n");
	return;
    }


#ifdef PLX_DEBUG
	printf("PLX 9030 device found ! devno = 0x%x\n",devno);
#endif

	membaseCsr   = PCI_PLX9030_MEMADDR;

	/* set base address */
	pci_write_config_dword(devno, PCI_BASE_ADDRESS_0, membaseCsr);

	/* enable mapped memory and IO addresses */
	pci_write_config_dword(devno,
			       PCI_COMMAND,
			       PCI_COMMAND_MEMORY |
			       PCI_COMMAND_MASTER);


	/* configure GBIOC */
	sysOutLong((membaseCsr + P9030_GPIOC),   0x00000FC0);		/* CS2/CS3 enable */

	/* configure CS0 (SRAM) */
	sysOutLong((membaseCsr + P9030_LAS0BA),  0x00000001);		/* enable space base */
	sysOutLong((membaseCsr + P9030_LAS0RR),  0x0FE00000);		/* 2 MByte */
	sysOutLong((membaseCsr + P9030_LAS0BRD), 0x51928900);		/* 4 wait states */
	sysOutLong((membaseCsr + P9030_CS0BASE), 0x00100001);		/* enable 2 MByte */
	/* remap CS0 (SRAM) */
	pci_write_config_dword(devno, PCI_BASE_ADDRESS_2, SRAM_BASE);

	/* configure CS1 (ST16552 / CHAN A) */
	sysOutLong((membaseCsr + P9030_LAS1BA),  0x00400001);		/* enable space base */
	sysOutLong((membaseCsr + P9030_LAS1RR),  0x0FFFFF00);		/* 256 byte */
	sysOutLong((membaseCsr + P9030_LAS1BRD), 0x55122900);		/* 4 wait states */
	sysOutLong((membaseCsr + P9030_CS1BASE), 0x00400081);		/* enable 256 Byte */
	/* remap CS1 (ST16552 / CHAN A) */
	/* remap CS1 (ST16552 / CHAN A) */
	pci_write_config_dword(devno, PCI_BASE_ADDRESS_3, ST16552_A_BASE);

	/* configure CS2 (ST16552 / CHAN B) */
	sysOutLong((membaseCsr + P9030_LAS2BA),  0x00800001);		/* enable space base */
	sysOutLong((membaseCsr + P9030_LAS2RR),  0x0FFFFF00);		/* 256 byte */
	sysOutLong((membaseCsr + P9030_LAS2BRD), 0x55122900);		/* 4 wait states */
	sysOutLong((membaseCsr + P9030_CS2BASE), 0x00800081);		/* enable 256 Byte */
	/* remap CS2 (ST16552 / CHAN B) */
	pci_write_config_dword(devno, PCI_BASE_ADDRESS_4, ST16552_B_BASE);

	/* configure CS3 (BCSR) */
	sysOutLong((membaseCsr + P9030_LAS3BA),  0x00C00001);		/* enable space base */
	sysOutLong((membaseCsr + P9030_LAS3RR),  0x0FFFFF00);		/* 256 byte */
	sysOutLong((membaseCsr + P9030_LAS3BRD), 0x55357A80);		/* 9 wait states */
	sysOutLong((membaseCsr + P9030_CS3BASE), 0x00C00081);		/* enable 256 Byte */
	/* remap CS3 (DISPLAY and BCSR) */
	pci_write_config_dword(devno, PCI_BASE_ADDRESS_5, BCSR_BASE);
}

void sysOutLong(ulong address, ulong value)
{
	*(ulong*)address = cpu_to_le32(value);
}
