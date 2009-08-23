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

/* stuff specific for the sc520, but independent of implementation */

#include <common.h>
#include <pci.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>

static struct {
	u8 priority;
	u16 level_reg;
	u8 level_bit;
} sc520_irq[] = {
	{ SC520_IRQ0,  0, 0x01 },
	{ SC520_IRQ1,  0, 0x02 },
	{ SC520_IRQ2,  1, 0x02 },
	{ SC520_IRQ3,  0, 0x08 },
	{ SC520_IRQ4,  0, 0x10 },
	{ SC520_IRQ5,  0, 0x20 },
	{ SC520_IRQ6,  0, 0x40 },
	{ SC520_IRQ7,  0, 0x80 },

	{ SC520_IRQ8,  1, 0x01 },
	{ SC520_IRQ9,  1, 0x02 },
	{ SC520_IRQ10, 1, 0x04 },
	{ SC520_IRQ11, 1, 0x08 },
	{ SC520_IRQ12, 1, 0x10 },
	{ SC520_IRQ13, 1, 0x20 },
	{ SC520_IRQ14, 1, 0x40 },
	{ SC520_IRQ15, 1, 0x80 }
};


/* The interrupt used for PCI INTA-INTD  */
int sc520_pci_ints[15] = {
	-1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1
};

/* utility function to configure a pci interrupt */
int pci_sc520_set_irq(int pci_pin, int irq)
{
	int i;

# if 1
	printf("set_irq(): map INT%c to IRQ%d\n", pci_pin + 'A', irq);
#endif
	if (irq < 0 || irq > 15) {
		return -1; /* illegal irq */
	}

	if (pci_pin < 0 || pci_pin > 15) {
		return -1; /* illegal pci int pin */
	}

	/* first disable any non-pci interrupt source that use
	 * this level */

	/* PCI interrupt mapping (A through D)*/
	for (i=0; i<=3 ;i++) {
		if (sc520_mmcr->pci_int_map[i] == sc520_irq[irq].priority)
			sc520_mmcr->pci_int_map[i] = SC520_IRQ_DISABLED;
	}

	/* GP IRQ interrupt mapping */
	for (i=0; i<=10 ;i++) {
		if (sc520_mmcr->gp_int_map[i] == sc520_irq[irq].priority)
			sc520_mmcr->gp_int_map[i] = SC520_IRQ_DISABLED;
	}

	/* Set the trigger to level */
	sc520_mmcr->pic_mode[sc520_irq[irq].level_reg] =
		sc520_mmcr->pic_mode[sc520_irq[irq].level_reg] | sc520_irq[irq].level_bit;


	if (pci_pin < 4) {
		/* PCI INTA-INTD */
		/* route the interrupt */
		sc520_mmcr->pci_int_map[pci_pin] = sc520_irq[irq].priority;
	} else {
		/* GPIRQ0-GPIRQ10 used for additional PCI INTS */
		sc520_mmcr->gp_int_map[pci_pin - 4] = sc520_irq[irq].priority;

		/* also set the polarity in this case */
		sc520_mmcr->intpinpol = sc520_mmcr->intpinpol | (1 << (pci_pin-4));
	}

	/* register the pin */
	sc520_pci_ints[pci_pin] = irq;


	return 0; /* OK */
}

void pci_sc520_init(struct pci_controller *hose)
{
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region(hose->regions + 0,
		       SC520_PCI_MEMORY_BUS,
		       SC520_PCI_MEMORY_PHYS,
		       SC520_PCI_MEMORY_SIZE,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       SC520_PCI_MEM_BUS,
		       SC520_PCI_MEM_PHYS,
		       SC520_PCI_MEM_SIZE,
		       PCI_REGION_MEM);

	/* ISA/PCI memory space */
	pci_set_region(hose->regions + 2,
		       SC520_ISA_MEM_BUS,
		       SC520_ISA_MEM_PHYS,
		       SC520_ISA_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region(hose->regions + 3,
		       SC520_PCI_IO_BUS,
		       SC520_PCI_IO_PHYS,
		       SC520_PCI_IO_SIZE,
		       PCI_REGION_IO);

	/* ISA/PCI I/O space */
	pci_set_region(hose->regions + 4,
		       SC520_ISA_IO_BUS,
		       SC520_ISA_IO_PHYS,
		       SC520_ISA_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 5;

	pci_setup_type1(hose,
			SC520_REG_ADDR,
			SC520_REG_DATA);

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);

	/* enable target memory acceses on host brige */
	pci_write_config_word(0, PCI_COMMAND,
			      PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

}
