// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek USB3.1 gen2 xsphy Driver
 *
 * Copyright (c) 2026 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 *
 * Based on Linux mtk-xsphy driver:
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *
 * And U-Boot mtk-tphy driver:
 * Copyright (c) 2015 - 2019 MediaTek Inc.
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *	   Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <mapmem.h>
#include <regmap.h>
#include <syscon.h>

#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include <dt-bindings/phy/phy.h>

/* u2 phy banks */
#define SSUSB_SIFSLV_MISC		0x000
#define SSUSB_SIFSLV_U2FREQ		0x100
#define SSUSB_SIFSLV_U2PHY_COM		0x300

/* u3 phy shared banks */
#define SSPXTP_SIFSLV_DIG_GLB		0x000
#define SSPXTP_SIFSLV_PHYA_GLB		0x100

/* u3 phy banks */
#define SSPXTP_SIFSLV_DIG_LN_TOP	0x000
#define SSPXTP_SIFSLV_DIG_LN_TX0	0x100
#define SSPXTP_SIFSLV_DIG_LN_RX0	0x200
#define SSPXTP_SIFSLV_DIG_LN_DAIF	0x300
#define SSPXTP_SIFSLV_PHYA_LN		0x400

#define XSP_U2FREQ_FMCR0	((SSUSB_SIFSLV_U2FREQ) + 0x00)
#define P2F_RG_FREQDET_EN		BIT(24)
#define P2F_RG_CYCLECNT			GENMASK(23, 0)

#define XSP_U2FREQ_MMONR0	((SSUSB_SIFSLV_U2FREQ) + 0x0c)

#define XSP_U2FREQ_FMMONR1	((SSUSB_SIFSLV_U2FREQ) + 0x10)
#define P2F_RG_FRCK_EN			BIT(8)
#define P2F_USB_FM_VALID		BIT(0)

#define XSP_USBPHYACR0		((SSUSB_SIFSLV_U2PHY_COM) + 0x00)
#define P2A0_RG_INTR_EN			BIT(5)

#define XSP_USBPHYACR1		((SSUSB_SIFSLV_U2PHY_COM) + 0x04)
#define P2A1_RG_INTR_CAL		GENMASK(23, 19)
#define P2A1_RG_VRT_SEL			GENMASK(14, 12)
#define P2A1_RG_TERM_SEL		GENMASK(10, 8)

#define XSP_USBPHYACR5		((SSUSB_SIFSLV_U2PHY_COM) + 0x014)
#define P2A5_RG_HSTX_SRCAL_EN		BIT(15)
#define P2A5_RG_HSTX_SRCTRL		GENMASK(14, 12)

#define XSP_USBPHYACR6		((SSUSB_SIFSLV_U2PHY_COM) + 0x018)
#define P2A6_RG_BC11_SW_EN		BIT(23)
#define P2A6_RG_OTG_VBUSCMP_EN		BIT(20)

#define XSP_U2PHYDTM1		((SSUSB_SIFSLV_U2PHY_COM) + 0x06C)
#define P2D_FORCE_IDDIG			BIT(9)
#define P2D_RG_VBUSVALID		BIT(5)
#define P2D_RG_SESSEND			BIT(4)
#define P2D_RG_AVALID			BIT(2)
#define P2D_RG_IDDIG			BIT(1)

#define SSPXTP_PHYA_GLB_00	((SSPXTP_SIFSLV_PHYA_GLB) + 0x00)
#define RG_XTP_GLB_BIAS_INTR_CTRL	GENMASK(21, 16)

#define SSPXTP_PHYA_LN_04	((SSPXTP_SIFSLV_PHYA_LN) + 0x04)
#define RG_XTP_LN0_TX_IMPSEL		GENMASK(4, 0)

#define SSPXTP_PHYA_LN_14	((SSPXTP_SIFSLV_PHYA_LN) + 0x014)
#define RG_XTP_LN0_RX_IMPSEL		GENMASK(4, 0)

#define XSP_REF_CLK_MHZ		26
#define XSP_SLEW_RATE_COEF	17
#define XSP_SR_COEF_DIVISOR	1000
#define XSP_FM_DET_CYCLE_CNT	1024

/* PHY switch between pcie/usb3/sgmii */
#define USB_PHY_SWITCH_CTRL	0x0
#define RG_PHY_SW_TYPE		GENMASK(3, 0)
#define RG_PHY_SW_PCIE		0x0
#define RG_PHY_SW_USB3		0x1
#define RG_PHY_SW_SGMII		0x2

