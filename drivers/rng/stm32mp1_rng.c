// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_RNG

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <reset.h>
#include <rng.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>

#define RNG_CR 0x00
#define RNG_CR_RNGEN BIT(2)
#define RNG_CR_CED BIT(5)

#define RNG_SR 0x04
#define RNG_SR_SEIS BIT(6)
#define RNG_SR_CEIS BIT(5)
#define RNG_SR_SECS BIT(2)
#define RNG_SR_DRDY BIT(0)

#define RNG_DR 0x08

struct stm32_rng_platdata {
	fdt_addr_t base;
	struct clk clk;
	struct reset_ctl rst;
};

static int stm32_rng_read(struct udevice *dev, void *data, size_t len)
{
	int retval, i;
	u32 sr, count, reg;
	size_t increment;
	struct stm32_rng_platdata *pdata = dev_get_platdata(dev);

	while (len > 0) {
		retval = readl_poll_timeout(pdata->base + RNG_SR, sr,
					    sr & RNG_SR_DRDY, 10000);
		if (retval)
			return retval;

		if (sr & (RNG_SR_SEIS | RNG_SR_SECS)) {
			/* As per SoC TRM */
			clrbits_le32(pdata->base + RNG_SR, RNG_SR_SEIS);
			for (i = 0; i < 12; i++)
				readl(pdata->base + RNG_DR);
			if (readl(pdata->base + RNG_SR) & RNG_SR_SEIS) {
				log_err("RNG Noise");
				return -EIO;
			}
			/* start again */
			continue;
		}

		/*
		 * Once the DRDY bit is set, the RNG_DR register can
		 * be read four consecutive times.
		 */
		count = 4;
		while (len && count) {
			reg = readl(pdata->base + RNG_DR);
			memcpy(data, &reg, min(len, sizeof(u32)));
			increment = min(len, sizeof(u32));
			data += increment;
			len -= increment;
			count--;
		}
	}

	return 0;
}

static int stm32_rng_init(struct stm32_rng_platdata *pdata)
{
	int err;

	err = clk_enable(&pdata->clk);
	if (err)
		return err;

	/* Disable CED */
	writel(RNG_CR_RNGEN | RNG_CR_CED, pdata->base + RNG_CR);

	/* clear error indicators */
	writel(0, pdata->base + RNG_SR);

	return 0;
}

static int stm32_rng_cleanup(struct stm32_rng_platdata *pdata)
{
	writel(0, pdata->base + RNG_CR);

	return clk_disable(&pdata->clk);
}

static int stm32_rng_probe(struct udevice *dev)
{
	struct stm32_rng_platdata *pdata = dev_get_platdata(dev);

	reset_assert(&pdata->rst);
	udelay(20);
	reset_deassert(&pdata->rst);

	return stm32_rng_init(pdata);
}

static int stm32_rng_remove(struct udevice *dev)
{
	struct stm32_rng_platdata *pdata = dev_get_platdata(dev);

	return stm32_rng_cleanup(pdata);
}

static int stm32_rng_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32_rng_platdata *pdata = dev_get_platdata(dev);
	int err;

	pdata->base = dev_read_addr(dev);
	if (!pdata->base)
		return -ENOMEM;

	err = clk_get_by_index(dev, 0, &pdata->clk);
	if (err)
		return err;

	err = reset_get_by_index(dev, 0, &pdata->rst);
	if (err)
		return err;

	return 0;
}

static const struct dm_rng_ops stm32_rng_ops = {
	.read = stm32_rng_read,
};

static const struct udevice_id stm32_rng_match[] = {
	{
		.compatible = "st,stm32-rng",
	},
	{},
};

U_BOOT_DRIVER(stm32_rng) = {
	.name = "stm32-rng",
	.id = UCLASS_RNG,
	.of_match = stm32_rng_match,
	.ops = &stm32_rng_ops,
	.probe = stm32_rng_probe,
	.remove = stm32_rng_remove,
	.platdata_auto_alloc_size = sizeof(struct stm32_rng_platdata),
	.ofdata_to_platdata = stm32_rng_ofdata_to_platdata,
};
