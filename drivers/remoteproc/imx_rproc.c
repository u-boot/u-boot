// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2025 NXP
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <dm/device_compat.h>
#include <linux/arm-smccc.h>
#include <linux/types.h>
#include <regmap.h>
#include <remoteproc.h>
#include <scmi_nxp_protocols.h>
#include <syscon.h>

#include "imx_rproc.h"

#define IMX7D_SRC_SCR			0x0C
#define IMX7D_ENABLE_M4			BIT(3)
#define IMX7D_SW_M4P_RST		BIT(2)
#define IMX7D_SW_M4C_RST		BIT(1)
#define IMX7D_SW_M4C_NON_SCLR_RST	BIT(0)

#define IMX7D_M4_RST_MASK		(IMX7D_ENABLE_M4 | IMX7D_SW_M4P_RST \
					 | IMX7D_SW_M4C_RST \
					 | IMX7D_SW_M4C_NON_SCLR_RST)

#define IMX7D_M4_START			(IMX7D_ENABLE_M4 | IMX7D_SW_M4P_RST \
					 | IMX7D_SW_M4C_RST)
#define IMX7D_M4_STOP			(IMX7D_ENABLE_M4 | IMX7D_SW_M4C_RST | \
					 IMX7D_SW_M4C_NON_SCLR_RST)

#define IMX_RPROC_MEM_MAX		32

#define IMX_SIP_RPROC			0xC2000005
#define IMX_SIP_RPROC_START		0x00
#define IMX_SIP_RPROC_STARTED		0x01
#define IMX_SIP_RPROC_STOP		0x02

/* Must align with System Manager Firmware */
#define IMX95_M7_CPUID			1
#define IMX95_M7_LMID			1

/* Logical Machine API Operation */
#define IMX_RPROC_FLAGS_SM_LMM_OP	BIT(0)
/* CPU API Operation */
#define IMX_RPROC_FLAGS_SM_CPU_OP	BIT(1)
/* Linux has permission to handle the Logical Machine of remote cores */
#define IMX_RPROC_FLAGS_SM_LMM_AVAIL	BIT(2)

struct imx_rproc {
	const struct imx_rproc_dcfg	*dcfg;
	struct regmap *regmap;
	u32				flags;
	/* For System Manager based system */
	struct udevice			*lmm_dev;
	struct udevice			*cpu_dev;
	ulong				reset_vector;
};

/* att flags: lower 16 bits specifying core, higher 16 bits for flags  */
/* M4 own area. Can be mapped at probe */
#define ATT_OWN         BIT(31)
#define ATT_IOMEM       BIT(30)
#define ATT_ECC         BIT(29)

static int imx_rproc_arm_smc_start(struct udevice *dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(IMX_SIP_RPROC, IMX_SIP_RPROC_START, 0, 0, 0, 0, 0, 0, &res);

	return res.a0;
}

static int imx_rproc_mmio_start(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;

	return regmap_update_bits(priv->regmap, dcfg->src_reg, dcfg->src_mask, dcfg->src_start);
}

static int imx_rproc_sm_start(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int ret;

	if (priv->flags & IMX_RPROC_FLAGS_SM_CPU_OP) {
		ret = scmi_imx_cpu_reset_vector_set(priv->cpu_dev, dcfg->cpuid, 0,
						    priv->reset_vector, true, false, false);
		if (ret) {
			dev_err(dev, "Failed to set reset vector cpuid(%u): %d\n",
				dcfg->cpuid, ret);
			return ret;
		}

		return scmi_imx_cpu_start(priv->cpu_dev, dcfg->cpuid, true);
	}

	ret = scmi_imx_lmm_reset_vector_set(priv->lmm_dev, dcfg->lmid, dcfg->cpuid, 0,
					    priv->reset_vector);
	if (ret) {
		dev_err(dev, "Failed to set reset vector lmid(%u), cpuid(%u): %d\n",
			dcfg->lmid, dcfg->cpuid, ret);
		return ret;
	}

	ret = scmi_imx_lmm_power_boot(priv->lmm_dev, dcfg->lmid, true);
	if (ret) {
		dev_err(dev, "Failed to boot lmm(%d): %d\n", ret, dcfg->lmid);
		return ret;
	}

	return 0;
}