struct mtk_xsphy_instance {
	void __iomem *port_base;
	struct device_node *np;
	struct clk ref_clk;	/* reference clock of analog phy */
	u32 index;
	u32 type;
	struct regmap *type_sw;
	u32 type_sw_reg;
	u32 type_sw_index;
	/* only for HQA test */
	u32 efuse_intr;
	u32 efuse_tx_imp;
	u32 efuse_rx_imp;
	/* u2 eye diagram */
	u32 eye_src;
	u32 eye_vrt;
	u32 eye_term;
};

struct mtk_xsphy {
	struct udevice *dev;
	void __iomem *sif_base;
	struct mtk_xsphy_instance **phys;
	u32 nphys;
	u32 src_ref_clk_mhz; /* reference clock for slew rate calibrate */
	u32 src_coef; /* coefficient for slew rate calibrate */
};

static void mtk_xsphy_u2_slew_rate_calibrate(struct mtk_xsphy *xsphy,
					     struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;
	u32 calib_val;
	u32 fm_out;
	u32 tmp;

	/* use force value */
	if (instance->eye_src)
		return;

	/* enable USB ring oscillator */
	setbits_le32(pbase + XSP_USBPHYACR5, P2A5_RG_HSTX_SRCAL_EN);
	/* wait for clock to become stable */
	udelay(1);

	/* enable free run clock */
	setbits_le32(pbase + XSP_U2FREQ_FMMONR1, P2F_RG_FRCK_EN);

	/* set cycle count as 1024 */
	clrsetbits_le32(pbase + XSP_U2FREQ_FMCR0, P2F_RG_CYCLECNT,
			FIELD_PREP(P2F_RG_CYCLECNT, XSP_FM_DET_CYCLE_CNT));

	/* enable frequency meter */
	setbits_le32(pbase + XSP_U2FREQ_FMCR0, P2F_RG_FREQDET_EN);

	/* ignore return value */
	readl_poll_sleep_timeout(pbase + XSP_U2FREQ_FMMONR1, tmp,
				 (tmp & P2F_USB_FM_VALID), 10, 200);

	fm_out = readl(pbase + XSP_U2FREQ_MMONR0);

	/* disable frequency meter */
	clrbits_le32(pbase + XSP_U2FREQ_FMCR0, P2F_RG_FREQDET_EN);

	/* disable free run clock */
	clrbits_le32(pbase + XSP_U2FREQ_FMMONR1, P2F_RG_FRCK_EN);

	if (fm_out) {
		/* (1024 / FM_OUT) x reference clock frequency x coefficient */
		tmp = xsphy->src_ref_clk_mhz * xsphy->src_coef;
		tmp = (tmp * XSP_FM_DET_CYCLE_CNT) / fm_out;
		calib_val = DIV_ROUND_CLOSEST(tmp, XSP_SR_COEF_DIVISOR);
	} else {
		/* if FM detection fail, set default value */
		calib_val = 3;
	}
	dev_dbg(xsphy->dev, "phy.%u, fm_out:%u, calib:%u (clk:%u, coef:%u)\n",
		instance->index, fm_out, calib_val, xsphy->src_ref_clk_mhz,
		xsphy->src_coef);

	/* set HS slew rate */
	clrsetbits_le32(pbase + XSP_USBPHYACR5, P2A5_RG_HSTX_SRCTRL,
			FIELD_PREP(P2A5_RG_HSTX_SRCTRL, calib_val));

	/* disable USB ring oscillator */
	clrbits_le32(pbase + XSP_USBPHYACR5, P2A5_RG_HSTX_SRCAL_EN);
}

static void mtk_xsphy_u2_instance_init(struct mtk_xsphy *xsphy,
				       struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;

	/* DP/DM BC1.1 path Disable */
	clrbits_le32(pbase + XSP_USBPHYACR6, P2A6_RG_BC11_SW_EN);

	setbits_le32(pbase + XSP_USBPHYACR0, P2A0_RG_INTR_EN);
}

