/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

#ifndef _ASM_IC_SC520_PCI_H_
#define _ASM_IC_SC520_PCI_H_ 1

/* bus mapping constants (used for PCI core initialization) */																																																 /* bus mapping constants */
#define SC520_REG_ADDR		0x00000cf8
#define SC520_REG_DATA		0x00000cfc

#define SC520_ISA_MEM_PHYS	0x00000000
#define SC520_ISA_MEM_BUS	0x00000000
#define SC520_ISA_MEM_SIZE	0x01000000

#define SC520_ISA_IO_PHYS	0x00000000
#define SC520_ISA_IO_BUS	0x00000000
#define SC520_ISA_IO_SIZE	0x00001000

/* PCI I/O space from 0x1000 to 0xdfff
 * (make 0xe000-0xfdff available for stuff like PCCard boot) */
#define SC520_PCI_IO_PHYS	0x00001000
#define SC520_PCI_IO_BUS	0x00001000
#define SC520_PCI_IO_SIZE	0x0000d000

/* system memory from 0x00000000 to 0x0fffffff */
#define	SC520_PCI_MEMORY_PHYS	0x00000000
#define	SC520_PCI_MEMORY_BUS	0x00000000
#define SC520_PCI_MEMORY_SIZE	0x10000000

/* PCI bus memory from 0x10000000 to 0x26ffffff
 * (make 0x27000000 - 0x27ffffff available for stuff like PCCard boot) */
#define SC520_PCI_MEM_PHYS	0x10000000
#define SC520_PCI_MEM_BUS	0x10000000
#define SC520_PCI_MEM_SIZE	0x17000000

/* pin number used for PCI interrupt mappings */
#define SC520_PCI_INTA 0
#define SC520_PCI_INTB 1
#define SC520_PCI_INTC 2
#define SC520_PCI_INTD 3
#define SC520_PCI_GPIRQ0 4
#define SC520_PCI_GPIRQ1 5
#define SC520_PCI_GPIRQ2 6
#define SC520_PCI_GPIRQ3 7
#define SC520_PCI_GPIRQ4 8
#define SC520_PCI_GPIRQ5 9
#define SC520_PCI_GPIRQ6 10
#define SC520_PCI_GPIRQ7 11
#define SC520_PCI_GPIRQ8 12
#define SC520_PCI_GPIRQ9 13
#define SC520_PCI_GPIRQ10 14

extern int sc520_pci_ints[];

void pci_sc520_init(struct pci_controller *hose);
int pci_set_regions(struct pci_controller *hose);
int pci_sc520_set_irq(int pci_pin, int irq);

#endif
