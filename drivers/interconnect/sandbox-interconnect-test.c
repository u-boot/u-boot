// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025 Linaro Limited
 */

#include <dm.h>
#include <malloc.h>
#include <interconnect.h>
#include <asm/io.h>
#include <linux/err.h>

struct sandbox_interconnect_test {
	struct icc_path *path;
};

int sandbox_interconnect_test_get(struct udevice *dev, char *name)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);

	priv->path = of_icc_get(dev, name);
	if (IS_ERR(priv->path))
		return PTR_ERR(priv->path);

	if (!priv->path)
		return -ENOSYS;

	return 0;
}

int sandbox_interconnect_test_get_index(struct udevice *dev, int index)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);

	priv->path = of_icc_get_by_index(dev, index);
	if (IS_ERR(priv->path))
		return PTR_ERR(priv->path);

	if (!priv->path)
		return -ENOSYS;

	return 0;
}

int sandbox_interconnect_test_enable(struct udevice *dev)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);

	return icc_enable(priv->path);
}

int sandbox_interconnect_test_disable(struct udevice *dev)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);

	return icc_disable(priv->path);
}

int sandbox_interconnect_test_set_bw(struct udevice *dev, u32 avg_bw, u32 peak_bw)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);

	return icc_set_bw(priv->path, avg_bw, peak_bw);
}

int sandbox_interconnect_test_put(struct udevice *dev)
{
	struct sandbox_interconnect_test *priv = dev_get_priv(dev);
	int ret;

	ret = icc_put(priv->path);
	if (ret)
		return ret;

	priv->path = NULL;

	return 0;
}

static const struct udevice_id sandbox_interconnect_test_ids[] = {
	{ .compatible = "sandbox,interconnect-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_interconnect_test) = {
	.name = "sandbox_interconnect_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_interconnect_test_ids,
	.priv_auto = sizeof(struct sandbox_interconnect_test),
};
