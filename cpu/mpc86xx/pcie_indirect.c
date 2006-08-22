/*
 * Support for indirect PCI bridges.
 *
 * Copyright (c) Freescale Semiconductor, Inc.
 * 2006. All rights reserved.
 *
 * Jason Jin <Jason.jin@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * partly derived from
 * arch/powerpc/platforms/86xx/mpc86xx_pcie.c
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define PCI_CFG_OUT 	out_be32
#define PEX_FIX		out_be32(hose->cfg_addr+0x4, 0x0400ffff)

static int
indirect_read_config_pcie(struct pci_controller *hose,
			  pci_dev_t dev,
			  int offset,
			  int len,
			  u32 *val)
{
	int bus = PCI_BUS(dev);

	volatile unsigned char *cfg_data;
	u32 temp;

	PEX_FIX;
	if (bus == 0xff) {
		PCI_CFG_OUT(hose->cfg_addr,
			    dev | (offset & 0xfc) | 0x80000001);
	} else {
		PCI_CFG_OUT(hose->cfg_addr,
			    dev | (offset & 0xfc) | 0x80000000);
	}
	/*
	 * Note: the caller has already checked that offset is
	 * suitably aligned and that len is 1, 2 or 4.
	 */
	/* ERRATA PCI-Ex 12 - Configuration Address/Data Alignment */
	cfg_data = hose->cfg_data;
	PEX_FIX;
	temp = in_le32((u32 *) cfg_data);
	switch (len) {
	case 1:
		*val = (temp >> (((offset & 3)) * 8)) & 0xff;
		break;
	case 2:
		*val = (temp >> (((offset & 3)) * 8)) & 0xffff;
		break;
	default:
		*val = temp;
		break;
	}

	return 0;
}

static int
indirect_write_config_pcie(struct pci_controller *hose,
			   pci_dev_t dev,
			   int offset,
			   int len,
			   u32 val)
{
	int bus = PCI_BUS(dev);
	volatile unsigned char *cfg_data;
	u32 temp;

	PEX_FIX;
	if (bus == 0xff) {
		PCI_CFG_OUT(hose->cfg_addr,
			    dev | (offset & 0xfc) | 0x80000001);
	} else {
		PCI_CFG_OUT(hose->cfg_addr,
			    dev | (offset & 0xfc) | 0x80000000);
	}

	/*
	 * Note: the caller has already checked that offset is
	 * suitably aligned and that len is 1, 2 or 4.
	 */
	/* ERRATA PCI-Ex 12 - Configuration Address/Data Alignment */
	cfg_data = hose->cfg_data;
	switch (len) {
	case 1:
		PEX_FIX;
		temp = in_le32((u32 *) cfg_data);
		temp = (temp & ~(0xff << ((offset & 3) * 8))) |
		    (val << ((offset & 3) * 8));
		PEX_FIX;
		out_le32((u32 *) cfg_data, temp);
		break;
	case 2:
		PEX_FIX;
		temp = in_le32((u32 *) cfg_data);
		temp = (temp & ~(0xffff << ((offset & 3) * 8)));
		temp |= (val << ((offset & 3) * 8));
		PEX_FIX;
		out_le32((u32 *) cfg_data, temp);
		break;
	default:
		PEX_FIX;
		out_le32((u32 *) cfg_data, val);
		break;
	}
	PEX_FIX;
	return 0;
}

static int
indirect_read_config_byte_pcie(struct pci_controller *hose,
			       pci_dev_t dev,
			       int offset,
			       u8 *val)
{
	u32 val32;
	indirect_read_config_pcie(hose, dev, offset, 1, &val32);
	*val = (u8) val32;
	return 0;
}

static int
indirect_read_config_word_pcie(struct pci_controller *hose,
			       pci_dev_t dev,
			       int offset,
			       u16 *val)
{
	u32 val32;
	indirect_read_config_pcie(hose, dev, offset, 2, &val32);
	*val = (u16) val32;
	return 0;
}

static int
indirect_read_config_dword_pcie(struct pci_controller *hose,
				pci_dev_t dev,
				int offset,
				u32 *val)
{
	return indirect_read_config_pcie(hose, dev, offset, 4, val);
}

static int
indirect_write_config_byte_pcie(struct pci_controller *hose,
				pci_dev_t dev,
				int offset,
				u8 val)
{
	return indirect_write_config_pcie(hose, dev, offset, 1, (u32) val);
}

static int
indirect_write_config_word_pcie(struct pci_controller *hose,
				pci_dev_t dev,
				int offset,
				unsigned short val)
{
	return indirect_write_config_pcie(hose, dev, offset, 2, (u32) val);
}

static int
indirect_write_config_dword_pcie(struct pci_controller *hose,
				 pci_dev_t dev,
				 int offset,
				 u32 val)
{
	return indirect_write_config_pcie(hose, dev, offset, 4, val);
}

void
pcie_setup_indirect(struct pci_controller *hose, u32 cfg_addr, u32 cfg_data)
{
	pci_set_ops(hose,
		    indirect_read_config_byte_pcie,
		    indirect_read_config_word_pcie,
		    indirect_read_config_dword_pcie,
		    indirect_write_config_byte_pcie,
		    indirect_write_config_word_pcie,
		    indirect_write_config_dword_pcie);

	hose->cfg_addr = (unsigned int *)cfg_addr;
	hose->cfg_data = (unsigned char *)cfg_data;
}

#endif				/* CONFIG_PCI */
