/*
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/pwm.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk_pwm_priv {
	struct rk3288_pwm *regs;
	struct rk3288_grf *grf;
};

static int rk_pwm_set_config(struct udevice *dev, uint channel, uint period_ns,
			     uint duty_ns)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct rk3288_pwm *regs = priv->regs;
	unsigned long period, duty;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);
	writel(PWM_SEL_SRC_CLK | PWM_OUTPUT_LEFT | PWM_LP_DISABLE |
		PWM_CONTINUOUS | PWM_DUTY_POSTIVE | PWM_INACTIVE_POSTIVE |
		RK_PWM_DISABLE,
		&regs->ctrl);

	period = lldiv((uint64_t)(PD_BUS_PCLK_HZ / 1000) * period_ns, 1000000);
	duty = lldiv((uint64_t)(PD_BUS_PCLK_HZ / 1000) * duty_ns, 1000000);

	writel(period, &regs->period_hpr);
	writel(duty, &regs->duty_lpr);
	debug("%s: period=%lu, duty=%lu\n", __func__, period, duty);

	return 0;
}

static int rk_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct rk3288_pwm *regs = priv->regs;

	debug("%s: Enable '%s'\n", __func__, dev->name);
	clrsetbits_le32(&regs->ctrl, RK_PWM_ENABLE, enable ? RK_PWM_ENABLE : 0);

	return 0;
}

static int rk_pwm_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);
	struct regmap *map;

	priv->regs = (struct rk3288_pwm *)dev_get_addr(dev);
	map = syscon_get_regmap_by_driver_data(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(map))
		return PTR_ERR(map);
	priv->grf = regmap_get_range(map, 0);

	return 0;
}

static int rk_pwm_probe(struct udevice *dev)
{
	struct rk_pwm_priv *priv = dev_get_priv(dev);

	rk_setreg(&priv->grf->soc_con2, 1 << 0);

	return 0;
}

static const struct pwm_ops rk_pwm_ops = {
	.set_config	= rk_pwm_set_config,
	.set_enable	= rk_pwm_set_enable,
};

static const struct udevice_id rk_pwm_ids[] = {
	{ .compatible = "rockchip,rk3288-pwm" },
	{ }
};

U_BOOT_DRIVER(rk_pwm) = {
	.name	= "rk_pwm",
	.id	= UCLASS_PWM,
	.of_match = rk_pwm_ids,
	.ops	= &rk_pwm_ops,
	.ofdata_to_platdata	= rk_pwm_ofdata_to_platdata,
	.probe		= rk_pwm_probe,
	.priv_auto_alloc_size	= sizeof(struct rk_pwm_priv),
};
