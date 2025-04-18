// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (c) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <cpu_func.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode_graph.h>
#include <fdtdec.h>
#include <log.h>
#include <panel.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/powergate.h>

#include "dc.h"

/* Holder of Tegra per-SOC DC differences */
struct tegra_dc_soc_info {
	bool has_timer;
	bool has_rgb;
	bool has_pgate;
};

/* Information about the display controller */
struct tegra_lcd_priv {
	int width;			/* width in pixels */
	int height;			/* height in pixels */
	enum video_log2_bpp log2_bpp;	/* colour depth */
	struct display_timing timing;
	struct udevice *panel;		/* Panels attached to RGB */
	struct udevice *bridge;		/* Bridge linked with DC */
	struct dc_ctlr *dc;		/* Display controller regmap */
	const struct tegra_dc_soc_info *soc;
	fdt_addr_t frame_buffer;	/* Address of frame buffer */
	unsigned pixel_clock;		/* Pixel clock in Hz */
	struct clk *clk;
	struct clk *clk_parent;
	ulong scdiv;			/* Clock divider used by disp_clk_ctrl */
	bool rotation;			/* 180 degree panel turn */
	int pipe;			/* DC controller: 0 for A, 1 for B */
};

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 2560,
	LCD_MAX_HEIGHT		= 1600,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

static void update_window(struct tegra_lcd_priv *priv,
			  struct disp_ctl_win *win)
{
	struct dc_ctlr *dc = priv->dc;
	unsigned h_dda, v_dda;
	unsigned long val;

	val = readl(&dc->cmd.disp_win_header);
	val |= WINDOW_A_SELECT;
	writel(val, &dc->cmd.disp_win_header);

	writel(win->fmt, &dc->win.color_depth);

	clrsetbits_le32(&dc->win.byte_swap, BYTE_SWAP_MASK,
			BYTE_SWAP_NOSWAP << BYTE_SWAP_SHIFT);

	val = win->out_x << H_POSITION_SHIFT;
	val |= win->out_y << V_POSITION_SHIFT;
	writel(val, &dc->win.pos);

	val = win->out_w << H_SIZE_SHIFT;
	val |= win->out_h << V_SIZE_SHIFT;
	writel(val, &dc->win.size);

	val = (win->w * win->bpp / 8) << H_PRESCALED_SIZE_SHIFT;
	val |= win->h << V_PRESCALED_SIZE_SHIFT;
	writel(val, &dc->win.prescaled_size);

	writel(0, &dc->win.h_initial_dda);
	writel(0, &dc->win.v_initial_dda);

	h_dda = (win->w * 0x1000) / max(win->out_w - 1, 1U);
	v_dda = (win->h * 0x1000) / max(win->out_h - 1, 1U);

	val = h_dda << H_DDA_INC_SHIFT;
	val |= v_dda << V_DDA_INC_SHIFT;
	writel(val, &dc->win.dda_increment);

	writel(win->stride, &dc->win.line_stride);
	writel(0, &dc->win.buf_stride);

	val = WIN_ENABLE;
	if (win->bpp < 24)
		val |= COLOR_EXPAND;

	if (priv->rotation)
		val |= H_DIRECTION | V_DIRECTION;

	writel(val, &dc->win.win_opt);

	writel((unsigned long)win->phys_addr, &dc->winbuf.start_addr);
	writel(win->x, &dc->winbuf.addr_h_offset);
	writel(win->y, &dc->winbuf.addr_v_offset);

	writel(0xff00, &dc->win.blend_nokey);
	writel(0xff00, &dc->win.blend_1win);

	val = GENERAL_ACT_REQ | WIN_A_ACT_REQ;
	val |= GENERAL_UPDATE | WIN_A_UPDATE;
	writel(val, &dc->cmd.state_ctrl);
}