static void mtk_xsphy_u2_instance_power_on(struct mtk_xsphy *xsphy,
					   struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;

	setbits_le32(pbase + XSP_USBPHYACR6, P2A6_RG_OTG_VBUSCMP_EN);

	clrsetbits_le32(pbase + XSP_U2PHYDTM1,
			P2D_RG_VBUSVALID | P2D_RG_AVALID | P2D_RG_SESSEND,
			P2D_RG_VBUSVALID | P2D_RG_AVALID);

	dev_dbg(xsphy->dev, "%s(%u)\n", __func__, instance->index);
}

static void mtk_xsphy_u2_instance_power_off(struct mtk_xsphy *xsphy,
					    struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;

	clrbits_le32(pbase + XSP_USBPHYACR6, P2A6_RG_OTG_VBUSCMP_EN);

	clrsetbits_le32(pbase + XSP_U2PHYDTM1,
			P2D_RG_VBUSVALID | P2D_RG_AVALID | P2D_RG_SESSEND,
			P2D_RG_SESSEND);

	dev_dbg(xsphy->dev, "%s(%u)\n", __func__, instance->index);
}

static void mtk_xsphy_u2_instance_set_mode(struct mtk_xsphy *xsphy,
					   struct mtk_xsphy_instance *instance,
					   enum phy_mode mode)
{
	u32 tmp;

	tmp = readl(instance->port_base + XSP_U2PHYDTM1);

	switch (mode) {
	case PHY_MODE_USB_DEVICE:
		tmp |= P2D_FORCE_IDDIG | P2D_RG_IDDIG;
		break;
	case PHY_MODE_USB_HOST:
		tmp |= P2D_FORCE_IDDIG;
		tmp &= ~P2D_RG_IDDIG;
		break;
	case PHY_MODE_USB_OTG:
		tmp &= ~(P2D_FORCE_IDDIG | P2D_RG_IDDIG);
		break;
	default:
		return;
	}

	writel(tmp, instance->port_base + XSP_U2PHYDTM1);
}

static void mtk_xsphy_parse_property(struct mtk_xsphy *xsphy,
				     struct mtk_xsphy_instance *instance)
{
	ofnode node = np_to_ofnode(instance->np);

	switch (instance->type) {
	case PHY_TYPE_USB2:
		ofnode_read_u32(node, "mediatek,efuse-intr", &instance->efuse_intr);
		ofnode_read_u32(node, "mediatek,eye-src", &instance->eye_src);
		ofnode_read_u32(node, "mediatek,eye-vrt", &instance->eye_vrt);
		ofnode_read_u32(node, "mediatek,eye-term", &instance->eye_term);

		dev_dbg(xsphy->dev, "intr:%u, src:%u, vrt:%u, term:%u\n",
			instance->efuse_intr, instance->eye_src,
			instance->eye_vrt, instance->eye_term);
		return;
	case PHY_TYPE_USB3:
		ofnode_read_u32(node, "mediatek,efuse-intr", &instance->efuse_intr);
		ofnode_read_u32(node, "mediatek,efuse-tx-imp", &instance->efuse_tx_imp);
		ofnode_read_u32(node, "mediatek,efuse-rx-imp", &instance->efuse_rx_imp);

		dev_dbg(xsphy->dev, "intr:%u, tx-imp:%u, rx-imp:%u\n",
			instance->efuse_intr, instance->efuse_tx_imp,
			instance->efuse_rx_imp);
		return;
	case PHY_TYPE_PCIE:
	case PHY_TYPE_SGMII:
		/* nothing to do */
		return;
	default:
		dev_err(xsphy->dev, "incompatible PHY type\n");
		return;
	}
}

static void mtk_xsphy_u2_props_set(struct mtk_xsphy *xsphy,
				   struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;

	if (instance->efuse_intr)
		clrsetbits_le32(pbase + XSP_USBPHYACR1, P2A1_RG_INTR_CAL,
				FIELD_PREP(P2A1_RG_INTR_CAL, instance->efuse_intr));

	if (instance->eye_src)
		clrsetbits_le32(pbase + XSP_USBPHYACR5, P2A5_RG_HSTX_SRCTRL,
				FIELD_PREP(P2A5_RG_HSTX_SRCTRL, instance->eye_src));

	if (instance->eye_vrt)
		clrsetbits_le32(pbase + XSP_USBPHYACR1, P2A1_RG_VRT_SEL,
				FIELD_PREP(P2A1_RG_VRT_SEL, instance->eye_vrt));

	if (instance->eye_term)
		clrsetbits_le32(pbase + XSP_USBPHYACR1, P2A1_RG_TERM_SEL,
				FIELD_PREP(P2A1_RG_TERM_SEL, instance->eye_term));
}

