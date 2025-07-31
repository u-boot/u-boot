// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 * Author: Cheick Traore <cheick.traore@foss.st.com>
 *
 * Originally based on the Linux kernel v6.10 drivers/pwm/pwm-stm32.c.
 */

#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/timers.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/time.h>

#define CCMR_CHANNEL_SHIFT	8
#define CCMR_CHANNEL_MASK	0xFF

struct stm32_pwm_priv {
	bool have_complementary_output;
	bool invert_polarity;
};

static u32 active_channels(struct stm32_timers_plat *plat)
{
	return readl(plat->base + TIM_CCER) & TIM_CCER_CCXE;
}

static int stm32_pwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev_get_parent(dev));
	struct stm32_timers_priv *priv = dev_get_priv(dev_get_parent(dev));
	unsigned long long prd, div, dty;
	unsigned int prescaler = 0;
	u32 ccmr, mask, shift;

	if (duty_ns > period_ns)
		return -EINVAL;

	/*
	 * Period and prescaler values depends on clock rate
	 * First we need to find the minimal value for prescaler such that
	 *
	 *        period_ns * clkrate
	 *   ------------------------------ < max_arr + 1
	 *   NSEC_PER_SEC * (prescaler + 1)
	 *
	 * This equation is equivalent to
	 *
	 *        period_ns * clkrate
	 *   ---------------------------- < prescaler + 1
	 *   NSEC_PER_SEC * (max_arr + 1)
	 *
	 * Using integer division and knowing that the right hand side is
	 * integer, this is further equivalent to
	 *
	 *   (period_ns * clkrate) // (NSEC_PER_SEC * (max_arr + 1)) ≤ prescaler
	 */

	div = (unsigned long long)priv->rate * period_ns;
	do_div(div, NSEC_PER_SEC);
	prd = div;

	do_div(div, priv->max_arr + 1);
	prescaler = div;
	if (prescaler > MAX_TIM_PSC)
		return -EINVAL;

	do_div(prd, prescaler + 1);
	if (!prd)
		return -EINVAL;

	/*
	 * All channels share the same prescaler and counter so when two
	 * channels are active at the same time we can't change them
	 */
	if (active_channels(plat) & ~(1 << channel * 4)) {
		u32 psc, arr;

		psc = readl(plat->base + TIM_PSC);
		arr = readl(plat->base + TIM_ARR);
		if (psc != prescaler || arr != prd - 1)
			return -EBUSY;
	}

	writel(prescaler, plat->base + TIM_PSC);
	writel(prd - 1, plat->base + TIM_ARR);
	setbits_le32(plat->base + TIM_CR1, TIM_CR1_ARPE);

	/* Calculate the duty cycles */
	dty = prd * duty_ns;
	do_div(dty, period_ns);

	writel(dty, plat->base + TIM_CCRx(channel + 1));

	/* Configure output mode */
	shift = (channel & 0x1) * CCMR_CHANNEL_SHIFT;
	ccmr = (TIM_CCMR_PE | TIM_CCMR_M1) << shift;
	mask = CCMR_CHANNEL_MASK << shift;
	if (channel < 2)
		clrsetbits_le32(plat->base + TIM_CCMR1, mask, ccmr);
	else
		clrsetbits_le32(plat->base + TIM_CCMR2, mask, ccmr);

	setbits_le32(plat->base + TIM_BDTR, TIM_BDTR_MOE);

	return 0;
}

static int stm32_pwm_set_enable(struct udevice *dev, uint channel,
				bool enable)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev_get_parent(dev));
	struct stm32_pwm_priv *priv = dev_get_priv(dev);
	u32 mask;

	/* Enable channel */
	mask = TIM_CCER_CC1E << (channel * 4);
	if (priv->have_complementary_output)
		mask |= TIM_CCER_CC1NE << (channel * 4);

	if (enable) {
		setbits_le32(plat->base + TIM_CCER, mask);
		/* Make sure that registers are updated */
		setbits_le32(plat->base + TIM_EGR, TIM_EGR_UG);
		/* Enable controller */
		setbits_le32(plat->base + TIM_CR1, TIM_CR1_CEN);
	} else {
		clrbits_le32(plat->base + TIM_CCER, mask);
		/* When all channels are disabled, we can disable the controller */
		if (!active_channels(plat))
			clrbits_le32(plat->base + TIM_CR1, TIM_CR1_CEN);
	}

	return 0;
}

static int stm32_pwm_set_invert(struct udevice *dev, uint channel,
				bool polarity)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev_get_parent(dev));
	struct stm32_pwm_priv *priv = dev_get_priv(dev);
	u32 mask;

	mask = TIM_CCER_CC1P << (channel * 4);
	if (priv->have_complementary_output)
		mask |= TIM_CCER_CC1NP << (channel * 4);

	clrsetbits_le32(plat->base + TIM_CCER, mask, polarity ? mask : 0);

	return 0;
}

static void stm32_pwm_detect_complementary(struct udevice *dev)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev_get_parent(dev));
	struct stm32_pwm_priv *priv = dev_get_priv(dev);
	u32 ccer, val;

	if (plat->ipidr) {
		/* Simply read from HWCFGR the number of complementary outputs (MP25). */
		val = readl(plat->base + TIM_HWCFGR1);
		priv->have_complementary_output = !!FIELD_GET(TIM_HWCFGR1_NB_OF_DT, val);
		return;
	}

	/*
	 * If complementary bit doesn't exist writing 1 will have no
	 * effect so we can detect it.
	 */
	setbits_le32(plat->base + TIM_CCER, TIM_CCER_CC1NE);
	ccer = readl(plat->base + TIM_CCER);
	clrbits_le32(plat->base + TIM_CCER, TIM_CCER_CC1NE);

	priv->have_complementary_output = (ccer != 0);
}

static int stm32_pwm_probe(struct udevice *dev)
{
	struct stm32_timers_priv *timer = dev_get_priv(dev_get_parent(dev));

	if (timer->rate > 1000000000) {
		dev_err(dev, "Clock freq too high (%lu)\n", timer->rate);
		return -EINVAL;
	}

	stm32_pwm_detect_complementary(dev);

	return 0;
}

static const struct pwm_ops stm32_pwm_ops = {
	.set_config	= stm32_pwm_set_config,
	.set_enable	= stm32_pwm_set_enable,
	.set_invert	= stm32_pwm_set_invert,
};

static const struct udevice_id stm32_pwm_ids[] = {
	{ .compatible = "st,stm32-pwm" },
	{ .compatible = "st,stm32mp25-pwm" },
	{ }
};

U_BOOT_DRIVER(stm32_pwm) = {
	.name		= "stm32_pwm",
	.id		= UCLASS_PWM,
	.of_match	= stm32_pwm_ids,
	.ops		= &stm32_pwm_ops,
	.probe          = stm32_pwm_probe,
	.priv_auto	= sizeof(struct stm32_pwm_priv),
};
