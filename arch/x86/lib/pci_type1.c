/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Support for type PCI configuration cycles.
 * based on pci_indirect.c
 */
#include <common.h>
#include <asm/io.h>
#include <pci.h>

#define cfg_read(val, addr, op)		(*val = op((int)(addr)))
#define cfg_write(val, addr, op)	op((val), (int)(addr))

#define TYPE1_PCI_OP(rw, size, type, op, mask)				\
static int								\
type1_##rw##_config_##size(struct pci_controller *hose,			\
			      pci_dev_t dev, int offset, type val)	\
{									\
	outl(dev | (offset & 0xfc) | 0x80000000, (int)hose->cfg_addr);	\
	cfg_##rw(val, hose->cfg_data + (offset & mask), op);		\
	return 0;							\
}

TYPE1_PCI_OP(read, byte, u8 *, inb, 3)
TYPE1_PCI_OP(read, word, u16 *, inw, 2)
TYPE1_PCI_OP(read, dword, u32 *, inl, 0)

TYPE1_PCI_OP(write, byte, u8, outb, 3)
TYPE1_PCI_OP(write, word, u16, outw, 2)
TYPE1_PCI_OP(write, dword, u32, outl, 0)

/* bus mapping constants (used for PCI core initialization) */
#define PCI_REG_ADDR		0x00000cf8
#define PCI_REG_DATA		0x00000cfc

void pci_setup_type1(struct pci_controller *hose)
{
	pci_set_ops(hose,
		    type1_read_config_byte,
		    type1_read_config_word,
		    type1_read_config_dword,
		    type1_write_config_byte,
		    type1_write_config_word,
		    type1_write_config_dword);

	hose->cfg_addr = (unsigned int *)PCI_REG_ADDR;
	hose->cfg_data = (unsigned char *)PCI_REG_DATA;
}
