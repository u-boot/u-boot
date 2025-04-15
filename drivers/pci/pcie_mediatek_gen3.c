// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek PCIe host controller driver.
 *
 * Copyright (c) 2023 John Crispin <john@phrozen.org>
 * Driver is based on u-boot gen1/2 and upstream linux gen3 code
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/list.h>
#include "pci_internal.h"

/* PCIe shared registers */
#define PCIE_CFG_ADDR			0x20
#define PCIE_CFG_DATA			0x24

#define PCIE_SETTING_REG		0x80

#define PCIE_PCI_IDS_1			0x9c
#define PCIE_RC_MODE			BIT(0)
#define PCI_CLASS(class)		((class) << 8)

#define PCIE_CFGNUM_REG			0x140
#define PCIE_CFG_DEVFN(devfn)		((devfn) & GENMASK(7, 0))
#define PCIE_CFG_BUS(bus)		(((bus) << 8) & GENMASK(15, 8))
#define PCIE_CFG_BYTE_EN(bytes)		(((bytes) << 16) & GENMASK(19, 16))
#define PCIE_CFG_FORCE_BYTE_EN		BIT(20)
#define PCIE_CFG_OFFSET_ADDR		0x1000
#define PCIE_CFG_HEADER(bus, devfn)	(PCIE_CFG_BUS(bus) | PCIE_CFG_DEVFN(devfn))

#define PCIE_RST_CTRL_REG		0x148
#define PCIE_MAC_RSTB			BIT(0)
#define PCIE_PHY_RSTB			BIT(1)
#define PCIE_BRG_RSTB			BIT(2)
#define PCIE_PE_RSTB			BIT(3)

#define PCIE_LINK_STATUS_REG		0x154
#define PCIE_PORT_LINKUP		BIT(8)

#define PCIE_INT_ENABLE_REG		0x180

#define PCIE_MISC_CTRL_REG		0x348
#define PCIE_DISABLE_DVFSRC_VLT_REQ	BIT(1)

#define PCIE_TRANS_TABLE_BASE_REG       0x800
#define PCIE_ATR_SRC_ADDR_MSB_OFFSET    0x4
#define PCIE_ATR_TRSL_ADDR_LSB_OFFSET   0x8
#define PCIE_ATR_TRSL_ADDR_MSB_OFFSET   0xc
#define PCIE_ATR_TRSL_PARAM_OFFSET      0x10
#define PCIE_ATR_TLB_SET_OFFSET         0x20

#define PCIE_MAX_TRANS_TABLES           8
#define PCIE_ATR_EN                     BIT(0)
#define PCIE_ATR_SIZE(size) \
	(((((size) - 1) << 1) & GENMASK(6, 1)) | PCIE_ATR_EN)
#define PCIE_ATR_ID(id)                 ((id) & GENMASK(3, 0))
#define PCIE_ATR_TYPE_MEM               PCIE_ATR_ID(0)
#define PCIE_ATR_TYPE_IO                PCIE_ATR_ID(1)
#define PCIE_ATR_TLP_TYPE(type)         (((type) << 16) & GENMASK(18, 16))
#define PCIE_ATR_TLP_TYPE_MEM           PCIE_ATR_TLP_TYPE(0)
#define PCIE_ATR_TLP_TYPE_IO            PCIE_ATR_TLP_TYPE(2)

struct mtk_pcie {
	void __iomem *base;
	void *priv;
	struct clk pl_250m_ck;
	struct clk tl_26m_ck;
	struct clk peri_26m_ck;
	struct clk top_133m_ck;
	struct reset_ctl reset_phy;
	struct reset_ctl reset_mac;
	struct phy phy;
};

static pci_dev_t convert_bdf(const struct udevice *controller, pci_dev_t bdf)
{
	int bdfs[3];

	bdfs[0] = PCI_BUS(bdf);
	bdfs[1] = PCI_DEV(bdf);
	bdfs[2] = PCI_FUNC(bdf);

	/*
	 * One MediaTek PCIe Gen3 controller has only one port, where PCI bus 0 on
	 * this port represents the controller itself and bus 1 represents the
	 * external PCIe device. If multiple PCIe controllers are probed in U-Boot,
	 * U-Boot will use bus numbers greater than 2 as input parameters. Therefore,
	 * we should convert the BDF bus number to either 0 or 1 by subtracting the
	 * offset by controller->seq_
	 */

	bdfs[0] = bdfs[0] - controller->seq_;

	return PCI_BDF(bdfs[0], bdfs[1], bdfs[2]);
}

