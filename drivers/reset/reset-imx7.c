// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017, Impinj, Inc.
 */

#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm.h>
#include <dt-bindings/reset/imx7-reset.h>
#include <dt-bindings/reset/imx8mp-reset.h>
#include <dt-bindings/reset/imx8mq-reset.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <linux/delay.h>

struct imx_reset_priv {
	void __iomem *base;
	struct reset_ops ops;
};

struct imx7_src_signal {
	unsigned int offset, bit;
};

enum imx7_src_registers {
	SRC_A7RCR0		= 0x0004,
	SRC_M4RCR		= 0x000c,
	SRC_ERCR		= 0x0014,
	SRC_HSICPHY_RCR		= 0x001c,
	SRC_USBOPHY1_RCR	= 0x0020,
	SRC_USBOPHY2_RCR	= 0x0024,
	SRC_MIPIPHY_RCR		= 0x0028,
	SRC_PCIEPHY_RCR		= 0x002c,
	SRC_DDRC_RCR		= 0x1000,
};

static const struct imx7_src_signal imx7_src_signals[IMX7_RESET_NUM] = {
	[IMX7_RESET_A7_CORE_POR_RESET0]	= { SRC_A7RCR0, BIT(0) },
	[IMX7_RESET_A7_CORE_POR_RESET1]	= { SRC_A7RCR0, BIT(1) },
	[IMX7_RESET_A7_CORE_RESET0]	= { SRC_A7RCR0, BIT(4) },
	[IMX7_RESET_A7_CORE_RESET1]	= { SRC_A7RCR0, BIT(5) },
	[IMX7_RESET_A7_DBG_RESET0]	= { SRC_A7RCR0, BIT(8) },
	[IMX7_RESET_A7_DBG_RESET1]	= { SRC_A7RCR0, BIT(9) },
	[IMX7_RESET_A7_ETM_RESET0]	= { SRC_A7RCR0, BIT(12) },
	[IMX7_RESET_A7_ETM_RESET1]	= { SRC_A7RCR0, BIT(13) },
	[IMX7_RESET_A7_SOC_DBG_RESET]	= { SRC_A7RCR0, BIT(20) },
	[IMX7_RESET_A7_L2RESET]		= { SRC_A7RCR0, BIT(21) },
	[IMX7_RESET_SW_M4C_RST]		= { SRC_M4RCR, BIT(1) },
	[IMX7_RESET_SW_M4P_RST]		= { SRC_M4RCR, BIT(2) },
	[IMX7_RESET_EIM_RST]		= { SRC_ERCR, BIT(0) },
	[IMX7_RESET_HSICPHY_PORT_RST]	= { SRC_HSICPHY_RCR, BIT(1) },
	[IMX7_RESET_USBPHY1_POR]	= { SRC_USBOPHY1_RCR, BIT(0) },
	[IMX7_RESET_USBPHY1_PORT_RST]	= { SRC_USBOPHY1_RCR, BIT(1) },
	[IMX7_RESET_USBPHY2_POR]	= { SRC_USBOPHY2_RCR, BIT(0) },
	[IMX7_RESET_USBPHY2_PORT_RST]	= { SRC_USBOPHY2_RCR, BIT(1) },
	[IMX7_RESET_MIPI_PHY_MRST]	= { SRC_MIPIPHY_RCR, BIT(1) },
	[IMX7_RESET_MIPI_PHY_SRST]	= { SRC_MIPIPHY_RCR, BIT(2) },
	[IMX7_RESET_PCIEPHY]		= { SRC_PCIEPHY_RCR, BIT(2) | BIT(1) },
	[IMX7_RESET_PCIEPHY_PERST]	= { SRC_PCIEPHY_RCR, BIT(3) },
	[IMX7_RESET_PCIE_CTRL_APPS_EN]	= { SRC_PCIEPHY_RCR, BIT(6) },
	[IMX7_RESET_PCIE_CTRL_APPS_TURNOFF] = { SRC_PCIEPHY_RCR, BIT(11) },
	[IMX7_RESET_DDRC_PRST]		= { SRC_DDRC_RCR, BIT(0) },
	[IMX7_RESET_DDRC_CORE_RST]	= { SRC_DDRC_RCR, BIT(1) },
};

static int imx7_reset_deassert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	const struct imx7_src_signal *sig = imx7_src_signals;
	u32 val;

	if (rst->id >= IMX7_RESET_NUM)
		return -EINVAL;

	if (rst->id == IMX7_RESET_PCIEPHY) {
		/*
		 * wait for more than 10us to release phy g_rst and
		 * btnrst
		 */
		udelay(10);
	}

	val = readl(priv->base + sig[rst->id].offset);
	switch (rst->id) {
	case IMX7_RESET_PCIE_CTRL_APPS_EN:
		val |= sig[rst->id].bit;
		break;
	default:
		val &= ~sig[rst->id].bit;
		break;
	}
	writel(val, priv->base + sig[rst->id].offset);

	return 0;
}

