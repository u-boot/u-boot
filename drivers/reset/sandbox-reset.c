// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <reset-uclass.h>
#include <asm/io.h>
#include <asm/reset.h>
#include <linux/delay.h>

#define SANDBOX_RESET_SIGNALS 101

struct sandbox_reset_signal {
	bool asserted;
	bool requested;
	int reset_count;
};

struct sandbox_reset {
	struct sandbox_reset_signal signals[SANDBOX_RESET_SIGNALS];
};

static int sandbox_reset_request(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	if (reset_ctl->id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	sbr->signals[reset_ctl->id].requested = true;
	sbr->signals[reset_ctl->id].reset_count = 0;
	return 0;
}

static int sandbox_reset_free(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	sbr->signals[reset_ctl->id].requested = false;
	return 0;
}

static int sandbox_reset_assert(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	sbr->signals[reset_ctl->id].asserted = true;

	return 0;
}

static int sandbox_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p)\n", __func__, reset_ctl);

	sbr->signals[reset_ctl->id].asserted = false;

	return 0;
}

static int sandbox_reset_reset(struct reset_ctl *reset_ctl, ulong delay_us)
{
	struct sandbox_reset *sbr = dev_get_priv(reset_ctl->dev);

	debug("%s(reset_ctl=%p, delay_us=%lu)\n", __func__, reset_ctl,
	      delay_us);

	sbr->signals[reset_ctl->id].asserted = true;
	udelay(delay_us);
	sbr->signals[reset_ctl->id].asserted = false;
	sbr->signals[reset_ctl->id].reset_count++;

	return 0;
}

static int sandbox_reset_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int sandbox_reset_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static const struct udevice_id sandbox_reset_ids[] = {
	{ .compatible = "sandbox,reset-ctl" },
	{ }
};

static const struct reset_ops sandbox_reset_reset_ops = {
	.request = sandbox_reset_request,
	.rfree = sandbox_reset_free,
	.rst_assert = sandbox_reset_assert,
	.rst_deassert = sandbox_reset_deassert,
	.rst_reset = sandbox_reset_reset,
};

U_BOOT_DRIVER(sandbox_reset) = {
	.name = "sandbox_reset",
	.id = UCLASS_RESET,
	.of_match = sandbox_reset_ids,
	.bind = sandbox_reset_bind,
	.probe = sandbox_reset_probe,
	.priv_auto	= sizeof(struct sandbox_reset),
	.ops = &sandbox_reset_reset_ops,
};

/*
 * Second sandbox reset controller for tests: same assert/deassert
 * behaviour as sandbox_reset, but no rst_reset so reset_reset() uses
 * the core assert / udelay / deassert fallback (reset_count never bumps).
 */
static const struct udevice_id sandbox_reset_fallback_ids[] = {
	{ .compatible = "sandbox,reset-ctl-fallback-only" },
	{ }
};

static const struct reset_ops sandbox_reset_fallback_reset_ops = {
	.request = sandbox_reset_request,
	.rfree = sandbox_reset_free,
	.rst_assert = sandbox_reset_assert,
	.rst_deassert = sandbox_reset_deassert,
};

U_BOOT_DRIVER(sandbox_reset_fallback) = {
	.name = "sandbox_reset_fallback",
	.id = UCLASS_RESET,
	.of_match = sandbox_reset_fallback_ids,
	.bind = sandbox_reset_bind,
	.probe = sandbox_reset_probe,
	.priv_auto = sizeof(struct sandbox_reset),
	.ops = &sandbox_reset_fallback_reset_ops,
};

int sandbox_reset_query(struct udevice *dev, unsigned long id)
{
	struct sandbox_reset *sbr = dev_get_priv(dev);

	debug("%s(dev=%p, id=%ld)\n", __func__, dev, id);

	if (id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	return sbr->signals[id].asserted;
}

int sandbox_reset_is_requested(struct udevice *dev, unsigned long id)
{
	struct sandbox_reset *sbr = dev_get_priv(dev);

	debug("%s(dev=%p, id=%ld)\n", __func__, dev, id);

	if (id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	return sbr->signals[id].requested;
}

int sandbox_reset_get_count(struct udevice *dev, unsigned long id)
{
	struct sandbox_reset *sbr = dev_get_priv(dev);

	debug("%s(dev=%p, id=%ld)\n", __func__, dev, id);

	if (id >= SANDBOX_RESET_SIGNALS)
		return -EINVAL;

	return sbr->signals[id].reset_count;
}
