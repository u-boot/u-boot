/*
 * VR4131 PCIU support code for TANBAC Evaluation board TB0229.
 *
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include <common.h>
#include <pci.h>
#include <asm/addrspace.h>

#define VR4131_PCIMMAW1REG	(volatile unsigned int *)(CKSEG1 + 0x0f000c00)
#define VR4131_PCIMMAW2REG	(volatile unsigned int *)(CKSEG1 + 0x0f000c04)
#define VR4131_PCITAW1REG	(volatile unsigned int *)(CKSEG1 + 0x0f000c08)
#define VR4131_PCITAW2REG	(volatile unsigned int *)(CKSEG1 + 0x0f000c0c)
#define VR4131_PCIMIOAWREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c10)
#define VR4131_PCICONFDREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c14)
#define VR4131_PCICONFAREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c18)
#define VR4131_PCIMAILREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c1c)
#define VR4131_BUSERRADREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c24)
#define VR4131_INTCNTSTAREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c28)
#define VR4131_PCIEXACCREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c2c)
#define VR4131_PCIRECONTREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c30)
#define VR4131_PCIENREG		(volatile unsigned int *)(CKSEG1 + 0x0f000c34)
#define VR4131_PCICLKSELREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c38)
#define VR4131_PCITRDYREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c3c)
#define VR4131_PCICLKRUNREG	(volatile unsigned int *)(CKSEG1 + 0x0f000c60)
#define VR4131_PCIHOSTCONFIG	(volatile unsigned int *)(CKSEG1 + 0x0f000d00)
#define VR4131_VENDORIDREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d00)
#define VR4131_DEVICEIDREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d00)
#define VR4131_COMMANDREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d04)
#define VR4131_STATUSREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d04)
#define VR4131_REVREG		(volatile unsigned int *)(CKSEG1 + 0x0f000d08)
#define VR4131_CLASSREG		(volatile unsigned int *)(CKSEG1 + 0x0f000d08)
#define VR4131_CACHELSREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d0c)
#define VR4131_LATTIMERRG	(volatile unsigned int *)(CKSEG1 + 0x0f000d0c)
#define VR4131_MAILBAREG	(volatile unsigned int *)(CKSEG1 + 0x0f000d10)
#define VR4131_PCIMBA1REG	(volatile unsigned int *)(CKSEG1 + 0x0f000d14)
#define VR4131_PCIMBA2REG	(volatile unsigned int *)(CKSEG1 + 0x0f000d18)

/*#define VR41XX_PCIIRQ_OFFSET    (VR41XX_IRQ_MAX + 1)	*/
/*#define VR41XX_PCIIRQ_MAX       (VR41XX_IRQ_MAX + 12)	*/
/*#define VR4122_PCI_HOST_BASE    0xa0000000		*/

volatile unsigned int *pciconfigaddr;
volatile unsigned int *pciconfigdata;

#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

/*
 *	Access PCI Configuration Register for VR4131
 */

static int vr4131_pci_config_access (u8 access_type, u32 dev, u32 reg,
				     u32 * data)
{
	u32 bus;
	u32 device;

	bus = ((dev & 0xff0000) >> 16);
	device = ((dev & 0xf800) >> 11);

	if (bus == 0) {
		/* Type 0 Configuration */
		*VR4131_PCICONFAREG = (u32) (1UL << device | (reg & 0xfc));
	} else {
		/* Type 1 Configuration */
		*VR4131_PCICONFAREG = (u32) (dev | ((reg / 4) << 2) | 1);
	}

	if (access_type == PCI_ACCESS_WRITE) {
		*VR4131_PCICONFDREG = *data;
	} else {
		*data = *VR4131_PCICONFDREG;
	}

	return (0);
}

static int vr4131_pci_read_config_byte (u32 hose, u32 dev, u32 reg, u8 * val)
{
	u32 data;

	if (vr4131_pci_config_access (PCI_ACCESS_READ, dev, reg, &data))
		return -1;

	*val = (data >> ((reg & 3) << 3)) & 0xff;

	return 0;
}


