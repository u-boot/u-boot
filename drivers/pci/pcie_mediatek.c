// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek PCIe host controller driver.
 *
 * Copyright (c) 2017-2019 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *	   Honghui Zhang <honghui.zhang@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <linux/list.h>

/* PCIe shared registers */
#define PCIE_SYS_CFG		0x00
#define PCIE_INT_ENABLE		0x0c
#define PCIE_CFG_ADDR		0x20
#define PCIE_CFG_DATA		0x24

/* PCIe per port registers */
#define PCIE_BAR0_SETUP		0x10
#define PCIE_CLASS		0x34
#define PCIE_LINK_STATUS	0x50

#define PCIE_PORT_INT_EN(x)	BIT(20 + (x))
#define PCIE_PORT_PERST(x)	BIT(1 + (x))
#define PCIE_PORT_LINKUP	BIT(0)
#define PCIE_BAR_MAP_MAX	GENMASK(31, 16)

#define PCIE_BAR_ENABLE		BIT(0)
#define PCIE_REVISION_ID	BIT(0)
#define PCIE_CLASS_CODE		(0x60400 << 8)
#define PCIE_CONF_REG(regn)	(((regn) & GENMASK(7, 2)) | \
				((((regn) >> 8) & GENMASK(3, 0)) << 24))
#define PCIE_CONF_ADDR(regn, bdf) \
				(PCIE_CONF_REG(regn) | (bdf))

/* MediaTek specific configuration registers */
#define PCIE_FTS_NUM		0x70c
#define PCIE_FTS_NUM_MASK	GENMASK(15, 8)
#define PCIE_FTS_NUM_L0(x)	((x) & 0xff << 8)

#define PCIE_FC_CREDIT		0x73c
#define PCIE_FC_CREDIT_MASK	(GENMASK(31, 31) | GENMASK(28, 16))
#define PCIE_FC_CREDIT_VAL(x)	((x) << 16)

struct mtk_pcie_port {
	void __iomem *base;
	struct list_head list;
	struct mtk_pcie *pcie;
	struct reset_ctl reset;
	struct clk sys_ck;
	struct phy phy;
	u32 slot;
};

struct mtk_pcie {
	void __iomem *base;
	struct clk free_ck;
	struct list_head ports;
};

static int mtk_pcie_config_address(struct udevice *udev, pci_dev_t bdf,
				   uint offset, void **paddress)
{
	struct mtk_pcie *pcie = dev_get_priv(udev);

	writel(PCIE_CONF_ADDR(offset, bdf), pcie->base + PCIE_CFG_ADDR);
	*paddress = pcie->base + PCIE_CFG_DATA + (offset & 3);

	return 0;
}

static int mtk_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, mtk_pcie_config_address,
					    bdf, offset, valuep, size);
}

static int mtk_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, mtk_pcie_config_address,
					     bdf, offset, value, size);
}

static const struct dm_pci_ops mtk_pcie_ops = {
	.read_config	= mtk_pcie_read_config,
	.write_config	= mtk_pcie_write_config,
};

static void mtk_pcie_port_free(struct mtk_pcie_port *port)
{
	list_del(&port->list);
	free(port);
}

