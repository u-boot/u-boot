// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Google Inc.
 */

#include <dm.h>
#include <clk.h>
#include <div64.h>
#include <log.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pwm.h>
#include <linux/time.h>

#define PWM_PDIV_WIDTH		8
#define PWM_PDIV_MAX		BIT(PWM_PDIV_WIDTH)
#define PWM_FDIV_WIDTH		13

struct tegra_pwm_priv {
	struct pwm_ctlr *regs;
	u64 clk_rate;
	u32 min_period_ns;
	u8 polarity;
};

static int tegra_pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);

	if (channel >= 4)
		return -EINVAL;

	clrsetbits_8(&priv->polarity, BIT(channel), (polarity << channel));

	return 0;
}

static int tegra_pwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_ctlr *regs = priv->regs;
	u64 pulse_width;
	u32 reg;
	s64 rate;

	if (channel >= 4)
		return -EINVAL;
	debug("%s: Configure '%s' channel %u\n", __func__, dev->name, channel);

	if (period_ns < priv->min_period_ns) {
		debug("%s: Channel %u period too low, period_ns %u minimum %u\n",
		      __func__, channel, period_ns, priv->min_period_ns);
		return -EINVAL;
	}

	/*
	 * Convert from duty_ns / period_ns to a fixed number of duty ticks
	 * per (1 << PWM_PDIV_WIDTH) cycles and make sure to round to the
	 * nearest integer during division.
	 */
	pulse_width = duty_ns * PWM_PDIV_MAX;
	pulse_width = DIV_ROUND_CLOSEST_ULL(pulse_width, period_ns);

	if (priv->polarity & BIT(channel))
		pulse_width = PWM_PDIV_MAX - pulse_width;

	if (pulse_width > PWM_PDIV_MAX) {
		debug("%s: Channel %u pulse_width too high %llu\n",
		      __func__, channel, pulse_width);
		return -EINVAL;
	}

	/*
	 * Since the actual PWM divider is the register's frequency divider
	 * field plus 1, we need to decrement to get the correct value to
	 * write to the register.
	 */
	rate = (priv->clk_rate * period_ns) / ((u64)NSEC_PER_SEC << PWM_PDIV_WIDTH) - 1;
	if (rate < 0) {
		debug("%s: Channel %u rate is not positive\n", __func__, channel);
		return -EINVAL;
	}

	if (rate >> PWM_FDIV_WIDTH) {
		debug("%s: Channel %u rate too high %llu\n", __func__, channel, rate);
		return -EINVAL;
	}

	reg = pulse_width << PWM_WIDTH_SHIFT;
	reg |= rate << PWM_DIVIDER_SHIFT;
	reg |= PWM_ENABLE_MASK;
	writel(reg, &regs[channel].control);

	return 0;
}

static int tegra_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_ctlr *regs = priv->regs;

	if (channel >= 4)
		return -EINVAL;
	debug("%s: Enable '%s' channel %u\n", __func__, dev->name, channel);
	clrsetbits_le32(&regs[channel].control, PWM_ENABLE_MASK,
			enable ? PWM_ENABLE_MASK : 0);

	return 0;
}

static int tegra_pwm_of_to_plat(struct udevice *dev)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);

	return 0;
}

static int tegra_pwm_probe(struct udevice *dev)
{
	struct tegra_pwm_priv *priv = dev_get_priv(dev);
	const u32 pwm_max_freq = dev_get_driver_data(dev);
	struct clk *clk;

	clk = devm_clk_get(dev, NULL);
	if (IS_ERR(clk)) {
		debug("%s: Could not get PWM clock: %ld\n", __func__, PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	priv->clk_rate = clock_start_periph_pll(clk->id, CLOCK_ID_PERIPH,
						pwm_max_freq);
	priv->min_period_ns = (NSEC_PER_SEC / (pwm_max_freq >> PWM_PDIV_WIDTH)) + 1;

	debug("%s: clk_rate = %llu min_period_ns = %u\n", __func__,
	      priv->clk_rate, priv->min_period_ns);

	return 0;
}

static const struct pwm_ops tegra_pwm_ops = {
	.set_config	= tegra_pwm_set_config,
	.set_enable	= tegra_pwm_set_enable,
	.set_invert	= tegra_pwm_set_invert,
};

static const struct udevice_id tegra_pwm_ids[] = {
	{ .compatible = "nvidia,tegra20-pwm", .data = 48 * 1000000 },
	{ .compatible = "nvidia,tegra114-pwm", .data = 408 * 1000000 },
	{ }
};

U_BOOT_DRIVER(tegra_pwm) = {
	.name	= "tegra_pwm",
	.id	= UCLASS_PWM,
	.of_match = tegra_pwm_ids,
	.ops	= &tegra_pwm_ops,
	.of_to_plat	= tegra_pwm_of_to_plat,
	.probe		= tegra_pwm_probe,
	.priv_auto	= sizeof(struct tegra_pwm_priv),
};
