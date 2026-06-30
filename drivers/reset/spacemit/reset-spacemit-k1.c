// SPDX-License-Identifier: GPL-2.0+
/*
 * SpacemiT K1 reset driver (syscon-bound).
 *
 * Copyright (C) 2022 Spacemit Inc.
 * Copyright (C) 2025 Huan Zhou <pericycle.cc@gmail.com>
 * Copyright (C) 2026 RISCstar Ltd.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/spacemit,k1-syscon.h>
#include <linux/bitops.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <soc/spacemit/k1-reset.h>
#include <soc/spacemit/k1-syscon.h>

/* ===================================================================
 * Per-syscon reset signal tables.
 *
 * Indexed by the kernel-side per-syscon-local IDs from
 * <dt-bindings/clock/spacemit,k1-syscon.h>. Each entry is
 * (offset, assert_mask, deassert_mask): bits in assert_mask are set
 * when the reset line is asserted; bits in deassert_mask are set when
 * deasserted; the union (assert_mask | deassert_mask) is the set of
 * bits the controller will overwrite on each transition.
 *
 * Layout mirrors the kernel-side K1 reset driver.
 * ===================================================================
 */

struct spacemit_k1_reset_data {
	u32 offset;
	u32 assert_mask;
	u32 deassert_mask;
};

#define RESET_DATA(o, a, d) {					\
	.offset = (o), .assert_mask = (a), .deassert_mask = (d)	\
}

static const struct spacemit_k1_reset_data k1_mpmu_resets[] = {
	[RESET_WDT]	= RESET_DATA(MPMU_WDTPCR,		BIT(2), 0),
};

static const struct spacemit_k1_reset_data k1_apbc_resets[] = {
	[RESET_UART0]	= RESET_DATA(APBC_UART1_CLK_RST,	BIT(2), 0),
	[RESET_UART2]	= RESET_DATA(APBC_UART2_CLK_RST,	BIT(2), 0),
	[RESET_UART3]	= RESET_DATA(APBC_UART3_CLK_RST,	BIT(2), 0),
	[RESET_UART4]	= RESET_DATA(APBC_UART4_CLK_RST,	BIT(2), 0),
	[RESET_UART5]	= RESET_DATA(APBC_UART5_CLK_RST,	BIT(2), 0),
	[RESET_UART6]	= RESET_DATA(APBC_UART6_CLK_RST,	BIT(2), 0),
	[RESET_UART7]	= RESET_DATA(APBC_UART7_CLK_RST,	BIT(2), 0),
	[RESET_UART8]	= RESET_DATA(APBC_UART8_CLK_RST,	BIT(2), 0),
	[RESET_UART9]	= RESET_DATA(APBC_UART9_CLK_RST,	BIT(2), 0),
	[RESET_GPIO]	= RESET_DATA(APBC_GPIO_CLK_RST,		BIT(2), 0),
	[RESET_PWM0]	= RESET_DATA(APBC_PWM0_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM1]	= RESET_DATA(APBC_PWM1_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM2]	= RESET_DATA(APBC_PWM2_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM3]	= RESET_DATA(APBC_PWM3_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM4]	= RESET_DATA(APBC_PWM4_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM5]	= RESET_DATA(APBC_PWM5_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM6]	= RESET_DATA(APBC_PWM6_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM7]	= RESET_DATA(APBC_PWM7_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM8]	= RESET_DATA(APBC_PWM8_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM9]	= RESET_DATA(APBC_PWM9_CLK_RST,		BIT(2), BIT(0)),
	[RESET_PWM10]	= RESET_DATA(APBC_PWM10_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM11]	= RESET_DATA(APBC_PWM11_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM12]	= RESET_DATA(APBC_PWM12_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM13]	= RESET_DATA(APBC_PWM13_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM14]	= RESET_DATA(APBC_PWM14_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM15]	= RESET_DATA(APBC_PWM15_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM16]	= RESET_DATA(APBC_PWM16_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM17]	= RESET_DATA(APBC_PWM17_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM18]	= RESET_DATA(APBC_PWM18_CLK_RST,	BIT(2), BIT(0)),
	[RESET_PWM19]	= RESET_DATA(APBC_PWM19_CLK_RST,	BIT(2), BIT(0)),
	[RESET_SSP3]	= RESET_DATA(APBC_SSP3_CLK_RST,		BIT(2), 0),
	[RESET_RTC]	= RESET_DATA(APBC_RTC_CLK_RST,		BIT(2), 0),
	[RESET_TWSI0]	= RESET_DATA(APBC_TWSI0_CLK_RST,	BIT(2), 0),
	[RESET_TWSI1]	= RESET_DATA(APBC_TWSI1_CLK_RST,	BIT(2), 0),
	[RESET_TWSI2]	= RESET_DATA(APBC_TWSI2_CLK_RST,	BIT(2), 0),
	[RESET_TWSI4]	= RESET_DATA(APBC_TWSI4_CLK_RST,	BIT(2), 0),
	[RESET_TWSI5]	= RESET_DATA(APBC_TWSI5_CLK_RST,	BIT(2), 0),
	[RESET_TWSI6]	= RESET_DATA(APBC_TWSI6_CLK_RST,	BIT(2), 0),
	[RESET_TWSI7]	= RESET_DATA(APBC_TWSI7_CLK_RST,	BIT(2), 0),
	[RESET_TWSI8]	= RESET_DATA(APBC_TWSI8_CLK_RST,	BIT(2), 0),
	[RESET_TIMERS1]	= RESET_DATA(APBC_TIMERS1_CLK_RST,	BIT(2), 0),
	[RESET_TIMERS2]	= RESET_DATA(APBC_TIMERS2_CLK_RST,	BIT(2), 0),
	[RESET_AIB]	= RESET_DATA(APBC_AIB_CLK_RST,		BIT(2), 0),
	[RESET_ONEWIRE]	= RESET_DATA(APBC_ONEWIRE_CLK_RST,	BIT(2), 0),
	[RESET_SSPA0]	= RESET_DATA(APBC_SSPA0_CLK_RST,	BIT(2), 0),
	[RESET_SSPA1]	= RESET_DATA(APBC_SSPA1_CLK_RST,	BIT(2), 0),
	[RESET_DRO]	= RESET_DATA(APBC_DRO_CLK_RST,		BIT(2), 0),
	[RESET_IR]	= RESET_DATA(APBC_IR_CLK_RST,		BIT(2), 0),
	[RESET_TSEN]	= RESET_DATA(APBC_TSEN_CLK_RST,		BIT(2), 0),
	[RESET_IPC_AP2AUD] = RESET_DATA(APBC_IPC_AP2AUD_CLK_RST, BIT(2), 0),
	[RESET_CAN0]	= RESET_DATA(APBC_CAN0_CLK_RST,		BIT(2), 0),
};