static int imx7_reset_assert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	const struct imx7_src_signal *sig = imx7_src_signals;
	u32 val;

	if (rst->id >= IMX7_RESET_NUM)
		return -EINVAL;

	val = readl(priv->base + sig[rst->id].offset);
	switch (rst->id) {
	case IMX7_RESET_PCIE_CTRL_APPS_EN:
		val &= ~sig[rst->id].bit;
		break;
	default:
		val |= sig[rst->id].bit;
		break;
	}
	writel(val, priv->base + sig[rst->id].offset);

	return 0;
}

enum imx8mq_src_registers {
	SRC_A53RCR0		= 0x0004,
	SRC_HDMI_RCR		= 0x0030,
	SRC_DISP_RCR		= 0x0034,
	SRC_GPU_RCR		= 0x0040,
	SRC_VPU_RCR		= 0x0044,
	SRC_PCIE2_RCR		= 0x0048,
	SRC_MIPIPHY1_RCR	= 0x004c,
	SRC_MIPIPHY2_RCR	= 0x0050,
	SRC_DDRC2_RCR		= 0x1004,
};

static const struct imx7_src_signal imx8mq_src_signals[IMX8MQ_RESET_NUM] = {
	[IMX8MQ_RESET_A53_CORE_POR_RESET0]	= { SRC_A53RCR0, BIT(0) },
	[IMX8MQ_RESET_A53_CORE_POR_RESET1]	= { SRC_A53RCR0, BIT(1) },
	[IMX8MQ_RESET_A53_CORE_POR_RESET2]	= { SRC_A53RCR0, BIT(2) },
	[IMX8MQ_RESET_A53_CORE_POR_RESET3]	= { SRC_A53RCR0, BIT(3) },
	[IMX8MQ_RESET_A53_CORE_RESET0]		= { SRC_A53RCR0, BIT(4) },
	[IMX8MQ_RESET_A53_CORE_RESET1]		= { SRC_A53RCR0, BIT(5) },
	[IMX8MQ_RESET_A53_CORE_RESET2]		= { SRC_A53RCR0, BIT(6) },
	[IMX8MQ_RESET_A53_CORE_RESET3]		= { SRC_A53RCR0, BIT(7) },
	[IMX8MQ_RESET_A53_DBG_RESET0]		= { SRC_A53RCR0, BIT(8) },
	[IMX8MQ_RESET_A53_DBG_RESET1]		= { SRC_A53RCR0, BIT(9) },
	[IMX8MQ_RESET_A53_DBG_RESET2]		= { SRC_A53RCR0, BIT(10) },
	[IMX8MQ_RESET_A53_DBG_RESET3]		= { SRC_A53RCR0, BIT(11) },
	[IMX8MQ_RESET_A53_ETM_RESET0]		= { SRC_A53RCR0, BIT(12) },
	[IMX8MQ_RESET_A53_ETM_RESET1]		= { SRC_A53RCR0, BIT(13) },
	[IMX8MQ_RESET_A53_ETM_RESET2]		= { SRC_A53RCR0, BIT(14) },
	[IMX8MQ_RESET_A53_ETM_RESET3]		= { SRC_A53RCR0, BIT(15) },
	[IMX8MQ_RESET_A53_SOC_DBG_RESET]	= { SRC_A53RCR0, BIT(20) },
	[IMX8MQ_RESET_A53_L2RESET]		= { SRC_A53RCR0, BIT(21) },
	[IMX8MQ_RESET_SW_NON_SCLR_M4C_RST]	= { SRC_M4RCR, BIT(0) },
	[IMX8MQ_RESET_OTG1_PHY_RESET]		= { SRC_USBOPHY1_RCR, BIT(0) },
	[IMX8MQ_RESET_OTG2_PHY_RESET]		= { SRC_USBOPHY2_RCR, BIT(0) },
	[IMX8MQ_RESET_MIPI_DSI_RESET_BYTE_N]	= { SRC_MIPIPHY_RCR, BIT(1) },
	[IMX8MQ_RESET_MIPI_DSI_RESET_N]		= { SRC_MIPIPHY_RCR, BIT(2) },
	[IMX8MQ_RESET_MIPI_DSI_DPI_RESET_N]	= { SRC_MIPIPHY_RCR, BIT(3) },
	[IMX8MQ_RESET_MIPI_DSI_ESC_RESET_N]	= { SRC_MIPIPHY_RCR, BIT(4) },
	[IMX8MQ_RESET_MIPI_DSI_PCLK_RESET_N]	= { SRC_MIPIPHY_RCR, BIT(5) },
	[IMX8MQ_RESET_PCIEPHY]			= { SRC_PCIEPHY_RCR,
						    BIT(2) | BIT(1) },
	[IMX8MQ_RESET_PCIEPHY_PERST]		= { SRC_PCIEPHY_RCR, BIT(3) },
	[IMX8MQ_RESET_PCIE_CTRL_APPS_EN]	= { SRC_PCIEPHY_RCR, BIT(6) },
	[IMX8MQ_RESET_PCIE_CTRL_APPS_TURNOFF]	= { SRC_PCIEPHY_RCR, BIT(11) },
	[IMX8MQ_RESET_HDMI_PHY_APB_RESET]	= { SRC_HDMI_RCR, BIT(0) },
	[IMX8MQ_RESET_DISP_RESET]		= { SRC_DISP_RCR, BIT(0) },
	[IMX8MQ_RESET_GPU_RESET]		= { SRC_GPU_RCR, BIT(0) },
	[IMX8MQ_RESET_VPU_RESET]		= { SRC_VPU_RCR, BIT(0) },
	[IMX8MQ_RESET_PCIEPHY2]			= { SRC_PCIE2_RCR,
						    BIT(2) | BIT(1) },
	[IMX8MQ_RESET_PCIEPHY2_PERST]		= { SRC_PCIE2_RCR, BIT(3) },
	[IMX8MQ_RESET_PCIE2_CTRL_APPS_EN]	= { SRC_PCIE2_RCR, BIT(6) },
	[IMX8MQ_RESET_PCIE2_CTRL_APPS_TURNOFF]	= { SRC_PCIE2_RCR, BIT(11) },
	[IMX8MQ_RESET_MIPI_CSI1_CORE_RESET]	= { SRC_MIPIPHY1_RCR, BIT(0) },
	[IMX8MQ_RESET_MIPI_CSI1_PHY_REF_RESET]	= { SRC_MIPIPHY1_RCR, BIT(1) },
	[IMX8MQ_RESET_MIPI_CSI1_ESC_RESET]	= { SRC_MIPIPHY1_RCR, BIT(2) },
	[IMX8MQ_RESET_MIPI_CSI2_CORE_RESET]	= { SRC_MIPIPHY2_RCR, BIT(0) },
	[IMX8MQ_RESET_MIPI_CSI2_PHY_REF_RESET]	= { SRC_MIPIPHY2_RCR, BIT(1) },
	[IMX8MQ_RESET_MIPI_CSI2_ESC_RESET]	= { SRC_MIPIPHY2_RCR, BIT(2) },
	[IMX8MQ_RESET_DDRC1_PRST]		= { SRC_DDRC_RCR, BIT(0) },
	[IMX8MQ_RESET_DDRC1_CORE_RESET]		= { SRC_DDRC_RCR, BIT(1) },
	[IMX8MQ_RESET_DDRC1_PHY_RESET]		= { SRC_DDRC_RCR, BIT(2) },
	[IMX8MQ_RESET_DDRC2_PHY_RESET]		= { SRC_DDRC2_RCR, BIT(0) },
	[IMX8MQ_RESET_DDRC2_CORE_RESET]		= { SRC_DDRC2_RCR, BIT(1) },
	[IMX8MQ_RESET_DDRC2_PRST]		= { SRC_DDRC2_RCR, BIT(2) },
};

