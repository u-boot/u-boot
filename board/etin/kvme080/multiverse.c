/*
 * multiverse.c
 *
 * VME driver for Multiverse
 *
 * Author : Sangmoon Kim
 *	    dogoil@etinsys.com
 *
 * Copyright 2005 ETIN SYSTEMS Co.,Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <common.h>
#include <asm/io.h>
#include <pci.h>

#include "multiverse.h"

static unsigned long vme_asi_addr;
static unsigned long vme_iack_addr;
static unsigned long pci_reg_addr;
static unsigned long vme_reg_addr;

int multiv_reset(unsigned long base)
{
	writeb(0x09, base + VME_SLAVE32_AM);
	writeb(0x39, base + VME_SLAVE24_AM);
	writeb(0x29, base + VME_SLAVE16_AM);
	writeb(0x2f, base + VME_SLAVE_REG_AM);
	writeb((VME_A32_SLV_BUS >> 24) & 0xff, base + VME_SLAVE32_A);
	writeb((VME_A24_SLV_BUS >> 16) & 0xff, base + VME_SLAVE24_A);
	writeb((VME_A16_SLV_BUS >> 8 ) & 0xff, base + VME_SLAVE16_A);
#ifdef A32_SLV_WINDOW
	if (readb(base + VME_STATUS) & VME_STATUS_SYSCON) {
		writeb(((~(VME_A32_SLV_SIZE-1)) >> 24) & 0xff,
				base + VME_SLAVE32_MASK);
		writeb(0x01, base + VME_SLAVE32_EN);
	} else {
		writeb(0xff, base + VME_SLAVE32_MASK);
		writeb(0x00, base + VME_SLAVE32_EN);
	}
#else
	writeb(0xff, base + VME_SLAVE32_MASK);
	writeb(0x00, base + VME_SLAVE32_EN);
#endif
#ifdef A24_SLV_WINDOW
	if (readb(base + VME_STATUS) & VME_STATUS_SYSCON) {
		writeb(((~(VME_A24_SLV_SIZE-1)) >> 16) & 0xff,
				base + VME_SLAVE24_MASK);
		writeb(0x01, base + VME_SLAVE24_EN);
	} else {
		writeb(0xff, base + VME_SLAVE24_MASK);
		writeb(0x00, base + VME_SLAVE24_EN);
	}
#else
	writeb(0xff, base + VME_SLAVE24_MASK);
	writeb(0x00, base + VME_SLAVE24_EN);
#endif
#ifdef A16_SLV_WINDOW
	if (readb(base + VME_STATUS) & VME_STATUS_SYSCON) {
		writeb(((~(VME_A16_SLV_SIZE-1)) >> 8) & 0xff,
				base + VME_SLAVE16_MASK);
		writeb(0x01, base + VME_SLAVE16_EN);
	} else {
		writeb(0xff, base + VME_SLAVE16_MASK);
		writeb(0x00, base + VME_SLAVE16_EN);
	}
#else
	writeb(0xff, base + VME_SLAVE16_MASK);
	writeb(0x00, base + VME_SLAVE16_EN);
#endif
#ifdef REG_SLV_WINDOW
	if (readb(base + VME_STATUS) & VME_STATUS_SYSCON) {
		writeb(((~(VME_REG_SLV_SIZE-1)) >> 16) & 0xff,
				base + VME_SLAVE_REG_MASK);
		writeb(0x01, base + VME_SLAVE_REG_EN);
	} else {
		writeb(0xf8, base + VME_SLAVE_REG_MASK);
	}
#else
	writeb(0xf8, base + VME_SLAVE_REG_MASK);
#endif
	writeb(0x09, base + VME_MASTER32_AM);
	writeb(0x39, base + VME_MASTER24_AM);
	writeb(0x29, base + VME_MASTER16_AM);
	writeb(0x2f, base + VME_MASTER_REG_AM);
	writel(0x00000000, base + VME_RMW_ADRS);
	writeb(0x00, base + VME_IRQ);
	writeb(0x00, base + VME_INT_EN);
	writel(0x00000000, base + VME_IRQ1_REG);
	writel(0x00000000, base + VME_IRQ2_REG);
	writel(0x00000000, base + VME_IRQ3_REG);
	writel(0x00000000, base + VME_IRQ4_REG);
	writel(0x00000000, base + VME_IRQ5_REG);
	writel(0x00000000, base + VME_IRQ6_REG);
	writel(0x00000000, base + VME_IRQ7_REG);
	return 0;
}

void multiv_auto_slot_id(unsigned long base)
{
	unsigned int vector;
	int slot_id = 1;
	if (readb(base + VME_CTRL) & VME_CTRL_SYSFAIL) {
		*(volatile unsigned int*)(base + VME_IRQ2_REG) = 0xfe;
		writeb(readb(base + VME_IRQ) | 0x04, base + VME_IRQ);
		writeb(readb(base + VME_CTRL) & ~VME_CTRL_SYSFAIL,
				base + VME_CTRL);
		while (readb(base + VME_STATUS) & VME_STATUS_SYSFAIL);
		if (readb(base + VME_STATUS) & VME_STATUS_SYSCON) {
			while (readb(base + VME_INT) & 0x04) {
				vector = *(volatile unsigned int*)
					(vme_iack_addr + VME_IACK2);
				*(unsigned char*)(vme_asi_addr + 0x7ffff)
					= (slot_id << 3) & 0xff;
				slot_id ++;
				if (slot_id > 31)
					break;
			}
		}
	}
}

int multiverse_init(void)
{
	int i;
	pci_dev_t pdev;
	unsigned int bar[6];

	pdev = pci_find_device(0x1895, 0x0001, 0);

	if (pdev == 0)
		return -1;

	for (i = 0; i < 6; i++)
		pci_read_config_dword (pdev,
				PCI_BASE_ADDRESS_0 + i * 4, &bar[i]);

	pci_reg_addr = bar[0];
	vme_reg_addr = bar[1] + 0x00F00000;
	vme_iack_addr = bar[1] + 0x00200000;
	vme_asi_addr = bar[3];

	pci_write_config_dword (pdev, PCI_COMMAND,
		PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	writel(0xFF000000, pci_reg_addr + P_TA1);
	writel(0x04, pci_reg_addr + P_IMG_CTRL1);
	writel(0xf0000000, pci_reg_addr + P_TA2);
	writel(0x04, pci_reg_addr + P_IMG_CTRL2);
	writel(0xF1000000, pci_reg_addr + P_TA3);
	writel(0x04, pci_reg_addr + P_IMG_CTRL3);
	writel(VME_A32_MSTR_BUS, pci_reg_addr + P_TA5);
	writel(~(VME_A32_MSTR_SIZE-1), pci_reg_addr + P_AM5);
	writel(0x04, pci_reg_addr + P_IMG_CTRL5);

	writel(VME_A32_SLV_BUS, pci_reg_addr + W_BA1);
	writel(~(VME_A32_SLV_SIZE-1), pci_reg_addr + W_AM1);
	writel(VME_A32_SLV_LOCAL, pci_reg_addr + W_TA1);
	writel(0x04, pci_reg_addr + W_IMG_CTRL1);

	writel(0xF0000000, pci_reg_addr + W_BA2);
	writel(0xFF000000, pci_reg_addr + W_AM2);
	writel(VME_A24_SLV_LOCAL, pci_reg_addr + W_TA2);
	writel(0x04, pci_reg_addr + W_IMG_CTRL2);

	writel(0xFF000000, pci_reg_addr + W_BA3);
	writel(0xFF000000, pci_reg_addr + W_AM3);
	writel(VME_A16_SLV_LOCAL, pci_reg_addr + W_TA3);
	writel(0x04, pci_reg_addr + W_IMG_CTRL3);

	writel(0x00000001, pci_reg_addr + W_ERR_CS);
	writel(0x00000001, pci_reg_addr + P_ERR_CS);

	multiv_reset(vme_reg_addr);
	writeb(readb(vme_reg_addr + VME_CTRL) | VME_CTRL_SHORT_D,
		vme_reg_addr + VME_CTRL);

	multiv_auto_slot_id(vme_reg_addr);

	return 0;
}
