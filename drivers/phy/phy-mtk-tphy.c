// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 - 2019 MediaTek Inc.
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *	   Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/devres.h>

#include <dt-bindings/phy/phy.h>

/* version V1 sub-banks offset base address */
/* banks shared by multiple phys */
#define SSUSB_SIFSLV_V1_SPLLC		0x000	/* shared by u3 phys */
#define SSUSB_SIFSLV_V1_CHIP		0x300	/* shared by u3 phys */
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V1_U3PHYD		0x000
#define SSUSB_SIFSLV_V1_U3PHYA		0x200

#define U3P_U3_CHIP_GPIO_CTLD		0x0c
#define P3C_REG_IP_SW_RST		BIT(31)
#define P3C_MCU_BUS_CK_GATE_EN		BIT(30)
#define P3C_FORCE_IP_SW_RST		BIT(29)

#define U3P_U3_CHIP_GPIO_CTLE		0x10
#define P3C_RG_SWRST_U3_PHYD		BIT(25)
#define P3C_RG_SWRST_U3_PHYD_FORCE_EN	BIT(24)

#define U3P_U3_PHYA_REG0		0x000
#define P3A_RG_CLKDRV_OFF		GENMASK(3, 2)
#define P3A_RG_CLKDRV_OFF_VAL(x)	((0x3 & (x)) << 2)

#define U3P_U3_PHYA_REG1		0x004
#define P3A_RG_CLKDRV_AMP		GENMASK(31, 29)
#define P3A_RG_CLKDRV_AMP_VAL(x)	((0x7 & (x)) << 29)

#define U3P_U3_PHYA_DA_REG0		0x100
#define P3A_RG_XTAL_EXT_PE2H		GENMASK(17, 16)
#define P3A_RG_XTAL_EXT_PE2H_VAL(x)	((0x3 & (x)) << 16)
#define P3A_RG_XTAL_EXT_PE1H		GENMASK(13, 12)
#define P3A_RG_XTAL_EXT_PE1H_VAL(x)	((0x3 & (x)) << 12)
#define P3A_RG_XTAL_EXT_EN_U3		GENMASK(11, 10)
#define P3A_RG_XTAL_EXT_EN_U3_VAL(x)	((0x3 & (x)) << 10)

#define U3P_U3_PHYA_DA_REG4		0x108
#define P3A_RG_PLL_DIVEN_PE2H		GENMASK(21, 19)
#define P3A_RG_PLL_BC_PE2H		GENMASK(7, 6)
#define P3A_RG_PLL_BC_PE2H_VAL(x)	((0x3 & (x)) << 6)

#define U3P_U3_PHYA_DA_REG5		0x10c
#define P3A_RG_PLL_BR_PE2H		GENMASK(29, 28)
#define P3A_RG_PLL_BR_PE2H_VAL(x)	((0x3 & (x)) << 28)
#define P3A_RG_PLL_IC_PE2H		GENMASK(15, 12)
#define P3A_RG_PLL_IC_PE2H_VAL(x)	((0xf & (x)) << 12)

#define U3P_U3_PHYA_DA_REG6		0x110
#define P3A_RG_PLL_IR_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_IR_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG7		0x114
#define P3A_RG_PLL_BP_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_BP_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG20		0x13c
#define P3A_RG_PLL_DELTA1_PE2H		GENMASK(31, 16)
#define P3A_RG_PLL_DELTA1_PE2H_VAL(x)	((0xffff & (x)) << 16)

#define U3P_U3_PHYA_DA_REG25		0x148
#define P3A_RG_PLL_DELTA_PE2H		GENMASK(15, 0)
#define P3A_RG_PLL_DELTA_PE2H_VAL(x)	(0xffff & (x))

#define U3P_U3_PHYD_RXDET1		0x128
#define P3D_RG_RXDET_STB2_SET		GENMASK(17, 9)
#define P3D_RG_RXDET_STB2_SET_VAL(x)	((0x1ff & (x)) << 9)

#define U3P_U3_PHYD_RXDET2		0x12c
#define P3D_RG_RXDET_STB2_SET_P3	GENMASK(8, 0)
#define P3D_RG_RXDET_STB2_SET_P3_VAL(x)	(0x1ff & (x))

struct u3phy_banks {
	void __iomem *spllc;
	void __iomem *chip;
	void __iomem *phyd; /* include u3phyd_bank2 */
	void __iomem *phya; /* include u3phya_da */
};

struct mtk_phy_instance {
	void __iomem *port_base;
	const struct device_node *np;

	struct u3phy_banks u3_banks;

	/* reference clock of anolog phy */
	struct clk ref_clk;
	u32 index;
	u8 type;
};

struct mtk_tphy {
	void __iomem *sif_base;
	struct mtk_phy_instance **phys;
	int nphys;
};

