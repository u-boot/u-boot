// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas R-Car Gen5 CPG driver
 *
 * Copyright (C) 2026 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/clk-provider.h>

#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>

#include <dt-bindings/clock/r8a78000-clock-scmi.h>

#if IS_ENABLED(CONFIG_CLK_SCMI)
struct gen5_clk_priv {
	struct udevice	*clk;
	u32		basever;
};

static struct clk *gen5_clk_get_by_scmi_id(struct clk *clk)
{
	struct gen5_clk_priv *priv = dev_get_priv(clk->dev);
	struct udevice *sdev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_CLK, sdev, uc)
		if (sdev->seq_ == priv->clk->seq_ + clk->id + 1)
			return dev_get_clk_ptr(sdev);

	return NULL;
}

static ulong gen5_clk_round_rate(struct clk *clk, ulong rate)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_round_rate(scmi, rate);
}

static ulong gen5_clk_get_rate(struct clk *clk)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_get_rate(scmi);
}

static ulong gen5_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_set_rate(scmi, rate);
}

static int gen5_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_set_parent(scmi, parent);
}

static int gen5_clk_enable(struct clk *clk)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_enable(scmi);
}

static int gen5_clk_disable(struct clk *clk)
{
	struct clk *scmi = gen5_clk_get_by_scmi_id(clk);

	if (!scmi)
		return -ENODEV;

	return clk_disable(scmi);
}

struct clk_map_in {
	u16 dt_id;	/* DT binding clock ID */
	u16 fw_id;	/* SCMI firmware clock ID */
};

#define GEN5_SCMI_SDK_4_28		0x010a0000
#define GEN5_SCMI_SDK_4_29		0x010b0000
#define GEN5_SCMI_SDK_4_30		0x010c0000
#define GEN5_SCMI_SDK_4_31		0x010d0000
#define GEN5_SCMI_SDK_4_32		0x010e0000

static const struct clk_map_in gen5_clk_map_dt_sdk_4_28[] = {
	{ SCP_CLOCK_ID_MDLC_UFS0, 202 },
	{ SCP_CLOCK_ID_MDLC_UFS1, 203 },
	{ SCP_CLOCK_ID_MDLC_SDHI0, 204 },
	{ SCP_CLOCK_ID_MDLC_XPCS0, 316 },
	{ SCP_CLOCK_ID_MDLC_XPCS1, 317 },
	{ SCP_CLOCK_ID_MDLC_XPCS2, 318 },
	{ SCP_CLOCK_ID_MDLC_XPCS3, 319 },
	{ SCP_CLOCK_ID_MDLC_XPCS4, 320 },
	{ SCP_CLOCK_ID_MDLC_XPCS5, 321 },
	{ SCP_CLOCK_ID_MDLC_XPCS6, 322 },
	{ SCP_CLOCK_ID_MDLC_XPCS7, 323 },
	{ SCP_CLOCK_ID_MDLC_RSW3, 324 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSN, 325 },
	{ SCP_CLOCK_ID_MDLC_RSW3AES, 326 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES0, 327 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES1, 328 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES2, 329 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES3, 330 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES4, 331 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES5, 332 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES6, 333 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES7, 334 },
	{ SCP_CLOCK_ID_MDLC_RSW3MFWD, 335 },
	{ SCP_CLOCK_ID_MDLC_MPPHY01, 344 },
	{ SCP_CLOCK_ID_MDLC_MPPHY11, 345 },
	{ SCP_CLOCK_ID_MDLC_MPPHY21, 346 },
	{ SCP_CLOCK_ID_MDLC_MPPHY31, 347 },
	{ SCP_CLOCK_ID_MDLC_MPPHY02, 348 },
	{ SCP_CLOCK_ID_CLK_S0D6_PERE_MAIN, 1691 },
};

