/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/pirq_routing.h>
#include <asm/arch/device.h>
#include <asm/arch/tnc.h>
#include <asm/arch/irq.h>

static struct irq_routing_table *pirq_routing_table;

bool pirq_check_irq_routed(int link, u8 irq)
{
	u8 pirq;

	pirq = x86_pci_read_config8(TNC_LPC, LINK_N2V(link));
	pirq &= 0xf;

	/* IRQ# 0/1/2/8/13 are reserved */
	if (pirq < 3 || pirq == 8 || pirq == 13)
		return false;

	return pirq == irq ? true : false;
}

int pirq_translate_link(int link)
{
	return LINK_V2N(link);
}

void pirq_assign_irq(int link, u8 irq)
{
	/* IRQ# 0/1/2/8/13 are reserved */
	if (irq < 3 || irq == 8 || irq == 13)
		return;

	x86_pci_write_config8(TNC_LPC, LINK_N2V(link), irq);
}

static inline void fill_irq_info(struct irq_info **slotp, int *entries, u8 bus,
				 u8 device, u8 func, u8 pin, u8 pirq)
{
	struct irq_info *slot = *slotp;

	slot->bus = bus;
	slot->devfn = (device << 3) | func;
	slot->irq[pin - 1].link = LINK_N2V(pirq);
	slot->irq[pin - 1].bitmap = PIRQ_BITMAP;
	(*entries)++;
	(*slotp)++;
}

/* PCIe port downstream INTx swizzle */
static inline u8 pin_swizzle(u8 pin, int port)
{
	return (pin + port) % 4;
}

__weak int board_fill_irq_info(struct irq_info *slot)
{
	return 0;
}

static int create_pirq_routing_table(void)
{
	struct irq_routing_table *rt;
	struct irq_info *slot;
	int irq_entries = 0;
	pci_dev_t tcf_bdf;
	u8 tcf_bus, bus;
	int i;

	rt = malloc(sizeof(struct irq_routing_table));
	if (!rt)
		return -ENOMEM;
	memset((char *)rt, 0, sizeof(struct irq_routing_table));

	/* Populate the PIRQ table fields */
	rt->signature = PIRQ_SIGNATURE;
	rt->version = PIRQ_VERSION;
	rt->rtr_bus = 0;
	rt->rtr_devfn = (TNC_LPC_DEV << 3) | TNC_LPC_FUNC;
	rt->rtr_vendor = PCI_VENDOR_ID_INTEL;
	rt->rtr_device = PCI_DEVICE_ID_INTEL_ICH7_31;

	slot = rt->slots;

	/*
	 * Now fill in the irq_info entries in the PIRQ table
	 *
	 * We start from internal TunnelCreek PCI devices first, then
	 * followed by all the 4 PCIe ports downstream devices, including
	 * the Queensbay platform Topcliff chipset devices.
	 */
	fill_irq_info(&slot, &irq_entries, 0, TNC_IGD_DEV,
		      TNC_IGD_FUNC, INTA, PIRQE);
	fill_irq_info(&slot, &irq_entries, 0, TNC_SDVO_DEV,
		      TNC_SDVO_FUNC, INTA, PIRQF);
	fill_irq_info(&slot, &irq_entries, 0, TNC_HDA_DEV,
		      TNC_HDA_FUNC, INTA, PIRQG);
	fill_irq_info(&slot, &irq_entries, 0, TNC_PCIE0_DEV,
		      TNC_PCIE0_FUNC, INTA, PIRQE);
	fill_irq_info(&slot, &irq_entries, 0, TNC_PCIE1_DEV,
		      TNC_PCIE1_FUNC, INTA, PIRQF);
	fill_irq_info(&slot, &irq_entries, 0, TNC_PCIE2_DEV,
		      TNC_PCIE2_FUNC, INTA, PIRQG);
	fill_irq_info(&slot, &irq_entries, 0, TNC_PCIE3_DEV,
		      TNC_PCIE3_FUNC, INTA, PIRQH);

	/* Check which PCIe port the Topcliff chipset is connected to */
	tcf_bdf = pci_find_device(PCI_VENDOR_ID_INTEL, 0x8800, 0);
	tcf_bus = PCI_BUS(tcf_bdf);
	for (i = 0; i < 4; i++) {
		bus = x86_pci_read_config8(PCI_BDF(0, TNC_PCIE0_DEV + i, 0),
					   PCI_SECONDARY_BUS);
		if (bus == tcf_bus)
			break;
	}

	/* Fill in the Topcliff chipset devices' irq info */
	if (i < 4) {
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_PCIE_PORT_DEV,
			      TCF_PCIE_PORT_FUNC, INTA, pin_swizzle(PIRQA, i));

		tcf_bus++;
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_0,
			      TCF_GBE_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_0,
			      TCF_GPIO_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_2,
			      TCF_USB1_OHCI0_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_2,
			      TCF_USB1_OHCI1_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_2,
			      TCF_USB1_OHCI2_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_2,
			      TCF_USB1_EHCI_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_2,
			      TCF_USB_DEVICE_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_4,
			      TCF_SDIO0_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_4,
			      TCF_SDIO1_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_6,
			      TCF_SATA_FUNC, INTD, pin_swizzle(PIRQD, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_8,
			      TCF_USB2_OHCI0_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_8,
			      TCF_USB2_OHCI1_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_8,
			      TCF_USB2_OHCI2_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_8,
			      TCF_USB2_EHCI_FUNC, INTA, pin_swizzle(PIRQA, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_10,
			      TCF_DMA1_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_10,
			      TCF_UART0_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_10,
			      TCF_UART1_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_10,
			      TCF_UART2_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_10,
			      TCF_UART3_FUNC, INTB, pin_swizzle(PIRQB, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_12,
			      TCF_DMA2_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_12,
			      TCF_SPI_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_12,
			      TCF_I2C_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_12,
			      TCF_CAN_FUNC, INTC, pin_swizzle(PIRQC, i));
		fill_irq_info(&slot, &irq_entries, tcf_bus, TCF_DEV_12,
			      TCF_1588_FUNC, INTC, pin_swizzle(PIRQC, i));
	}

	/* Call board-specific routine to fill in add-in card's irq info */
	irq_entries += board_fill_irq_info(slot);

	rt->size = irq_entries * sizeof(struct irq_info) + 32;

	pirq_routing_table = rt;

	return 0;
}

void pirq_init(void)
{
	struct tnc_rcba *rcba;
	u32 base;

	base = x86_pci_read_config32(TNC_LPC, LPC_RCBA);
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
	 * A/B/C/D directly and not configurable, we route internal PCI
	 * device's INTx to PIRQ E/F/G/H.
	 */
	writew(PIRQE, &rcba->d02ir);
	writew(PIRQF, &rcba->d03ir);
	writew(PIRQG, &rcba->d27ir);
	writew(PIRQH, &rcba->d31ir);
	writew(PIRQE, &rcba->d23ir);
	writew(PIRQF, &rcba->d24ir);
	writew(PIRQG, &rcba->d25ir);
	writew(PIRQH, &rcba->d26ir);

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
	return copy_pirq_routing_table(addr, pirq_routing_table);
}