static int update_display_mode(struct tegra_lcd_priv *priv)
{
	struct dc_disp_reg *disp = &priv->dc->disp;
	struct display_timing *dt = &priv->timing;
	unsigned long val;

	writel(0x0, &disp->disp_timing_opt);

	writel(1 | 1 << 16, &disp->ref_to_sync);
	writel(dt->hsync_len.typ | dt->vsync_len.typ << 16, &disp->sync_width);
	writel(dt->hback_porch.typ | dt->vback_porch.typ << 16,
	       &disp->back_porch);
	writel((dt->hfront_porch.typ - 1) | (dt->vfront_porch.typ - 1) << 16,
	       &disp->front_porch);
	writel(dt->hactive.typ | (dt->vactive.typ << 16), &disp->disp_active);

	if (priv->soc->has_rgb) {
		val = DE_SELECT_ACTIVE << DE_SELECT_SHIFT;
		val |= DE_CONTROL_NORMAL << DE_CONTROL_SHIFT;
		writel(val, &disp->data_enable_opt);

		val = DATA_FORMAT_DF1P1C << DATA_FORMAT_SHIFT;
		val |= DATA_ALIGNMENT_MSB << DATA_ALIGNMENT_SHIFT;
		val |= DATA_ORDER_RED_BLUE << DATA_ORDER_SHIFT;
		writel(val, &disp->disp_interface_ctrl);

		writel(0x00010001, &disp->shift_clk_opt);
	}

	val = PIXEL_CLK_DIVIDER_PCD1 << PIXEL_CLK_DIVIDER_SHIFT;
	val |= priv->scdiv << SHIFT_CLK_DIVIDER_SHIFT;
	writel(val, &disp->disp_clk_ctrl);

	return 0;
}

/* Start up the display and turn on power to PWMs */
static void basic_init(struct dc_cmd_reg *cmd)
{
	u32 val;

	writel(0x00000100, &cmd->gen_incr_syncpt_ctrl);
	writel(0x0000011a, &cmd->cont_syncpt_vsync);
	writel(0x00000000, &cmd->int_type);
	writel(0x00000000, &cmd->int_polarity);
	writel(0x00000000, &cmd->int_mask);
	writel(0x00000000, &cmd->int_enb);

	val = PW0_ENABLE | PW1_ENABLE | PW2_ENABLE;
	val |= PW3_ENABLE | PW4_ENABLE | PM0_ENABLE;
	val |= PM1_ENABLE;
	writel(val, &cmd->disp_pow_ctrl);

	val = readl(&cmd->disp_cmd);
	val &= ~CTRL_MODE_MASK;
	val |= CTRL_MODE_C_DISPLAY << CTRL_MODE_SHIFT;
	writel(val, &cmd->disp_cmd);
}

static void basic_init_timer(struct dc_disp_reg *disp)
{
	writel(0x00000020, &disp->mem_high_pri);
	writel(0x00000001, &disp->mem_high_pri_timer);
}

static const u32 rgb_enb_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_polarity_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x01000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_data_tab[PIN_REG_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static const u32 rgb_sel_tab[PIN_OUTPUT_SEL_COUNT] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00210222,
	0x00002200,
	0x00020000,
};

static void rgb_enable(struct tegra_lcd_priv *priv)
{
	struct dc_com_reg *com = &priv->dc->com;
	struct display_timing *dt = &priv->timing;
	u32 value;
	int i;

	for (i = 0; i < PIN_REG_COUNT; i++) {
		writel(rgb_enb_tab[i], &com->pin_output_enb[i]);
		writel(rgb_polarity_tab[i], &com->pin_output_polarity[i]);
		writel(rgb_data_tab[i], &com->pin_output_data[i]);
	}

	/* configure H- and V-sync signal polarities */
	value = readl(&com->pin_output_polarity[1]);

	if (dt->flags & DISPLAY_FLAGS_HSYNC_LOW)
		value |= LHS_OUTPUT_POLARITY_LOW;
	else
		value &= ~LHS_OUTPUT_POLARITY_LOW;

	if (dt->flags & DISPLAY_FLAGS_VSYNC_LOW)
		value |= LVS_OUTPUT_POLARITY_LOW;
	else
		value &= ~LVS_OUTPUT_POLARITY_LOW;

	/* configure pixel data signal polarity */
	if (dt->flags & DISPLAY_FLAGS_PIXDATA_POSEDGE)
		value &= ~LSC0_OUTPUT_POLARITY_LOW;
	else
		value |= LSC0_OUTPUT_POLARITY_LOW;

	writel(value, &com->pin_output_polarity[1]);

	/* configure data enable signal polarity */
	value = readl(&com->pin_output_polarity[3]);

	if (dt->flags & DISPLAY_FLAGS_DE_LOW)
		value |= LSPI_OUTPUT_POLARITY_LOW;
	else
		value &= ~LSPI_OUTPUT_POLARITY_LOW;

	writel(value, &com->pin_output_polarity[3]);

	for (i = 0; i < PIN_OUTPUT_SEL_COUNT; i++)
		writel(rgb_sel_tab[i], &com->pin_output_sel[i]);
}

