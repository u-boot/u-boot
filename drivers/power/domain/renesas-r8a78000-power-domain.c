// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas R-Car Gen5 MDLC driver
 *
 * Copyright (C) 2026 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/bitfield.h>
#include <power-domain-uclass.h>
#include <reset-uclass.h>

#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>

#include <dt-bindings/power/r8a78000-power-scmi.h>
#include <dt-bindings/reset/r8a78000-reset-scmi.h>

#define PKC_PROT_LOCK			0xa5a5a500
#define PKC_PROT_UNLOCK			0xa5a5a501

#define MDLC_MSRESS_STANDBY		0
#define MDLC_MSRESS_RESET		1
#define MDLC_MSRESS_STOP		2
#define MDLC_MSRESS_RUN			3

#define MDLC_MSRES00			0x900
#define MDLC_MSRESS00			0x960
#define MDLC_PKCPROT1			0xcf4

struct gen5_mdlc_priv {
#if IS_ENABLED(CONFIG_SCMI_POWER_DOMAIN)
	struct udevice	*pd;
#endif
#if IS_ENABLED(CONFIG_RESET_SCMI)
	struct udevice	*rst;
#endif
#if IS_ENABLED(CONFIG_SCMI_POWER_DOMAIN) || IS_ENABLED(CONFIG_RESET_SCMI)
	u32		basever;
#endif
#if !IS_ENABLED(CONFIG_SCMI_POWER_DOMAIN) && !IS_ENABLED(CONFIG_RESET_SCMI)
	void __iomem	*base;
#endif
};

static int gen5_pd_of_xlate(struct power_domain *power_domain,
			    struct ofnode_phandle_args *args)
{
	/* Perform direct remap until the bindings stabilize. */
	power_domain->id = args->args[0];

	return 0;
}

#if IS_ENABLED(CONFIG_SCMI_POWER_DOMAIN)
static int gen5_pd_on(struct power_domain *power_domain)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(power_domain->dev->parent);
	struct power_domain_ops *ops = (struct power_domain_ops *)priv->pd->driver->ops;
	struct power_domain scmi = {
		.dev = priv->pd,
		.id = power_domain->id
	};

	return ops->on(&scmi);
}

static int gen5_pd_off(struct power_domain *power_domain)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(power_domain->dev->parent);
	struct power_domain_ops *ops = (struct power_domain_ops *)priv->pd->driver->ops;
	struct power_domain scmi = {
		.dev = priv->pd,
		.id = power_domain->id
	};

	return ops->off(&scmi);
}

static const struct power_domain_ops pd_gen5_ops = {
	.on = gen5_pd_on,
	.off = gen5_pd_off,
	.of_xlate = gen5_pd_of_xlate,
};

static int gen5_pd_probe(struct udevice *dev)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(dev->parent);
	struct udevice *agent;
	int ret;

	if (!priv->basever) {
		ret = uclass_get_device(UCLASS_SCMI_AGENT, 0, &agent);
		if (ret)
			return ret;

		if (!agent)
			return -ENODEV;

		priv->basever = scmi_impl_version(agent);
	}

	return uclass_get_device_by_driver(UCLASS_POWER_DOMAIN,
					   DM_DRIVER_GET(scmi_power_domain),
					   &priv->pd);
}

U_BOOT_DRIVER(pd_gen5) = {
	.name		= "pd_gen5",
	.id		= UCLASS_POWER_DOMAIN,
	.ops		= &pd_gen5_ops,
	.probe		= gen5_pd_probe,
	.flags		= DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};
#else
static const struct power_domain_ops pd_gen5_ops = {
	.of_xlate = gen5_pd_of_xlate,
};

U_BOOT_DRIVER(pd_gen5) = {
	.name		= "pd_gen5",
	.id		= UCLASS_POWER_DOMAIN,
	.ops		= &pd_gen5_ops,
	.flags		= DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};
#endif

#if IS_ENABLED(CONFIG_RESET_SCMI)
static int gen5_reset_assert(struct reset_ctl *reset_ctl)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	struct reset_ops *ops = (struct reset_ops *)priv->rst->driver->ops;
	struct reset_ctl scmi = {
		.dev = priv->rst,
		.id = reset_ctl->id
	};

	return ops->rst_assert(&scmi);
}

static int gen5_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	struct reset_ops *ops = (struct reset_ops *)priv->rst->driver->ops;
	struct reset_ctl scmi = {
		.dev = priv->rst,
		.id = reset_ctl->id
	};

	return ops->rst_deassert(&scmi);
}

struct rst_map_in {
	u16 dt_id;	/* DT binding clock ID */
	u16 fw_id;	/* SCMI firmware clock ID */
};

#define GEN5_SCMI_SDK_4_28		0x010a0000
#define GEN5_SCMI_SDK_4_29		0x010b0000
#define GEN5_SCMI_SDK_4_30		0x010c0000
#define GEN5_SCMI_SDK_4_31		0x010d0000
#define GEN5_SCMI_SDK_4_32		0x010e0000

