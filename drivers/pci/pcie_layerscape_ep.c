// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 * Layerscape PCIe EP driver
 */

#include <common.h>
#include <dm.h>
#include <dm/devres.h>
#include <errno.h>
#include <pci_ep.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <linux/log2.h>
#include "pcie_layerscape.h"

DECLARE_GLOBAL_DATA_PTR;

static void ls_pcie_ep_enable_cfg(struct ls_pcie_ep *pcie_ep)
{
	struct ls_pcie *pcie = pcie_ep->pcie;
	u32 config;

	config = ctrl_readl(pcie,  PCIE_PF_CONFIG);
	config |= PCIE_CONFIG_READY;
	ctrl_writel(pcie, config, PCIE_PF_CONFIG);
}

static int ls_ep_set_bar(struct udevice *dev, uint fn, struct pci_bar *ep_bar)
{
	struct ls_pcie_ep *pcie_ep = dev_get_priv(dev);
	struct ls_pcie *pcie = pcie_ep->pcie;
	dma_addr_t bar_phys = ep_bar->phys_addr;
	enum pci_barno bar = ep_bar->barno;
	u32 reg = PCI_BASE_ADDRESS_0 + (4 * bar);
	int flags = ep_bar->flags;
	int type, idx;
	u64 size;

	idx  = bar;
	/* BAR size is 2^(aperture + 11) */
	size = max_t(size_t, ep_bar->size, FSL_PCIE_EP_MIN_APERTURE);

	if (!(flags & PCI_BASE_ADDRESS_SPACE))
		type = PCIE_ATU_TYPE_MEM;
	else
		type = PCIE_ATU_TYPE_IO;

	ls_pcie_atu_inbound_set(pcie, idx, bar, bar_phys, type);

	dbi_writel(pcie, lower_32_bits(size - 1), reg + PCIE_NO_SRIOV_BAR_BASE);
	dbi_writel(pcie, flags, reg);

	if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64) {
		dbi_writel(pcie, upper_32_bits(size - 1),
			   reg + 4 + PCIE_NO_SRIOV_BAR_BASE);
		dbi_writel(pcie, 0, reg + 4);
	}

	return 0;
}

static struct pci_ep_ops ls_pcie_ep_ops = {
	.set_bar = ls_ep_set_bar,
};

static void ls_pcie_ep_setup_atu(struct ls_pcie_ep *pcie_ep)
{
	struct ls_pcie *pcie = pcie_ep->pcie;
	u64 phys = CONFIG_SYS_PCI_EP_MEMORY_BASE;

	/* ATU 0 : INBOUND : map BAR0 */
	ls_pcie_atu_inbound_set(pcie, 0, PCIE_ATU_TYPE_MEM, 0, phys);
	/* ATU 1 : INBOUND : map BAR1 */
	phys += PCIE_BAR1_SIZE;
	ls_pcie_atu_inbound_set(pcie, 1, PCIE_ATU_TYPE_MEM, 1, phys);
	/* ATU 2 : INBOUND : map BAR2 */
	phys += PCIE_BAR2_SIZE;
	ls_pcie_atu_inbound_set(pcie, 2, PCIE_ATU_TYPE_MEM, 2, phys);
	/* ATU 3 : INBOUND : map BAR4 */
	phys = CONFIG_SYS_PCI_EP_MEMORY_BASE + PCIE_BAR4_SIZE;
	ls_pcie_atu_inbound_set(pcie, 3, PCIE_ATU_TYPE_MEM, 4, phys);

	/* ATU 0 : OUTBOUND : map MEM */
	ls_pcie_atu_outbound_set(pcie, 0,
				 PCIE_ATU_TYPE_MEM,
				 pcie_ep->addr_res.start,
				 0,
				 CONFIG_SYS_PCI_MEMORY_SIZE);
}

/* BAR0 and BAR1 are 32bit BAR2 and BAR4 are 64bit */
static void ls_pcie_ep_setup_bar(void *bar_base, int bar, u32 size)
{
	/* The least inbound window is 4KiB */
	if (size < 4 * 1024)
		return;

	switch (bar) {
	case 0:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_0);
		break;
	case 1:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_1);
		break;
	case 2:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_2);
		writel(0, bar_base + PCI_BASE_ADDRESS_3);
		break;
	case 4:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_4);
		writel(0, bar_base + PCI_BASE_ADDRESS_5);
		break;
	default:
		break;
	}
}