static void mtk_pcie_config_tlp_header(const struct udevice *bus,
				       pci_dev_t devfn,
				       int where, int size)
{
	struct mtk_pcie *pcie = dev_get_priv(bus);
	int bytes;
	u32 val;

	devfn = convert_bdf(bus, devfn);

	size = 1 << size;
	bytes = (GENMASK(size - 1, 0) & 0xf) << (where & 0x3);

	val = PCIE_CFG_FORCE_BYTE_EN | PCIE_CFG_BYTE_EN(bytes) |
	      PCIE_CFG_HEADER(PCI_BUS(devfn), (devfn >> 8));

	writel(val, pcie->base + PCIE_CFGNUM_REG);
}

static int mtk_pcie_config_address(const struct udevice *udev, pci_dev_t bdf,
				   uint offset, void **paddress)
{
	struct mtk_pcie *pcie = dev_get_priv(udev);

	*paddress = pcie->base + PCIE_CFG_OFFSET_ADDR + offset;

	return 0;
}

static int mtk_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	int ret;

	mtk_pcie_config_tlp_header(bus, bdf, offset, size);
	ret = pci_generic_mmap_read_config(bus, mtk_pcie_config_address,
					   bdf, offset, valuep, size);
	return ret;
}

static int mtk_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	mtk_pcie_config_tlp_header(bus, bdf, offset, size);

	switch (size) {
	case PCI_SIZE_8:
	case PCI_SIZE_16:
		value <<= (offset & 0x3) * 8;
	case PCI_SIZE_32:
		break;
	default:
		return -EINVAL;
	}

	return pci_generic_mmap_write_config(bus, mtk_pcie_config_address,
					     bdf, (offset & ~0x3), value, PCI_SIZE_32);
}

static const struct dm_pci_ops mtk_pcie_ops = {
	.read_config	= mtk_pcie_read_config,
	.write_config	= mtk_pcie_write_config,
};

static int mtk_pcie_set_trans_table(struct udevice *dev, struct mtk_pcie *pcie,
				    u64 cpu_addr, u64 pci_addr, u64 size,
				    unsigned long type, int num)
{
	void __iomem *table;
	u32 val;

	if (num >= PCIE_MAX_TRANS_TABLES) {
		dev_err(dev, "not enough translate table for addr: %#llx, limited to [%d]\n",
			(unsigned long long)cpu_addr, PCIE_MAX_TRANS_TABLES);
		return -ENODEV;
	}

	dev_dbg(dev, "set trans table %d: %#llx %#llx, %#llx\n", num, cpu_addr,
		pci_addr, size);
	table = pcie->base + PCIE_TRANS_TABLE_BASE_REG +
		num * PCIE_ATR_TLB_SET_OFFSET;

	writel(lower_32_bits(cpu_addr) | PCIE_ATR_SIZE(fls(size) - 1), table);
	writel(upper_32_bits(cpu_addr), table + PCIE_ATR_SRC_ADDR_MSB_OFFSET);
	writel(lower_32_bits(pci_addr), table + PCIE_ATR_TRSL_ADDR_LSB_OFFSET);
	writel(upper_32_bits(pci_addr), table + PCIE_ATR_TRSL_ADDR_MSB_OFFSET);

	if (type == PCI_REGION_IO)
		val = PCIE_ATR_TYPE_IO | PCIE_ATR_TLP_TYPE_IO;
	else
		val = PCIE_ATR_TYPE_MEM | PCIE_ATR_TLP_TYPE_MEM;
	writel(val, table + PCIE_ATR_TRSL_PARAM_OFFSET);

	return 0;
}

