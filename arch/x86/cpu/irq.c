/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pci.h>
#include <asm/pirq_routing.h>

DECLARE_GLOBAL_DATA_PTR;

static struct irq_router irq_router;
static struct irq_routing_table *pirq_routing_table;

bool pirq_check_irq_routed(int link, u8 irq)
{
	u8 pirq;
	int base = irq_router.link_base;

	if (irq_router.config == PIRQ_VIA_PCI)
		pirq = x86_pci_read_config8(irq_router.bdf,
					    LINK_N2V(link, base));
	else
		pirq = readb(irq_router.ibase + LINK_N2V(link, base));

	pirq &= 0xf;

	/* IRQ# 0/1/2/8/13 are reserved */
	if (pirq < 3 || pirq == 8 || pirq == 13)
		return false;

	return pirq == irq ? true : false;
}

int pirq_translate_link(int link)
{
	return LINK_V2N(link, irq_router.link_base);
}

void pirq_assign_irq(int link, u8 irq)
{
	int base = irq_router.link_base;

	/* IRQ# 0/1/2/8/13 are reserved */
	if (irq < 3 || irq == 8 || irq == 13)
		return;

	if (irq_router.config == PIRQ_VIA_PCI)
		x86_pci_write_config8(irq_router.bdf,
				      LINK_N2V(link, base), irq);
	else
		writeb(irq, irq_router.ibase + LINK_N2V(link, base));
}

static inline void fill_irq_info(struct irq_info **slotp, int *entries, u8 bus,
				 u8 device, u8 func, u8 pin, u8 pirq)
{
	struct irq_info *slot = *slotp;

	slot->bus = bus;
	slot->devfn = (device << 3) | func;
	slot->irq[pin - 1].link = LINK_N2V(pirq, irq_router.link_base);
	slot->irq[pin - 1].bitmap = irq_router.irq_mask;
	(*entries)++;
	(*slotp)++;
}

__weak void cpu_irq_init(void)
{
	return;
}

static int create_pirq_routing_table(void)
{
	const void *blob = gd->fdt_blob;
	struct fdt_pci_addr addr;
	int node;
	int len, count;
	const u32 *cell;
	struct irq_routing_table *rt;
	struct irq_info *slot;
	int irq_entries = 0;
	int i;
	int ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_INTEL_IRQ_ROUTER);
	if (node < 0) {
		debug("%s: Cannot find irq router node\n", __func__);
		return -EINVAL;
	}

	ret = fdtdec_get_pci_addr(blob, node, FDT_PCI_SPACE_CONFIG,
				  "reg", &addr);
	if (ret)
		return ret;

	/* extract the bdf from fdt_pci_addr */
	irq_router.bdf = addr.phys_hi & 0xffff00;

	ret = fdt_find_string(blob, node, "intel,pirq-config", "pci");
	if (!ret) {
		irq_router.config = PIRQ_VIA_PCI;
	} else {
		ret = fdt_find_string(blob, node, "intel,pirq-config", "ibase");
		if (!ret)
			irq_router.config = PIRQ_VIA_IBASE;
		else
			return -EINVAL;
	}

	ret = fdtdec_get_int_array(blob, node, "intel,pirq-link",
				   &irq_router.link_base, 1);
	if (ret)
		return ret;

	irq_router.irq_mask = fdtdec_get_int(blob, node,
					     "intel,pirq-mask", PIRQ_BITMAP);

	if (irq_router.config == PIRQ_VIA_IBASE) {
		int ibase_off;

		ibase_off = fdtdec_get_int(blob, node, "intel,ibase-offset", 0);
		if (!ibase_off)
			return -EINVAL;

		/*
		 * Here we assume that the IBASE register has already been
		 * properly configured by U-Boot before.
		 *
		 * By 'valid' we mean:
		 *   1) a valid memory space carved within system memory space
		 *      assigned to IBASE register block.
		 *   2) memory range decoding is enabled.
		 * Hence we don't do any santify test here.
		 */
		irq_router.ibase = x86_pci_read_config32(irq_router.bdf,
							 ibase_off);
		irq_router.ibase &= ~0xf;
	}

	cell = fdt_getprop(blob, node, "intel,pirq-routing", &len);
	if (!cell)
		return -EINVAL;

	if ((len % sizeof(struct pirq_routing)) == 0)
		count = len / sizeof(struct pirq_routing);
	else
		return -EINVAL;

	rt = malloc(sizeof(struct irq_routing_table));
	if (!rt)
		return -ENOMEM;
	memset((char *)rt, 0, sizeof(struct irq_routing_table));

	/* Populate the PIRQ table fields */
	rt->signature = PIRQ_SIGNATURE;
	rt->version = PIRQ_VERSION;
	rt->rtr_bus = 0;
	rt->rtr_devfn = (PCI_DEV(irq_router.bdf) << 3) |
			PCI_FUNC(irq_router.bdf);
	rt->rtr_vendor = PCI_VENDOR_ID_INTEL;
	rt->rtr_device = PCI_DEVICE_ID_INTEL_ICH7_31;

	slot = rt->slots;

	/* Now fill in the irq_info entries in the PIRQ table */
	for (i = 0; i < count; i++) {
		struct pirq_routing pr;

		pr.bdf = fdt_addr_to_cpu(cell[0]);
		pr.pin = fdt_addr_to_cpu(cell[1]);
		pr.pirq = fdt_addr_to_cpu(cell[2]);

		debug("irq_info %d: b.d.f %x.%x.%x INT%c PIRQ%c\n",
		      i, PCI_BUS(pr.bdf), PCI_DEV(pr.bdf),
		      PCI_FUNC(pr.bdf), 'A' + pr.pin - 1,
		      'A' + pr.pirq);
		fill_irq_info(&slot, &irq_entries, PCI_BUS(pr.bdf),
			      PCI_DEV(pr.bdf), PCI_FUNC(pr.bdf),
			      pr.pin, pr.pirq);
		cell += sizeof(struct pirq_routing) / sizeof(u32);
	}

	rt->size = irq_entries * sizeof(struct irq_info) + 32;

	pirq_routing_table = rt;

	return 0;
}

void pirq_init(void)
{
	cpu_irq_init();

	if (create_pirq_routing_table()) {
		debug("Failed to create pirq routing table\n");
	} else {
		/* Route PIRQ */
		pirq_route_irqs(pirq_routing_table->slots,
				get_irq_slot_count(pirq_routing_table));
	}
}

u32 write_pirq_routing_table(u32 addr)
{
	if (!pirq_routing_table)
		return addr;

	return copy_pirq_routing_table(addr, pirq_routing_table);
}
