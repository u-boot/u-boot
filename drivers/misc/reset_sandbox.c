/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset.h>
#include <asm/state.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

static int sandbox_warm_reset_request(struct udevice *dev, enum reset_t type)
{
	struct sandbox_state *state = state_get_current();

	switch (type) {
	case RESET_WARM:
		state->last_reset = type;
		break;
	default:
		return -ENOSYS;
	}
	if (!state->reset_allowed[type])
		return -EACCES;

	return -EINPROGRESS;
}

static int sandbox_reset_request(struct udevice *dev, enum reset_t type)
{
	struct sandbox_state *state = state_get_current();

	/*
	 * If we have a device tree, the device we created from platform data
	 * (see the U_BOOT_DEVICE() declaration below) should not do anything.
	 * If we are that device, return an error.
	 */
	if (state->fdt_fname && dev->of_offset == -1)
		return -ENODEV;

	switch (type) {
	case RESET_COLD:
		state->last_reset = type;
		break;
	case RESET_POWER:
		state->last_reset = type;
		if (!state->reset_allowed[type])
			return -EACCES;
		sandbox_exit();
		break;
	default:
		return -ENOSYS;
	}
	if (!state->reset_allowed[type])
		return -EACCES;

	return -EINPROGRESS;
}

static struct reset_ops sandbox_reset_ops = {
	.request	= sandbox_reset_request,
};

static const struct udevice_id sandbox_reset_ids[] = {
	{ .compatible = "sandbox,reset" },
	{ }
};

U_BOOT_DRIVER(reset_sandbox) = {
	.name		= "reset_sandbox",
	.id		= UCLASS_RESET,
	.of_match	= sandbox_reset_ids,
	.ops		= &sandbox_reset_ops,
};

static struct reset_ops sandbox_warm_reset_ops = {
	.request	= sandbox_warm_reset_request,
};

static const struct udevice_id sandbox_warm_reset_ids[] = {
	{ .compatible = "sandbox,warm-reset" },
	{ }
};

U_BOOT_DRIVER(warm_reset_sandbox) = {
	.name		= "warm_reset_sandbox",
	.id		= UCLASS_RESET,
	.of_match	= sandbox_warm_reset_ids,
	.ops		= &sandbox_warm_reset_ops,
};

/* This is here in case we don't have a device tree */
U_BOOT_DEVICE(reset_sandbox_non_fdt) = {
	.name = "reset_sandbox",
};