static int mtk_pcie_startup_port(struct mtk_pcie_port *port)
{
	struct mtk_pcie *pcie = port->pcie;
	u32 slot = PCI_DEV(port->slot << 11);
	u32 val;
	int err;

	/* assert port PERST_N */
	setbits_le32(pcie->base + PCIE_SYS_CFG, PCIE_PORT_PERST(port->slot));
	/* de-assert port PERST_N */
	clrbits_le32(pcie->base + PCIE_SYS_CFG, PCIE_PORT_PERST(port->slot));

	/* 100ms timeout value should be enough for Gen1/2 training */
	err = readl_poll_timeout(port->base + PCIE_LINK_STATUS, val,
				 !!(val & PCIE_PORT_LINKUP), 100000);
	if (err)
		return -ETIMEDOUT;

	/* disable interrupt */
	clrbits_le32(pcie->base + PCIE_INT_ENABLE,
		     PCIE_PORT_INT_EN(port->slot));

	/* map to all DDR region. We need to set it before cfg operation. */
	writel(PCIE_BAR_MAP_MAX | PCIE_BAR_ENABLE,
	       port->base + PCIE_BAR0_SETUP);

	/* configure class code and revision ID */
	writel(PCIE_CLASS_CODE | PCIE_REVISION_ID, port->base + PCIE_CLASS);

	/* configure FC credit */
	writel(PCIE_CONF_ADDR(PCIE_FC_CREDIT, slot),
	       pcie->base + PCIE_CFG_ADDR);
	clrsetbits_le32(pcie->base + PCIE_CFG_DATA, PCIE_FC_CREDIT_MASK,
			PCIE_FC_CREDIT_VAL(0x806c));

	/* configure RC FTS number to 250 when it leaves L0s */
	writel(PCIE_CONF_ADDR(PCIE_FTS_NUM, slot), pcie->base + PCIE_CFG_ADDR);
	clrsetbits_le32(pcie->base + PCIE_CFG_DATA, PCIE_FTS_NUM_MASK,
			PCIE_FTS_NUM_L0(0x50));

	return 0;
}

static void mtk_pcie_enable_port(struct mtk_pcie_port *port)
{
	int err;

	err = clk_enable(&port->sys_ck);
	if (err)
		goto exit;

	err = reset_assert(&port->reset);
	if (err)
		goto exit;

	err = reset_deassert(&port->reset);
	if (err)
		goto exit;

	err = generic_phy_init(&port->phy);
	if (err)
		goto exit;

	err = generic_phy_power_on(&port->phy);
	if (err)
		goto exit;

	if (!mtk_pcie_startup_port(port))
		return;

	pr_err("Port%d link down\n", port->slot);
exit:
	mtk_pcie_port_free(port);
}

static int mtk_pcie_parse_port(struct udevice *dev, u32 slot)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port;
	char name[10];
	int err;

	port = devm_kzalloc(dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	snprintf(name, sizeof(name), "port%d", slot);
	port->base = dev_remap_addr_name(dev, name);
	if (!port->base)
		return -ENOENT;

	snprintf(name, sizeof(name), "sys_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->sys_ck);
	if (err)
		return err;

	err = reset_get_by_index(dev, slot, &port->reset);
	if (err)
		return err;

	err = generic_phy_get_by_index(dev, slot, &port->phy);
	if (err)
		return err;

	port->slot = slot;
	port->pcie = pcie;

	INIT_LIST_HEAD(&port->list);
	list_add_tail(&port->list, &pcie->ports);

	return 0;
}

static int mtk_pcie_probe(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port, *tmp;
	ofnode subnode;
	int err;

	INIT_LIST_HEAD(&pcie->ports);

	pcie->base = dev_remap_addr_name(dev, "subsys");
	if (!pcie->base)
		return -ENOENT;

	err = clk_get_by_name(dev, "free_ck", &pcie->free_ck);
	if (err)
		return err;

	/* enable top level clock */
	err = clk_enable(&pcie->free_ck);
	if (err)
		return err;

	dev_for_each_subnode(subnode, dev) {
		struct fdt_pci_addr addr;
		u32 slot = 0;

		if (!ofnode_is_available(subnode))
			continue;

		err = ofnode_read_pci_addr(subnode, 0, "reg", &addr);
		if (err)
			return err;

		slot = PCI_DEV(addr.phys_hi);

		err = mtk_pcie_parse_port(dev, slot);
		if (err)
			return err;
	}

	/* enable each port, and then check link status */
	list_for_each_entry_safe(port, tmp, &pcie->ports, list)
		mtk_pcie_enable_port(port);

	return 0;
}

static const struct udevice_id mtk_pcie_ids[] = {
	{ .compatible = "mediatek,mt7623-pcie", },
	{ }
};

U_BOOT_DRIVER(pcie_mediatek) = {
	.name	= "pcie_mediatek",
	.id	= UCLASS_PCI,
	.of_match = mtk_pcie_ids,
	.ops	= &mtk_pcie_ops,
	.probe	= mtk_pcie_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_pcie),
};