static int imx8mq_reset_deassert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	const struct imx7_src_signal *sig = imx8mq_src_signals;
	u32 val;

	if (rst->id >= IMX8MQ_RESET_NUM)
		return -EINVAL;

	if (rst->id == IMX8MQ_RESET_PCIEPHY ||
	    rst->id == IMX8MQ_RESET_PCIEPHY2) {
		/*
		 * wait for more than 10us to release phy g_rst and
		 * btnrst
		 */
		udelay(10);
	}

	val = readl(priv->base + sig[rst->id].offset);
	switch (rst->id) {
	case IMX8MQ_RESET_PCIE_CTRL_APPS_EN:
	case IMX8MQ_RESET_PCIE2_CTRL_APPS_EN:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_PCLK_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_ESC_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_DPI_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_RESET_BYTE_N:	/* fallthrough */
		val |= sig[rst->id].bit;
		break;
	default:
		val &= ~sig[rst->id].bit;
		break;
	}
	writel(val, priv->base + sig[rst->id].offset);

	return 0;
}

static int imx8mq_reset_assert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	const struct imx7_src_signal *sig = imx8mq_src_signals;
	u32 val;

	if (rst->id >= IMX8MQ_RESET_NUM)
		return -EINVAL;

	val = readl(priv->base + sig[rst->id].offset);
	switch (rst->id) {
	case IMX8MQ_RESET_PCIE_CTRL_APPS_EN:
	case IMX8MQ_RESET_PCIE2_CTRL_APPS_EN:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_PCLK_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_ESC_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_DPI_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_RESET_N:	/* fallthrough */
	case IMX8MQ_RESET_MIPI_DSI_RESET_BYTE_N:	/* fallthrough */
		val &= ~sig[rst->id].bit;
		break;
	default:
		val |= sig[rst->id].bit;
		break;
	}
	writel(val, priv->base + sig[rst->id].offset);

	return 0;
}

