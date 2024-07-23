// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Samsung Exynos TRNG driver (True Random Number Generator).
 */

#include <clk.h>
#include <dm.h>
#include <rng.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/arm-smccc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/time.h>

#define EXYNOS_TRNG_CLKDIV		0x0
#define EXYNOS_TRNG_CLKDIV_MASK		GENMASK(15, 0)
#define EXYNOS_TRNG_CLOCK_RATE		500000

#define EXYNOS_TRNG_CTRL		0x20
#define EXYNOS_TRNG_CTRL_RNGEN		BIT(31)

#define EXYNOS_TRNG_POST_CTRL		0x30
#define EXYNOS_TRNG_ONLINE_CTRL		0x40
#define EXYNOS_TRNG_ONLINE_STAT		0x44
#define EXYNOS_TRNG_ONLINE_MAXCHI2	0x48
#define EXYNOS_TRNG_FIFO_CTRL		0x50
#define EXYNOS_TRNG_FIFO_0		0x80
#define EXYNOS_TRNG_FIFO_1		0x84
#define EXYNOS_TRNG_FIFO_2		0x88
#define EXYNOS_TRNG_FIFO_3		0x8c
#define EXYNOS_TRNG_FIFO_4		0x90
#define EXYNOS_TRNG_FIFO_5		0x94
#define EXYNOS_TRNG_FIFO_6		0x98
#define EXYNOS_TRNG_FIFO_7		0x9c
#define EXYNOS_TRNG_FIFO_LEN		8
#define EXYNOS_TRNG_FIFO_TIMEOUT	(1 * USEC_PER_SEC)

#define EXYNOS_SMC_CALL_VAL(func_num)			\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,		\
			   ARM_SMCCC_SMC_32,		\
			   ARM_SMCCC_OWNER_SIP,		\
			   func_num)

/* SMC command for DTRNG access */
#define SMC_CMD_RANDOM			EXYNOS_SMC_CALL_VAL(0x1012)

/* SMC_CMD_RANDOM: arguments */
#define HWRNG_INIT			0x0
#define HWRNG_EXIT			0x1
#define HWRNG_GET_DATA			0x2

/* SMC_CMD_RANDOM: return values */
#define HWRNG_RET_OK			0x0
#define HWRNG_RET_RETRY_ERROR		0x2

#define HWRNG_MAX_TRIES			100

/**
 * struct exynos_trng_variant - Chip specific data
 *
 * @smc: Set "true" if TRNG block has to be accessed via SMC calls
 * @init: (Optional) TRNG initialization function to call on probe
 * @exit: (Optional) TRNG deinitialization function to call on remove
 * @read: Function to read the random data from TRNG block
 */
struct exynos_trng_variant {
	bool smc;
	int (*init)(struct udevice *dev);
	void (*exit)(struct udevice *dev);
	int (*read)(struct udevice *dev, void *data, size_t len);
};

/**
 * struct exynos_trng_priv - Driver's private data
 *
 * @base: Base address of MMIO registers of TRNG block
 * @clk: Operating clock (needed for TRNG block functioning)
 * @pclk: Bus clock (needed for interfacing the TRNG block registers)
 * @data: Chip specific data
 */
struct exynos_trng_priv {
	void __iomem *base;
	struct clk *clk;
	struct clk *pclk;
	const struct exynos_trng_variant *data;
};

static int exynos_trng_read_reg(struct udevice *dev, void *data, size_t len)
{
	struct exynos_trng_priv *trng = dev_get_priv(dev);
	int val;

	len = min_t(size_t, len, EXYNOS_TRNG_FIFO_LEN * 4);
	writel_relaxed(len * 8, trng->base + EXYNOS_TRNG_FIFO_CTRL);
	val = readl_poll_timeout(trng->base + EXYNOS_TRNG_FIFO_CTRL, val,
				 val == 0, EXYNOS_TRNG_FIFO_TIMEOUT);
	if (val < 0)
		return val;

	memcpy_fromio(data, trng->base + EXYNOS_TRNG_FIFO_0, len);

	return 0;
}

static int exynos_trng_read_smc(struct udevice *dev, void *data, size_t len)
{
	struct arm_smccc_res res;
	unsigned int copied = 0;
	u32 *buf = data;
	int tries = 0;

	while (copied < len) {
		arm_smccc_smc(SMC_CMD_RANDOM, HWRNG_GET_DATA, 0, 0, 0, 0, 0, 0,
			      &res);
		switch (res.a0) {
		case HWRNG_RET_OK:
			*buf++ = res.a2;
			*buf++ = res.a3;
			copied += 8;
			tries = 0;
			break;
		case HWRNG_RET_RETRY_ERROR:
			if (++tries >= HWRNG_MAX_TRIES)
				return -EIO;
			udelay(10);
			break;
		default:
			return -EIO;
		}
	}

	return 0;
}