static const struct spacemit_k1_reset_data k1_apmu_resets[] = {
	[RESET_CCIC_4X]		= RESET_DATA(APMU_CCIC_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_CCIC1_PHY]	= RESET_DATA(APMU_CCIC_CLK_RES_CTRL,	0, BIT(2)),
	[RESET_SDH_AXI]		= RESET_DATA(APMU_SDH0_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_SDH0]		= RESET_DATA(APMU_SDH0_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_SDH1]		= RESET_DATA(APMU_SDH1_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_SDH2]		= RESET_DATA(APMU_SDH2_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_USBP1_AXI]	= RESET_DATA(APMU_USB_CLK_RES_CTRL,	0, BIT(4)),
	[RESET_USB_AXI]		= RESET_DATA(APMU_USB_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_USB30_AHB]	= RESET_DATA(APMU_USB_CLK_RES_CTRL,	0, BIT(9)),
	[RESET_USB30_VCC]	= RESET_DATA(APMU_USB_CLK_RES_CTRL,	0, BIT(10)),
	[RESET_USB30_PHY]	= RESET_DATA(APMU_USB_CLK_RES_CTRL,	0, BIT(11)),
	[RESET_QSPI]		= RESET_DATA(APMU_QSPI_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_QSPI_BUS]	= RESET_DATA(APMU_QSPI_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_DMA]		= RESET_DATA(APMU_DMA_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_AES]		= RESET_DATA(APMU_AES_CLK_RES_CTRL,	0, BIT(4)),
	[RESET_VPU]		= RESET_DATA(APMU_VPU_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_GPU]		= RESET_DATA(APMU_GPU_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_EMMC]		= RESET_DATA(APMU_PMUA_EM_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_EMMC_X]		= RESET_DATA(APMU_PMUA_EM_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_AUDIO_SYS]	= RESET_DATA(APMU_AUDIO_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_AUDIO_MCU]	= RESET_DATA(APMU_AUDIO_CLK_RES_CTRL,	0, BIT(2)),
	[RESET_AUDIO_APMU]	= RESET_DATA(APMU_AUDIO_CLK_RES_CTRL,	0, BIT(3)),
	[RESET_HDMI]		= RESET_DATA(APMU_HDMI_CLK_RES_CTRL,	0, BIT(9)),
	[RESET_PCIE0_DBI]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_0,	0, BIT(3)),
	[RESET_PCIE0_SLAVE]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_0,	0, BIT(4)),
	[RESET_PCIE0_MASTER]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_0,	0, BIT(5)),
	[RESET_PCIE0_GLOBAL]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_0,	BIT(8), 0),
	[RESET_PCIE1_DBI]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_1,	0, BIT(3)),
	[RESET_PCIE1_SLAVE]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_1,	0, BIT(4)),
	[RESET_PCIE1_MASTER]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_1,	0, BIT(5)),
	[RESET_PCIE1_GLOBAL]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_1,	BIT(8), 0),
	[RESET_PCIE2_DBI]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_2,	0, BIT(3)),
	[RESET_PCIE2_SLAVE]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_2,	0, BIT(4)),
	[RESET_PCIE2_MASTER]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_2,	0, BIT(5)),
	[RESET_PCIE2_GLOBAL]	= RESET_DATA(APMU_PCIE_CLK_RES_CTRL_2,	BIT(8), 0),
	[RESET_EMAC0]		= RESET_DATA(APMU_EMAC0_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_EMAC1]		= RESET_DATA(APMU_EMAC1_CLK_RES_CTRL,	0, BIT(1)),
	[RESET_JPG]		= RESET_DATA(APMU_JPG_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_CCIC2PHY]	= RESET_DATA(APMU_CSI_CCIC2_CLK_RES_CTRL, 0, BIT(2)),
	[RESET_CCIC3PHY]	= RESET_DATA(APMU_CSI_CCIC2_CLK_RES_CTRL, 0, BIT(29)),
	[RESET_CSI]		= RESET_DATA(APMU_CSI_CCIC2_CLK_RES_CTRL, 0, BIT(1)),
	[RESET_ISP]		= RESET_DATA(APMU_ISP_CLK_RES_CTRL,	0, BIT(0)),
	[RESET_ISP_CPP]		= RESET_DATA(APMU_ISP_CLK_RES_CTRL,	0, BIT(27)),
	[RESET_ISP_BUS]		= RESET_DATA(APMU_ISP_CLK_RES_CTRL,	0, BIT(3)),
	[RESET_ISP_CI]		= RESET_DATA(APMU_ISP_CLK_RES_CTRL,	0, BIT(16)),
	[RESET_DPU_MCLK]	= RESET_DATA(APMU_LCD_CLK_RES_CTRL2,	0, BIT(9)),
	[RESET_DPU_ESC]		= RESET_DATA(APMU_LCD_CLK_RES_CTRL1,	0, BIT(3)),
	[RESET_DPU_HCLK]	= RESET_DATA(APMU_LCD_CLK_RES_CTRL1,	0, BIT(4)),
	[RESET_DPU_SPIBUS]	= RESET_DATA(APMU_LCD_SPI_CLK_RES_CTRL,	0, BIT(4)),
	[RESET_DPU_SPI_HBUS]	= RESET_DATA(APMU_LCD_SPI_CLK_RES_CTRL,	0, BIT(2)),
	[RESET_V2D]		= RESET_DATA(APMU_LCD_CLK_RES_CTRL1,	0, BIT(27)),
	[RESET_MIPI]		= RESET_DATA(APMU_LCD_CLK_RES_CTRL1,	0, BIT(15)),
	[RESET_MC]		= RESET_DATA(APMU_PMUA_MC_CTRL,		0, BIT(0)),
};

