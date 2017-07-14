/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/grf_rk3368.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk3368_pinctrl_priv {
	struct rk3368_grf *grf;
	struct rk3368_pmu_grf *pmugrf;
};

static void pinctrl_rk3368_uart_config(struct rk3368_pinctrl_priv *priv,
				       int uart_id)
{
	struct rk3368_grf *grf = priv->grf;
	struct rk3368_pmu_grf *pmugrf = priv->pmugrf;

	switch (uart_id) {
	case PERIPH_ID_UART2:
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A6_MASK | GPIO2A5_MASK,
			     GPIO2A6_UART2_SIN | GPIO2A5_UART2_SOUT);
		break;
	case PERIPH_ID_UART0:
		break;
	case PERIPH_ID_UART1:
		break;
	case PERIPH_ID_UART3:
		break;
	case PERIPH_ID_UART4:
		rk_clrsetreg(&pmugrf->gpio0d_iomux,
			     GPIO0D0_MASK | GPIO0D1_MASK |
			     GPIO0D2_MASK | GPIO0D3_MASK,
			     GPIO0D0_GPIO | GPIO0D1_GPIO |
			     GPIO0D2_UART4_SOUT | GPIO0D3_UART4_SIN);
		break;
	default:
		debug("uart id = %d iomux error!\n", uart_id);
		break;
	}
}

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static void pinctrl_rk3368_gmac_config(struct rk3368_grf *grf, int gmac_id)
{
	rk_clrsetreg(&grf->gpio3b_iomux,
		     GPIO3B0_MASK | GPIO3B1_MASK |
		     GPIO3B2_MASK | GPIO3B5_MASK |
		     GPIO3B6_MASK | GPIO3B7_MASK,
		     GPIO3B0_MAC_TXD0 | GPIO3B1_MAC_TXD1 |
		     GPIO3B2_MAC_TXD2 | GPIO3B5_MAC_TXEN |
		     GPIO3B6_MAC_TXD3 | GPIO3B7_MAC_RXD0);
	rk_clrsetreg(&grf->gpio3c_iomux,
		     GPIO3C0_MASK | GPIO3C1_MASK |
		     GPIO3C2_MASK | GPIO3C3_MASK |
		     GPIO3C4_MASK | GPIO3C5_MASK |
		     GPIO3C6_MASK,
		     GPIO3C0_MAC_RXD1 | GPIO3C1_MAC_RXD2 |
		     GPIO3C2_MAC_RXD3 | GPIO3C3_MAC_MDC |
		     GPIO3C4_MAC_RXDV | GPIO3C5_MAC_RXEN |
		     GPIO3C6_MAC_CLK);
	rk_clrsetreg(&grf->gpio3d_iomux,
		     GPIO3D0_MASK | GPIO3D1_MASK |
		     GPIO3D4_MASK,
		     GPIO3D0_MAC_MDIO | GPIO3D1_MAC_RXCLK |
		     GPIO3D4_MAC_TXCLK);
}
#endif

static void pinctrl_rk3368_sdmmc_config(struct rk3368_grf *grf, int mmc_id)
{
	switch (mmc_id) {
	case PERIPH_ID_EMMC:
		debug("mmc id = %d setting registers!\n", mmc_id);
		rk_clrsetreg(&grf->gpio1c_iomux,
			     GPIO1C2_MASK | GPIO1C3_MASK |
			     GPIO1C4_MASK | GPIO1C5_MASK |
			     GPIO1C6_MASK | GPIO1C7_MASK,
			     GPIO1C2_EMMC_DATA0 |
			     GPIO1C3_EMMC_DATA1 |
			     GPIO1C4_EMMC_DATA2 |
			     GPIO1C5_EMMC_DATA3 |
			     GPIO1C6_EMMC_DATA4 |
			     GPIO1C7_EMMC_DATA5);
		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D0_MASK | GPIO1D1_MASK |
			     GPIO1D2_MASK | GPIO1D3_MASK,
			     GPIO1D0_EMMC_DATA6 |
			     GPIO1D1_EMMC_DATA7 |
			     GPIO1D2_EMMC_CMD |
			     GPIO1D3_EMMC_PWREN);
		rk_clrsetreg(&grf->gpio2a_iomux,
			     GPIO2A3_MASK | GPIO2A4_MASK,
			     GPIO2A3_EMMC_RSTNOUT |
			     GPIO2A4_EMMC_CLKOUT);
		break;
	case PERIPH_ID_SDCARD:
		/*
		 * We assume that the BROM has already set this up
		 * correctly for us and that there's nothing to do
		 * here.
		 */
		break;
	default:
		debug("mmc id = %d iomux error!\n", mmc_id);
		break;
	}
}

static int rk3368_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rk3368_pinctrl_priv *priv = dev_get_priv(dev);

	debug("%s: func=%d, flags=%x\n", __func__, func, flags);
	switch (func) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
		pinctrl_rk3368_uart_config(priv, func);
		break;
	case PERIPH_ID_EMMC:
	case PERIPH_ID_SDCARD:
		pinctrl_rk3368_sdmmc_config(priv->grf, func);
		break;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case PERIPH_ID_GMAC:
		pinctrl_rk3368_gmac_config(priv->grf, func);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk3368_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(periph),
				   "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 59:
		return PERIPH_ID_UART4;
	case 58:
		return PERIPH_ID_UART3;
	case 57:
		return PERIPH_ID_UART2;
	case 56:
		return PERIPH_ID_UART1;
	case 55:
		return PERIPH_ID_UART0;
	case 35:
		return PERIPH_ID_EMMC;
	case 32:
		return PERIPH_ID_SDCARD;
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case 27:
		return PERIPH_ID_GMAC;
#endif
	}

	return -ENOENT;
}

static int rk3368_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rk3368_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;

	return rk3368_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rk3368_pinctrl_ops = {
	.set_state_simple	= rk3368_pinctrl_set_state_simple,
	.request	= rk3368_pinctrl_request,
	.get_periph_id	= rk3368_pinctrl_get_periph_id,
};

static int rk3368_pinctrl_probe(struct udevice *dev)
{
	struct rk3368_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);

	debug("%s: grf=%p pmugrf:%p\n", __func__, priv->grf, priv->pmugrf);

	return ret;
}

static const struct udevice_id rk3368_pinctrl_ids[] = {
	{ .compatible = "rockchip,rk3368-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3368) = {
	.name		= "rockchip_rk3368_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3368_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rk3368_pinctrl_priv),
	.ops		= &rk3368_pinctrl_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= rk3368_pinctrl_probe,
};