static const struct rst_map_in gen5_rst_map_dt_sdk_4_28[] = {
	{ SCP_RESET_DOMAIN_ID_UFS0, 202 },
	{ SCP_RESET_DOMAIN_ID_UFS1, 203 },
	{ SCP_RESET_DOMAIN_ID_XPCS0, 316 },
	{ SCP_RESET_DOMAIN_ID_XPCS1, 317 },
	{ SCP_RESET_DOMAIN_ID_XPCS2, 318 },
	{ SCP_RESET_DOMAIN_ID_XPCS3, 319 },
	{ SCP_RESET_DOMAIN_ID_XPCS4, 320 },
	{ SCP_RESET_DOMAIN_ID_XPCS5, 321 },
	{ SCP_RESET_DOMAIN_ID_XPCS6, 322 },
	{ SCP_RESET_DOMAIN_ID_XPCS7, 323 },
	{ SCP_RESET_DOMAIN_ID_MPPHY01, 344 },
	{ SCP_RESET_DOMAIN_ID_MPPHY11, 345 },
	{ SCP_RESET_DOMAIN_ID_MPPHY21, 346 },
	{ SCP_RESET_DOMAIN_ID_MPPHY31, 347 },
	{ SCP_RESET_DOMAIN_ID_MPPHY02, 348 },
};

static const struct rst_map_in gen5_rst_map_dt_sdk_4_31[] = {
	{ SCP_RESET_DOMAIN_ID_UFS0, 198 },
	{ SCP_RESET_DOMAIN_ID_UFS1, 199 },
	{ SCP_RESET_DOMAIN_ID_XPCS0, 312 },
	{ SCP_RESET_DOMAIN_ID_XPCS1, 313 },
	{ SCP_RESET_DOMAIN_ID_XPCS2, 314 },
	{ SCP_RESET_DOMAIN_ID_XPCS3, 315 },
	{ SCP_RESET_DOMAIN_ID_XPCS4, 316 },
	{ SCP_RESET_DOMAIN_ID_XPCS5, 317 },
	{ SCP_RESET_DOMAIN_ID_XPCS6, 318 },
	{ SCP_RESET_DOMAIN_ID_XPCS7, 319 },
	{ SCP_RESET_DOMAIN_ID_MPPHY01, 340 },
	{ SCP_RESET_DOMAIN_ID_MPPHY11, 341 },
	{ SCP_RESET_DOMAIN_ID_MPPHY21, 342 },
	{ SCP_RESET_DOMAIN_ID_MPPHY31, 343 },
	{ SCP_RESET_DOMAIN_ID_MPPHY02, 344 },
};

static int gen5_reset_of_xlate(struct reset_ctl *reset_ctl,
			       struct ofnode_phandle_args *args)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	const struct rst_map_in *map;
	unsigned int map_size;
	int i;

	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (priv->basever == GEN5_SCMI_SDK_4_28) {
		map = gen5_rst_map_dt_sdk_4_28;
		map_size = ARRAY_SIZE(gen5_rst_map_dt_sdk_4_28);
	} else if (priv->basever == GEN5_SCMI_SDK_4_31 ||
		   priv->basever == GEN5_SCMI_SDK_4_32) {
		map = gen5_rst_map_dt_sdk_4_31;
		map_size = ARRAY_SIZE(gen5_rst_map_dt_sdk_4_31);
	} else {
		printf("Unsupported SCMI base protocol version %x\n", priv->basever);
		return -EINVAL;
	}

	reset_ctl->id = -1;
	for (i = 0; i < map_size; i++) {
		if (map[i].dt_id != args->args[0])
			continue;
		reset_ctl->id = map[i].fw_id;
		break;
	}

	return 0;
}

static const struct reset_ops rst_gen5_ops = {
	.rst_assert = gen5_reset_assert,
	.rst_deassert = gen5_reset_deassert,
	.of_xlate = gen5_reset_of_xlate,
};

static int gen5_rst_probe(struct udevice *dev)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(dev->parent);
	struct udevice *agent;
	int ret = 0;

	if (!priv->basever) {
		ret = uclass_get_device(UCLASS_SCMI_AGENT, 0, &agent);
		if (ret)
			return ret;

		if (!agent)
			return -ENODEV;

		priv->basever = scmi_impl_version(agent);
	}

	return uclass_get_device_by_driver(UCLASS_RESET,
					   DM_DRIVER_GET(scmi_reset_domain),
					   &priv->rst);
}
#else
static int mdlc_wait_for_reset(struct reset_ctl *reset_ctl)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	const u32 offset = (reset_ctl->id / 16) * 4;
	void __iomem *res = priv->base + MDLC_MSRES00 + offset;
	void __iomem *stat = priv->base + MDLC_MSRESS00 + offset;
	u32 val;
	int ret;

	/* Wait 100ms for reset controller to synchronize. */
	ret = readl_poll_timeout(res, val, val == readl(stat), 100000);
	if (ret < 0)
		dev_err(reset_ctl->dev, "Reset controller out of sync!\n");

	return ret;
}