static const struct spacemit_k1_reset_data k1_apbc2_resets[] = {
	[RESET_APBC2_UART1]	= RESET_DATA(APBC2_UART1_CLK_RST,	BIT(2), 0),
	[RESET_APBC2_SSP2]	= RESET_DATA(APBC2_SSP2_CLK_RST,	BIT(2), 0),
	[RESET_APBC2_TWSI3]	= RESET_DATA(APBC2_TWSI3_CLK_RST,	BIT(2), 0),
	[RESET_APBC2_RTC]	= RESET_DATA(APBC2_RTC_CLK_RST,		BIT(2), 0),
	[RESET_APBC2_TIMERS0]	= RESET_DATA(APBC2_TIMERS0_CLK_RST,	BIT(2), 0),
	[RESET_APBC2_KPC]	= RESET_DATA(APBC2_KPC_CLK_RST,		BIT(2), 0),
	[RESET_APBC2_GPIO]	= RESET_DATA(APBC2_GPIO_CLK_RST,	BIT(2), 0),
};

/* ===================================================================
 * Driver
 * ===================================================================
 */

struct spacemit_k1_reset_priv {
	void __iomem *base;
	const struct spacemit_k1_reset_data *table;
	size_t table_size;
};

static int spacemit_k1_reset_xfer(struct reset_ctl *rst, bool assert)
{
	struct spacemit_k1_reset_priv *priv = dev_get_priv(rst->dev);
	const struct spacemit_k1_reset_data *e;
	u32 v;

	if (rst->id >= priv->table_size)
		return -EINVAL;

	e = &priv->table[rst->id];
	if (e->assert_mask == 0 && e->deassert_mask == 0)
		return -EINVAL;	/* not owned by this syscon */

	v = readl(priv->base + e->offset);
	v &= ~(e->assert_mask | e->deassert_mask);
	v |= assert ? e->assert_mask : e->deassert_mask;
	writel(v, priv->base + e->offset);

	return 0;
}

