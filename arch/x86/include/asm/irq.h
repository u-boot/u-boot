/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ARCH_IRQ_H_
#define _ARCH_IRQ_H_

#include <dt-bindings/interrupt-router/intel-irq.h>

/**
 * Intel interrupt router configuration mechanism
 *
 * There are two known ways of Intel interrupt router configuration mechanism
 * so far. On most cases, the IRQ routing configuraiton is controlled by PCI
 * configuraiton registers on the legacy bridge, normally PCI BDF(0, 31, 0).
 * On some newer platforms like BayTrail and Braswell, the IRQ routing is now
 * in the IBASE register block where IBASE is memory-mapped.
 */
enum pirq_config {
	PIRQ_VIA_PCI,
	PIRQ_VIA_IBASE
};

/**
 * Intel interrupt router control block
 *
 * Its members' value will be filled in based on device tree's input.
 *
 * @config:	PIRQ_VIA_PCI or PIRQ_VIA_IBASE
 * @link_base:	link value base number
 * @irq_mask:	IRQ mask reprenting the 16 IRQs in 8259, bit N is 1 means
 *		IRQ N is available to be routed
 * @lb_bdf:	irq router's PCI bus/device/function number encoding
 * @ibase:	IBASE register block base address
 */
struct irq_router {
	int config;
	u32 link_base;
	u16 irq_mask;
	u32 bdf;
	u32 ibase;
};

struct pirq_routing {
	int bdf;
	int pin;
	int pirq;
};

/* PIRQ link number and value conversion */
#define LINK_V2N(link, base)	(link - base)
#define LINK_N2V(link, base)	(link + base)

#define PIRQ_BITMAP		0xdef8

/**
 * irq_router_common_init() - Perform common x86 interrupt init
 *
 * This creates the PIRQ routing table and routes the IRQs
 */
int irq_router_common_init(struct udevice *dev);

#endif /* _ARCH_IRQ_H_ */