static int vr4131_pci_read_config_word (u32 hose, u32 dev, u32 reg, u16 * val)
{
	u32 data;

	if (reg & 1)
		return -1;

	if (vr4131_pci_config_access (PCI_ACCESS_READ, dev, reg, &data))
		return -1;

	*val = (data >> ((reg & 3) << 3)) & 0xffff;

	return 0;
}


static int vr4131_pci_read_config_dword (u32 hose, u32 dev, u32 reg,
					 u32 * val)
{
	u32 data = 0;

	if (reg & 3)
		return -1;

	if (vr4131_pci_config_access (PCI_ACCESS_READ, dev, reg, &data))
		return -1;

	*val = data;

	return (0);
}

static int vr4131_pci_write_config_byte (u32 hose, u32 dev, u32 reg, u8 val)
{
	u32 data = 0;

	if (vr4131_pci_config_access (PCI_ACCESS_READ, dev, reg, &data))
		return -1;

	data = (data & ~(0xff << ((reg & 3) << 3))) | (val <<
						       ((reg & 3) << 3));

	if (vr4131_pci_config_access (PCI_ACCESS_WRITE, dev, reg, &data))
		return -1;

	return 0;
}


static int vr4131_pci_write_config_word (u32 hose, u32 dev, u32 reg, u16 val)
{
	u32 data = 0;

	if (reg & 1)
		return -1;

	if (vr4131_pci_config_access (PCI_ACCESS_READ, dev, reg, &data))
		return -1;

	data = (data & ~(0xffff << ((reg & 3) << 3))) | (val <<
							 ((reg & 3) << 3));

	if (vr4131_pci_config_access (PCI_ACCESS_WRITE, dev, reg, &data))
		return -1;

	return 0;
}

static int vr4131_pci_write_config_dword (u32 hose, u32 dev, u32 reg, u32 val)
{
	u32 data;

	if (reg & 3) {
		return -1;
	}

	data = val;

	if (vr4131_pci_config_access (PCI_ACCESS_WRITE, dev, reg, &data))
		return -1;

	return (0);
}


/*
 *	Initialize VR4131 PCIU
 */

vr4131_pciu_init ()
{
	/* PCI clock */
	*VR4131_PCICLKSELREG = 0x00000002;

	/* PCI memory and I/O space */
	*VR4131_PCIMMAW1REG = 0x100F9010;
	*VR4131_PCIMMAW2REG = 0x140FD014;
	*VR4131_PCIMIOAWREG = 0x160FD000;

	/* Target memory window */
	*VR4131_PCITAW1REG = 0x00081000;	/* 64MB */
	*VR4131_PCITAW2REG = 0x00000000;

	*VR4131_MAILBAREG = 0UL;
	*VR4131_PCIMBA1REG = 0UL;

	*VR4131_PCITRDYREG = 0x00008004;

	*VR4131_PCIENREG = 0x00000004;	/* PCI enable */
	*VR4131_COMMANDREG = 0x02000007;
}

/*
 *	Initialize Module
 */

void init_vr4131_pci (struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	vr4131_pciu_init ();	/* Initialize VR4131 PCIU */

	/* PCI memory space #1 */
	pci_set_region (hose->regions + 0,
			0x10000000, 0xb0000000, 0x04000000, PCI_REGION_MEM);

	/* PCI memory space #2 */
	pci_set_region (hose->regions + 1,
			0x14000000, 0xb4000000, 0x02000000, PCI_REGION_MEM);


	/* PCI I/O space */
	pci_set_region (hose->regions + 2,
			0x16000000, 0xb6000000, 0x02000000, PCI_REGION_IO);

	/* System memory space */
	pci_set_region (hose->regions + 3,
			0x00000000,
			0x80000000,
			0x04000000, PCI_REGION_MEM | PCI_REGION_MEMORY);

	hose->region_count = 4;

	hose->read_byte = vr4131_pci_read_config_byte;
	hose->read_word = vr4131_pci_read_config_word;
	hose->read_dword = vr4131_pci_read_config_dword;
	hose->write_byte = vr4131_pci_write_config_byte;
	hose->write_word = vr4131_pci_write_config_word;
	hose->write_dword = vr4131_pci_write_config_dword;

	pci_register_hose (hose);

	hose->last_busno = pci_hose_scan (hose);

	return;
}