static int setup_window(struct tegra_lcd_priv *priv,
			struct disp_ctl_win *win)
{
	if (priv->rotation) {
		win->x = priv->width * 2 - 1;
		win->y = priv->height - 1;
	} else {
		win->x = 0;
		win->y = 0;
	}

	win->w = priv->width;
	win->h = priv->height;
	win->out_x = 0;
	win->out_y = 0;
	win->out_w = priv->width;
	win->out_h = priv->height;
	win->phys_addr = priv->frame_buffer;
	win->stride = priv->width * (1 << priv->log2_bpp) / 8;

	log_debug("%s: depth = %d\n", __func__, priv->log2_bpp);

	switch (priv->log2_bpp) {
	case VIDEO_BPP32:
		win->fmt = COLOR_DEPTH_R8G8B8A8;
		win->bpp = 32;
		break;
	case VIDEO_BPP16:
		win->fmt = COLOR_DEPTH_B5G6R5;
		win->bpp = 16;
		break;

	default:
		log_debug("Unsupported LCD bit depth\n");
		return -1;
	}

	return 0;
}

/**
 * Register a new display based on device tree configuration.
 *
 * The frame buffer can be positioned by U-Boot or overridden by the fdt.
 * You should pass in the U-Boot address here, and check the contents of
 * struct tegra_lcd_priv to see what was actually chosen.
 *
 * @param priv			Driver's private data
 * @param default_lcd_base	Default address of LCD frame buffer
 * Return: 0 if ok, -1 on error (unsupported bits per pixel)
 */
static int tegra_display_probe(struct tegra_lcd_priv *priv,
			       void *default_lcd_base)
{
	struct disp_ctl_win window;
	unsigned long rate = clk_get_rate(priv->clk_parent);
	int ret;

	priv->frame_buffer = (u32)default_lcd_base;

	/*
	 * We halve the rate if DISP1 parent is PLLD, since actual parent
	 * is plld_out0 which is PLLD divided by 2.
	 */
	if (priv->clk_parent->id == CLOCK_ID_DISPLAY ||
	    priv->clk_parent->id == CLOCK_ID_DISPLAY2)
		rate /= 2;

	/*
	 * The pixel clock divider is in 7.1 format (where the bottom bit
	 * represents 0.5). Here we calculate the divider needed to get from
	 * the display clock (typically 600MHz) to the pixel clock. We round
	 * up or down as required.
	 */
	if (!priv->scdiv)
		priv->scdiv = ((rate * 2 + priv->pixel_clock / 2)
						/ priv->pixel_clock) - 2;
	log_debug("Display clock %lu, divider %lu\n", rate, priv->scdiv);

	clock_start_periph_pll(priv->clk->id, priv->clk_parent->id,
			       rate);

	basic_init(&priv->dc->cmd);

	if (priv->soc->has_timer)
		basic_init_timer(&priv->dc->disp);

	if (priv->soc->has_rgb)
		rgb_enable(priv);

	if (priv->pixel_clock)
		update_display_mode(priv);

	ret = setup_window(priv, &window);
	if (ret)
		return ret;

	update_window(priv, &window);

	return 0;
}

static int tegra_lcd_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	int ret;

	/* Initialize the Tegra display controller */
	if (priv->soc->has_pgate) {
		uint powergate;

		if (priv->pipe)
			powergate = TEGRA_POWERGATE_DISB;
		else
			powergate = TEGRA_POWERGATE_DIS;

		ret = tegra_powergate_power_off(powergate);
		if (ret < 0) {
			log_debug("failed to power off DISP gate: %d", ret);
			return ret;
		}

		ret = tegra_powergate_sequence_power_up(powergate,
							priv->clk->id);
		if (ret < 0) {
			log_debug("failed to power up DISP gate: %d", ret);
			return ret;
		}
	}

	/* Get shift clock divider from Tegra DSI if used */
	if (priv->bridge) {
		if (!strcmp(priv->bridge->driver->name, "tegra_dsi")) {
			struct tegra_dc_plat *dc_plat = dev_get_plat(priv->bridge);

			priv->scdiv = dc_plat->scdiv;
		}
	}

	/* Clean the framebuffer area */
	memset((u8 *)plat->base, 0, plat->size);
	flush_dcache_all();

	ret = tegra_display_probe(priv, (void *)plat->base);
	if (ret) {
		log_debug("%s: Failed to probe display driver\n", __func__);
		return ret;
	}

	if (priv->panel) {
		ret = panel_enable_backlight(priv->panel);
		if (ret) {
			log_debug("%s: Cannot enable backlight, ret=%d\n", __func__, ret);
			return ret;
		}
	}

	if (priv->bridge) {
		ret = video_bridge_attach(priv->bridge);
		if (ret) {
			log_debug("%s: Cannot attach bridge, ret=%d\n", __func__, ret);
			return ret;
		}
	}

	mmu_set_region_dcache_behaviour(priv->frame_buffer, plat->size,
					DCACHE_WRITETHROUGH);

	/* Enable flushing after LCD writes if requested */
	video_set_flush_dcache(dev, true);

	uc_priv->xsize = priv->width;
	uc_priv->ysize = priv->height;
	uc_priv->bpix = priv->log2_bpp;
	log_debug("LCD frame buffer at %08x, size %x\n", priv->frame_buffer,
		  plat->size);

	if (priv->panel) {
		ret = panel_set_backlight(priv->panel, BACKLIGHT_DEFAULT);
		if (ret)
			return ret;
	}

	if (priv->bridge) {
		ret = video_bridge_set_backlight(priv->bridge, BACKLIGHT_DEFAULT);
		if (ret)
			return ret;
	}

	return 0;
}

