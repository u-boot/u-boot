// SPDX-License-Identifier: GPL-2.0+
/*
 * i.MX8 LCD interface driver inspired from the Linux driver
 * Copyright 2019 NXP
 * Copyright 2024 Bootlin
 * Adapted by Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <asm/io.h>
#include <asm/mach-imx/dma.h>
#include <clk.h>
#include <dm.h>
#include <panel.h>
#include <power-domain.h>
#include <video.h>
#include <video_bridge.h>
#include <linux/delay.h>

#include "../videomodes.h"

#define LCDIFV3_CTRL 0x0
#define LCDIFV3_CTRL_SET 0x4
#define LCDIFV3_CTRL_CLR 0x8
#define   CTRL_INV_HS BIT(0)
#define   CTRL_INV_VS BIT(1)
#define   CTRL_INV_DE BIT(2)
#define   CTRL_INV_PXCK BIT(3)
#define   CTRL_CLK_GATE BIT(30)
#define   CTRL_SW_RESET BIT(31)

#define LCDIFV3_DISP_PARA 0x10
#define   DISP_PARA_DISP_MODE_NORMAL 0
#define   DISP_PARA_LINE_PATTERN_RGB_YUV 0
#define   DISP_PARA_DISP_ON BIT(31)

#define LCDIFV3_DISP_SIZE 0x14
#define   DISP_SIZE_DELTA_X(x) ((x) & 0xffff)
#define   DISP_SIZE_DELTA_Y(x) ((x) << 16)

#define LCDIFV3_HSYN_PARA 0x18
#define   HSYN_PARA_FP_H(x) ((x) & 0xffff)
#define   HSYN_PARA_BP_H(x) ((x) << 16)

#define LCDIFV3_VSYN_PARA 0x1C
#define   VSYN_PARA_FP_V(x) ((x) & 0xffff)
#define   VSYN_PARA_BP_V(x) ((x) << 16)

#define LCDIFV3_VSYN_HSYN_WIDTH 0x20
#define   VSYN_HSYN_PW_H(x) ((x) & 0xffff)
#define   VSYN_HSYN_PW_V(x) ((x) << 16)

#define LCDIFV3_CTRLDESCL0_1 0x200
#define   CTRLDESCL0_1_WIDTH(x) ((x) & 0xffff)
#define   CTRLDESCL0_1_HEIGHT(x) ((x) << 16)

#define LCDIFV3_CTRLDESCL0_3 0x208
#define   CTRLDESCL0_3_PITCH(x) ((x) & 0xFFFF)

#define LCDIFV3_CTRLDESCL_LOW0_4 0x20C
#define LCDIFV3_CTRLDESCL_HIGH0_4 0x210

#define LCDIFV3_CTRLDESCL0_5 0x214
#define   CTRLDESCL0_5_YUV_FORMAT(x) (((x) & 0x3) << 14)
#define   CTRLDESCL0_5_BPP(x) (((x) & 0xf) << 24)
#define     BPP32_ARGB8888 0x9
#define   CTRLDESCL0_5_SHADOW_LOAD_EN BIT(30)
#define   CTRLDESCL0_5_EN BIT(31)

struct lcdifv3_priv {
	void __iomem *base;
	struct clk pix_clk;
	struct power_domain pd;
	struct udevice *panel;
	struct udevice *bridge;
};

static void lcdifv3_set_mode(struct lcdifv3_priv *priv,
			     struct display_timing *timings)
{
	u32 reg;

	writel(DISP_SIZE_DELTA_X(timings->hactive.typ) |
	       DISP_SIZE_DELTA_Y(timings->vactive.typ),
	       priv->base + LCDIFV3_DISP_SIZE);

	writel(HSYN_PARA_FP_H(timings->hfront_porch.typ) |
	       HSYN_PARA_BP_H(timings->hback_porch.typ),
	       priv->base + LCDIFV3_HSYN_PARA);

	writel(VSYN_PARA_BP_V(timings->vback_porch.typ) |
	       VSYN_PARA_FP_V(timings->vfront_porch.typ),
	       priv->base + LCDIFV3_VSYN_PARA);

	writel(VSYN_HSYN_PW_H(timings->hsync_len.typ) |
	       VSYN_HSYN_PW_V(timings->vsync_len.typ),
	       priv->base + LCDIFV3_VSYN_HSYN_WIDTH);

	writel(CTRLDESCL0_1_WIDTH(timings->hactive.typ) |
	       CTRLDESCL0_1_HEIGHT(timings->vactive.typ),
	       priv->base + LCDIFV3_CTRLDESCL0_1);

	if (timings->flags & DISPLAY_FLAGS_HSYNC_LOW)
		writel(CTRL_INV_HS, priv->base + LCDIFV3_CTRL_SET);
	else
		writel(CTRL_INV_HS, priv->base + LCDIFV3_CTRL_CLR);

	if (timings->flags & DISPLAY_FLAGS_VSYNC_LOW)
		writel(CTRL_INV_VS, priv->base + LCDIFV3_CTRL_SET);
	else
		writel(CTRL_INV_VS, priv->base + LCDIFV3_CTRL_CLR);

	if (timings->flags & DISPLAY_FLAGS_DE_LOW)
		writel(CTRL_INV_DE, priv->base + LCDIFV3_CTRL_SET);
	else
		writel(CTRL_INV_DE, priv->base + LCDIFV3_CTRL_CLR);

	if (timings->flags & DISPLAY_FLAGS_PIXDATA_POSEDGE)
		writel(CTRL_INV_PXCK, priv->base + LCDIFV3_CTRL_SET);
	else
		writel(CTRL_INV_PXCK, priv->base + LCDIFV3_CTRL_CLR);

	writel(0, priv->base + LCDIFV3_DISP_PARA);

	reg = readl(priv->base + LCDIFV3_CTRLDESCL0_5);
	reg &= ~(CTRLDESCL0_5_BPP(0xf) | CTRLDESCL0_5_YUV_FORMAT(0x3));
	reg |= CTRLDESCL0_5_BPP(BPP32_ARGB8888);
	writel(reg, priv->base + LCDIFV3_CTRLDESCL0_5);
}

static void lcdifv3_enable_controller(struct lcdifv3_priv *priv)
{
	u32 reg;

	reg = readl(priv->base + LCDIFV3_DISP_PARA);
	reg |= DISP_PARA_DISP_ON;
	writel(reg, priv->base + LCDIFV3_DISP_PARA);

	reg = readl(priv->base + LCDIFV3_CTRLDESCL0_5);
	reg |= CTRLDESCL0_5_EN;
	writel(reg, priv->base + LCDIFV3_CTRLDESCL0_5);
}

static int lcdifv3_video_sync(struct udevice *dev)
{
	struct lcdifv3_priv *priv = dev_get_priv(dev);
	u32 reg;

	reg = readl(priv->base + LCDIFV3_CTRLDESCL0_5);
	reg |= CTRLDESCL0_5_SHADOW_LOAD_EN;
	writel(reg, priv->base + LCDIFV3_CTRLDESCL0_5);

	return 0;
}

static void lcdifv3_init(struct udevice *dev, struct display_timing *timings)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct lcdifv3_priv *priv = dev_get_priv(dev);

	clk_set_rate(&priv->pix_clk, timings->pixelclock.typ);

	writel(CTRL_SW_RESET | CTRL_CLK_GATE, priv->base + LCDIFV3_CTRL_CLR);
	udelay(10);

	lcdifv3_set_mode(priv, timings);

	writel(plat->base & 0xFFFFFFFF, priv->base + LCDIFV3_CTRLDESCL_LOW0_4);
	writel(plat->base >> 32, priv->base + LCDIFV3_CTRLDESCL_HIGH0_4);

	writel(CTRLDESCL0_3_PITCH(timings->hactive.typ * 4), /* 32bpp */
	       priv->base + LCDIFV3_CTRLDESCL0_3);

	lcdifv3_enable_controller(priv);
}