static int imx_rproc_start(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int ret;

	if (!dcfg->ops || !dcfg->ops->start)
		return -EOPNOTSUPP;

	ret = dcfg->ops->start(dev);
	if (ret)
		dev_err(dev, "Failed to enable remote core!\n");

	return ret;
}

static int imx_rproc_arm_smc_stop(struct udevice *dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(IMX_SIP_RPROC, IMX_SIP_RPROC_STOP, 0, 0, 0, 0, 0, 0, &res);
	if (res.a1)
		dev_info(dev, "Not in wfi, force stopped\n");

	return res.a0;
}

static int imx_rproc_mmio_stop(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;

	return regmap_update_bits(priv->regmap, dcfg->src_reg, dcfg->src_mask, dcfg->src_stop);
}

static int imx_rproc_sm_stop(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;

	if (priv->flags & IMX_RPROC_FLAGS_SM_CPU_OP)
		return scmi_imx_cpu_start(priv->cpu_dev, dcfg->cpuid, false);

	return scmi_imx_lmm_shutdown(priv->lmm_dev, dcfg->lmid, 0);
}

static int imx_rproc_stop(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int ret;

	if (!dcfg->ops || !dcfg->ops->stop)
		return -EOPNOTSUPP;

	ret = dcfg->ops->stop(dev);
	if (ret)
		dev_err(dev, "Failed to stop remote core\n");

	return ret;
}

static int imx_rproc_arm_smc_is_running(struct udevice *dev)
{
	struct arm_smccc_res res;

	arm_smccc_smc(IMX_SIP_RPROC, IMX_SIP_RPROC_STARTED, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0)
		return 0;

	return 1;
}

static int imx_rproc_mmio_is_running(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int ret;
	u32 val;

	ret = regmap_read(priv->regmap, dcfg->src_reg, &val);
	if (ret) {
		dev_err(dev, "Failed to read src\n");
		return ret;
	}

	if ((val & dcfg->src_mask) != dcfg->src_stop)
		return 0;

	return 1;
}

static int imx_rproc_sm_is_running(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int ret;
	bool started = false;

	ret = scmi_imx_cpu_started(priv->cpu_dev, dcfg->cpuid, &started);
	if (ret || !started)
		return 1;

	return 0;
}

static int imx_rproc_is_running(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;

	if (!dcfg->ops || !dcfg->ops->is_running)
		return 0;

	return dcfg->ops->is_running(dev);
}

static int imx_rproc_init(struct udevice *dev)
{
	return 0;
}

static int imx_rproc_da_to_sys(struct udevice *dev, u64 da, size_t len, u64 *sys, bool *is_iomem)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int i;

	/* parse address translation table */
	for (i = 0; i < dcfg->att_size; i++) {
		const struct imx_rproc_att *att = &dcfg->att[i];

		if (da >= att->da && da + len < att->da + att->size) {
			unsigned int offset = da - att->da;

			*sys = att->sa + offset;

			if (is_iomem)
				*is_iomem = att->flags & ATT_IOMEM;

			return 0;
		}
	}

	dev_err(dev, "Translation failed: da = 0x%llx len = 0x%zx\n", da, len);

	return -ENOENT;
}

static void *imx_rproc_device_to_virt(struct udevice *dev, ulong da, ulong size, bool *is_iomem)
{
	u64 sys;

	if (imx_rproc_da_to_sys(dev, da, size, &sys, is_iomem))
		return NULL;

	dev_dbg(dev, "da = 0x%lx len = 0x%lx sys = 0x%llx\n", da, size, sys);

	return phys_to_virt(sys);
}

