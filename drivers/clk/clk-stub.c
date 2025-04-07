// SPDX-License-Identifier: GPL-2.0
/*
 * Stub clk driver for non-essential clocks.
 *
 * This driver should be used for clock controllers
 * which are described as dependencies in DT but aren't
 * actually necessary for hardware functionality.
 */

#include <clk-uclass.h>
#include <dm.h>

/* NOP parent nodes to stub clocks */
static const struct udevice_id nop_parent_ids[] = {
	{ .compatible = "qcom,rpm-proc" },
	{ .compatible = "qcom,glink-rpm" },
	{ .compatible = "qcom,glink-smd-rpm" },
	{ }
};

U_BOOT_DRIVER(nop_parent) = {
	.name = "nop_parent",
	.id = UCLASS_NOP,
	.of_match = nop_parent_ids,
	.bind = dm_scan_fdt_dev,
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

static ulong stub_clk_set_rate(struct clk *clk, ulong rate)
{
	return (clk->rate = rate);
}

static ulong stub_clk_get_rate(struct clk *clk)
{
	return clk->rate;
}

static int stub_clk_nop(struct clk *clk)
{
	return 0;
}

static struct clk_ops stub_clk_ops = {
	.set_rate = stub_clk_set_rate,
	.get_rate = stub_clk_get_rate,
	.enable = stub_clk_nop,
	.disable = stub_clk_nop,
};

static const struct udevice_id stub_clk_ids[] = {
	{ .compatible = "qcom,rpmcc" },
	{ .compatible = "qcom,sdm845-rpmh-clk" },
	{ .compatible = "qcom,sc7280-rpmh-clk" },
	{ .compatible = "qcom,sm8150-rpmh-clk" },
	{ .compatible = "qcom,sm8250-rpmh-clk" },
	{ .compatible = "qcom,sm8550-rpmh-clk" },
	{ .compatible = "qcom,sm8650-rpmh-clk" },
	{ }
};

U_BOOT_DRIVER(clk_stub) = {
	.name = "clk_stub",
	.id = UCLASS_CLK,
	.ops = &stub_clk_ops,
	.of_match = stub_clk_ids,
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};

