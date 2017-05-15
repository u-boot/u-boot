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
			     GPIO2A6_UART2_SIN << GPIO2A6_SHIFT |
			     GPIO2A5_UART2_SOUT << GPIO2A5_SHIFT);
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
			     GPIO0D0_GPIO << GPIO0D0_SHIFT |
			     GPIO0D1_GPIO << GPIO0D1_SHIFT |
			     GPIO0D2_UART4_SOUT << GPIO0D2_SHIFT |
			     GPIO0D3_UART4_SIN << GPIO0D3_SHIFT);
		break;
	default:
		debug("uart id = %d iomux error!\n", uart_id);
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
