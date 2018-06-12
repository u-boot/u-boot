/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
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
 * @link_num:	number of PIRQ links supported
 * @irq_mask:	IRQ mask reprenting the 16 IRQs in 8259, bit N is 1 means
 *		IRQ N is available to be routed
 * @lb_bdf:	irq router's PCI bus/device/function number encoding
 * @ibase:	IBASE register block base address
 * @actl_8bit:	ACTL register width is 8-bit (for ICH series chipset)
 * @actl_addr:	ACTL register offset
 */
struct irq_router {
	int config;
	u32 link_base;
	int link_num;
	u16 irq_mask;
	u32 bdf;
	u32 ibase;
	bool actl_8bit;
	int actl_addr;
};

struct pirq_routing {
	int bdf;
	int pin;
	int pirq;
};

/**
 * pirq_reg_to_linkno() - Convert a PIRQ routing register offset to link number
 *
 * @reg:	PIRQ routing register offset from the base address
 * @base:	PIRQ routing register block base address
 * @return:	PIRQ link number (0 for PIRQA, 1 for PIRQB, etc)
 */
static inline int pirq_reg_to_linkno(int reg, int base)
{
	return reg - base;
}

/**
 * pirq_linkno_to_reg() - Convert a PIRQ link number to routing register offset
 *
 * @linkno:	PIRQ link number (0 for PIRQA, 1 for PIRQB, etc)
 * @base:	PIRQ routing register block base address
 * @return:	PIRQ routing register offset from the base address
 */
static inline int pirq_linkno_to_reg(int linkno, int base)
{
	return linkno + base;
}

#define PIRQ_BITMAP		0xdef8

#endif /* _ARCH_IRQ_H_ */
