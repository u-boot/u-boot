// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2018, The Linux Foundation. All rights reserved.
// Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.

#include <dm.h>
#include <dm/lists.h>
#include <power-domain.h>
#include <asm/io.h>
#include <linux/errno.h>

#include <power-domain-uclass.h>
#include <soc/qcom/cmd-db.h>
#include <soc/qcom/rpmh.h>
#include <dt-bindings/power/qcom-rpmpd.h>
#include <dm/device_compat.h>

#define RPMH_ARC_MAX_LEVELS	16

/**
 * struct rpmhpd - top level RPMh power domain resource data structure
 * @dev:                rpmh power domain controller device
 * @pd:                 generic_pm_domain corresponding to the power domain
 * @parent:             generic_pm_domain corresponding to the parent's power domain
 * @enable_corner:      lowest non-zero corner
 * @level:              An array of level (vlvl) to corner (hlvl) mappings
 *                      derived from cmd-db
 * @level_count:        Number of levels supported by the power domain. max
 *                      being 16 (0 - 15)
 * @enabled:            true if the power domain is enabled
 * @res_name:           Resource name used for cmd-db lookup
 * @addr:               Resource address as looped up using resource name from
 * @skip_retention_level: Indicate that retention level should not be used for the power domain
 */
struct rpmhpd {
	struct udevice	*dev;
	struct power_domain pd;
	struct power_domain *parent;
	unsigned int	enable_corner;
	u32		level[RPMH_ARC_MAX_LEVELS];
	size_t		level_count;
	bool		enabled;
	const char	*res_name;
	u32		addr;
	bool		skip_retention_level;
};

struct rpmhpd_desc {
	struct rpmhpd **rpmhpds;
	size_t num_pds;
};

/* RPMH powerdomains */
static struct rpmhpd mmcx_ao;
static struct rpmhpd mmcx = {
	.res_name = "mmcx.lvl",
};

static struct rpmhpd mmcx_ao = {
	.res_name = "mmcx.lvl",
};

/* SA8775P RPMH power domains */
static struct rpmhpd *sa8775p_rpmhpds[] = {
	[SA8775P_MMCX] = &mmcx,
	[SA8775P_MMCX_AO] = &mmcx_ao,
};

static const struct rpmhpd_desc sa8775p_desc = {
	.rpmhpds = sa8775p_rpmhpds,
	.num_pds = ARRAY_SIZE(sa8775p_rpmhpds),
};

/* stub RPMH power domains mapped for unsupported platforms */
static struct rpmhpd *stub_rpmhpds[] = {};

static const struct rpmhpd_desc stub_desc = {
	.rpmhpds = stub_rpmhpds,
	.num_pds = ARRAY_SIZE(stub_rpmhpds),
};

static const struct udevice_id rpmhpd_match_table[] = {
	{ .compatible = "qcom,sa8775p-rpmhpd", .data = (ulong)&sa8775p_desc },
	{ .compatible = "qcom,qcs615-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,qcs8300-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,qdu1000-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sa8155p-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sa8540p-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sar2130p-rpmhpd", .data = (ulong)&stub_desc},
	{ .compatible = "qcom,sc7180-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sc7280-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sc8180x-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sc8280xp-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sdm670-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sdm845-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sdx55-rpmhpd", .data = (ulong)&stub_desc},
	{ .compatible = "qcom,sdx65-rpmhpd", .data = (ulong)&stub_desc},
	{ .compatible = "qcom,sdx75-rpmhpd", .data = (ulong)&stub_desc},
	{ .compatible = "qcom,sm4450-rpmhpd", .data = (ulong)&stub_desc},
	{ .compatible = "qcom,sm6350-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm7150-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8150-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8250-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8350-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8450-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8550-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8650-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,sm8750-rpmhpd", .data = (ulong)&stub_desc },
	{ .compatible = "qcom,x1e80100-rpmhpd", .data = (ulong)&stub_desc },
	{ }
};

static int rpmhpd_send_corner(struct rpmhpd *pd, int state,
			      unsigned int corner, bool sync)
{
	struct tcs_cmd cmd = {
		.addr = pd->addr,
		.data = corner,
	};

	return rpmh_write(pd->dev, state, &cmd, 1);
}