static int imx_rproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	const struct imx_rproc_dcfg *dcfg = priv->dcfg;
	int i;

	if (IS_ENABLED(CONFIG_IMX_SM_LMM)) {
		if (!(priv->flags & (IMX_RPROC_FLAGS_SM_LMM_AVAIL | IMX_RPROC_FLAGS_SM_CPU_OP)))
			return -EACCES;
	}

	/* Only used for SM based System */
	priv->reset_vector = rproc_elf_get_boot_addr(dev, addr) & GENMASK(31, 16);

	/*
	 * Before loading elf, need do ECC initialization by clearing the memory
	 * region, if ATT_ECC is set.
	 */
	for (i = 0; i < dcfg->att_size; i++) {
		const struct imx_rproc_att *att = &dcfg->att[i];

		if (!(att->flags & ATT_ECC))
			continue;

		if (att->flags & ATT_IOMEM)
			memset_io((void __iomem *)(long)att->sa, 0, att->size);
		else
			memset((void *)(long)att->sa, 0, att->size);
	}

	return rproc_elf_load_image(dev, addr, size);
}

static const struct dm_rproc_ops imx_rproc_ops = {
	.init = imx_rproc_init,
	.start = imx_rproc_start,
	.stop = imx_rproc_stop,
	.load = imx_rproc_load,
	.device_to_virt = imx_rproc_device_to_virt,
	.is_running = imx_rproc_is_running,
};

static int imx_rproc_probe(struct udevice *dev)
{
	struct imx_rproc *priv = dev_get_priv(dev);
	struct imx_rproc_dcfg *dcfg = (struct imx_rproc_dcfg *)dev_get_driver_data(dev);
	struct scmi_imx_lmm_info info;
	ofnode node;
	int ret;

	node = dev_ofnode(dev);

	priv->dcfg = dcfg;

	if (dcfg->method == IMX_RPROC_MMIO) {
		priv->regmap = syscon_regmap_lookup_by_phandle(dev, "syscon");
		if (IS_ERR(priv->regmap)) {
			dev_err(dev, "No syscon: %ld\n", PTR_ERR(priv->regmap));
			return PTR_ERR(priv->regmap);
		}
		return 0;
	} else {
		if (IS_ENABLED(CONFIG_IMX_SM_LMM)) {
			struct udevice *lmm_dev;

			ret = uclass_get_device_by_name(UCLASS_SCMI_BASE, "protocol@80", &lmm_dev);
			if (ret) {
				dev_err(dev, "Failed to get SM LMM protocol dev\n");
				return ret;
			}

			priv->lmm_dev = lmm_dev;

			ret = scmi_imx_lmm_info(lmm_dev, LMM_ID_DISCOVER, &info);
			if (ret) {
				dev_err(dev, "Failed to get lmm info\n");
				return ret;
			}

			if (dcfg->lmid != info.lmid) {
				ret = scmi_imx_lmm_power_boot(lmm_dev, dcfg->lmid, false);
				if (ret == -EACCES) {
					dev_err(dev, "Remoteproc not under U-boot control: only support detect running\n");
				} else if (ret) {
					dev_err(dev, "power on lmm fail:%d\n", ret);
					return ret;
				} else {
					priv->flags |= IMX_RPROC_FLAGS_SM_LMM_AVAIL;
				}
			} else {
				priv->flags |= IMX_RPROC_FLAGS_SM_CPU_OP;
			}
		}

		if (IS_ENABLED(CONFIG_IMX_SM_CPU)) {
			struct udevice *cpu_dev;

			ret = uclass_get_device_by_name(UCLASS_SCMI_BASE, "protocol@82", &cpu_dev);
			if (ret) {
				dev_err(dev, "Failed to get SM LMM protocol dev\n");
				return ret;
			}

			priv->cpu_dev = cpu_dev;
		}
	}

	return 0;
}

