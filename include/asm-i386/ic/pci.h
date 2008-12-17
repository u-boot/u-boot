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
int pci_sc520_set_irq(int pci_pin, int irq);

#endif
