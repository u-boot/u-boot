/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ehci.h"
#include <linux/mbus.h>
#include <asm/arch/cpu.h>
#include <dm.h>

#if defined(CONFIG_KIRKWOOD)
#include <asm/arch/soc.h>
#elif defined(CONFIG_ORION5X)
#include <asm/arch/orion5x.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define USB_WINDOW_CTRL(i)	(0x320 + ((i) << 4))
#define USB_WINDOW_BASE(i)	(0x324 + ((i) << 4))
#define USB_TARGET_DRAM		0x0

/*
 * USB 2.0 Bridge Address Decoding registers setup
 */
#ifdef CONFIG_DM_USB

struct ehci_mvebu_priv {
	struct ehci_ctrl ehci;
	fdt_addr_t hcd_base;
};

/*
 * Once all the older Marvell SoC's (Orion, Kirkwood) are converted
 * to the common mvebu archticture including the mbus setup, this
 * will be the only function needed to configure the access windows
 */
static void usb_brg_adrdec_setup(u32 base)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	for (i = 0; i < 4; i++) {
		writel(0, base + USB_WINDOW_CTRL(i));
		writel(0, base + USB_WINDOW_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		/* Write size, attributes and target id to control register */
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       base + USB_WINDOW_CTRL(i));

		/* Write base address to base register */
		writel(cs->base, base + USB_WINDOW_BASE(i));
	}
}

static int ehci_mvebu_probe(struct udevice *dev)
{
	struct ehci_mvebu_priv *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;

	/*
	 * Get the base address for EHCI controller from the device node
	 */
	priv->hcd_base = dev_get_addr(dev);
	if (priv->hcd_base == FDT_ADDR_T_NONE) {
		debug("Can't get the EHCI register base address\n");
		return -ENXIO;
	}

	usb_brg_adrdec_setup(priv->hcd_base);

	hccr = (struct ehci_hccr *)(priv->hcd_base + 0x100);
	hcor = (struct ehci_hcor *)
		((u32)hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	debug("ehci-marvell: init hccr %x and hcor %x hc_length %d\n",
	      (u32)hccr, (u32)hcor,
	      (u32)HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
}

static int ehci_mvebu_remove(struct udevice *dev)
{
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "marvell,orion-ehci", },
	{ }
};

U_BOOT_DRIVER(ehci_mvebu) = {
	.name	= "ehci_mvebu",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_mvebu_probe,
	.remove = ehci_mvebu_remove,
	.ops	= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_mvebu_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

#else
#define MVUSB_BASE(port)	MVUSB0_BASE

static void usb_brg_adrdec_setup(int index)
{
	int i;
	u32 size, base, attrib;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {

		/* Enable DRAM bank */
		switch (i) {
		case 0:
			attrib = MVUSB0_CPU_ATTR_DRAM_CS0;
			break;
		case 1:
			attrib = MVUSB0_CPU_ATTR_DRAM_CS1;
			break;
		case 2:
			attrib = MVUSB0_CPU_ATTR_DRAM_CS2;
			break;
		case 3:
			attrib = MVUSB0_CPU_ATTR_DRAM_CS3;
			break;
		default:
			/* invalide bank, disable access */
			attrib = 0;
			break;
		}

		size = gd->bd->bi_dram[i].size;
		base = gd->bd->bi_dram[i].start;
		if ((size) && (attrib))
			writel(MVCPU_WIN_CTRL_DATA(size, USB_TARGET_DRAM,
						   attrib, MVCPU_WIN_ENABLE),
				MVUSB0_BASE + USB_WINDOW_CTRL(i));
		else
			writel(MVCPU_WIN_DISABLE,
			       MVUSB0_BASE + USB_WINDOW_CTRL(i));

		writel(base, MVUSB0_BASE + USB_WINDOW_BASE(i));
	}
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	usb_brg_adrdec_setup(index);

	*hccr = (struct ehci_hccr *)(MVUSB_BASE(index) + 0x100);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
			+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("ehci-marvell: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}

#endif /* CONFIG_DM_USB */
