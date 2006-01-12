/*
 * Support for indirect PCI bridges.
 *
 * Copyright (C) 1998 Gabriel Paubert.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <common.h>

#ifdef CONFIG_PCI
#if (!defined(__I386__) && !defined(CONFIG_IXDP425))

#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define cfg_read(val, addr, type, op)	*val = op((type)(addr))
#define cfg_write(val, addr, type, op)	op((type *)(addr), (val))

#ifdef CONFIG_IXP425
extern unsigned char	in_8 (volatile unsigned *addr);
extern unsigned short	in_le16 (volatile unsigned *addr);
extern unsigned		in_le32 (volatile unsigned *addr);
extern void		out_8 (volatile unsigned *addr, char val);
extern void		out_le16 (volatile unsigned *addr, unsigned short val);
extern void		out_le32 (volatile unsigned *addr, unsigned int val);
#endif	/* CONFIG_IXP425 */

#if defined(CONFIG_MPC8260)
#define INDIRECT_PCI_OP(rw, size, type, op, mask)			 \
static int								 \
indirect_##rw##_config_##size(struct pci_controller *hose, 		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000000); 	 \
	sync();								 \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	 \
	return 0;    					 		 \
}
#elif defined(CONFIG_E500)
#define INDIRECT_PCI_OP(rw, size, type, op, mask)                        \
static int                                                               \
indirect_##rw##_config_##size(struct pci_controller *hose,               \
			      pci_dev_t dev, int offset, type val)       \
{                                                                        \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	*(hose->cfg_addr) = dev | (offset & 0xfc) | 0x80000000;          \
	sync();                                                          \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);       \
	return 0;                                                        \
}
#elif defined(CONFIG_440GX) || defined(CONFIG_440EP) || defined(CONFIG_440GR)
#define INDIRECT_PCI_OP(rw, size, type, op, mask)			 \
static int								 \
indirect_##rw##_config_##size(struct pci_controller *hose, 		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	if (PCI_BUS(dev) > 0)                                            \
		out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000001); \
	else                                                             \
		out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000000); \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	 \
	return 0;    					 		 \
}
#else
#define INDIRECT_PCI_OP(rw, size, type, op, mask)			 \
static int								 \
indirect_##rw##_config_##size(struct pci_controller *hose, 		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	u32 b, d,f;							 \
	b = PCI_BUS(dev); d = PCI_DEV(dev); f = PCI_FUNC(dev);		 \
	b = b - hose->first_busno;					 \
	dev = PCI_BDF(b, d, f);						 \
	out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000000); 	 \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	 \
	return 0;    					 		 \
}
#endif

#define INDIRECT_PCI_OP_ERRATA6(rw, size, type, op, mask)		 \
static int								 \
indirect_##rw##_config_##size(struct pci_controller *hose, 		 \
			      pci_dev_t dev, int offset, type val)	 \
{									 \
	unsigned int msr = mfmsr();					 \
	mtmsr(msr & ~(MSR_EE | MSR_CE));				 \
	out_le32(hose->cfg_addr, dev | (offset & 0xfc) | 0x80000000); 	 \
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	 \
	out_le32(hose->cfg_addr, 0x00000000); 				 \
	mtmsr(msr);							 \
	return 0;    					 		 \
}

INDIRECT_PCI_OP(read, byte, u8 *, in_8, 3)
INDIRECT_PCI_OP(read, word, u16 *, in_le16, 2)
INDIRECT_PCI_OP(read, dword, u32 *, in_le32, 0)
#ifdef CONFIG_405GP
INDIRECT_PCI_OP_ERRATA6(write, byte, u8, out_8, 3)
INDIRECT_PCI_OP_ERRATA6(write, word, u16, out_le16, 2)
INDIRECT_PCI_OP_ERRATA6(write, dword, u32, out_le32, 0)
#else
INDIRECT_PCI_OP(write, byte, u8, out_8, 3)
INDIRECT_PCI_OP(write, word, u16, out_le16, 2)
INDIRECT_PCI_OP(write, dword, u32, out_le32, 0)
#endif

void pci_setup_indirect(struct pci_controller* hose, u32 cfg_addr, u32 cfg_data)
{
	pci_set_ops(hose,
		    indirect_read_config_byte,
		    indirect_read_config_word,
		    indirect_read_config_dword,
		    indirect_write_config_byte,
		    indirect_write_config_word,
		    indirect_write_config_dword);

	hose->cfg_addr = (unsigned int *) cfg_addr;
	hose->cfg_data = (unsigned char *) cfg_data;
}

#endif	/* !__I386__ && !CONFIG_IXDP425 */
#endif	/* CONFIG_PCI */
