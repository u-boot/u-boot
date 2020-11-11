// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Driver for Amlogic hardware random number generator
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <rng.h>
#include <asm/io.h>

struct meson_rng_platdata {
	fdt_addr_t base;
	struct clk clk;
};

/**
 * meson_rng_read() - fill buffer with random bytes
 *
 * @buffer:	buffer to receive data
 * @size:	size of buffer
 *
 * Return:	0
 */
static int meson_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct meson_rng_platdata *pdata = dev_get_platdata(dev);
	char *buffer = (char *)data;

	while (len) {
		u32 rand = readl(pdata->base);
		size_t step;

		if (len >= 4)
			step = 4;
		else
			step = len;
		memcpy(buffer, &rand, step);
		buffer += step;
		len -= step;
	}
	return 0;
}

/**
 * meson_rng_probe() - probe rng device
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_probe(struct udevice *dev)
{
	struct meson_rng_platdata *pdata = dev_get_platdata(dev);
	int err;

	err = clk_enable(&pdata->clk);
	if (err)
		return err;

	return 0;
}

/**
 * meson_rng_remove() - deinitialize rng device
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_remove(struct udevice *dev)
{
	struct meson_rng_platdata *pdata = dev_get_platdata(dev);

	return clk_disable(&pdata->clk);
}

/**
 * meson_rng_ofdata_to_platdata() - transfer device tree data to plaform data
 *
 * @dev:	device
 * Return:	0 if ok
 */
static int meson_rng_ofdata_to_platdata(struct udevice *dev)
{
	struct meson_rng_platdata *pdata = dev_get_platdata(dev);
	int err;

	pdata->base = dev_read_addr(dev);
	if (!pdata->base)
		return -ENODEV;

	/* Get optional "core" clock */
	err = clk_get_by_name(dev, "core", &pdata->clk);
	if (err && err != -ENODATA)
		return err;

	return 0;
}

static const struct dm_rng_ops meson_rng_ops = {
	.read = meson_rng_read,
};

static const struct udevice_id meson_rng_match[] = {
	{
		.compatible = "amlogic,meson-rng",
	},
	{},
};

U_BOOT_DRIVER(meson_rng) = {
	.name = "meson-rng",
	.id = UCLASS_RNG,
	.of_match = meson_rng_match,
	.ops = &meson_rng_ops,
	.probe = meson_rng_probe,
	.remove = meson_rng_remove,
	.platdata_auto_alloc_size = sizeof(struct meson_rng_platdata),
	.ofdata_to_platdata = meson_rng_ofdata_to_platdata,
};
