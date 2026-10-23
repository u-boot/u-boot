// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek display pipeline component helpers
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <dt-bindings/memory/mtk-memory-port.h>
#include <errno.h>
#include <mtk_smi.h>

#include "mtk_disp_comp.h"

void mtk_disp_comp_write(struct udevice *dev, u32 offset, u32 val)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);

	writel(val, priv->base + offset);
}

void mtk_disp_comp_update(struct udevice *dev, u32 offset, u32 val, u32 mask)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);
	void __iomem *reg = priv->base + offset;
	u32 tmp;

	tmp = readl(reg);
	tmp = (tmp & ~mask) | (val & mask);
	writel(tmp, reg);
}

int mtk_disp_comp_enable(struct udevice *dev)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);
	int ret;

	/* ungate the feeding SMI larb first, if any */
	if (priv->larb) {
		ret = mtk_smi_larb_enable(priv->larb);
		if (ret)
			return ret;
	}

	ret = clk_enable_bulk(&priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to enable clocks: %d\n", ret);
		if (priv->larb)
			mtk_smi_larb_disable(priv->larb);
		return ret;
	}

	return 0;
}

int mtk_disp_comp_disable(struct udevice *dev)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);
	int ret;

	ret = clk_disable_bulk(&priv->clk_bulk);
	if (ret)
		dev_err(dev, "failed to disable clocks: %d\n", ret);

	if (priv->larb)
		mtk_smi_larb_disable(priv->larb);

	return ret;
}

int mtk_disp_comp_get_larb(struct udevice *dev)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	ofnode iommu, node;
	u32 larb_id, id;
	int i, ret;

	ret = dev_read_phandle_with_args(dev, "iommus", "#iommu-cells", 0, 0,
					 &args);
	if (ret) {
		dev_err(dev, "cannot parse iommus: %d\n", ret);
		return ret;
	}
	iommu = args.node;
	larb_id = MTK_M4U_TO_LARB(args.args[0]);
	priv->larb_port = MTK_M4U_TO_PORT(args.args[0]);

	for (i = 0; ; i++) {
		node = ofnode_parse_phandle(iommu, "mediatek,larbs", i);
		if (!ofnode_valid(node)) {
			dev_err(dev, "no larb %u in iommu\n", larb_id);
			return -ENODEV;
		}
		if (!ofnode_read_u32(node, "mediatek,larb-id", &id) &&
		    id == larb_id)
			break;
	}

	return uclass_get_device_by_ofnode(UCLASS_MEMORY, node, &priv->larb);
}

int mtk_disp_comp_probe(struct udevice *dev)
{
	struct mtk_disp_comp_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_bulk(dev, &priv->clk_bulk);
	if (ret) {
		dev_err(dev, "failed to get clocks: %d\n", ret);
		return ret;
	}

	return 0;
}