static int tegra_lcd_configure_rgb(struct udevice *dev, ofnode rgb)
{
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	ofnode remote;
	int ret;

	/* DC can have only 1 port */
	remote = ofnode_graph_get_remote_node(rgb, -1, -1);

	ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote, &priv->panel);
	if (!ret)
		return 0;

	ret = uclass_get_device_by_ofnode(UCLASS_VIDEO_BRIDGE, remote, &priv->bridge);
	if (!ret)
		return 0;

	/* Try legacy method if graph did not work */
	remote = ofnode_parse_phandle(rgb, "nvidia,panel", 0);
	if (!ofnode_valid(remote))
		return -EINVAL;

	ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote, &priv->panel);
	if (ret) {
		log_debug("%s: Cannot find panel for '%s' (ret=%d)\n",
			  __func__, dev->name, ret);

		ret = uclass_get_device_by_ofnode(UCLASS_VIDEO_BRIDGE, remote,
						  &priv->bridge);
		if (ret) {
			log_err("%s: Cannot find panel or bridge for '%s' (ret=%d)\n",
				__func__, dev->name, ret);
			return ret;
		}
	}

	return 0;
}

static int tegra_lcd_configure_internal(struct udevice *dev)
{
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	struct tegra_dc_plat *dc_plat;
	ofnode host1x = ofnode_get_parent(dev_ofnode(dev));
	ofnode node;
	int ret;

	switch (priv->pipe) {
	case 0: /* DC0 is usually used for DSI */
		/* Check for ganged DSI configuration */
		ofnode_for_each_subnode(node, host1x)
			if (ofnode_name_eq(node, "dsi") && ofnode_is_enabled(node) &&
			    ofnode_read_bool(node, "nvidia,ganged-mode"))
				goto exit;

		/* If no master DSI found loop for any active DSI */
		ofnode_for_each_subnode(node, host1x)
			if (ofnode_name_eq(node, "dsi") && ofnode_is_enabled(node))
				goto exit;

		log_err("%s: failed to find DSI device for '%s'\n",
			__func__, dev->name);

		return -ENODEV;
	case 1: /* DC1 is usually used for HDMI */
		ofnode_for_each_subnode(node, host1x)
			if (ofnode_name_eq(node, "hdmi"))
				goto exit;

		log_err("%s: failed to find HDMI device for '%s'\n",
			__func__, dev->name);

		return -ENODEV;
	default:
		log_debug("Unsupported DC selection\n");
		return -EINVAL;
	}

exit:
	ret = uclass_get_device_by_ofnode(UCLASS_VIDEO_BRIDGE, node, &priv->bridge);
	if (ret) {
		log_err("%s: failed to get DSI/HDMI device for '%s' (ret %d)\n",
			__func__, dev->name, ret);
		return ret;
	}

	priv->clk_parent = devm_clk_get(priv->bridge, "parent");
	if (IS_ERR(priv->clk_parent)) {
		log_debug("%s: Could not get DC clock parent from DSI/HDMI: %ld\n",
			  __func__, PTR_ERR(priv->clk_parent));
		return PTR_ERR(priv->clk_parent);
	}

	dc_plat = dev_get_plat(priv->bridge);

	/* Fill the platform data for internal devices */
	dc_plat->dev = dev;
	dc_plat->dc = priv->dc;
	dc_plat->pipe = priv->pipe;

	return 0;
}

