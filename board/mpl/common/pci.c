/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*
 * Adapted for PIP405 03.07.01
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * TODO: Clean-up
 */

#include <common.h>
#include <pci.h>
#include "isa.h"

#ifdef CONFIG_405GP
#ifdef CONFIG_PCI

DECLARE_GLOBAL_DATA_PTR;

#include "piix4_pci.h"
#include "pci_parts.h"

void pci_pip405_write_regs(struct pci_controller *hose, pci_dev_t dev,
			   struct pci_config_table *entry)
{
	struct pci_pip405_config_entry *table;
	int i;

	table = (struct pci_pip405_config_entry*) entry->priv[0];

	for (i=0; table[i].width; i++)
	{
#ifdef DEBUG
		printf("Reg 0x%02X Value 0x%08lX Width %02d written\n",
		       table[i].index, table[i].val, table[i].width);
#endif

		switch(table[i].width)
		{
		case 1: pci_hose_write_config_byte(hose, dev, table[i].index, table[i].val); break;
		case 2: pci_hose_write_config_word(hose, dev, table[i].index, table[i].val); break;
		case 4: pci_hose_write_config_dword(hose, dev, table[i].index, table[i].val); break;
		}
	}
}


static void pci_pip405_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char int_line = 0xff;
	unsigned char pin;
	/*
	 * Write pci interrupt line register
	 */
	if(PCI_DEV(dev)==0) /* Device0 = PPC405 -> skip */
		return;
	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &pin);
	if ((pin == 0) || (pin > 4))
	    return;

	int_line = ((PCI_DEV(dev) + (pin-1) + 10) % 4) + 28;
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, int_line);
#ifdef DEBUG
	printf("Fixup IRQ: dev %d (%x) int line %d 0x%x\n",
	       PCI_DEV(dev),dev,int_line,int_line);
#endif
}

extern void pci_405gp_init(struct pci_controller *hose);


static struct pci_controller hose = {
  config_table: pci_pip405_config_table,
  fixup_irq: pci_pip405_fixup_irq,
};


void pci_init_board(void)
{
	/*we want the ptrs to RAM not flash (ie don't use init list)*/
	hose.fixup_irq    = pci_pip405_fixup_irq;
	hose.config_table = pci_pip405_config_table;
#ifdef DEBUG
	printf("Init PCI: fixup_irq=%p config_table=%p hose=%p\n",pci_pip405_fixup_irq,pci_pip405_config_table,hose);
#endif
	pci_405gp_init(&hose);
}

#endif /* CONFIG_PCI */
#endif /* CONFIG_405GP */