static int rpmhpd_power_on(struct power_domain *pd)
{
	int ret;
	unsigned int corner;
	struct rpmhpd **rpmhpds;
	const struct rpmhpd_desc *desc;
	struct rpmhpd *curr_rpmhpd;

	desc = (const struct rpmhpd_desc *)dev_get_driver_data(pd->dev);
	if (!desc)
		return -EINVAL;

	rpmhpds = desc->rpmhpds;
	curr_rpmhpd = rpmhpds[pd->id];

	/* Do nothing for undefined power domains */
	if (!curr_rpmhpd) {
		log_warning("Power domain id (%ld) not supported\n",
			    pd->id);
		return 0;
	}

	corner = curr_rpmhpd->enable_corner;

	ret = rpmhpd_send_corner(curr_rpmhpd, RPMH_ACTIVE_ONLY_STATE, corner,
				 false);
	if (!ret)
		curr_rpmhpd->enabled = true;

	return ret;
}

static int rpmhpd_power_off(struct power_domain *pd)
{
	int ret;
	unsigned int corner;
	struct rpmhpd **rpmhpds;
	const struct rpmhpd_desc *desc;
	struct rpmhpd *curr_rpmhpd;

	desc = (const struct rpmhpd_desc *)dev_get_driver_data(pd->dev);
	if (!desc)
		return -EINVAL;

	rpmhpds = desc->rpmhpds;
	curr_rpmhpd = rpmhpds[pd->id];

	/* Do nothing for undefined power domains */
	if (!curr_rpmhpd) {
		log_warning("Power domain id (%ld) not supported\n",
			    pd->id);
		return 0;
	}

	corner = 0;

	ret = rpmhpd_send_corner(curr_rpmhpd, RPMH_ACTIVE_ONLY_STATE, corner,
				 false);
	if (!ret)
		curr_rpmhpd->enabled = false;

	return ret;
}

static int rpmhpd_update_level_mapping(struct rpmhpd *rpmhpd)
{
	int i;
	const u16 *buf;

	buf = cmd_db_read_aux_data(rpmhpd->res_name, &rpmhpd->level_count);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	/* 2 bytes used for each command DB aux data entry */
	rpmhpd->level_count >>= 1;

	if (rpmhpd->level_count > RPMH_ARC_MAX_LEVELS)
		return -EINVAL;

	for (i = 0; i < rpmhpd->level_count; i++) {
		if (rpmhpd->skip_retention_level && buf[i] == RPMH_REGULATOR_LEVEL_RETENTION)
			continue;

		rpmhpd->level[i] = buf[i];

		/* Remember the first corner with non-zero level */
		if (!rpmhpd->level[rpmhpd->enable_corner] && rpmhpd->level[i])
			rpmhpd->enable_corner = i;

		/*
		 * The AUX data may be zero padded. These 0 valued entries at
		 * the end of the map must be ignored.
		 */
		if (i > 0 && rpmhpd->level[i] == 0) {
			rpmhpd->level_count = i;
			break;
		}
		debug("%s: ARC hlvl=%2d --> vlvl=%4u\n", rpmhpd->res_name, i,
		      rpmhpd->level[i]);
	}

	return 0;
}

static int rpmhpd_probe(struct udevice *dev)
{
	int i, ret = 0;
	struct rpmhpd **rpmhpds;
	struct rpmhpd *priv;
	const struct rpmhpd_desc *desc;

	desc = (const struct rpmhpd_desc *)dev_get_driver_data(dev);
	if (!desc)
		return -EINVAL;

	rpmhpds = desc->rpmhpds;

	for (i = 0; i < desc->num_pds; i++) {
		if (!rpmhpds[i])
			continue;

		priv = rpmhpds[i];
		priv->dev = dev;
		priv->addr = cmd_db_read_addr(priv->res_name);
		if (!priv->addr) {
			dev_err(dev, "Could not find RPMh address for resource %s\n",
				priv->res_name);
			return -ENODEV;
		}

		ret = cmd_db_read_slave_id(priv->res_name);
		if (ret != CMD_DB_HW_ARC) {
			dev_err(dev, "RPMh slave ID mismatch\n");
			return -EINVAL;
		}

		ret = rpmhpd_update_level_mapping(priv);
		if (ret)
			return ret;
	}

	return ret;
}

static const struct power_domain_ops qcom_rpmhpd_power_ops = {
	.on = rpmhpd_power_on,
	.off = rpmhpd_power_off,
};

U_BOOT_DRIVER(qcom_rpmhpd_drv) = {
	.name = "qcom_rpmhpd_drv",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = rpmhpd_match_table,
	.probe = rpmhpd_probe,
	.ops = &qcom_rpmhpd_power_ops,
};