static void mtk_xsphy_u3_props_set(struct mtk_xsphy *xsphy,
				   struct mtk_xsphy_instance *instance)
{
	void __iomem *pbase = instance->port_base;

	if (instance->efuse_intr)
		clrsetbits_le32(xsphy->sif_base + SSPXTP_PHYA_GLB_00,
				RG_XTP_GLB_BIAS_INTR_CTRL,
				FIELD_PREP(RG_XTP_GLB_BIAS_INTR_CTRL, instance->efuse_intr));

	if (instance->efuse_tx_imp)
		clrsetbits_le32(pbase + SSPXTP_PHYA_LN_04, RG_XTP_LN0_TX_IMPSEL,
				FIELD_PREP(RG_XTP_LN0_TX_IMPSEL, instance->efuse_tx_imp));

	if (instance->efuse_rx_imp)
		clrsetbits_le32(pbase + SSPXTP_PHYA_LN_14, RG_XTP_LN0_RX_IMPSEL,
				FIELD_PREP(RG_XTP_LN0_RX_IMPSEL, instance->efuse_rx_imp));
}

/* type switch for usb3/pcie/sgmii */
static int mtk_xsphy_type_syscon_get(struct udevice *dev,
				     struct mtk_xsphy_instance *instance,
				     ofnode dn)
{
	struct ofnode_phandle_args args;
	int ret;

	if (!ofnode_read_bool(dn, "mediatek,syscon-type"))
		return 0;

	ret = ofnode_parse_phandle_with_args(dn, "mediatek,syscon-type",
					     NULL, 2, 0, &args);
	if (ret)
		return ret;

	instance->type_sw_reg = args.args[0];
	instance->type_sw_index = args.args[1] & 0x3; /* <=3 */
	instance->type_sw = syscon_node_to_regmap(args.node);
	if (IS_ERR(instance->type_sw))
		return PTR_ERR(instance->type_sw);

	dev_dbg(dev, "phy-%s.%d: type_sw - reg %#x, index %d\n",
		dev->name, instance->index, instance->type_sw_reg,
		instance->type_sw_index);

	return 0;
}

static int mtk_xsphy_type_set(struct mtk_xsphy_instance *instance)
{
	int type;
	u32 offset;

	if (!instance->type_sw)
		return 0;

	switch (instance->type) {
	case PHY_TYPE_USB3:
		type = RG_PHY_SW_USB3;
		break;
	case PHY_TYPE_PCIE:
		type = RG_PHY_SW_PCIE;
		break;
	case PHY_TYPE_SGMII:
		type = RG_PHY_SW_SGMII;
		break;
	case PHY_TYPE_USB2:
	default:
		return 0;
	}

	offset = instance->type_sw_index * BITS_PER_BYTE;
	regmap_update_bits(instance->type_sw, instance->type_sw_reg,
			   RG_PHY_SW_TYPE << offset, type << offset);

	return 0;
}

static int mtk_xsphy_init(struct phy *phy)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = xsphy->phys[phy->id];
	int ret;

	ret = clk_enable(&instance->ref_clk);
	if (ret) {
		dev_err(xsphy->dev, "failed to enable ref_clk\n");
		return ret;
	}

	switch (instance->type) {
	case PHY_TYPE_USB2:
		mtk_xsphy_u2_instance_init(xsphy, instance);
		mtk_xsphy_u2_props_set(xsphy, instance);
		break;
	case PHY_TYPE_USB3:
		mtk_xsphy_u3_props_set(xsphy, instance);
		break;
	case PHY_TYPE_PCIE:
	case PHY_TYPE_SGMII:
		/* nothing to do, only used to set type */
		break;
	default:
		dev_err(xsphy->dev, "incompatible PHY type\n");
		clk_disable(&instance->ref_clk);
		return -EINVAL;
	}

	return 0;
}

static int mtk_xsphy_power_on(struct phy *phy)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = xsphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2) {
		mtk_xsphy_u2_instance_power_on(xsphy, instance);
		mtk_xsphy_u2_slew_rate_calibrate(xsphy, instance);
	}

	return 0;
}

static int mtk_xsphy_power_off(struct phy *phy)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = xsphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		mtk_xsphy_u2_instance_power_off(xsphy, instance);

	return 0;
}