enum imx8mp_src_registers {
	SRC_SUPERMIX_RCR	= 0x0018,
	SRC_AUDIOMIX_RCR	= 0x001c,
	SRC_MLMIX_RCR		= 0x0028,
	SRC_GPU2D_RCR		= 0x0038,
	SRC_GPU3D_RCR		= 0x003c,
	SRC_VPU_G1_RCR		= 0x0048,
	SRC_VPU_G2_RCR		= 0x004c,
	SRC_VPUVC8KE_RCR	= 0x0050,
	SRC_NOC_RCR		= 0x0054,
};

static const struct imx7_src_signal imx8mp_src_signals[IMX8MP_RESET_NUM] = {
	[IMX8MP_RESET_A53_CORE_POR_RESET0]	= { SRC_A53RCR0, BIT(0) },
	[IMX8MP_RESET_A53_CORE_POR_RESET1]	= { SRC_A53RCR0, BIT(1) },
	[IMX8MP_RESET_A53_CORE_POR_RESET2]	= { SRC_A53RCR0, BIT(2) },
	[IMX8MP_RESET_A53_CORE_POR_RESET3]	= { SRC_A53RCR0, BIT(3) },
	[IMX8MP_RESET_A53_CORE_RESET0]		= { SRC_A53RCR0, BIT(4) },
	[IMX8MP_RESET_A53_CORE_RESET1]		= { SRC_A53RCR0, BIT(5) },
	[IMX8MP_RESET_A53_CORE_RESET2]		= { SRC_A53RCR0, BIT(6) },
	[IMX8MP_RESET_A53_CORE_RESET3]		= { SRC_A53RCR0, BIT(7) },
	[IMX8MP_RESET_A53_DBG_RESET0]		= { SRC_A53RCR0, BIT(8) },
	[IMX8MP_RESET_A53_DBG_RESET1]		= { SRC_A53RCR0, BIT(9) },
	[IMX8MP_RESET_A53_DBG_RESET2]		= { SRC_A53RCR0, BIT(10) },
	[IMX8MP_RESET_A53_DBG_RESET3]		= { SRC_A53RCR0, BIT(11) },
	[IMX8MP_RESET_A53_ETM_RESET0]		= { SRC_A53RCR0, BIT(12) },
	[IMX8MP_RESET_A53_ETM_RESET1]		= { SRC_A53RCR0, BIT(13) },
	[IMX8MP_RESET_A53_ETM_RESET2]		= { SRC_A53RCR0, BIT(14) },
	[IMX8MP_RESET_A53_ETM_RESET3]		= { SRC_A53RCR0, BIT(15) },
	[IMX8MP_RESET_A53_SOC_DBG_RESET]	= { SRC_A53RCR0, BIT(20) },
	[IMX8MP_RESET_A53_L2RESET]		= { SRC_A53RCR0, BIT(21) },
	[IMX8MP_RESET_SW_NON_SCLR_M7C_RST]	= { SRC_M4RCR, BIT(0) },
	[IMX8MP_RESET_OTG1_PHY_RESET]		= { SRC_USBOPHY1_RCR, BIT(0) },
	[IMX8MP_RESET_OTG2_PHY_RESET]		= { SRC_USBOPHY2_RCR, BIT(0) },
	[IMX8MP_RESET_SUPERMIX_RESET]		= { SRC_SUPERMIX_RCR, BIT(0) },
	[IMX8MP_RESET_AUDIOMIX_RESET]		= { SRC_AUDIOMIX_RCR, BIT(0) },
	[IMX8MP_RESET_MLMIX_RESET]		= { SRC_MLMIX_RCR, BIT(0) },
	[IMX8MP_RESET_PCIEPHY]			= { SRC_PCIEPHY_RCR, BIT(2) },
	[IMX8MP_RESET_PCIEPHY_PERST]		= { SRC_PCIEPHY_RCR, BIT(3) },
	[IMX8MP_RESET_PCIE_CTRL_APPS_EN]	= { SRC_PCIEPHY_RCR, BIT(6) },
	[IMX8MP_RESET_PCIE_CTRL_APPS_TURNOFF]	= { SRC_PCIEPHY_RCR, BIT(11) },
	[IMX8MP_RESET_HDMI_PHY_APB_RESET]	= { SRC_HDMI_RCR, BIT(0) },
	[IMX8MP_RESET_MEDIA_RESET]		= { SRC_DISP_RCR, BIT(0) },
	[IMX8MP_RESET_GPU2D_RESET]		= { SRC_GPU2D_RCR, BIT(0) },
	[IMX8MP_RESET_GPU3D_RESET]		= { SRC_GPU3D_RCR, BIT(0) },
	[IMX8MP_RESET_GPU_RESET]		= { SRC_GPU_RCR, BIT(0) },
	[IMX8MP_RESET_VPU_RESET]		= { SRC_VPU_RCR, BIT(0) },
	[IMX8MP_RESET_VPU_G1_RESET]		= { SRC_VPU_G1_RCR, BIT(0) },
	[IMX8MP_RESET_VPU_G2_RESET]		= { SRC_VPU_G2_RCR, BIT(0) },
	[IMX8MP_RESET_VPUVC8KE_RESET]		= { SRC_VPUVC8KE_RCR, BIT(0) },
	[IMX8MP_RESET_NOC_RESET]		= { SRC_NOC_RCR, BIT(0) },
};