static void ls_pcie_ep_setup_bars(void *bar_base)
{
	/* BAR0 - 32bit - 4K configuration */
	ls_pcie_ep_setup_bar(bar_base, 0, PCIE_BAR0_SIZE);
	/* BAR1 - 32bit - 8K MSIX */
	ls_pcie_ep_setup_bar(bar_base, 1, PCIE_BAR1_SIZE);
	/* BAR2 - 64bit - 4K MEM descriptor */
	ls_pcie_ep_setup_bar(bar_base, 2, PCIE_BAR2_SIZE);
	/* BAR4 - 64bit - 1M MEM */
	ls_pcie_ep_setup_bar(bar_base, 4, PCIE_BAR4_SIZE);
}

static void ls_pcie_setup_ep(struct ls_pcie_ep *pcie_ep)
{
	u32 sriov;
	struct ls_pcie *pcie = pcie_ep->pcie;

	sriov = readl(pcie->dbi + PCIE_SRIOV);
	if (PCI_EXT_CAP_ID(sriov) == PCI_EXT_CAP_ID_SRIOV) {
		int pf, vf;

		for (pf = 0; pf < PCIE_PF_NUM; pf++) {
			for (vf = 0; vf <= PCIE_VF_NUM; vf++) {
				ctrl_writel(pcie, PCIE_LCTRL0_VAL(pf, vf),
					    PCIE_PF_VF_CTRL);

				ls_pcie_ep_setup_bars(pcie->dbi);
				ls_pcie_ep_setup_atu(pcie_ep);
			}
		}
		/* Disable CFG2 */
		ctrl_writel(pcie, 0, PCIE_PF_VF_CTRL);
	} else {
		ls_pcie_ep_setup_bars(pcie->dbi + PCIE_NO_SRIOV_BAR_BASE);
		ls_pcie_ep_setup_atu(pcie_ep);
	}

	ls_pcie_ep_enable_cfg(pcie_ep);
}

static int ls_pcie_ep_probe(struct udevice *dev)
{
	struct ls_pcie_ep *pcie_ep = dev_get_priv(dev);
	struct ls_pcie *pcie;
	u16 link_sta;
	int ret;

	pcie = devm_kmalloc(dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie_ep->pcie = pcie;

	pcie->dbi = (void __iomem *)devfdt_get_addr_index(dev, 0);
	if (!pcie->dbi)
		return -ENOMEM;

	pcie->ctrl = (void __iomem *)devfdt_get_addr_index(dev, 1);
	if (!pcie->ctrl)
		return -ENOMEM;

	ret = fdt_get_named_resource(gd->fdt_blob, dev_of_offset(dev),
				     "reg", "reg-names",
				     "addr_space", &pcie_ep->addr_res);
	if (ret) {
		printf("%s: resource \"addr_space\" not found\n", dev->name);
		return ret;
	}

	pcie->idx = ((unsigned long)pcie->dbi - PCIE_SYS_BASE_ADDR) /
		    PCIE_CCSR_SIZE;

	pcie->big_endian = fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
					   "big-endian");

	pcie->mode = readb(pcie->dbi + PCI_HEADER_TYPE) & 0x7f;
	if (pcie->mode != PCI_HEADER_TYPE_NORMAL)
		return 0;

	pcie_ep->max_functions = fdtdec_get_int(gd->fdt_blob,
						dev_of_offset(dev),
						"max-functions", 1);
	pcie_ep->num_ib_wins = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "num-ib-windows", 8);
	pcie_ep->num_ob_wins = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "num-ob-windows", 8);

	printf("PCIe%u: %s %s", pcie->idx, dev->name, "Endpoint");
	ls_pcie_setup_ep(pcie_ep);

	if (!ls_pcie_link_up(pcie)) {
		/* Let the user know there's no PCIe link */
		printf(": no link\n");
		return 0;
	}

	/* Print the negotiated PCIe link width */
	link_sta = readw(pcie->dbi + PCIE_LINK_STA);
	printf(": x%d gen%d\n", (link_sta & PCIE_LINK_WIDTH_MASK) >> 4,
	       link_sta & PCIE_LINK_SPEED_MASK);

	return 0;
}

static int ls_pcie_ep_remove(struct udevice *dev)
{
	return 0;
}

const struct udevice_id ls_pcie_ep_ids[] = {
	{ .compatible = "fsl,ls-pcie-ep" },
	{ }
};

U_BOOT_DRIVER(pci_layerscape_ep) = {
	.name = "pci_layerscape_ep",
	.id	= UCLASS_PCI_EP,
	.of_match = ls_pcie_ep_ids,
	.ops = &ls_pcie_ep_ops,
	.probe = ls_pcie_ep_probe,
	.remove = ls_pcie_ep_remove,
	.priv_auto_alloc_size = sizeof(struct ls_pcie_ep),
};
