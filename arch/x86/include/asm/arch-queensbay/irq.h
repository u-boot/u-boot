/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ARCH_IRQ_H_
#define _ARCH_IRQ_H_

enum pci_int_pin {
	INTX,
	INTA,
	INTB,
	INTC,
	INTD
};

enum pirq_pin {
	PIRQA,
	PIRQB,
	PIRQC,
	PIRQD,
	PIRQE,
	PIRQF,
	PIRQG,
	PIRQH
};

/* PIRQ link number and value conversion */
#define LINK_V2N(link)	(link - 0x60)
#define LINK_N2V(link)	(link + 0x60)

#define PIRQ_BITMAP	0xdee0

struct irq_info;

/**
 * board_fill_irq_info() - Board-specific irq_info fill routine
 *
 * This fills the irq_info table for any board-specific add-in cards.
 *
 * @slot:	pointer to the struct irq_info that is to be filled in
 * @return:	number of entries were written to the struct irq_info
 */
int board_fill_irq_info(struct irq_info *slot);

/**
 * pirq_init() - Initialize platform PIRQ routing
 *
 * This initializes the PIRQ routing on the platform and configures all PCI
 * devices' interrupt line register to a working IRQ number on the 8259 PIC.
 */
void pirq_init(void);

#endif /* _ARCH_IRQ_H_ */
