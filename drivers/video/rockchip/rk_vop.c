/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <regmap.h>
#include <syscon.h>
#include <video.h>
#include <asm/gpio.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/edp_rk3288.h>
#include <asm/arch/vop_rk3288.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dt-bindings/clock/rk3288-cru.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

struct rk_vop_priv {
	struct rk3288_vop *regs;
	struct rk3288_grf *grf;
};

void rkvop_enable(struct rk3288_vop *regs, ulong fbbase,
		  int fb_bits_per_pixel, const struct display_timing *edid)
{
	u32 lb_mode;
	u32 rgb_mode;
	u32 hactive = edid->hactive.typ;
	u32 vactive = edid->vactive.typ;

	writel(V_ACT_WIDTH(hactive - 1) | V_ACT_HEIGHT(vactive - 1),
	       &regs->win0_act_info);

	writel(V_DSP_XST(edid->hsync_len.typ + edid->hback_porch.typ) |
	       V_DSP_YST(edid->vsync_len.typ + edid->vback_porch.typ),
	       &regs->win0_dsp_st);

	writel(V_DSP_WIDTH(hactive - 1) |
		V_DSP_HEIGHT(vactive - 1),
		&regs->win0_dsp_info);

	clrsetbits_le32(&regs->win0_color_key, M_WIN0_KEY_EN | M_WIN0_KEY_COLOR,
			V_WIN0_KEY_EN(0) | V_WIN0_KEY_COLOR(0));

	switch (fb_bits_per_pixel) {
	case 16:
		rgb_mode = RGB565;
		writel(V_RGB565_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	case 24:
		rgb_mode = RGB888;
		writel(V_RGB888_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	case 32:
	default:
		rgb_mode = ARGB8888;
		writel(V_ARGB888_VIRWIDTH(hactive), &regs->win0_vir);
		break;
	}

	if (hactive > 2560)
		lb_mode = LB_RGB_3840X2;
	else if (hactive > 1920)
		lb_mode = LB_RGB_2560X4;
	else if (hactive > 1280)
		lb_mode = LB_RGB_1920X5;
	else
		lb_mode = LB_RGB_1280X8;

	clrsetbits_le32(&regs->win0_ctrl0,
			M_WIN0_LB_MODE | M_WIN0_DATA_FMT | M_WIN0_EN,
			V_WIN0_LB_MODE(lb_mode) | V_WIN0_DATA_FMT(rgb_mode) |
			V_WIN0_EN(1));

	writel(fbbase, &regs->win0_yrgb_mst);
	writel(0x01, &regs->reg_cfg_done); /* enable reg config */
}

void rkvop_mode_set(struct rk3288_vop *regs,
		    const struct display_timing *edid, enum vop_modes mode)
{
	u32 hactive = edid->hactive.typ;
	u32 vactive = edid->vactive.typ;
	u32 hsync_len = edid->hsync_len.typ;
	u32 hback_porch = edid->hback_porch.typ;
	u32 vsync_len = edid->vsync_len.typ;
	u32 vback_porch = edid->vback_porch.typ;
	u32 hfront_porch = edid->hfront_porch.typ;
	u32 vfront_porch = edid->vfront_porch.typ;
	uint flags;
	int mode_flags;

	switch (mode) {
	case VOP_MODE_HDMI:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_HDMI_OUT_EN(1));
		break;
	case VOP_MODE_EDP:
	default:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_EDP_OUT_EN(1));
		break;
	case VOP_MODE_LVDS:
		clrsetbits_le32(&regs->sys_ctrl, M_ALL_OUT_EN,
				V_RGB_OUT_EN(1));
		break;
	}

	if (mode == VOP_MODE_HDMI || mode == VOP_MODE_EDP)
		/* RGBaaa */
		mode_flags = 15;
	else
		/* RGB888 */
		mode_flags = 0;

	flags = V_DSP_OUT_MODE(mode_flags) |
		V_DSP_HSYNC_POL(!!(edid->flags & DISPLAY_FLAGS_HSYNC_HIGH)) |
		V_DSP_VSYNC_POL(!!(edid->flags & DISPLAY_FLAGS_VSYNC_HIGH));

	clrsetbits_le32(&regs->dsp_ctrl0,
			M_DSP_OUT_MODE | M_DSP_VSYNC_POL | M_DSP_HSYNC_POL,
			flags);

	writel(V_HSYNC(hsync_len) |
	       V_HORPRD(hsync_len + hback_porch + hactive + hfront_porch),
			&regs->dsp_htotal_hs_end);

	writel(V_HEAP(hsync_len + hback_porch + hactive) |
	       V_HASP(hsync_len + hback_porch),
	       &regs->dsp_hact_st_end);

	writel(V_VSYNC(vsync_len) |
	       V_VERPRD(vsync_len + vback_porch + vactive + vfront_porch),
	       &regs->dsp_vtotal_vs_end);

	writel(V_VAEP(vsync_len + vback_porch + vactive)|
	       V_VASP(vsync_len + vback_porch),
	       &regs->dsp_vact_st_end);

	writel(V_HEAP(hsync_len + hback_porch + hactive) |
	       V_HASP(hsync_len + hback_porch),
	       &regs->post_dsp_hact_info);

	writel(V_VAEP(vsync_len + vback_porch + vactive)|
	       V_VASP(vsync_len + vback_porch),
	       &regs->post_dsp_vact_info);

	writel(0x01, &regs->reg_cfg_done); /* enable reg config */
}

