// SPDX-License-Identifier: GPL-2.0
/*
 * Airoha True Random Number Generator driver
 *
 * Ported from the Linux driver drivers/char/hw_random/airoha-trng.c
 *
 * Copyright (C) 2024 Christian Marangi <ansuelsmth@gmail.com>
 */

#include <dm.h>
#include <rng.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/string.h>

#define TRNG_IP_RDY			0x800
#define   CNT_TRANS			GENMASK(15, 8)
#define   SAMPLE_RDY			BIT(0)
#define TRNG_NS_SEK_AND_DAT_EN		0x804
#define   RNG_EN			BIT(31)
#define   RAW_DATA_EN			BIT(16)
#define TRNG_HEALTH_TEST_SW_RST		0x808
#define   SW_RST			BIT(0) /* Active High */
#define TRNG_INTR_EN			0x818
#define   INTR_MASK			BIT(16)
#define   RST_STARTUP_INITR_EN		BIT(0)
#define TRNG_HEALTH_TEST_STATUS		0x824
#define   RST_STARTUP_TEST_DONE		BIT(18)
#define   RST_STARTUP_AP_TEST_FAIL	BIT(17)
#define   RST_STARTUP_RC_TEST_FAIL	BIT(16)
#define   RAW_DATA_VALID		BIT(7)
#define TRNG_RAW_DATA_OUT		0x828

#define TRNG_CNT_TRANS_VALID		0x80
#define TRNG_TIMEOUT_US			100000

struct airoha_trng {
	void __iomem *base;
};

static int airoha_trng_read(struct udevice *dev, void *data, size_t len)
{
	struct airoha_trng *trng = dev_get_priv(dev);
	u8 *buf = data;

	while (len) {
		size_t step = min(len, sizeof(u32));
		u32 status, val;
		int ret;

		ret = readl_poll_timeout(trng->base + TRNG_HEALTH_TEST_STATUS,
					 status, status & RAW_DATA_VALID,
					 TRNG_TIMEOUT_US);
		if (ret) {
			dev_err(dev, "Timeout waiting for TRNG RAW Data valid\n");
			return ret;
		}

		val = readl(trng->base + TRNG_RAW_DATA_OUT);
		memcpy(buf, &val, step);

		buf += step;
		len -= step;
	}

	return 0;
}

static int airoha_trng_probe(struct udevice *dev)
{
	struct airoha_trng *trng = dev_get_priv(dev);
	u32 val;
	int ret;

	trng->base = dev_read_addr_ptr(dev);
	if (!trng->base)
		return -EINVAL;

	/* No interrupts in U-Boot: keep the TRNG one masked and poll instead */
	val = readl(trng->base + TRNG_INTR_EN);
	val |= INTR_MASK | RST_STARTUP_INITR_EN;
	writel(val, trng->base + TRNG_INTR_EN);

	val = readl(trng->base + TRNG_NS_SEK_AND_DAT_EN);
	val |= RAW_DATA_EN | RNG_EN;
	writel(val, trng->base + TRNG_NS_SEK_AND_DAT_EN);

	/* Health Test runs only on the transition out of SW Reset */
	writel(SW_RST, trng->base + TRNG_HEALTH_TEST_SW_RST);
	writel(0, trng->base + TRNG_HEALTH_TEST_SW_RST);

	ret = readl_poll_timeout(trng->base + TRNG_HEALTH_TEST_STATUS, val,
				 val & RST_STARTUP_TEST_DONE, TRNG_TIMEOUT_US);
	if (ret) {
		dev_err(dev, "Timeout waiting for Health Check\n");
		return ret;
	}

	if (val & (RST_STARTUP_AP_TEST_FAIL | RST_STARTUP_RC_TEST_FAIL)) {
		dev_err(dev, "Health Check fail: %s test fail\n",
			val & RST_STARTUP_AP_TEST_FAIL ? "AP" : "RC");
		return -EIO;
	}

	ret = readl_poll_timeout(trng->base + TRNG_IP_RDY, val,
				 val & SAMPLE_RDY &&
				 FIELD_GET(CNT_TRANS, val) == TRNG_CNT_TRANS_VALID,
				 TRNG_TIMEOUT_US);
	if (ret) {
		dev_err(dev, "Timeout waiting for IP ready\n");
		return ret;
	}

	return 0;
}

static const struct dm_rng_ops airoha_trng_ops = {
	.read = airoha_trng_read,
};

static const struct udevice_id airoha_trng_match[] = {
	{ .compatible = "airoha,en7581-trng" },
	{ }
};

U_BOOT_DRIVER(airoha_trng) = {
	.name = "airoha-trng",
	.id = UCLASS_RNG,
	.of_match = airoha_trng_match,
	.ops = &airoha_trng_ops,
	.probe = airoha_trng_probe,
	.priv_auto = sizeof(struct airoha_trng),
};