static const struct imx_rproc_att imx_rproc_att_imx8mn[] = {
	/* dev addr , sys addr  , size	    , flags */
	/* ITCM   */
	{ 0x00000000, 0x007E0000, 0x00020000, ATT_OWN | ATT_IOMEM },
	/* OCRAM_S */
	{ 0x00180000, 0x00180000, 0x00009000, 0 },
	/* OCRAM */
	{ 0x00900000, 0x00900000, 0x00020000, 0 },
	/* OCRAM */
	{ 0x00920000, 0x00920000, 0x00020000, 0 },
	/* OCRAM */
	{ 0x00940000, 0x00940000, 0x00050000, 0 },
	/* QSPI Code - alias */
	{ 0x08000000, 0x08000000, 0x08000000, 0 },
	/* DDR (Code) - alias */
	{ 0x10000000, 0x40000000, 0x0FFE0000, 0 },
	/* DTCM */
	{ 0x20000000, 0x00800000, 0x00020000, ATT_OWN | ATT_IOMEM },
	/* OCRAM_S - alias */
	{ 0x20180000, 0x00180000, 0x00008000, ATT_OWN },
	/* OCRAM */
	{ 0x20200000, 0x00900000, 0x00020000, ATT_OWN },
	/* OCRAM */
	{ 0x20220000, 0x00920000, 0x00020000, ATT_OWN },
	/* OCRAM */
	{ 0x20240000, 0x00940000, 0x00040000, ATT_OWN },
	/* DDR (Data) */
	{ 0x40000000, 0x40000000, 0x80000000, 0 },
};

static const struct imx_rproc_plat_ops imx_rproc_ops_arm_smc = {
	.start		= imx_rproc_arm_smc_start,
	.stop		= imx_rproc_arm_smc_stop,
	.is_running	= imx_rproc_arm_smc_is_running,
};

static const struct imx_rproc_dcfg imx_rproc_cfg_imx8mn = {
	.att		= imx_rproc_att_imx8mn,
	.att_size	= ARRAY_SIZE(imx_rproc_att_imx8mn),
	.method		= IMX_RPROC_SMC,
	.ops		= &imx_rproc_ops_arm_smc,
};

static const struct imx_rproc_att imx_rproc_att_imx8mq[] = {
	/* dev addr , sys addr  , size	    , flags */
	/* TCML - alias */
	{ 0x00000000, 0x007e0000, 0x00020000, ATT_IOMEM},
	/* OCRAM_S */
	{ 0x00180000, 0x00180000, 0x00008000, 0 },
	/* OCRAM */
	{ 0x00900000, 0x00900000, 0x00020000, 0 },
	/* OCRAM */
	{ 0x00920000, 0x00920000, 0x00020000, 0 },
	/* QSPI Code - alias */
	{ 0x08000000, 0x08000000, 0x08000000, 0 },
	/* DDR (Code) - alias */
	{ 0x10000000, 0x40000000, 0x0FFE0000, 0 },
	/* TCML/U */
	{ 0x1FFE0000, 0x007E0000, 0x00040000, ATT_OWN  | ATT_IOMEM},
	/* OCRAM_S */
	{ 0x20180000, 0x00180000, 0x00008000, ATT_OWN },
	/* OCRAM */
	{ 0x20200000, 0x00900000, 0x00020000, ATT_OWN },
	/* OCRAM */
	{ 0x20220000, 0x00920000, 0x00020000, ATT_OWN },
	/* DDR (Data) */
	{ 0x40000000, 0x40000000, 0x80000000, 0 },
};

static const struct imx_rproc_plat_ops imx_rproc_ops_mmio = {
	.start		= imx_rproc_mmio_start,
	.stop		= imx_rproc_mmio_stop,
	.is_running	= imx_rproc_mmio_is_running,
};

static const struct imx_rproc_dcfg imx_rproc_cfg_imx8mq = {
	.src_reg	= IMX7D_SRC_SCR,
	.src_mask	= IMX7D_M4_RST_MASK,
	.src_start	= IMX7D_M4_START,
	.src_stop	= IMX7D_M4_STOP,
	.att		= imx_rproc_att_imx8mq,
	.att_size	= ARRAY_SIZE(imx_rproc_att_imx8mq),
	.method		= IMX_RPROC_MMIO,
	.ops		= &imx_rproc_ops_mmio,
};