/**
 * rk_display_init() - Try to enable the given display device
 *
 * This function performs many steps:
 * - Finds the display device being referenced by @ep_node
 * - Puts the VOP's ID into its uclass platform data
 * - Probes the device to set it up
 * - Reads the EDID timing information
 * - Sets up the VOP clocks, etc. for the selected pixel clock and display mode
 * - Enables the display (the display device handles this and will do different
 *     things depending on the display type)
 * - Tells the uclass about the display resolution so that the console will
 *     appear correctly
 *
 * @dev:	VOP device that we want to connect to the display
 * @fbbase:	Frame buffer address
 * @l2bpp	Log2 of bits-per-pixels for the display
 * @ep_node:	Device tree node to process - this is the offset of an endpoint
 *		node within the VOP's 'port' list.
 * @return 0 if OK, -ve if something went wrong
 */
int rk_display_init(struct udevice *dev, ulong fbbase,
		    enum video_log2_bpp l2bpp, int ep_node)
{
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *blob = gd->fdt_blob;
	struct rk_vop_priv *priv = dev_get_priv(dev);
	int vop_id, remote_vop_id;
	struct rk3288_vop *regs = priv->regs;
	struct display_timing timing;
	struct udevice *disp;
	int ret, remote, i, offset;
	struct display_plat *disp_uc_plat;
	struct clk clk;

	vop_id = fdtdec_get_int(blob, ep_node, "reg", -1);
	debug("vop_id=%d\n", vop_id);
	remote = fdtdec_lookup_phandle(blob, ep_node, "remote-endpoint");
	if (remote < 0)
		return -EINVAL;
	remote_vop_id = fdtdec_get_int(blob, remote, "reg", -1);
	debug("remote vop_id=%d\n", remote_vop_id);

	for (i = 0, offset = remote; i < 3 && offset > 0; i++)
		offset = fdt_parent_offset(blob, offset);
	if (offset < 0) {
		debug("%s: Invalid remote-endpoint position\n", dev->name);
		return -EINVAL;
	}

	ret = uclass_find_device_by_of_offset(UCLASS_DISPLAY, offset, &disp);
	if (ret) {
		debug("%s: device '%s' display not found (ret=%d)\n", __func__,
		      dev->name, ret);
		return ret;
	}

	disp_uc_plat = dev_get_uclass_platdata(disp);
	debug("Found device '%s', disp_uc_priv=%p\n", disp->name, disp_uc_plat);
	if (display_in_use(disp)) {
		debug("   - device in use\n");
		return -EBUSY;
	}

	disp_uc_plat->source_id = remote_vop_id;
	disp_uc_plat->src_dev = dev;

	ret = device_probe(disp);
	if (ret) {
		debug("%s: device '%s' display won't probe (ret=%d)\n",
		      __func__, dev->name, ret);
		return ret;
	}