static int spacemit_k1_reset_assert(struct reset_ctl *rst)
{
	return spacemit_k1_reset_xfer(rst, true);
}

static int spacemit_k1_reset_deassert(struct reset_ctl *rst)
{
	return spacemit_k1_reset_xfer(rst, false);
}

static int spacemit_k1_reset_request(struct reset_ctl *rst)
{
	struct spacemit_k1_reset_priv *priv = dev_get_priv(rst->dev);

	return rst->id < priv->table_size ? 0 : -EINVAL;
}

static const struct reset_ops spacemit_k1_reset_ops = {
	.request	= spacemit_k1_reset_request,
	.rst_assert	= spacemit_k1_reset_assert,
	.rst_deassert	= spacemit_k1_reset_deassert,
};

static int spacemit_k1_reset_probe(struct udevice *dev)
{
	struct spacemit_k1_reset_priv *priv = dev_get_priv(dev);

	priv->base = (void __iomem *)dev_remap_addr(dev);
	if (!priv->base)
		return -ENODEV;

	return 0;
}

U_BOOT_DRIVER(spacemit_k1_reset) = {
	.name		= "spacemit_k1_reset",
	.id		= UCLASS_RESET,
	.ops		= &spacemit_k1_reset_ops,
	.probe		= spacemit_k1_reset_probe,
	.priv_auto	= sizeof(struct spacemit_k1_reset_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};

int spacemit_k1_reset_bind(struct udevice *parent,
			   enum spacemit_k1_reset_syscon syscon)
{
	struct spacemit_k1_reset_priv *priv;
	struct udevice *rst_dev;
	const struct spacemit_k1_reset_data *table;
	size_t table_size;
	int ret;

	switch (syscon) {
	case SPACEMIT_K1_RESET_MPMU:
		table = k1_mpmu_resets;
		table_size = ARRAY_SIZE(k1_mpmu_resets);
		break;
	case SPACEMIT_K1_RESET_APMU:
		table = k1_apmu_resets;
		table_size = ARRAY_SIZE(k1_apmu_resets);
		break;
	case SPACEMIT_K1_RESET_APBC:
		table = k1_apbc_resets;
		table_size = ARRAY_SIZE(k1_apbc_resets);
		break;
	case SPACEMIT_K1_RESET_APBC2:
		table = k1_apbc2_resets;
		table_size = ARRAY_SIZE(k1_apbc2_resets);
		break;
	default:
		return -EINVAL;
	}

	ret = device_bind_driver_to_node(parent, "spacemit_k1_reset", "reset",
					 dev_ofnode(parent), &rst_dev);
	if (ret)
		return ret;

	priv = malloc(sizeof(*priv));
	if (!priv) {
		device_unbind(rst_dev);
		return -ENOMEM;
	}
	priv->table = table;
	priv->table_size = table_size;
	dev_set_priv(rst_dev, priv);

	return 0;
}
