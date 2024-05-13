// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Edgeble AI Technologies Pvt. Ltd.
 */

#include <dm.h>
#include <video.h>
#include <asm/io.h>
#include "rk_vop.h"

DECLARE_GLOBAL_DATA_PTR;

static void rk3328_set_pin_polarity(struct udevice *dev,
				    enum vop_modes mode, u32 polarity)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;

	switch (mode) {
	case VOP_MODE_HDMI:
		clrsetbits_le32(&regs->dsp_ctrl1,
				M_RK3399_DSP_HDMI_POL,
				V_RK3399_DSP_HDMI_POL(polarity));
		break;
	default:
		debug("%s: unsupported output mode %x\n", __func__, mode);
	}
}

static int rk3328_vop_probe(struct udevice *dev)
{
	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	return rk_vop_probe(dev);
}

static int rk3328_vop_remove(struct udevice *dev)
{
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct rk3288_vop *regs = priv->regs;
	struct rk3288_vop *win_regs = priv->regs + priv->win_offset;

	/* FIXME: Explicit disabling of WIN0 is needed to avoid iommu
	 * page-fault in Linux, better handling of iommu-address in
	 * Linux might drop this.
	 */
	clrbits_le32(&win_regs->win0_ctrl0, M_WIN0_EN);
	writel(0x01, &regs->reg_cfg_done);

	return 0;
}

struct rkvop_driverdata rk3328_driverdata = {
	.dsp_offset = 0x490,
	.win_offset = 0xd0,
	.features = VOP_FEATURE_OUTPUT_10BIT,
	.set_pin_polarity = rk3328_set_pin_polarity,
};

static const struct udevice_id rk3328_vop_ids[] = {
	{
		.compatible = "rockchip,rk3328-vop",
		.data = (ulong)&rk3328_driverdata
	},
	{ /* sentile */ }
};

static const struct video_ops rk3328_vop_ops = {
};

U_BOOT_DRIVER(rk3328_vop) = {
	.name	= "rk3328_vop",
	.id	= UCLASS_VIDEO,
	.of_match = rk3328_vop_ids,
	.ops	= &rk3328_vop_ops,
	.bind	= rk_vop_bind,
	.probe	= rk3328_vop_probe,
	.remove = rk3328_vop_remove,
	.priv_auto	= sizeof(struct rk_vop_priv),
	.flags	= DM_FLAG_PRE_RELOC | DM_FLAG_OS_PREPARE,
};
