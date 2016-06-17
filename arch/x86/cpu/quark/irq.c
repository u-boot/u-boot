/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/irq.h>
#include <asm/arch/device.h>
#include <asm/arch/quark.h>

int quark_irq_router_probe(struct udevice *dev)
{
	struct quark_rcba *rcba;
	u32 base;

	qrk_pci_read_config_dword(QUARK_LEGACY_BRIDGE, LB_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct quark_rcba *)base;

	/*
	 * Route Quark PCI device interrupt pin to PIRQ
	 *
	 * Route device#23's INTA/B/C/D to PIRQA/B/C/D
	 * Route device#20,21's INTA/B/C/D to PIRQE/F/G/H
	 */
	writew(PIRQC, &rcba->rmu_ir);
	writew(PIRQA | (PIRQB << 4) | (PIRQC << 8) | (PIRQD << 12),
	       &rcba->d23_ir);
	writew(PIRQD, &rcba->core_ir);
	writew(PIRQE | (PIRQF << 4) | (PIRQG << 8) | (PIRQH << 12),
	       &rcba->d20d21_ir);

	return irq_router_common_init(dev);
}

static const struct udevice_id quark_irq_router_ids[] = {
	{ .compatible = "intel,quark-irq-router" },
	{ }
};

U_BOOT_DRIVER(quark_irq_router_drv) = {
	.name		= "quark_intel_irq",
	.id		= UCLASS_IRQ,
	.of_match	= quark_irq_router_ids,
	.probe		= quark_irq_router_probe,
};