	ret = display_read_timing(disp, &timing);
	if (ret) {
		debug("%s: Failed to read timings\n", __func__);
		return ret;
	}

	ret = clk_get_by_index(dev, 1, &clk);
	if (!ret)
		ret = clk_set_rate(&clk, timing.pixelclock.typ);
	if (ret) {
		debug("%s: Failed to set pixel clock: ret=%d\n", __func__, ret);
		return ret;
	}

	rkvop_mode_set(regs, &timing, vop_id);

	rkvop_enable(regs, fbbase, 1 << l2bpp, &timing);

	ret = display_enable(disp, 1 << l2bpp, &timing);
	if (ret)
		return ret;

	uc_priv->xsize = timing.hactive.typ;
	uc_priv->ysize = timing.vactive.typ;
	uc_priv->bpix = l2bpp;
	debug("fb=%lx, size=%d %d\n", fbbase, uc_priv->xsize, uc_priv->ysize);

	return 0;
}

static int rk_vop_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	const void *blob = gd->fdt_blob;
	struct rk_vop_priv *priv = dev_get_priv(dev);
	struct udevice *reg;
	int ret, port, node;

	/* Before relocation we don't need to do anything */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->regs = (struct rk3288_vop *)dev_get_addr(dev);

	/* lcdc(vop) iodomain select 1.8V */
	rk_setreg(&priv->grf->io_vsel, 1 << 0);

	/*
	 * Try some common regulators. We should really get these from the
	 * device tree somehow.
	 */
	ret = regulator_autoset_by_name("vcc18_lcd", &reg);
	if (ret)
		debug("%s: Cannot autoset regulator vcc18_lcd\n", __func__);
	ret = regulator_autoset_by_name("VCC18_LCD", &reg);
	if (ret)
		debug("%s: Cannot autoset regulator VCC18_LCD\n", __func__);
	ret = regulator_autoset_by_name("vdd10_lcd_pwren_h", &reg);
	if (ret) {
		debug("%s: Cannot autoset regulator vdd10_lcd_pwren_h\n",
		      __func__);
	}
	ret = regulator_autoset_by_name("vdd10_lcd", &reg);
	if (ret) {
		debug("%s: Cannot autoset regulator vdd10_lcd\n",
		      __func__);
	}
	ret = regulator_autoset_by_name("VDD10_LCD", &reg);
	if (ret) {
		debug("%s: Cannot autoset regulator VDD10_LCD\n",
		      __func__);
	}
	ret = regulator_autoset_by_name("vcc33_lcd", &reg);
	if (ret)
		debug("%s: Cannot autoset regulator vcc33_lcd\n", __func__);

	/*
	 * Try all the ports until we find one that works. In practice this
	 * tries EDP first if available, then HDMI.
	 *
	 * Note that rockchip_vop_set_clk() always uses NPLL as the source
	 * clock so it is currently not possible to use more than one display
	 * device simultaneously.
	 */
	port = fdt_subnode_offset(blob, dev_of_offset(dev), "port");
	if (port < 0)
		return -EINVAL;
	for (node = fdt_first_subnode(blob, port);
	     node > 0;
	     node = fdt_next_subnode(blob, node)) {
		ret = rk_display_init(dev, plat->base, VIDEO_BPP16, node);
		if (ret)
			debug("Device failed: ret=%d\n", ret);
		if (!ret)
			break;
	}
	video_set_flush_dcache(dev, 1);

	return ret;
}

static int rk_vop_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);

	plat->size = 1920 * 1080 * 2;

	return 0;
}

static const struct video_ops rk_vop_ops = {
};

static const struct udevice_id rk_vop_ids[] = {
	{ .compatible = "rockchip,rk3288-vop" },
	{ }
};

U_BOOT_DRIVER(rk_vop) = {
	.name	= "rk_vop",
	.id	= UCLASS_VIDEO,
	.of_match = rk_vop_ids,
	.ops	= &rk_vop_ops,
	.bind	= rk_vop_bind,
	.probe	= rk_vop_probe,
	.priv_auto_alloc_size	= sizeof(struct rk_vop_priv),
};