static const struct imx_rproc_att imx_rproc_att_imx93[] = {
	/* dev addr , sys addr  , size	    , flags */
	/* TCM CODE NON-SECURE */
	{ 0x0FFC0000, 0x201C0000, 0x00040000, ATT_OWN | ATT_IOMEM },

	/* TCM CODE SECURE */
	{ 0x1FFC0000, 0x201C0000, 0x00040000, ATT_OWN | ATT_IOMEM },

	/* TCM SYS NON-SECURE*/
	{ 0x20000000, 0x20200000, 0x00040000, ATT_OWN | ATT_IOMEM },

	/* TCM SYS SECURE*/
	{ 0x30000000, 0x20200000, 0x00040000, ATT_OWN | ATT_IOMEM },

	/* DDR */
	{ 0x80000000, 0x80000000, 0x10000000, 0 },
	{ 0x90000000, 0x80000000, 0x10000000, 0 },

	{ 0xC0000000, 0xC0000000, 0x10000000, 0 },
	{ 0xD0000000, 0xC0000000, 0x10000000, 0 },
};

static const struct imx_rproc_dcfg imx_rproc_cfg_imx93 = {
	.att		= imx_rproc_att_imx93,
	.att_size	= ARRAY_SIZE(imx_rproc_att_imx93),
	.method		= IMX_RPROC_SMC,
	.ops		= &imx_rproc_ops_arm_smc,
};

static const struct imx_rproc_att imx_rproc_att_imx95_m7[] = {
	/* dev addr , sys addr  , size	    , flags */
	/* TCM CODE NON-SECURE */
	{ 0x00000000, 0x203C0000, 0x00040000, ATT_OWN | ATT_IOMEM | ATT_ECC },

	/* TCM SYS NON-SECURE*/
	{ 0x20000000, 0x20400000, 0x00040000, ATT_OWN | ATT_IOMEM | ATT_ECC },

	/* DDR */
	{ 0x80000000, 0x80000000, 0x50000000, 0 },
};

static const struct imx_rproc_plat_ops imx_rproc_ops_sm = {
	.start		= imx_rproc_sm_start,
	.stop		= imx_rproc_sm_stop,
	.is_running	= imx_rproc_sm_is_running,
};

static const struct imx_rproc_dcfg imx_rproc_cfg_imx95_m7 = {
	.att		= imx_rproc_att_imx95_m7,
	.att_size	= ARRAY_SIZE(imx_rproc_att_imx95_m7),
	.ops		= &imx_rproc_ops_sm,
	.cpuid		= IMX95_M7_CPUID,
	.lmid		= IMX95_M7_LMID,
};

static const struct udevice_id imx_rproc_ids[] = {
	{ .compatible = "fsl,imx8mm-cm4", .data = (ulong)&imx_rproc_cfg_imx8mq },
	{ .compatible = "fsl,imx8mn-cm7", .data = (ulong)&imx_rproc_cfg_imx8mn, },
	{ .compatible = "fsl,imx8mp-cm7", .data = (ulong)&imx_rproc_cfg_imx8mn, },
	{ .compatible = "fsl,imx8mq-cm4", .data = (ulong)&imx_rproc_cfg_imx8mq },
	{ .compatible = "fsl,imx93-cm33", .data = (ulong)&imx_rproc_cfg_imx93 },
	{ .compatible = "fsl,imx95-cm7", .data = (ulong)&imx_rproc_cfg_imx95_m7 },
	{}
};

U_BOOT_DRIVER(imx_rproc) = {
	.name = "imx_rproc",
	.of_match = imx_rproc_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &imx_rproc_ops,
	.probe = imx_rproc_probe,
	.priv_auto = sizeof(struct imx_rproc),
};