static void mdlc_rmw_msres(struct reset_ctl *reset_ctl, const int val)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	const u32 offset = (reset_ctl->id / 16) * 4;
	const u32 mask = 3 << ((reset_ctl->id % 16) * 2);
	void __iomem *prot = priv->base + MDLC_PKCPROT1;
	void __iomem *res = priv->base + MDLC_MSRES00 + offset;
	u32 reg;

	reg = readl(res);
	reg &= ~mask;
	reg |= field_prep(mask, val);

	writel(PKC_PROT_UNLOCK, prot);
	writel(reg, res);
	writel(PKC_PROT_LOCK, prot);
}

static int gen5_reset_toggle(struct reset_ctl *reset_ctl, const u8 step1,
			     const u8 step2, const u8 step3, const u8 step4)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(reset_ctl->dev->parent);
	const u32 offset = (reset_ctl->id / 16) * 4;
	const u32 mask = 3 << ((reset_ctl->id % 16) * 2);
	void __iomem *stat = priv->base + MDLC_MSRESS00 + offset;
	u32 status;
	int ret;

	ret = mdlc_wait_for_reset(reset_ctl);
	if (ret)
		return ret;

	status = field_get(mask, readl(stat));
	if (status == step1) {
		mdlc_rmw_msres(reset_ctl, step2);
		ret = mdlc_wait_for_reset(reset_ctl);
		if (ret)
			return ret;
		status = field_get(mask, readl(stat));
	}

	if (status == step2 || status == step3) {
		mdlc_rmw_msres(reset_ctl, step4);
		ret = mdlc_wait_for_reset(reset_ctl);
		if (ret)
			return ret;
	}

	return 0;
}

static int gen5_reset_assert(struct reset_ctl *reset_ctl)
{
	return gen5_reset_toggle(reset_ctl,
				 MDLC_MSRESS_STOP, MDLC_MSRESS_STANDBY,
				 MDLC_MSRESS_RUN, MDLC_MSRESS_RESET);
}

static int gen5_reset_deassert(struct reset_ctl *reset_ctl)
{
	return gen5_reset_toggle(reset_ctl,
				 MDLC_MSRESS_STANDBY, MDLC_MSRESS_RESET,
				 MDLC_MSRESS_STOP, MDLC_MSRESS_RUN);
}

static int gen5_reset_of_xlate(struct reset_ctl *reset_ctl,
			       struct ofnode_phandle_args *args)
{
	/* Perform direct remap until the bindings stabilize. */
	reset_ctl->id = args->args[0];

	return 0;
}

static const struct reset_ops rst_gen5_ops = {
	.rst_assert = gen5_reset_assert,
	.rst_deassert = gen5_reset_deassert,
	.of_xlate = gen5_reset_of_xlate,
};

static int gen5_rst_probe(struct udevice *dev)
{
	struct gen5_mdlc_priv *priv = dev_get_priv(dev->parent);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}
#endif

U_BOOT_DRIVER(rst_gen5) = {
	.name		= "rst_gen5",
	.id		= UCLASS_RESET,
	.ops		= &rst_gen5_ops,
	.probe		= gen5_rst_probe,
	.flags		= DM_FLAG_OS_PREPARE | DM_FLAG_VITAL,
};

int gen5_mdlc_bind(struct udevice *parent)
{
	struct udevice *pdev, *rdev;
	struct driver *pdrv, *rdrv;
	int ret;

	pdrv = lists_driver_lookup_name("pd_gen5");
	if (!pdrv)
		return -ENOENT;

	rdrv = lists_driver_lookup_name("rst_gen5");
	if (!rdrv)
		return -ENOENT;

	ret = device_bind_with_driver_data(parent, pdrv, "pd_gen5", 0,
					   dev_ofnode(parent), &pdev);
	if (ret)
		return ret;

	ret = device_bind_with_driver_data(parent, rdrv, "rst_gen5", (ulong)pdev,
					   dev_ofnode(parent), &rdev);
	if (ret)
		device_unbind(pdev);

	return ret;
}

static const struct udevice_id r8a78000_mdlc_ids[] = {
	{ .compatible = "renesas,r8a78000-mdlc", },
	{ }
};

U_BOOT_DRIVER(mdlc_gen5) = {
	.name		= "mdlc_gen5",
	.id		= UCLASS_NOP,
	.of_match	= r8a78000_mdlc_ids,
	.bind		= gen5_mdlc_bind,
	.priv_auto	= sizeof(struct gen5_mdlc_priv),
};
