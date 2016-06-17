/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pci.h>
#include <asm/arch/device.h>
#include <asm/arch/tnc.h>

int queensbay_irq_router_probe(struct udevice *dev)
{
	struct tnc_rcba *rcba;
	u32 base;

	dm_pci_read_config32(dev->parent, LPC_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct tnc_rcba *)base;

	/* Make sure all internal PCI devices are using INTA */
	writel(INTA, &rcba->d02ip);
	writel(INTA, &rcba->d03ip);
	writel(INTA, &rcba->d27ip);
	writel(INTA, &rcba->d31ip);
	writel(INTA, &rcba->d23ip);
	writel(INTA, &rcba->d24ip);
	writel(INTA, &rcba->d25ip);
	writel(INTA, &rcba->d26ip);

	/*
	 * Route TunnelCreek PCI device interrupt pin to PIRQ
	 *
	 * Since PCIe downstream ports received INTx are routed to PIRQ
	 * A/B/C/D directly and not configurable, we have to route PCIe
	 * root ports' INTx to PIRQ A/B/C/D as well. For other devices
	 * on TunneCreek, route them to PIRQ E/F/G/H.
	 */
	writew(PIRQE, &rcba->d02ir);
	writew(PIRQF, &rcba->d03ir);
	writew(PIRQG, &rcba->d27ir);
	writew(PIRQH, &rcba->d31ir);
	writew(PIRQA, &rcba->d23ir);
	writew(PIRQB, &rcba->d24ir);
	writew(PIRQC, &rcba->d25ir);
	writew(PIRQD, &rcba->d26ir);

	return irq_router_common_init(dev);
}

static const struct udevice_id queensbay_irq_router_ids[] = {
	{ .compatible = "intel,queensbay-irq-router" },
	{ }
};

U_BOOT_DRIVER(queensbay_irq_router_drv) = {
	.name		= "queensbay_intel_irq",
	.id		= UCLASS_IRQ,
	.of_match	= queensbay_irq_router_ids,
	.probe		= queensbay_irq_router_probe,
};
