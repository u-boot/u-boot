/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/reset.h>

struct sandbox_reset_test {
	struct reset_ctl ctl;
};

int sandbox_reset_test_get(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_get_by_name(dev, "test", &sbrt->ctl);
}

int sandbox_reset_test_assert(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_assert(&sbrt->ctl);
}

int sandbox_reset_test_deassert(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_deassert(&sbrt->ctl);
}

int sandbox_reset_test_free(struct udevice *dev)
{
	struct sandbox_reset_test *sbrt = dev_get_priv(dev);

	return reset_free(&sbrt->ctl);
}

static const struct udevice_id sandbox_reset_test_ids[] = {
	{ .compatible = "sandbox,reset-ctl-test" },
	{ }
};

U_BOOT_DRIVER(sandbox_reset_test) = {
	.name = "sandbox_reset_test",
	.id = UCLASS_MISC,
	.of_match = sandbox_reset_test_ids,
	.priv_auto_alloc_size = sizeof(struct sandbox_reset_test),
};