static int lcdifv3_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct lcdifv3_priv *priv = dev_get_priv(dev);
	struct clk axi_clk, disp_axi_clk;
	struct display_timing timings;
	u32 fb_start, fb_end;
	int ret;

	ret = power_domain_get(dev, &priv->pd);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "pix", &priv->pix_clk);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "axi", &axi_clk);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "disp_axi", &disp_axi_clk);
	if (ret < 0)
		return ret;

	ret = power_domain_on(&priv->pd);
	if (ret)
		return ret;

	ret = clk_enable(&priv->pix_clk);
	if (ret)
		goto dis_pd;

	ret = clk_enable(&axi_clk);
	if (ret)
		goto dis_pix_clk;

	ret = clk_enable(&disp_axi_clk);
	if (ret)
		goto dis_axi_clk;

	priv->base = dev_remap_addr(dev);
	if (!priv->base) {
		ret = -EINVAL;
		goto dis_clks;
	}

	/* Attach bridge */
	ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, dev,
					    -1, -1, &priv->bridge);
	if (ret)
		goto dis_clks;

	ret = video_bridge_attach(priv->bridge);
	if (ret)
		goto dis_clks;

	ret = video_bridge_set_backlight(priv->bridge, 80);
	if (ret)
		goto dis_clks;

	/* Attach panels */
	ret = uclass_get_device_by_endpoint(UCLASS_PANEL, priv->bridge,
					    1, -1, &priv->panel);
	if (ret) {
		ret = uclass_get_device_by_endpoint(UCLASS_PANEL, priv->bridge,
						    2, -1, &priv->panel);
		if (ret)
			goto dis_clks;
	}

	ret = panel_get_display_timing(priv->panel, &timings);
	if (ret) {
		ret = ofnode_decode_display_timing(dev_ofnode(priv->panel),
						   0, &timings);
		if (ret) {
			printf("Cannot decode panel timings (%d)\n", ret);
			goto dis_clks;
		}
	}

	lcdifv3_init(dev, &timings);

	/* Only support 32bpp for now */
	uc_priv->bpix = VIDEO_BPP32;
	uc_priv->xsize = timings.hactive.typ;
	uc_priv->ysize = timings.vactive.typ;

	/* Enable dcache for the frame buffer */
	fb_start = plat->base & ~(MMU_SECTION_SIZE - 1);
	fb_end = ALIGN(plat->base + plat->size, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	video_set_flush_dcache(dev, true);

	return 0;

dis_clks:
	clk_disable(&disp_axi_clk);
dis_axi_clk:
	clk_disable(&axi_clk);
dis_pix_clk:
	clk_disable(&priv->pix_clk);
dis_pd:
	power_domain_off(&priv->pd);

	return ret;
}

static int lcdifv3_video_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	/* Max size supported by LCDIF */
	plat->size = 1920 * 1080 * VNBYTES(VIDEO_BPP32);

	return 0;
}

static const struct udevice_id lcdifv3_video_ids[] = {
	{ .compatible = "fsl,imx8mp-lcdif" },
	{ }
};

static struct video_ops lcdifv3_video_ops = {
	.video_sync = lcdifv3_video_sync,
};

U_BOOT_DRIVER(lcdifv3_video) = {
	.name = "lcdif",
	.id = UCLASS_VIDEO,
	.of_match = lcdifv3_video_ids,
	.bind = lcdifv3_video_bind,
	.ops = &lcdifv3_video_ops,
	.probe = lcdifv3_video_probe,
	.priv_auto = sizeof(struct lcdifv3_priv),
	.flags = DM_FLAG_PRE_RELOC | DM_FLAG_OS_PREPARE,
};