static int exynos_trng_init_reg(struct udevice *dev)
{
	const u32 max_div = EXYNOS_TRNG_CLKDIV_MASK;
	struct exynos_trng_priv *trng = dev_get_priv(dev);
	unsigned long sss_rate;
	u32 div;

	sss_rate = clk_get_rate(trng->clk);

	/*
	 * For most TRNG circuits the clock frequency of under 500 kHz is safe.
	 * The clock divider should be an even number.
	 */
	div = sss_rate / EXYNOS_TRNG_CLOCK_RATE;
	div -= div % 2; /* make sure it's even */
	if (div > max_div) {
		dev_err(dev, "Clock divider too large: %u", div);
		return -ERANGE;
	}
	writel_relaxed(div, trng->base + EXYNOS_TRNG_CLKDIV);

	/* Enable the generator */
	writel_relaxed(EXYNOS_TRNG_CTRL_RNGEN, trng->base + EXYNOS_TRNG_CTRL);

	/* Disable post-processing */
	writel_relaxed(0, trng->base + EXYNOS_TRNG_POST_CTRL);

	return 0;
}

static int exynos_trng_init_smc(struct udevice *dev)
{
	struct arm_smccc_res res;
	int ret = 0;

	arm_smccc_smc(SMC_CMD_RANDOM, HWRNG_INIT, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != HWRNG_RET_OK) {
		dev_err(dev, "SMC command for TRNG init failed (%d)\n",
			(int)res.a0);
		ret = -EIO;
	}
	if ((int)res.a0 == -1)
		dev_info(dev, "Make sure LDFW is loaded\n");

	return ret;
}

static void exynos_trng_exit_smc(struct udevice *dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SMC_CMD_RANDOM, HWRNG_EXIT, 0, 0, 0, 0, 0, 0, &res);
}

static int exynos_trng_read(struct udevice *dev, void *data, size_t len)
{
	struct exynos_trng_priv *trng = dev_get_priv(dev);

	return trng->data->read(dev, data, len);
}

static int exynos_trng_of_to_plat(struct udevice *dev)
{
	struct exynos_trng_priv *trng = dev_get_priv(dev);

	trng->data = (struct exynos_trng_variant *)dev_get_driver_data(dev);
	if (!trng->data->smc) {
		trng->base = dev_read_addr_ptr(dev);
		if (!trng->base)
			return -EINVAL;
	}

	trng->clk = devm_clk_get(dev, "secss");
	if (IS_ERR(trng->clk))
		return PTR_ERR(trng->clk);

	trng->pclk = devm_clk_get_optional(dev, "pclk");
	if (IS_ERR(trng->pclk))
		return PTR_ERR(trng->pclk);

	return 0;
}

static int exynos_trng_probe(struct udevice *dev)
{
	struct exynos_trng_priv *trng = dev_get_priv(dev);
	int ret;

	ret = clk_enable(trng->pclk);
	if (ret)
		return ret;

	ret = clk_enable(trng->clk);
	if (ret)
		return ret;

	if (trng->data->init)
		ret = trng->data->init(dev);

	return ret;
}

static int exynos_trng_remove(struct udevice *dev)
{
	struct exynos_trng_priv *trng = dev_get_priv(dev);

	if (trng->data->exit)
		trng->data->exit(dev);

	/* Keep SSS clocks enabled, they are needed for EL3_MON and kernel */

	return 0;
}

static const struct dm_rng_ops exynos_trng_ops = {
	.read	= exynos_trng_read,
};

static const struct exynos_trng_variant exynos5250_trng_data = {
	.init	= exynos_trng_init_reg,
	.read	= exynos_trng_read_reg,
};

static const struct exynos_trng_variant exynos850_trng_data = {
	.smc	= true,
	.init	= exynos_trng_init_smc,
	.exit	= exynos_trng_exit_smc,
	.read	= exynos_trng_read_smc,
};

static const struct udevice_id exynos_trng_match[] = {
	{
		.compatible = "samsung,exynos5250-trng",
		.data = (ulong)&exynos5250_trng_data,
	}, {
		.compatible = "samsung,exynos850-trng",
		.data = (ulong)&exynos850_trng_data,
	},
	{ },
};

U_BOOT_DRIVER(exynos_trng) = {
	.name		= "exynos-trng",
	.id		= UCLASS_RNG,
	.of_match	= exynos_trng_match,
	.of_to_plat	= exynos_trng_of_to_plat,
	.probe		= exynos_trng_probe,
	.remove		= exynos_trng_remove,
	.ops		= &exynos_trng_ops,
	.priv_auto	= sizeof(struct exynos_trng_priv),
};
