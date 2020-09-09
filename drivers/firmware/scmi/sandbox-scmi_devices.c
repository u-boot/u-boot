// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020, Linaro Limited
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/scmi_test.h>
#include <dm/device_compat.h>

/*
 * Simulate to some extent a SCMI exchange.
 * This drivers gets SCMI resources and offers API function to the
 * SCMI test sequence manipulate the resources, currently clocks.
 */

#define SCMI_TEST_DEVICES_CLK_COUNT		3

/*
 * struct sandbox_scmi_device_priv - Storage for device handles used by test
 * @clk:		Array of clock instances used by tests
 * @devices:		Resources exposed by sandbox_scmi_devices_ctx()
 */
struct sandbox_scmi_device_priv {
	struct clk clk[SCMI_TEST_DEVICES_CLK_COUNT];
	struct sandbox_scmi_devices devices;
};

struct sandbox_scmi_devices *sandbox_scmi_devices_ctx(struct udevice *dev)
{
	struct sandbox_scmi_device_priv *priv = dev_get_priv(dev);

	if (priv)
		return &priv->devices;

	return NULL;
}

static int sandbox_scmi_devices_probe(struct udevice *dev)
{
	struct sandbox_scmi_device_priv *priv = dev_get_priv(dev);
	int ret;
	size_t n;

	priv->devices = (struct sandbox_scmi_devices){
		.clk = priv->clk,
		.clk_count = SCMI_TEST_DEVICES_CLK_COUNT,
	};

	for (n = 0; n < SCMI_TEST_DEVICES_CLK_COUNT; n++) {
		ret = clk_get_by_index(dev, n, priv->devices.clk + n);
		if (ret) {
			dev_err(dev, "%s: Failed on clk %zu\n", __func__, n);
			return ret;
		}
	}

	return 0;
}

static const struct udevice_id sandbox_scmi_devices_ids[] = {
	{ .compatible = "sandbox,scmi-devices" },
	{ }
};

U_BOOT_DRIVER(sandbox_scmi_devices) = {
	.name = "sandbox-scmi_devices",
	.id = UCLASS_MISC,
	.of_match = sandbox_scmi_devices_ids,
	.priv_auto_alloc_size = sizeof(struct sandbox_scmi_device_priv),
	.probe = sandbox_scmi_devices_probe,
};
