// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 * Peng Fan <peng.fan@nxp.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/clock.h>
#include <dt-bindings/clock/imx8qxp-clock.h>
#include <dt-bindings/soc/imx_rsrc.h>
#include <misc.h>

#include "clk-imx8.h"

__weak ulong imx8_clk_get_rate(struct clk *clk)
{
	return 0;
}

__weak ulong imx8_clk_set_rate(struct clk *clk, unsigned long rate)
{
	return 0;
}

__weak int __imx8_clk_enable(struct clk *clk, bool enable)
{
	return -EINVAL;
}

static int imx8_clk_disable(struct clk *clk)
{
	return __imx8_clk_enable(clk, 0);
}

static int imx8_clk_enable(struct clk *clk)
{
	return __imx8_clk_enable(clk, 1);
}

#if IS_ENABLED(CONFIG_CMD_CLK)
static void imx8_clk_dump(struct udevice *dev)
{
	struct clk clk;
	unsigned long rate;
	int i, ret;

	printf("Clk\t\tHz\n");

	for (i = 0; i < num_clks; i++) {
		clk.id = imx8_clk_names[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0) {
			debug("%s clk_request() failed: %d\n", __func__, ret);
			continue;
		}

		rate = clk_get_rate(&clk);

		if (!rate) {
			printf("clk ID %lu not supported yet\n",
			       imx8_clk_names[i].id);
			continue;
		}

		printf("%s(%3lu):\t%lu\n",
		       imx8_clk_names[i].name, imx8_clk_names[i].id, rate);
	}
}
#endif

static struct clk_ops imx8_clk_ops = {
	.set_rate = imx8_clk_set_rate,
	.get_rate = imx8_clk_get_rate,
	.enable = imx8_clk_enable,
	.disable = imx8_clk_disable,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump = imx8_clk_dump,
#endif
};

static int imx8_clk_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id imx8_clk_ids[] = {
	{ .compatible = "fsl,imx8qxp-clk" },
	{ .compatible = "fsl,imx8qm-clk" },
	{ },
};

U_BOOT_DRIVER(imx8_clk) = {
	.name = "clk_imx8",
	.id = UCLASS_CLK,
	.of_match = imx8_clk_ids,
	.ops = &imx8_clk_ops,
	.probe = imx8_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
