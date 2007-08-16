/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
 */

/*
 * PCI Configuration space access support
 */
#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/immap.h>

#if defined(CONFIG_PCI)
/* System RAM mapped over PCI */
#define CFG_PCI_SYS_MEM_BUS	CFG_SDRAM_BASE
#define CFG_PCI_SYS_MEM_PHYS	CFG_SDRAM_BASE
#define CFG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define cfg_read(val, addr, type, op)		*val = op((type)(addr));
#define cfg_write(val, addr, type, op)		op((type *)(addr), (val));

#define PCI_OP(rw, size, type, op, mask)				\
int pci_##rw##_cfg_##size(struct pci_controller *hose,			\
	pci_dev_t dev, int offset, type val)				\
{									\
	u32 addr = 0;							\
	u16 cfg_type = 0;						\
	addr = ((offset & 0xfc) | cfg_type | (dev)  | 0x80000000);	\
	out_be32(hose->cfg_addr, addr);					\
	__asm__ __volatile__("nop");					\
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	\
	out_be32(hose->cfg_addr, addr & 0x7fffffff);			\
	__asm__ __volatile__("nop");					\
	return 0;							\
}

PCI_OP(read, byte, u8 *, in_8, 3)
PCI_OP(read, word, u16 *, in_le16, 2)
PCI_OP(write, byte, u8, out_8, 3)
PCI_OP(write, word, u16, out_le16, 2)
PCI_OP(write, dword, u32, out_le32, 0)

int pci_read_cfg_dword(struct pci_controller *hose, pci_dev_t dev,
		       int offset, u32 * val)
{
	u32 addr;
	u32 tmpv;
	u32 mask = 2;		/* word access */
	/* Read lower 16 bits */
	addr = ((offset & 0xfc) | (dev) | 0x80000000);
	out_be32(hose->cfg_addr, addr);
	__asm__ __volatile__("nop");
	*val = (u32) in_le16((u16 *) (hose->cfg_data + (offset & mask)));
	out_be32(hose->cfg_addr, addr & 0x7fffffff);
	__asm__ __volatile__("nop");

	/* Read upper 16 bits */
	offset += 2;
	addr = ((offset & 0xfc) | 1 | (dev) | 0x80000000);
	out_be32(hose->cfg_addr, addr);
	__asm__ __volatile__("nop");
	tmpv = (u32) in_le16((u16 *) (hose->cfg_data + (offset & mask)));
	out_be32(hose->cfg_addr, addr & 0x7fffffff);
	__asm__ __volatile__("nop");

	/* combine results into dword value */
	*val = (tmpv << 16) | *val;

	return 0;
}

void pci_mcf5445x_init(struct pci_controller *hose)
{
	volatile pci_t *pci = (volatile pci_t *)MMAP_PCI;
	volatile pciarb_t *pciarb = (volatile pciarb_t *)MMAP_PCIARB;
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	u32 barEn = 0;

	pciarb->acr = 0x001f001f;

	/* Set PCIGNT1, PCIREQ1, PCIREQ0/PCIGNTIN, PCIGNT0/PCIREQOUT,
	   PCIREQ2, PCIGNT2 */
	gpio->par_pci =
	    GPIO_PAR_PCI_GNT3_GNT3 | GPIO_PAR_PCI_GNT2 | GPIO_PAR_PCI_GNT1 |
	    GPIO_PAR_PCI_GNT0 | GPIO_PAR_PCI_REQ3_REQ3 | GPIO_PAR_PCI_REQ2 |
	    GPIO_PAR_PCI_REQ1 | GPIO_PAR_PCI_REQ0;

	pci->tcr1 |= PCI_TCR1_P;

	/* Initiator windows */
	pci->iw0btar = CFG_PCI_MEM_PHYS;
	pci->iw1btar = CFG_PCI_IO_PHYS;
	pci->iw2btar = CFG_PCI_CFG_PHYS;

	pci->iwcr =
	    PCI_IWCR_W0C_EN | PCI_IWCR_W1C_EN | PCI_IWCR_W1C_IO |
	    PCI_IWCR_W2C_EN | PCI_IWCR_W2C_IO;

	/* Enable bus master and mem access */
	pci->scr = PCI_SCR_MW | PCI_SCR_B | PCI_SCR_M;

	/* Cache line size and master latency */
	pci->cr1 = PCI_CR1_CLS(8) | PCI_CR1_LTMR(0xFF);
	pci->cr2 = 0;

#ifdef CFG_PCI_BAR0
	pci->bar0 = PCI_BAR_BAR0(CFG_PCI_BAR0);
	pci->tbatr0 = CFG_PCI_TBATR0 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B0E;
#endif
#ifdef CFG_PCI_BAR1
	pci->bar1 = PCI_BAR_BAR1(CFG_PCI_BAR1);
	pci->tbatr1 = CFG_PCI_TBATR1 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B1E;
#endif
#ifdef CFG_PCI_BAR2
	pci->bar2 = PCI_BAR_BAR2(CFG_PCI_BAR2);
	pci->tbatr2 = CFG_PCI_TBATR2 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B2E;
#endif
#ifdef CFG_PCI_BAR3
	pci->bar3 = PCI_BAR_BAR3(CFG_PCI_BAR3);
	pci->tbatr3 = CFG_PCI_TBATR3 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B3E;
#endif
#ifdef CFG_PCI_BAR4
	pci->bar4 = PCI_BAR_BAR4(CFG_PCI_BAR4);
	pci->tbatr4 = CFG_PCI_TBATR4 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B4E;
#endif
#ifdef CFG_PCI_BAR5
	pci->bar5 = PCI_BAR_BAR5(CFG_PCI_BAR5);
	pci->tbatr5 = CFG_PCI_TBATR5 | PCI_TBATR_EN;
	barEn |= PCI_TCR1_B5E;
#endif

	pci->tcr2 = barEn;

	/* Deassert reset bit */
	pci->gscr &= ~PCI_GSCR_PR;
	udelay(1000);

	/* Enable PCI bus master support */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_set_region(hose->regions + 0, CFG_PCI_MEM_BUS, CFG_PCI_MEM_PHYS,
		       CFG_PCI_MEM_SIZE, PCI_REGION_MEM);

	pci_set_region(hose->regions + 1, CFG_PCI_IO_BUS, CFG_PCI_IO_PHYS,
		       CFG_PCI_IO_SIZE, PCI_REGION_IO);

	pci_set_region(hose->regions + 2, CFG_PCI_SYS_MEM_BUS,
		       CFG_PCI_SYS_MEM_PHYS, CFG_PCI_SYS_MEM_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	hose->region_count = 3;

	hose->cfg_addr = &(pci->car);
	hose->cfg_data = (volatile unsigned char *)CFG_PCI_CFG_BUS;

	pci_set_ops(hose, pci_read_cfg_byte, pci_read_cfg_word,
		    pci_read_cfg_dword, pci_write_cfg_byte, pci_write_cfg_word,
		    pci_write_cfg_dword);

	/* Hose scan */
	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
}
#endif				/* CONFIG_PCI */