static void pcie_phy_instance_init(struct mtk_tphy *tphy,
				   struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG0,
			P3A_RG_XTAL_EXT_PE1H | P3A_RG_XTAL_EXT_PE2H,
			P3A_RG_XTAL_EXT_PE1H_VAL(0x2) |
			P3A_RG_XTAL_EXT_PE2H_VAL(0x2));

	/* ref clk drive */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG1, P3A_RG_CLKDRV_AMP,
			P3A_RG_CLKDRV_AMP_VAL(0x4));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG0, P3A_RG_CLKDRV_OFF,
			P3A_RG_CLKDRV_OFF_VAL(0x1));

	/* SSC delta -5000ppm */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG20,
			P3A_RG_PLL_DELTA1_PE2H,
			P3A_RG_PLL_DELTA1_PE2H_VAL(0x3c));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG25,
			P3A_RG_PLL_DELTA_PE2H,
			P3A_RG_PLL_DELTA_PE2H_VAL(0x36));

	/* change pll BW 0.6M */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG5,
			P3A_RG_PLL_BR_PE2H | P3A_RG_PLL_IC_PE2H,
			P3A_RG_PLL_BR_PE2H_VAL(0x1) |
			P3A_RG_PLL_IC_PE2H_VAL(0x1));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG4,
			P3A_RG_PLL_DIVEN_PE2H | P3A_RG_PLL_BC_PE2H,
			P3A_RG_PLL_BC_PE2H_VAL(0x3));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG6,
			P3A_RG_PLL_IR_PE2H, P3A_RG_PLL_IR_PE2H_VAL(0x2));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG7,
			P3A_RG_PLL_BP_PE2H, P3A_RG_PLL_BP_PE2H_VAL(0xa));

	/* Tx Detect Rx Timing: 10us -> 5us */
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET1,
			P3D_RG_RXDET_STB2_SET,
			P3D_RG_RXDET_STB2_SET_VAL(0x10));
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET2,
			P3D_RG_RXDET_STB2_SET_P3,
			P3D_RG_RXDET_STB2_SET_P3_VAL(0x10));

	/* wait for PCIe subsys register to active */
	udelay(3000);
}

static void pcie_phy_instance_power_on(struct mtk_tphy *tphy,
				       struct mtk_phy_instance *instance)
{
	struct u3phy_banks *bank = &instance->u3_banks;

	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void pcie_phy_instance_power_off(struct mtk_tphy *tphy,
					struct mtk_phy_instance *instance)

{
	struct u3phy_banks *bank = &instance->u3_banks;

	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void phy_v1_banks_init(struct mtk_tphy *tphy,
			      struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_PCIE:
		u3_banks->spllc = tphy->sif_base + SSUSB_SIFSLV_V1_SPLLC;
		u3_banks->chip = tphy->sif_base + SSUSB_SIFSLV_V1_CHIP;
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V1_U3PHYD;
		u3_banks->phya = instance->port_base + SSUSB_SIFSLV_V1_U3PHYA;
		break;
	default:
		return;
	}
}

static int mtk_phy_init(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];
	int ret;

	ret = clk_enable(&instance->ref_clk);
	if (ret)
		return ret;

	switch (instance->type) {
	case PHY_TYPE_PCIE:
		pcie_phy_instance_init(tphy, instance);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mtk_phy_power_on(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	pcie_phy_instance_power_on(tphy, instance);

	return 0;
}

static int mtk_phy_power_off(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	pcie_phy_instance_power_off(tphy, instance);

	return 0;
}

static int mtk_phy_exit(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	clk_disable(&instance->ref_clk);

	return 0;
}

static int mtk_phy_xlate(struct phy *phy,
			 struct ofnode_phandle_args *args)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = NULL;
	const struct device_node *phy_np = ofnode_to_np(args->node);
	u32 index;

	if (!phy_np) {
		dev_err(phy->dev, "null pointer phy node\n");
		return -EINVAL;
	}

	if (args->args_count < 1) {
		dev_err(phy->dev, "invalid number of cells in 'phy' property\n");
		return -EINVAL;
	}

	for (index = 0; index < tphy->nphys; index++)
		if (phy_np == tphy->phys[index]->np) {
			instance = tphy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(phy->dev, "failed to find appropriate phy\n");
		return -EINVAL;
	}

	phy->id = index;
	instance->type = args->args[1];
	if (!(instance->type == PHY_TYPE_USB2 ||
	      instance->type == PHY_TYPE_USB3 ||
	      instance->type == PHY_TYPE_PCIE ||
	      instance->type == PHY_TYPE_SATA)) {
		dev_err(phy->dev, "unsupported device type\n");
		return -EINVAL;
	}

	phy_v1_banks_init(tphy, instance);

	return 0;
}

static const struct phy_ops mtk_tphy_ops = {
	.init		= mtk_phy_init,
	.exit		= mtk_phy_exit,
	.power_on	= mtk_phy_power_on,
	.power_off	= mtk_phy_power_off,
	.of_xlate	= mtk_phy_xlate,
};

static int mtk_tphy_probe(struct udevice *dev)
{
	struct mtk_tphy *tphy = dev_get_priv(dev);
	ofnode subnode;
	int index = 0;

	dev_for_each_subnode(subnode, dev)
		tphy->nphys++;

	tphy->phys = devm_kcalloc(dev, tphy->nphys, sizeof(*tphy->phys),
				  GFP_KERNEL);
	if (!tphy->phys)
		return -ENOMEM;

	tphy->sif_base = dev_read_addr_ptr(dev);
	if (!tphy->sif_base)
		return -ENOENT;

	dev_for_each_subnode(subnode, dev) {
		struct mtk_phy_instance *instance;
		fdt_addr_t addr;
		int err;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance)
			return -ENOMEM;

		addr = ofnode_get_addr(subnode);
		if (addr == FDT_ADDR_T_NONE)
			return -ENOMEM;

		instance->port_base = map_sysmem(addr, 0);
		instance->index = index;
		instance->np = ofnode_to_np(subnode);
		tphy->phys[index] = instance;
		index++;

		err = clk_get_optional_nodev(subnode, "ref",
					     &instance->ref_clk);
		if (err)
			return err;
	}

	return 0;
}

static const struct udevice_id mtk_tphy_id_table[] = {
	{ .compatible = "mediatek,generic-tphy-v1", },
	{ }
};

U_BOOT_DRIVER(mtk_tphy) = {
	.name		= "mtk-tphy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_tphy_id_table,
	.ops		= &mtk_tphy_ops,
	.probe		= mtk_tphy_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_tphy),
};
