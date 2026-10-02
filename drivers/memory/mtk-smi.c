// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek SMI local arbiter (larb) driver
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <linux/arm-smccc.h>
#include <mtk_smi.h>

/*
 * On the newer larbs the mmu_en bits of SMI_LARB_NONSEC_CON live in the
 * secure world, where "the mmu_en bits of LARB_NONSEC_CON have no effect",
 * as the Linux driver puts it. Which ports of a larb go through the IOMMU
 * is told to the secure monitor instead; U-Boot sets up no page table, so
 * it asks for none, and the DMA masters use physical addresses.
 */
#define MTK_SIP_SMC_CONVENTION		(IS_ENABLED(CONFIG_ARM64) ? \
					 ARM_SMCCC_SMC_64 : ARM_SMCCC_SMC_32)

#define MTK_SIP_KERNEL_IOMMU_CONTROL	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
							   MTK_SIP_SMC_CONVENTION, \
							   ARM_SMCCC_OWNER_SIP, 0x514)
#define IOMMU_ATF_CMD_CONFIG_SMI_LARB	0

/**
 * struct mtk_smi_larb_data - per-SoC larb data
 * @port_sec_ctl: the port mmu_en bits are controlled by the secure world
 */
struct mtk_smi_larb_data {
	bool port_sec_ctl;
};

/**
 * struct mtk_smi_larb_priv - private data of an SMI larb
 * @clk_bulk: clocks of the larb
 * @data: per-SoC larb data, NULL when there is none
 * @enable_count: number of DMA masters that enabled the larb
 *
 * The SMI local arbiters gate a multimedia subsystem's access to DRAM. Linux
 * handles them as part of its SMI/IOMMU framework (drivers/memory/mtk-smi.c);
 * U-Boot has no such framework, so this minimal driver only ungates the larb
 * clocks on behalf of the DMA master sitting behind it.
 */
struct mtk_smi_larb_priv {
	struct clk_bulk clk_bulk;
	const struct mtk_smi_larb_data *data;
	unsigned int enable_count;
};

static const struct mtk_smi_larb_data mt8188_larb_data = {
	.port_sec_ctl = true,
};

static int mtk_smi_larb_bypass_iommu(struct udevice *dev)
{
	struct arm_smccc_res res;
	u32 larb_id;
	int ret;

	ret = dev_read_u32(dev, "mediatek,larb-id", &larb_id);
	if (ret) {
		dev_err(dev, "cannot read mediatek,larb-id: %d\n", ret);
		return ret;
	}

	/* the fourth argument is the mask of the ports using the IOMMU */
	arm_smccc_smc(MTK_SIP_KERNEL_IOMMU_CONTROL, IOMMU_ATF_CMD_CONFIG_SMI_LARB,
		      larb_id, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		dev_err(dev, "cannot take larb %u out of the iommu: %ld\n",
			larb_id, res.a0);
		return -EIO;
	}

	return 0;
}

int mtk_smi_larb_enable(struct udevice *dev)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);
	int ret;

	/* several DMA masters can sit behind the same larb */
	if (priv->enable_count++)
		return 0;

	ret = clk_enable_bulk(&priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		goto err;
	}

	/* as in Linux, only talk to the secure monitor once the larb is on */
	if (priv->data && priv->data->port_sec_ctl) {
		ret = mtk_smi_larb_bypass_iommu(dev);
		if (ret)
			goto err_disable_clks;
	}

	return 0;

err_disable_clks:
	clk_disable_bulk(&priv->clk_bulk);
err:
	priv->enable_count = 0;

	return ret;
}

int mtk_smi_larb_disable(struct udevice *dev)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->enable_count)
		return -EINVAL;

	if (--priv->enable_count)
		return 0;

	ret = clk_disable_bulk(&priv->clk_bulk);
	if (ret)
		dev_err(dev, "failed to disable clocks: %d\n", ret);

	return ret;
}

static int mtk_smi_larb_probe(struct udevice *dev)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_get_bulk(dev, &priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to get clocks: %d\n", ret);
		return ret;
	}

	priv->data = (const struct mtk_smi_larb_data *)dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id mtk_smi_larb_ids[] = {
	{
		.compatible = "mediatek,mt8188-smi-larb",
		.data = (ulong)&mt8188_larb_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_smi_larb) = {
	.name	   = "mtk_smi_larb",
	.id	   = UCLASS_MEMORY,
	.of_match  = mtk_smi_larb_ids,
	.probe	   = mtk_smi_larb_probe,
	.priv_auto = sizeof(struct mtk_smi_larb_priv),
};