static const struct clk_map_in gen5_clk_map_dt_sdk_4_31[] = {
	{ SCP_CLOCK_ID_MDLC_UFS0, 198 },
	{ SCP_CLOCK_ID_MDLC_UFS1, 199 },
	{ SCP_CLOCK_ID_MDLC_SDHI0, 200 },
	{ SCP_CLOCK_ID_MDLC_XPCS0, 312 },
	{ SCP_CLOCK_ID_MDLC_XPCS1, 313 },
	{ SCP_CLOCK_ID_MDLC_XPCS2, 314 },
	{ SCP_CLOCK_ID_MDLC_XPCS3, 315 },
	{ SCP_CLOCK_ID_MDLC_XPCS4, 316 },
	{ SCP_CLOCK_ID_MDLC_XPCS5, 317 },
	{ SCP_CLOCK_ID_MDLC_XPCS6, 318 },
	{ SCP_CLOCK_ID_MDLC_XPCS7, 319 },
	{ SCP_CLOCK_ID_MDLC_RSW3, 320 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSN, 321 },
	{ SCP_CLOCK_ID_MDLC_RSW3AES, 322 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES0, 323 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES1, 324 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES2, 325 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES3, 326 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES4, 327 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES5, 328 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES6, 329 },
	{ SCP_CLOCK_ID_MDLC_RSW3TSNTES7, 330 },
	{ SCP_CLOCK_ID_MDLC_RSW3MFWD, 331 },
	{ SCP_CLOCK_ID_MDLC_MPPHY01, 340 },
	{ SCP_CLOCK_ID_MDLC_MPPHY11, 341 },
	{ SCP_CLOCK_ID_MDLC_MPPHY21, 342 },
	{ SCP_CLOCK_ID_MDLC_MPPHY31, 343 },
	{ SCP_CLOCK_ID_MDLC_MPPHY02, 344 },
	{ SCP_CLOCK_ID_CLK_S0D6_PERE_MAIN, 1687 },
};

static int gen5_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	struct gen5_clk_priv *priv = dev_get_priv(clk->dev);
	const struct clk_map_in *map;
	unsigned int map_size;
	int i;

	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (priv->basever == GEN5_SCMI_SDK_4_28) {
		map = gen5_clk_map_dt_sdk_4_28;
		map_size = ARRAY_SIZE(gen5_clk_map_dt_sdk_4_28);
	} else if (priv->basever == GEN5_SCMI_SDK_4_31 ||
		   priv->basever == GEN5_SCMI_SDK_4_32) {
		map = gen5_clk_map_dt_sdk_4_31;
		map_size = ARRAY_SIZE(gen5_clk_map_dt_sdk_4_31);
	} else {
		printf("Unsupported SCMI base protocol version %x\n", priv->basever);
		return -EINVAL;
	}

	clk->id = -1;
	for (i = 0; i < map_size; i++) {
		if (map[i].dt_id != args->args[0])
			continue;
		clk->id = map[i].fw_id;
		break;
	}

	if (clk->id == -1)
		return -EINVAL;

	return 0;
}

static const struct clk_ops gen5_clk_ops = {
	.round_rate	= gen5_clk_round_rate,
	.get_rate	= gen5_clk_get_rate,
	.set_rate	= gen5_clk_set_rate,
	.set_parent	= gen5_clk_set_parent,
	.enable		= gen5_clk_enable,
	.disable	= gen5_clk_disable,
	.of_xlate	= gen5_clk_of_xlate,
};

static int gen5_clk_probe(struct udevice *dev)
{
	struct gen5_clk_priv *priv = dev_get_priv(dev);
	struct udevice *agent;
	int ret;

	ret = uclass_get_device(UCLASS_SCMI_AGENT, 0, &agent);
	if (ret)
		return ret;

	if (!agent)
		return -ENODEV;

	priv->basever = scmi_impl_version(agent);

	return uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(scmi_clock),
					   &priv->clk);
}
#else
static int gen5_clk_enable(struct clk *clk)
{
	return 0;
}

static int gen5_clk_disable(struct clk *clk)
{
	return 0;
}

static int gen5_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	clk->id = args->args[0];

	return 0;
}

static const struct clk_ops gen5_clk_ops = {
	.enable		= gen5_clk_enable,
	.disable	= gen5_clk_disable,
	.of_xlate	= gen5_clk_of_xlate,
};
#endif

static const struct udevice_id r8a78000_mdlc_ids[] = {
	{ .compatible = "renesas,r8a78000-cpg", },
	{ }
};

U_BOOT_DRIVER(clk_gen5) = {
	.name		= "clk_gen5",
	.id		= UCLASS_CLK,
	.of_match	= r8a78000_mdlc_ids,
	.priv_auto	= CONFIG_IS_ENABLED(CLK_SCMI, (sizeof(struct gen5_clk_priv)), (0)),
	.ops		= &gen5_clk_ops,
	.probe		= CONFIG_IS_ENABLED(CLK_SCMI, (gen5_clk_probe), (NULL)),
	.flags          = DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};
