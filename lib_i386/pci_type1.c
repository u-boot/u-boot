/*
 * Support for type PCI configuration cycles.
 * based on pci_indirect.c
 *
 * Copyright (C) 2002 Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <asm/io.h>
#include <pci.h>

#define cfg_read(val, addr, op)	*val = op((int)(addr))
#define cfg_write(val, addr, op)	op((val), (int)(addr))

#define TYPE1_PCI_OP(rw, size, type, op, mask)			 \
static int								 \
type1_##rw##_config_##size(struct pci_controller *hose, 		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	outl(dev | (offset & 0xfc) | 0x80000000, (int)hose->cfg_addr); 	 \
	cfg_##rw(val, hose->cfg_data + (offset & mask), op);	 \
	return 0;    					 		 \
}


TYPE1_PCI_OP(read, byte, u8 *, inb, 3)
TYPE1_PCI_OP(read, word, u16 *, inw, 2)
TYPE1_PCI_OP(read, dword, u32 *, inl, 0)

TYPE1_PCI_OP(write, byte, u8, outb, 3)
TYPE1_PCI_OP(write, word, u16, outw, 2)
TYPE1_PCI_OP(write, dword, u32, outl, 0)

void pci_setup_type1(struct pci_controller* hose, u32 cfg_addr, u32 cfg_data)
{
	pci_set_ops(hose,
		    type1_read_config_byte,
		    type1_read_config_word,
		    type1_read_config_dword,
		    type1_write_config_byte,
		    type1_write_config_word,
		    type1_write_config_dword);

	hose->cfg_addr = (unsigned int *) cfg_addr;
	hose->cfg_data = (unsigned char *) cfg_data;
}

#endif
