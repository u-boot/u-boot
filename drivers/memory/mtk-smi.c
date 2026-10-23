// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek SMI local arbiter (larb) and smi_common driver
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <linux/arm-smccc.h>
#include <linux/bitops.h>
#include <mtk_smi.h>

#define SMI_LARB_NONSEC_CON(port)	(0x380 + 4 * (port))
#define MMU_EN					BIT(0)

/*
 * On the newer larbs the mmu_en bits above live in the secure world, where
 * "the mmu_en bits of LARB_NONSEC_CON have no effect", as the Linux driver
 * puts it. Which ports of a larb go through the IOMMU is told to the secure
 * monitor instead; U-Boot sets up no page table, so it asks for none.
 */
#define MTK_SIP_SMC_CONVENTION		(IS_ENABLED(CONFIG_ARM64) ? \
					 ARM_SMCCC_SMC_64 : ARM_SMCCC_SMC_32)

#define MTK_SIP_KERNEL_IOMMU_CONTROL	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
							   MTK_SIP_SMC_CONVENTION, \
							   ARM_SMCCC_OWNER_SIP, 0x514)
#define IOMMU_ATF_CMD_CONFIG_SMI_LARB	0

/*
 * The SMI local arbiters gate a multimedia subsystem's access to DRAM. Linux
 * handles them as part of its SMI/IOMMU framework (its mtk-smi driver);
 * U-Boot has no such framework, so this minimal driver only ungates the
 * clocks of a larb and of the smi_common block behind it, on behalf of the
 * DMA master sitting in front of it.
 */

/**
 * struct mtk_smi_common_priv - private data of an smi_common block
 * @clk_bulk: clocks of the smi_common block
 * @enable_count: number of larbs that enabled the smi_common block
 */
struct mtk_smi_common_priv {
	struct clk_bulk clk_bulk;
	unsigned int enable_count;
};

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
 * @smi_common: smi_common device behind the larb, if handled by this driver
 * @base: larb register window
 * @data: per-SoC larb data, NULL when there is none
 * @enable_count: number of DMA masters that enabled the larb
 */
struct mtk_smi_larb_priv {
	struct clk_bulk clk_bulk;
	struct udevice *smi_common;
	void __iomem *base;
	const struct mtk_smi_larb_data *data;
	unsigned int enable_count;
};

/* the MT8189 and MT8366 larbs are the MT8188 IP */
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

static int mtk_smi_common_enable(struct udevice *dev)
{
	struct mtk_smi_common_priv *priv = dev_get_priv(dev);
	int ret;

	/* several larbs sit behind one smi_common, only the first ungates it */
	if (priv->enable_count++)
		return 0;

	ret = clk_enable_bulk(&priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		priv->enable_count = 0;
	}

	return ret;
}

static int mtk_smi_common_disable(struct udevice *dev)
{
	struct mtk_smi_common_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv->enable_count || --priv->enable_count)
		return 0;

	ret = clk_disable_bulk(&priv->clk_bulk);
	if (ret)
		dev_err(dev, "failed to disable clocks: %d\n", ret);

	return ret;
}

static int mtk_smi_common_probe(struct udevice *dev)
{
	struct mtk_smi_common_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_get_bulk(dev, &priv->clk_bulk);
	if (ret)
		dev_err(dev, "failed to get clocks: %d\n", ret);

	return ret;
}

static const struct udevice_id mtk_smi_common_ids[] = {
	{ .compatible = "mediatek,mt8189-smi-common" },
	{ .compatible = "mediatek,mt8365-smi-common" },
	{ .compatible = "mediatek,mt8366-smi-common" },
	{ }
};

U_BOOT_DRIVER(mtk_smi_common) = {
	.name	   = "mtk_smi_common",
	.id	   = UCLASS_MEMORY,
	.of_match  = mtk_smi_common_ids,
	.probe	   = mtk_smi_common_probe,
	.priv_auto = sizeof(struct mtk_smi_common_priv),
};

int mtk_smi_larb_enable(struct udevice *dev)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);
	int ret;

	/* one larb feeds several DMA masters, only the first ungates it */
	if (priv->enable_count++)
		return 0;

	if (priv->smi_common) {
		ret = mtk_smi_common_enable(priv->smi_common);
		if (ret)
			goto err;
	}

	ret = clk_enable_bulk(&priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		goto err_disable_common;
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
err_disable_common:
	if (priv->smi_common)
		mtk_smi_common_disable(priv->smi_common);
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

	if (priv->smi_common)
		mtk_smi_common_disable(priv->smi_common);

	return ret;
}

void mtk_smi_larb_port_set_pa(struct udevice *dev, unsigned int port)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);

	clrbits_le32(priv->base + SMI_LARB_NONSEC_CON(port), MMU_EN);
}

static int mtk_smi_larb_probe(struct udevice *dev)
{
	struct mtk_smi_larb_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_bulk(dev, &priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to get clocks: %d\n", ret);
		return ret;
	}

	/*
	 * Not every smi_common block is handled by this driver: where it
	 * isn't, its clocks are already ungated by the time the DMA masters
	 * run, so having no device for it is not an error.
	 */
	ret = uclass_get_device_by_phandle(UCLASS_MEMORY, dev, "mediatek,smi",
					   &priv->smi_common);
	if (ret == -ENODEV || ret == -ENOENT) {
		dev_dbg(dev, "no smi_common device: %d\n", ret);
		priv->smi_common = NULL;
	} else if (ret) {
		dev_err(dev, "failed to get smi_common device: %d\n", ret);
		return ret;
	}

	priv->data = (const struct mtk_smi_larb_data *)dev_get_driver_data(dev);

	return 0;
}

/*
 * The MT8365 larbs also carry the generic mediatek,mt8186-smi-larb
 * compatible, which is the one matched here. The MT8366 ones only have
 * their own, as that SoC is not upstream yet.
 */
static const struct udevice_id mtk_smi_larb_ids[] = {
	{
		.compatible = "mediatek,mt8186-smi-larb",
	},
	{
		.compatible = "mediatek,mt8188-smi-larb",
		.data = (ulong)&mt8188_larb_data,
	},
	{
		.compatible = "mediatek,mt8189-smi-larb",
		.data = (ulong)&mt8188_larb_data,
	},
	{
		.compatible = "mediatek,mt8366-smi-larb",
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
