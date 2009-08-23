/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <pci.h>
#include <asm/pci.h>
#include <asm/ic/pci.h>

static void pci_enet_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	/* a configurable lists of IRQs to steal when we need one */
	static int irq_list[] = {
		CONFIG_SYS_FIRST_PCI_IRQ,
		CONFIG_SYS_SECOND_PCI_IRQ,
		CONFIG_SYS_THIRD_PCI_IRQ,
		CONFIG_SYS_FORTH_PCI_IRQ
	};
	static int next_irq_index=0;

	uchar tmp_pin;
	int pin;

	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &tmp_pin);
	pin = tmp_pin;

	pin -= 1; /* PCI config space use 1-based numbering */
	if (pin == -1) {
		return; /* device use no irq */
	}

	/* map device number +  pin to a pin on the sc520 */
	switch (PCI_DEV(dev)) {
	case 12:	/* First Ethernet Chip */
		pin += SC520_PCI_INTA;
		break;

	case 13:	/* Second Ethernet Chip */
		pin += SC520_PCI_INTB;
		break;

	default:
		return;
	}

	pin &= 3; /* wrap around */

	if (sc520_pci_ints[pin] == -1) {
		/* re-route one interrupt for us */
		if (next_irq_index > 3) {
			return;
		}
		if (pci_sc520_set_irq(pin, irq_list[next_irq_index])) {
			return;
		}
		next_irq_index++;
	}

	if (-1 != sc520_pci_ints[pin]) {
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE,
					   sc520_pci_ints[pin]);
	}
	printf("fixup_irq: device %d pin %c irq %d\n",
	       PCI_DEV(dev), 'A' + pin, sc520_pci_ints[pin]);
}

static struct pci_controller enet_hose = {
	fixup_irq: pci_enet_fixup_irq,
};

void pci_init_board(void)
{
	pci_sc520_init(&enet_hose);
}