static int mtk_pcie_startup_port(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	u32 val;
	int i, err;

	/* Set as RC mode */
	val = readl(pcie->base + PCIE_SETTING_REG);
	val |= PCIE_RC_MODE;
	writel(val, pcie->base + PCIE_SETTING_REG);

	/* setup RC BARs */
	writel(PCI_BASE_ADDRESS_MEM_TYPE_64,
	       pcie->base + PCI_BASE_ADDRESS_0);
	writel(0x0, pcie->base + PCI_BASE_ADDRESS_1);

	/* setup interrupt pins */
	clrsetbits_le32(pcie->base + PCI_INTERRUPT_LINE,
			0xff00, 0x100);

	/* setup bus numbers */
	clrsetbits_le32(pcie->base + PCI_PRIMARY_BUS,
			0xffffff, 0x00ff0100);

	/* setup command register */
	clrsetbits_le32(pcie->base + PCI_PRIMARY_BUS,
			0xffff,
			PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
			PCI_COMMAND_MASTER | PCI_COMMAND_SERR);

	/* Set class code */
	val = readl(pcie->base + PCIE_PCI_IDS_1);
	val &= ~GENMASK(31, 8);
	val |= PCI_CLASS(PCI_CLASS_BRIDGE_PCI << 8);
	writel(val, pcie->base + PCIE_PCI_IDS_1);

	/* Mask all INTx interrupts */
	val = readl(pcie->base + PCIE_INT_ENABLE_REG);
	val &= ~0xFF000000;
	writel(val, pcie->base + PCIE_INT_ENABLE_REG);

	/* Disable DVFSRC voltage request */
	val = readl(pcie->base + PCIE_MISC_CTRL_REG);
	val |= PCIE_DISABLE_DVFSRC_VLT_REQ;
	writel(val, pcie->base + PCIE_MISC_CTRL_REG);

	/* Assert all reset signals */
	val = readl(pcie->base + PCIE_RST_CTRL_REG);
	val |= PCIE_MAC_RSTB | PCIE_PHY_RSTB | PCIE_BRG_RSTB | PCIE_PE_RSTB;
	writel(val, pcie->base + PCIE_RST_CTRL_REG);

	/*
	 * Described in PCIe CEM specification sections 2.2 (PERST# Signal)
	 * and 2.2.1 (Initial Power-Up (G3 to S0)).
	 * The deassertion of PERST# should be delayed 100ms (TPVPERL)
	 * for the power and clock to become stable.
	 */
	mdelay(100);

	/* De-assert reset signals */
	val &= ~(PCIE_MAC_RSTB | PCIE_PHY_RSTB | PCIE_BRG_RSTB);
	writel(val, pcie->base + PCIE_RST_CTRL_REG);

	mdelay(100);

	/* De-assert PERST# signals */
	val &= ~(PCIE_PE_RSTB);
	writel(val, pcie->base + PCIE_RST_CTRL_REG);

	/* 100ms timeout value should be enough for Gen1/2 training */
	err = readl_poll_timeout(pcie->base + PCIE_LINK_STATUS_REG, val,
				 !!(val & PCIE_PORT_LINKUP),
				 100 * 1000);
	if (err) {
		dev_dbg(dev, "no card detected\n");
		return -ETIMEDOUT;
	}
	dev_dbg(dev, "detected a card\n");

	for (i = 0; i < hose->region_count; i++) {
		struct pci_region *reg = &hose->regions[i];

		if (reg->flags != PCI_REGION_MEM)
			continue;

		mtk_pcie_set_trans_table(dev, pcie, reg->bus_start, reg->phys_start,
					 reg->size, reg->flags, 0);
	}

	return 0;
}

static int mtk_pcie_power_on(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	int err;

	pcie->base = dev_remap_addr_name(dev, "pcie-mac");
	if (!pcie->base)
		return -ENOENT;

	pcie->priv = dev;

	/* pcie-phy is optional (mt7988 doesn't need it) */
	generic_phy_get_by_name(dev, "pcie-phy", &pcie->phy);

	/*
	 * Upstream linux kernel devine these clock without clock-names
	 * and use clk bulk API to enable them all.
	 */
	err = clk_get_by_index(dev, 0, &pcie->pl_250m_ck);
	if (err)
		return err;

	err = clk_get_by_index(dev, 1, &pcie->tl_26m_ck);
	if (err)
		return err;

	err = clk_get_by_index(dev, 2, &pcie->peri_26m_ck);
	if (err)
		return err;

	err = clk_get_by_index(dev, 3, &pcie->top_133m_ck);
	if (err)
		return err;

	if (pcie->phy.dev) {
		err = generic_phy_init(&pcie->phy);
		if (err)
			return err;

		err = generic_phy_power_on(&pcie->phy);
		if (err)
			goto err_phy_on;
	}

	err = clk_enable(&pcie->pl_250m_ck);
	if (err)
		goto err_clk_pl_250m;

	err = clk_enable(&pcie->tl_26m_ck);
	if (err)
		goto err_clk_tl_26m;

	err = clk_enable(&pcie->peri_26m_ck);
	if (err)
		goto err_clk_peri_26m;

	err = clk_enable(&pcie->top_133m_ck);
	if (err)
		goto err_clk_top_133m;

	err = mtk_pcie_startup_port(dev);
	if (err)
		goto err_startup;

	return 0;

err_startup:
err_clk_top_133m:
	clk_disable(&pcie->top_133m_ck);
err_clk_peri_26m:
	clk_disable(&pcie->peri_26m_ck);
err_clk_tl_26m:
	clk_disable(&pcie->tl_26m_ck);
err_clk_pl_250m:
	clk_disable(&pcie->pl_250m_ck);
err_phy_on:
	if (pcie->phy.dev)
		generic_phy_exit(&pcie->phy);

	return err;
}

static int mtk_pcie_probe(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	int err;

	pcie->priv = dev;

	err = mtk_pcie_power_on(dev);
	if (err)
		return err;

	return 0;
}

static const struct udevice_id mtk_pcie_ids[] = {
	{ .compatible = "mediatek,mt8192-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_mediatek_gen3) = {
	.name		= "pcie_mediatek_gen3",
	.id		= UCLASS_PCI,
	.of_match	= mtk_pcie_ids,
	.ops		= &mtk_pcie_ops,
	.probe		= mtk_pcie_probe,
	.priv_auto	= sizeof(struct mtk_pcie),
};
