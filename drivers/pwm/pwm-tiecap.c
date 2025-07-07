// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ECAP PWM driver
 *
 * Copyright (C) 2025 BayLibre, SAS
 * Author: Sukrut Bellary <sbellary@baylibre.com>
 */

#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <pwm.h>
#include <asm/io.h>

/* eCAP module registers */
#define ECAP_PWM_CAP1			0x08
#define ECAP_PWM_CAP2			0x0C
#define ECAP_PWM_CAP3			0x10
#define ECAP_PWM_CAP4			0x14

#define ECAP_PWM_ECCTL2			0x2A
#define ECAP_PWM_ECCTL2_APWM_POL_LOW	BIT(10)
#define ECAP_PWM_ECCTL2_APWM_MODE	BIT(9)
#define ECAP_PWM_ECCTL2_TSCTR_FREERUN	BIT(4)
#define ECAP_PWM_ECCTL2_SYNC_SEL_DISA	(BIT(7) | BIT(6))

#define NSEC_PER_SEC			1000000000L

enum tiecap_pwm_polarity {
	TIECAP_PWM_POLARITY_NORMAL,
	TIECAP_PWM_POLARITY_INVERSED
};

enum tiecap_pwm_state {
	TIECAP_APWM_DISABLED,
	TIECAP_APWM_ENABLED
};

struct tiecap_pwm_priv {
	fdt_addr_t regs;
	u32 clk_rate;
	enum tiecap_pwm_state pwm_state;
};

static int tiecap_pwm_set_config(struct udevice *dev, uint channel,
				 uint period_ns, uint duty_ns)
{
	struct tiecap_pwm_priv *priv = dev_get_priv(dev);
	u32 period_cycles, duty_cycles;
	unsigned long long c;
	u16 value;

	c = priv->clk_rate;
	c = c * period_ns;
	do_div(c, NSEC_PER_SEC);
	period_cycles = (u32)c;

	if (period_cycles < 1) {
		period_cycles = 1;
		duty_cycles = 1;
	} else {
		c = priv->clk_rate;
		c = c * duty_ns;
		do_div(c, NSEC_PER_SEC);
		duty_cycles = (u32)c;
	}

	value = readw(priv->regs + ECAP_PWM_ECCTL2);

	/* Configure APWM mode & disable sync option */
	value |= ECAP_PWM_ECCTL2_APWM_MODE | ECAP_PWM_ECCTL2_SYNC_SEL_DISA;

	writew(value, priv->regs + ECAP_PWM_ECCTL2);

	if (priv->pwm_state == TIECAP_APWM_DISABLED) {
		/* Update active registers */
		writel(duty_cycles, priv->regs + ECAP_PWM_CAP2);
		writel(period_cycles, priv->regs + ECAP_PWM_CAP1);
	} else {
		/* Update shadow registers to configure period and
		 * compare values. This helps current pwm period to
		 * complete on reconfiguring.
		 */
		writel(duty_cycles, priv->regs + ECAP_PWM_CAP4);
		writel(period_cycles, priv->regs + ECAP_PWM_CAP3);
	}

	return 0;
}

static int tiecap_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct tiecap_pwm_priv *priv = dev_get_priv(dev);
	u16 value;

	value = readw(priv->regs + ECAP_PWM_ECCTL2);

	if (enable) {
		/*
		 * Enable 'Free run Time stamp counter mode' to start counter
		 * and  'APWM mode' to enable APWM output
		 */
		value |= ECAP_PWM_ECCTL2_TSCTR_FREERUN | ECAP_PWM_ECCTL2_APWM_MODE;
		priv->pwm_state = TIECAP_APWM_ENABLED;
	} else {
		/* Disable 'Free run Time stamp counter mode' to stop counter
		 * and 'APWM mode' to put APWM output to low
		 */
		value &= ~(ECAP_PWM_ECCTL2_TSCTR_FREERUN | ECAP_PWM_ECCTL2_APWM_MODE);
		priv->pwm_state = TIECAP_APWM_DISABLED;
	}

	writew(value, priv->regs + ECAP_PWM_ECCTL2);

	return 0;
}

static int tiecap_pwm_set_invert(struct udevice *dev, uint channel,
				 bool polarity)
{
	struct tiecap_pwm_priv *priv = dev_get_priv(dev);
	u16 value;

	value = readw(priv->regs + ECAP_PWM_ECCTL2);

	if (polarity == TIECAP_PWM_POLARITY_INVERSED)
		/* Duty cycle defines LOW period of PWM */
		value |= ECAP_PWM_ECCTL2_APWM_POL_LOW;
	else
		/* Duty cycle defines HIGH period of PWM */
		value &= ~ECAP_PWM_ECCTL2_APWM_POL_LOW;

	writew(value, priv->regs + ECAP_PWM_ECCTL2);

	return 0;
}

static int tiecap_pwm_of_to_plat(struct udevice *dev)
{
	struct tiecap_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "invalid address\n");
		return -EINVAL;
	}

	dev_dbg(dev, "regs=0x%08x\n", priv->regs);

	return 0;
}

static int tiecap_pwm_probe(struct udevice *dev)
{
	struct tiecap_pwm_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int err;

	err = clk_get_by_name(dev, "fck", &clk);
	if (err) {
		dev_err(dev, "failed to get clock\n");
		return err;
	}

	priv->clk_rate = clk_get_rate(&clk);
	if (IS_ERR_VALUE(priv->clk_rate) || !priv->clk_rate) {
		dev_err(dev, "failed to get clock rate\n");
		if (IS_ERR_VALUE(priv->clk_rate))
			return priv->clk_rate;

		return -EINVAL;
	}

	return 0;
}

static const struct pwm_ops tiecap_pwm_ops = {
	.set_config     = tiecap_pwm_set_config,
	.set_enable     = tiecap_pwm_set_enable,
	.set_invert     = tiecap_pwm_set_invert,
};

static const struct udevice_id tiecap_pwm_ids[] = {
	{ .compatible = "ti,am3352-ecap" },
	{ .compatible = "ti,am33xx-ecap" },
	{ }
};

U_BOOT_DRIVER(tiecap_pwm) = {
	.name   = "tiecap_pwm",
	.id     = UCLASS_PWM,
	.of_match = tiecap_pwm_ids,
	.ops    = &tiecap_pwm_ops,
	.probe  = tiecap_pwm_probe,
	.of_to_plat     = tiecap_pwm_of_to_plat,
	.priv_auto      = sizeof(struct tiecap_pwm_priv),
};