static int mtk_xsphy_exit(struct phy *phy)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = xsphy->phys[phy->id];

	clk_disable(&instance->ref_clk);

	return 0;
}

static int mtk_xsphy_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = xsphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		mtk_xsphy_u2_instance_set_mode(xsphy, instance, mode);

	return 0;
}

static int mtk_xsphy_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	struct mtk_xsphy *xsphy = dev_get_priv(phy->dev);
	struct mtk_xsphy_instance *instance = NULL;
	const struct device_node *phy_np = ofnode_to_np(args->node);
	u32 index;

	if (!phy_np) {
		dev_err(phy->dev, "null pointer phy node\n");
		return -EINVAL;
	}

	if (args->args_count != 2) {
		dev_err(phy->dev, "invalid number of cells in 'phy' property\n");
		return -EINVAL;
	}

	for (index = 0; index < xsphy->nphys; index++)
		if (phy_np == xsphy->phys[index]->np) {
			instance = xsphy->phys[index];
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
	      instance->type == PHY_TYPE_SGMII)) {
		dev_err(phy->dev, "unsupported PHY type\n");
		return -EINVAL;
	}

	mtk_xsphy_parse_property(xsphy, instance);
	mtk_xsphy_type_set(instance);

	return 0;
}

static const struct phy_ops mtk_xsphy_ops = {
	.init		= mtk_xsphy_init,
	.exit		= mtk_xsphy_exit,
	.power_on	= mtk_xsphy_power_on,
	.power_off	= mtk_xsphy_power_off,
	.set_mode	= mtk_xsphy_set_mode,
	.of_xlate	= mtk_xsphy_xlate,
};

static int mtk_xsphy_probe(struct udevice *dev)
{
	struct mtk_xsphy *xsphy = dev_get_priv(dev);
	fdt_addr_t sif_addr;
	ofnode subnode;
	int index = 0;

	xsphy->nphys = dev_get_child_count(dev);

	xsphy->phys = devm_kcalloc(dev, xsphy->nphys, sizeof(*xsphy->phys),
				   GFP_KERNEL);
	if (!xsphy->phys)
		return -ENOMEM;

	xsphy->dev = dev;

	sif_addr = ofnode_get_addr(dev_ofnode(dev));
	/* optional, may not exist if no u3 phys */
	if (sif_addr != FDT_ADDR_T_NONE)
		xsphy->sif_base = map_sysmem(sif_addr, 0);

	xsphy->src_ref_clk_mhz = XSP_REF_CLK_MHZ;
	xsphy->src_coef = XSP_SLEW_RATE_COEF;
	/* update parameters of slew rate calibrate if exist */
	ofnode_read_u32(dev_ofnode(dev), "mediatek,src-ref-clk-mhz",
			&xsphy->src_ref_clk_mhz);
	ofnode_read_u32(dev_ofnode(dev), "mediatek,src-coef", &xsphy->src_coef);

	dev_for_each_subnode(subnode, dev) {
		struct mtk_xsphy_instance *inst;
		fdt_addr_t addr;
		int ret;

		inst = devm_kzalloc(dev, sizeof(*inst), GFP_KERNEL);
		if (!inst)
			return -ENOMEM;

		xsphy->phys[index] = inst;

		addr = ofnode_get_addr(subnode);
		if (addr == FDT_ADDR_T_NONE)
			return -EADDRNOTAVAIL;

		inst->port_base = map_sysmem(addr, 0);
		inst->index = index;
		inst->np = ofnode_to_np(subnode);

		ret = clk_get_by_name_nodev(subnode, "ref", &inst->ref_clk);
		if (ret) {
			dev_err(dev, "failed to get ref_clk(id-%d)\n", index);
			return ret;
		}

		ret = mtk_xsphy_type_syscon_get(dev, inst, subnode);
		if (ret)
			return ret;

		index++;
	}

	return 0;
}

static const struct udevice_id mtk_xsphy_id_table[] = {
	{ .compatible = "mediatek,xsphy" },
	{ }
};

U_BOOT_DRIVER(mtk_xsphy) = {
	.name		= "mtk-xsphy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_xsphy_id_table,
	.ops		= &mtk_xsphy_ops,
	.probe		= mtk_xsphy_probe,
	.priv_auto	= sizeof(struct mtk_xsphy),
};