static int imx8mp_reset_set(struct reset_ctl *rst, bool assert)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	unsigned int bit, value;

	if (rst->id >= IMX8MP_RESET_NUM)
		return -EINVAL;

	bit = imx8mp_src_signals[rst->id].bit;
	value = assert ? bit : 0;

	switch (rst->id) {
	case IMX8MP_RESET_PCIEPHY:
		/*
		 * wait for more than 10us to release phy g_rst and
		 * btnrst
		 */
		if (!assert)
			udelay(10);
		break;

	case IMX8MP_RESET_PCIE_CTRL_APPS_EN:
	case IMX8MP_RESET_PCIEPHY_PERST:
		value = assert ? 0 : bit;
		break;
	}

	clrsetbits_le32(priv->base + imx8mp_src_signals[rst->id].offset, bit,
			value);

	return 0;
}

static int imx8mp_reset_assert(struct reset_ctl *rst)
{
	return imx8mp_reset_set(rst, true);
}

static int imx8mp_reset_deassert(struct reset_ctl *rst)
{
	return imx8mp_reset_set(rst, false);
}

static int imx_reset_assert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	return priv->ops.rst_assert(rst);
}

static int imx_reset_deassert(struct reset_ctl *rst)
{
	struct imx_reset_priv *priv = dev_get_priv(rst->dev);
	return priv->ops.rst_deassert(rst);
}

static const struct reset_ops imx7_reset_reset_ops = {
	.rst_assert = imx_reset_assert,
	.rst_deassert = imx_reset_deassert,
};

static const struct udevice_id imx7_reset_ids[] = {
	{ .compatible = "fsl,imx7d-src" },
	{ .compatible = "fsl,imx8mq-src" },
	{ .compatible = "fsl,imx8mp-src" },
	{ }
};

static int imx7_reset_probe(struct udevice *dev)
{
	struct imx_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -ENOMEM;

	if (device_is_compatible(dev, "fsl,imx8mq-src")) {
		priv->ops.rst_assert = imx8mq_reset_assert;
		priv->ops.rst_deassert = imx8mq_reset_deassert;
	} else if (device_is_compatible(dev, "fsl,imx7d-src")) {
		priv->ops.rst_assert = imx7_reset_assert;
		priv->ops.rst_deassert = imx7_reset_deassert;
	} else if (device_is_compatible(dev, "fsl,imx8mp-src")) {
		priv->ops.rst_assert = imx8mp_reset_assert;
		priv->ops.rst_deassert = imx8mp_reset_deassert;
	}

	return 0;
}

U_BOOT_DRIVER(imx7_reset) = {
	.name = "imx7_reset",
	.id = UCLASS_RESET,
	.of_match = imx7_reset_ids,
	.ops = &imx7_reset_reset_ops,
	.probe = imx7_reset_probe,
	.priv_auto = sizeof(struct imx_reset_priv),
};
