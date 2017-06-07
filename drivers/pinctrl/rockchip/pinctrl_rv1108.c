/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rv1108.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

struct rv1108_pinctrl_priv {
	struct rv1108_grf *grf;
};

static void pinctrl_rv1108_uart_config(struct rv1108_grf *grf, int uart_id)
{
	switch (uart_id) {
	case PERIPH_ID_UART0:
		rk_clrsetreg(&grf->gpio3a_iomux,
			     GPIO3A6_MASK | GPIO3A5_MASK,
			     GPIO3A6_UART1_SOUT << GPIO3A6_SHIFT |
			     GPIO3A5_UART1_SIN << GPIO3A5_SHIFT);
		break;
	case PERIPH_ID_UART1:
		rk_clrsetreg(&grf->gpio1d_iomux,
			     GPIO1D3_MASK | GPIO1D2_MASK | GPIO1D1_MASK |
			     GPIO1D0_MASK,
			     GPIO1D3_UART0_SOUT << GPIO1D3_SHIFT |
			     GPIO1D2_UART0_SIN << GPIO1D2_SHIFT |
			     GPIO1D1_UART0_RTSN << GPIO1D1_SHIFT |
			     GPIO1D0_UART0_CTSN << GPIO1D0_SHIFT);
		break;
	case PERIPH_ID_UART2:
		rk_clrsetreg(&grf->gpio2d_iomux,
			     GPIO2D2_MASK | GPIO2D1_MASK,
			     GPIO2D2_UART2_SOUT_M0 << GPIO2D2_SHIFT |
			     GPIO2D1_UART2_SIN_M0 << GPIO2D1_SHIFT);
		break;
	}
}

static void pinctrl_rv1108_gmac_config(struct rv1108_grf *grf, int func)
{
	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B7_MASK | GPIO1B6_MASK | GPIO1B5_MASK |
		     GPIO1B4_MASK | GPIO1B3_MASK | GPIO1B2_MASK,
		     GPIO1B7_GMAC_RXDV << GPIO1B7_SHIFT |
		     GPIO1B6_GMAC_RXD1 << GPIO1B6_SHIFT |
		     GPIO1B5_GMAC_RXD0 << GPIO1B5_SHIFT |
		     GPIO1B4_GMAC_TXEN << GPIO1B4_SHIFT |
		     GPIO1B3_GMAC_TXD1 << GPIO1B3_SHIFT |
		     GPIO1B2_GMAC_TXD0 << GPIO1B2_SHIFT);
	rk_clrsetreg(&grf->gpio1c_iomux,
		     GPIO1C5_MASK | GPIO1C4_MASK |
		     GPIO1C3_MASK | GPIO1C2_MASK,
		     GPIO1C5_GMAC_CLK << GPIO1C5_SHIFT |
		     GPIO1C4_GMAC_MDC << GPIO1C4_SHIFT |
		     GPIO1C3_GMAC_MDIO << GPIO1C3_SHIFT |
		     GPIO1C2_GMAC_RXER << GPIO1C2_SHIFT);
	writel(0xffff57f5, &grf->gpio1b_drv);
}

static void pinctrl_rv1108_sfc_config(struct rv1108_grf *grf)
{
	rk_clrsetreg(&grf->gpio2a_iomux, GPIO2A3_MASK | GPIO2A2_MASK |
		     GPIO2A1_MASK | GPIO2A0_MASK,
		     GPIO2A3_SFC_HOLD_IO3 << GPIO2A3_SHIFT |
		     GPIO2A2_SFC_WP_IO2 << GPIO2A2_SHIFT |
		     GPIO2A1_SFC_SO_IO1 << GPIO2A1_SHIFT |
		     GPIO2A0_SFC_SI_IO0 << GPIO2A0_SHIFT);
	rk_clrsetreg(&grf->gpio2b_iomux, GPIO2B7_MASK | GPIO2B4_MASK,
		     GPIO2B7_SFC_CLK << GPIO2B7_SHIFT |
		     GPIO2B4_SFC_CSN0 << GPIO2B4_SHIFT);
}

static int rv1108_pinctrl_request(struct udevice *dev, int func, int flags)
{
	struct rv1108_pinctrl_priv *priv = dev_get_priv(dev);

	switch (func) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
		pinctrl_rv1108_uart_config(priv->grf, func);
		break;
	case PERIPH_ID_GMAC:
		pinctrl_rv1108_gmac_config(priv->grf, func);
	case PERIPH_ID_SFC:
		pinctrl_rv1108_sfc_config(priv->grf);
	default:
		return -EINVAL;
	}

	return 0;
}

static int rv1108_pinctrl_get_periph_id(struct udevice *dev,
					struct udevice *periph)
{
	u32 cell[3];
	int ret;

	ret = dev_read_u32_array(periph, "interrupts", cell, ARRAY_SIZE(cell));
	if (ret < 0)
		return -EINVAL;

	switch (cell[1]) {
	case 11:
		return PERIPH_ID_SDCARD;
	case 13:
		return PERIPH_ID_EMMC;
	case 19:
		return PERIPH_ID_GMAC;
	case 30:
		return PERIPH_ID_I2C0;
	case 31:
		return PERIPH_ID_I2C1;
	case 32:
		return PERIPH_ID_I2C2;
	case 39:
		return PERIPH_ID_PWM0;
	case 44:
		return PERIPH_ID_UART0;
	case 45:
		return PERIPH_ID_UART1;
	case 46:
		return PERIPH_ID_UART2;
	case 56:
		return PERIPH_ID_SFC;
	}

	return -ENOENT;
}

static int rv1108_pinctrl_set_state_simple(struct udevice *dev,
					   struct udevice *periph)
{
	int func;

	func = rv1108_pinctrl_get_periph_id(dev, periph);
	if (func < 0)
		return func;

	return rv1108_pinctrl_request(dev, func, 0);
}

static struct pinctrl_ops rv1108_pinctrl_ops = {
	.set_state_simple	= rv1108_pinctrl_set_state_simple,
	.request		= rv1108_pinctrl_request,
	.get_periph_id		= rv1108_pinctrl_get_periph_id,
};

static int rv1108_pinctrl_probe(struct udevice *dev)
{
	struct rv1108_pinctrl_priv *priv = dev_get_priv(dev);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	return 0;
}

static const struct udevice_id rv1108_pinctrl_ids[] = {
	{.compatible = "rockchip,rv1108-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_rv1108) = {
	.name           = "pinctrl_rv1108",
	.id             = UCLASS_PINCTRL,
	.of_match       = rv1108_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rv1108_pinctrl_priv),
	.ops            = &rv1108_pinctrl_ops,
	.bind           = dm_scan_fdt_dev,
	.probe          = rv1108_pinctrl_probe,
};