static int tegra_lcd_of_to_plat(struct udevice *dev)
{
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	struct display_timing *timing;
	ofnode rgb;
	int ret;

	priv->dc = (struct dc_ctlr *)dev_read_addr_ptr(dev);
	if (!priv->dc) {
		log_debug("%s: No display controller address\n", __func__);
		return -EINVAL;
	}

	priv->soc = (struct tegra_dc_soc_info *)dev_get_driver_data(dev);

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		log_debug("%s: Could not get DC clock: %ld\n",
			  __func__, PTR_ERR(priv->clk));
		return PTR_ERR(priv->clk);
	}

	priv->clk_parent = devm_clk_get(dev, "parent");
	if (IS_ERR(priv->clk_parent)) {
		log_debug("%s: Could not get DC clock parent: %ld\n",
			  __func__, PTR_ERR(priv->clk_parent));
		return PTR_ERR(priv->clk_parent);
	}

	priv->rotation = dev_read_bool(dev, "nvidia,180-rotation");
	priv->pipe = dev_read_u32_default(dev, "nvidia,head", 0);

	/*
	 * Usual logic of Tegra video routing should be next:
	 * 1. Check rgb subnode for RGB/LVDS panels or bridges
	 * 2. If none found, then iterate through bridges bound,
	 *    looking for DSIA or DSIB for DC0 and HDMI for DC1.
	 * If none of above is valid, then configuration is not
	 * valid.
	 */

	rgb = dev_read_subnode(dev, "rgb");
	if (ofnode_valid(rgb) && ofnode_is_enabled(rgb)) {
		/* RGB is available, use it */
		ret = tegra_lcd_configure_rgb(dev, rgb);
		if (ret)
			return ret;
	} else {
		/* RGB is not available, check for internal devices */
		ret = tegra_lcd_configure_internal(dev);
		if (ret)
			return ret;
	}

	if (priv->panel) {
		ret = panel_get_display_timing(priv->panel, &priv->timing);
		if (ret) {
			ret = ofnode_decode_display_timing(rgb, 0, &priv->timing);
			if (ret) {
				log_debug("%s: Cannot read display timing for '%s' (ret=%d)\n",
					  __func__, dev->name, ret);
				return -EINVAL;
			}
		}
	}

	if (priv->bridge) {
		ret = video_bridge_get_display_timing(priv->bridge, &priv->timing);
		if (ret) {
			log_debug("%s: Cannot read display timing for '%s' (ret=%d)\n",
				  __func__, dev->name, ret);
			return -EINVAL;
		}
	}

	timing = &priv->timing;
	priv->width = timing->hactive.typ;
	priv->height = timing->vactive.typ;
	priv->pixel_clock = timing->pixelclock.typ;
	priv->log2_bpp = VIDEO_BPP16;

	return 0;
}

static int tegra_lcd_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
		(1 << LCD_MAX_LOG2_BPP) / 8;

	return dm_scan_fdt_dev(dev);
}

static const struct tegra_dc_soc_info tegra20_dc_soc_info = {
	.has_timer = true,
	.has_rgb = true,
	.has_pgate = false,
};

static const struct tegra_dc_soc_info tegra30_dc_soc_info = {
	.has_timer = false,
	.has_rgb = true,
	.has_pgate = false,
};

static const struct tegra_dc_soc_info tegra114_dc_soc_info = {
	.has_timer = false,
	.has_rgb = false,
	.has_pgate = true,
};

static const struct udevice_id tegra_lcd_ids[] = {
	{
		.compatible = "nvidia,tegra20-dc",
		.data = (ulong)&tegra20_dc_soc_info
	}, {
		.compatible = "nvidia,tegra30-dc",
		.data = (ulong)&tegra30_dc_soc_info
	}, {
		.compatible = "nvidia,tegra114-dc",
		.data = (ulong)&tegra114_dc_soc_info
	}, {
		.compatible = "nvidia,tegra124-dc",
		.data = (ulong)&tegra114_dc_soc_info
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(tegra_lcd) = {
	.name		= "tegra_lcd",
	.id		= UCLASS_VIDEO,
	.of_match	= tegra_lcd_ids,
	.bind		= tegra_lcd_bind,
	.probe		= tegra_lcd_probe,
	.of_to_plat	= tegra_lcd_of_to_plat,
	.priv_auto	= sizeof(struct tegra_lcd_priv),
};
