/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <pwm.h>
#include <video.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pwm.h>
#include <asm/arch/display.h>
#include <asm/arch-tegra/timer.h>

DECLARE_GLOBAL_DATA_PTR;

/* These are the stages we go throuh in enabling the LCD */
enum stage_t {
	STAGE_START,
	STAGE_PANEL_VDD,
	STAGE_LVDS,
	STAGE_BACKLIGHT_VDD,
	STAGE_PWM,
	STAGE_BACKLIGHT_EN,
	STAGE_DONE,
};

#define FDT_LCD_TIMINGS	4

enum {
	FDT_LCD_TIMING_REF_TO_SYNC,
	FDT_LCD_TIMING_SYNC_WIDTH,
	FDT_LCD_TIMING_BACK_PORCH,
	FDT_LCD_TIMING_FRONT_PORCH,

	FDT_LCD_TIMING_COUNT,
};

enum lcd_cache_t {
	FDT_LCD_CACHE_OFF		= 0,
	FDT_LCD_CACHE_WRITE_THROUGH	= 1 << 0,
	FDT_LCD_CACHE_WRITE_BACK	= 1 << 1,
	FDT_LCD_CACHE_FLUSH		= 1 << 2,
	FDT_LCD_CACHE_WRITE_BACK_FLUSH	= FDT_LCD_CACHE_WRITE_BACK |
						FDT_LCD_CACHE_FLUSH,
};

/* Information about the display controller */
struct tegra_lcd_priv {
	enum stage_t stage;	/* Current stage we are at */
	unsigned long timer_next; /* Time we can move onto next stage */
	int width;			/* width in pixels */
	int height;			/* height in pixels */

	/*
	 * log2 of number of bpp, in general, unless it bpp is 24 in which
	 * case this field holds 24 also! This is a U-Boot thing.
	 */
	int log2_bpp;
	struct disp_ctlr *disp;		/* Display controller to use */
	fdt_addr_t frame_buffer;	/* Address of frame buffer */
	unsigned pixel_clock;		/* Pixel clock in Hz */
	uint horiz_timing[FDT_LCD_TIMING_COUNT];	/* Horizontal timing */
	uint vert_timing[FDT_LCD_TIMING_COUNT];		/* Vertical timing */
	struct udevice *pwm;
	int pwm_channel;		/* PWM channel to use for backlight */
	enum lcd_cache_t cache_type;

	struct gpio_desc backlight_en;	/* GPIO for backlight enable */
	struct gpio_desc lvds_shutdown;	/* GPIO for lvds shutdown */
	struct gpio_desc backlight_vdd;	/* GPIO for backlight vdd */
	struct gpio_desc panel_vdd;	/* GPIO for panel vdd */
	/*
	 * Panel required timings
	 * Timing 1: delay between panel_vdd-rise and data-rise
	 * Timing 2: delay between data-rise and backlight_vdd-rise
	 * Timing 3: delay between backlight_vdd and pwm-rise
	 * Timing 4: delay between pwm-rise and backlight_en-rise
	 */
	uint panel_timings[FDT_LCD_TIMINGS];
};

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP16,
};

static void update_window(struct dc_ctlr *dc, struct disp_ctl_win *win)
{
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

static void write_pair(struct tegra_lcd_priv *priv, int item, u32 *reg)
{
	writel(priv->horiz_timing[item] |
			(priv->vert_timing[item] << 16), reg);
}

static int update_display_mode(struct dc_disp_reg *disp,
			       struct tegra_lcd_priv *priv)
{
	unsigned long val;
	unsigned long rate;
	unsigned long div;

	writel(0x0, &disp->disp_timing_opt);
	write_pair(priv, FDT_LCD_TIMING_REF_TO_SYNC, &disp->ref_to_sync);
	write_pair(priv, FDT_LCD_TIMING_SYNC_WIDTH, &disp->sync_width);
	write_pair(priv, FDT_LCD_TIMING_BACK_PORCH, &disp->back_porch);
	write_pair(priv, FDT_LCD_TIMING_FRONT_PORCH, &disp->front_porch);

	writel(priv->width | (priv->height << 16), &disp->disp_active);

	val = DE_SELECT_ACTIVE << DE_SELECT_SHIFT;
	val |= DE_CONTROL_NORMAL << DE_CONTROL_SHIFT;
	writel(val, &disp->data_enable_opt);

	val = DATA_FORMAT_DF1P1C << DATA_FORMAT_SHIFT;
	val |= DATA_ALIGNMENT_MSB << DATA_ALIGNMENT_SHIFT;
	val |= DATA_ORDER_RED_BLUE << DATA_ORDER_SHIFT;
	writel(val, &disp->disp_interface_ctrl);

	/*
	 * The pixel clock divider is in 7.1 format (where the bottom bit
	 * represents 0.5). Here we calculate the divider needed to get from
	 * the display clock (typically 600MHz) to the pixel clock. We round
	 * up or down as requried.
	 */
	rate = clock_get_periph_rate(PERIPH_ID_DISP1, CLOCK_ID_CGENERAL);
	div = ((rate * 2 + priv->pixel_clock / 2) / priv->pixel_clock) - 2;
	debug("Display clock %lu, divider %lu\n", rate, div);

	writel(0x00010001, &disp->shift_clk_opt);

	val = PIXEL_CLK_DIVIDER_PCD1 << PIXEL_CLK_DIVIDER_SHIFT;
	val |= div << SHIFT_CLK_DIVIDER_SHIFT;
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

static void rgb_enable(struct dc_com_reg *com)
{
	int i;

	for (i = 0; i < PIN_REG_COUNT; i++) {
		writel(rgb_enb_tab[i], &com->pin_output_enb[i]);
		writel(rgb_polarity_tab[i], &com->pin_output_polarity[i]);
		writel(rgb_data_tab[i], &com->pin_output_data[i]);
	}

	for (i = 0; i < PIN_OUTPUT_SEL_COUNT; i++)
		writel(rgb_sel_tab[i], &com->pin_output_sel[i]);
}

static int setup_window(struct disp_ctl_win *win,
			struct tegra_lcd_priv *priv)
{
	win->x = 0;
	win->y = 0;
	win->w = priv->width;
	win->h = priv->height;
	win->out_x = 0;
	win->out_y = 0;
	win->out_w = priv->width;
	win->out_h = priv->height;
	win->phys_addr = priv->frame_buffer;
	win->stride = priv->width * (1 << priv->log2_bpp) / 8;
	debug("%s: depth = %d\n", __func__, priv->log2_bpp);
	switch (priv->log2_bpp) {
	case 5:
	case 24:
		win->fmt = COLOR_DEPTH_R8G8B8A8;
		win->bpp = 32;
		break;
	case 4:
		win->fmt = COLOR_DEPTH_B5G6R5;
		win->bpp = 16;
		break;

	default:
		debug("Unsupported LCD bit depth");
		return -1;
	}

	return 0;
}

static void debug_timing(const char *name, unsigned int timing[])
{
#ifdef DEBUG
	int i;

	debug("%s timing: ", name);
	for (i = 0; i < FDT_LCD_TIMING_COUNT; i++)
		debug("%d ", timing[i]);
	debug("\n");
#endif
}

/**
 * Register a new display based on device tree configuration.
 *
 * The frame buffer can be positioned by U-Boot or overriden by the fdt.
 * You should pass in the U-Boot address here, and check the contents of
 * struct tegra_lcd_priv to see what was actually chosen.
 *
 * @param blob			Device tree blob
 * @param priv			Driver's private data
 * @param default_lcd_base	Default address of LCD frame buffer
 * @return 0 if ok, -1 on error (unsupported bits per pixel)
 */
static int tegra_display_probe(const void *blob, struct tegra_lcd_priv *priv,
			       void *default_lcd_base)
{
	struct disp_ctl_win window;
	struct dc_ctlr *dc;

	priv->frame_buffer = (u32)default_lcd_base;

	dc = (struct dc_ctlr *)priv->disp;

	/*
	 * A header file for clock constants was NAKed upstream.
	 * TODO: Put this into the FDT and fdt_lcd struct when we have clock
	 * support there
	 */
	clock_start_periph_pll(PERIPH_ID_HOST1X, CLOCK_ID_PERIPH,
			       144 * 1000000);
	clock_start_periph_pll(PERIPH_ID_DISP1, CLOCK_ID_CGENERAL,
			       600 * 1000000);
	basic_init(&dc->cmd);
	basic_init_timer(&dc->disp);
	rgb_enable(&dc->com);

	if (priv->pixel_clock)
		update_display_mode(&dc->disp, priv);

	if (setup_window(&window, priv))
		return -1;

	update_window(dc, &window);

	return 0;
}

/**
 * Handle the next stage of device init
 */
static int handle_stage(const void *blob, struct tegra_lcd_priv *priv)
{
	debug("%s: stage %d\n", __func__, priv->stage);

	/* do the things for this stage */
	switch (priv->stage) {
	case STAGE_START:
		/*
		 * It is possible that the FDT has requested that the LCD be
		 * disabled. We currently don't support this. It would require
		 * changes to U-Boot LCD subsystem to have LCD support
		 * compiled in but not used. An easier option might be to
		 * still have a frame buffer, but leave the backlight off and
		 * remove all mention of lcd in the stdout environment
		 * variable.
		 */

		funcmux_select(PERIPH_ID_DISP1, FUNCMUX_DEFAULT);
		break;
	case STAGE_PANEL_VDD:
		if (dm_gpio_is_valid(&priv->panel_vdd))
			dm_gpio_set_value(&priv->panel_vdd, 1);
		break;
	case STAGE_LVDS:
		if (dm_gpio_is_valid(&priv->lvds_shutdown))
			dm_gpio_set_value(&priv->lvds_shutdown, 1);
		break;
	case STAGE_BACKLIGHT_VDD:
		if (dm_gpio_is_valid(&priv->backlight_vdd))
			dm_gpio_set_value(&priv->backlight_vdd, 1);
		break;
	case STAGE_PWM:
		/* Enable PWM at 15/16 high, 32768 Hz with divider 1 */
		pinmux_set_func(PMUX_PINGRP_GPU, PMUX_FUNC_PWM);
		pinmux_tristate_disable(PMUX_PINGRP_GPU);

		pwm_set_config(priv->pwm, priv->pwm_channel, 0xdf, 0xff);
		pwm_set_enable(priv->pwm, priv->pwm_channel, true);
		break;
	case STAGE_BACKLIGHT_EN:
		if (dm_gpio_is_valid(&priv->backlight_en))
			dm_gpio_set_value(&priv->backlight_en, 1);
		break;
	case STAGE_DONE:
		break;
	}

	/* set up timer for next stage */
	priv->timer_next = timer_get_us();
	if (priv->stage < FDT_LCD_TIMINGS)
		priv->timer_next += priv->panel_timings[priv->stage] * 1000;

	/* move to next stage */
	priv->stage++;
	return 0;
}

/**
 * Perform the next stage of the LCD init if it is time to do so.
 *
 * LCD init can be time-consuming because of the number of delays we need
 * while waiting for the backlight power supply, etc. This function can
 * be called at various times during U-Boot operation to advance the
 * initialization of the LCD to the next stage if sufficient time has
 * passed since the last stage. It keeps track of what stage it is up to
 * and the time that it is permitted to move to the next stage.
 *
 * The final call should have wait=1 to complete the init.
 *
 * @param blob	fdt blob containing LCD information
 * @param wait	1 to wait until all init is complete, and then return
 *		0 to return immediately, potentially doing nothing if it is
 *		not yet time for the next init.
 */
static int tegra_lcd_check_next_stage(const void *blob,
				      struct tegra_lcd_priv *priv, int wait)
{
	if (priv->stage == STAGE_DONE)
		return 0;

	do {
		/* wait if we need to */
		debug("%s: stage %d\n", __func__, priv->stage);
		if (priv->stage != STAGE_START) {
			int delay = priv->timer_next - timer_get_us();

			if (delay > 0) {
				if (wait)
					udelay(delay);
				else
					return 0;
			}
		}

		if (handle_stage(blob, priv))
			return -1;
	} while (wait && priv->stage != STAGE_DONE);
	if (priv->stage == STAGE_DONE)
		debug("%s: LCD init complete\n", __func__);

	return 0;
}

static int tegra_lcd_probe(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int type = DCACHE_OFF;

	/* Initialize the Tegra display controller */
	if (tegra_display_probe(blob, priv, (void *)plat->base)) {
		printf("%s: Failed to probe display driver\n", __func__);
		return -1;
	}

	tegra_lcd_check_next_stage(blob, priv, 1);

	/* Set up the LCD caching as requested */
	if (priv->cache_type & FDT_LCD_CACHE_WRITE_THROUGH)
		type = DCACHE_WRITETHROUGH;
	else if (priv->cache_type & FDT_LCD_CACHE_WRITE_BACK)
		type = DCACHE_WRITEBACK;
	mmu_set_region_dcache_behaviour(priv->frame_buffer, plat->size, type);

	/* Enable flushing after LCD writes if requested */
	video_set_flush_dcache(dev, priv->cache_type & FDT_LCD_CACHE_FLUSH);

	uc_priv->xsize = priv->width;
	uc_priv->ysize = priv->height;
	uc_priv->bpix = priv->log2_bpp;
	debug("LCD frame buffer at %pa, size %x\n", &priv->frame_buffer,
	      plat->size);

	return 0;
}

static int tegra_lcd_ofdata_to_platdata(struct udevice *dev)
{
	struct tegra_lcd_priv *priv = dev_get_priv(dev);
	struct fdtdec_phandle_args args;
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;
	int front, back, ref;
	int panel_node;
	int rgb;
	int bpp, bit;
	int ret;

	priv->disp = (struct disp_ctlr *)dev_get_addr(dev);
	if (!priv->disp) {
		debug("%s: No display controller address\n", __func__);
		return -EINVAL;
	}

	rgb = fdt_subnode_offset(blob, node, "rgb");

	panel_node = fdtdec_lookup_phandle(blob, rgb, "nvidia,panel");
	if (panel_node < 0) {
		debug("%s: Cannot find panel information\n", __func__);
		return -EINVAL;
	}

	priv->width = fdtdec_get_int(blob, panel_node, "xres", -1);
	priv->height = fdtdec_get_int(blob, panel_node, "yres", -1);
	priv->pixel_clock = fdtdec_get_int(blob, panel_node, "clock", 0);
	if (!priv->pixel_clock || priv->width == -1 || priv->height == -1) {
		debug("%s: Pixel parameters missing\n", __func__);
		return -EINVAL;
	}

	back = fdtdec_get_int(blob, panel_node, "left-margin", -1);
	front = fdtdec_get_int(blob, panel_node, "right-margin", -1);
	ref = fdtdec_get_int(blob, panel_node, "hsync-len", -1);
	if ((back | front | ref) == -1) {
		debug("%s: Horizontal parameters missing\n", __func__);
		return -EINVAL;
	}

	/* Use a ref-to-sync of 1 always, and take this from the front porch */
	priv->horiz_timing[FDT_LCD_TIMING_REF_TO_SYNC] = 1;
	priv->horiz_timing[FDT_LCD_TIMING_SYNC_WIDTH] = ref;
	priv->horiz_timing[FDT_LCD_TIMING_BACK_PORCH] = back;
	priv->horiz_timing[FDT_LCD_TIMING_FRONT_PORCH] = front -
		priv->horiz_timing[FDT_LCD_TIMING_REF_TO_SYNC];
	debug_timing("horiz", priv->horiz_timing);

	back = fdtdec_get_int(blob, panel_node, "upper-margin", -1);
	front = fdtdec_get_int(blob, panel_node, "lower-margin", -1);
	ref = fdtdec_get_int(blob, panel_node, "vsync-len", -1);
	if ((back | front | ref) == -1) {
		debug("%s: Vertical parameters missing\n", __func__);
		return -EINVAL;
	}

	priv->vert_timing[FDT_LCD_TIMING_REF_TO_SYNC] = 1;
	priv->vert_timing[FDT_LCD_TIMING_SYNC_WIDTH] = ref;
	priv->vert_timing[FDT_LCD_TIMING_BACK_PORCH] = back;
	priv->vert_timing[FDT_LCD_TIMING_FRONT_PORCH] = front -
		priv->vert_timing[FDT_LCD_TIMING_REF_TO_SYNC];
	debug_timing("vert", priv->vert_timing);

	bpp = fdtdec_get_int(blob, panel_node, "nvidia,bits-per-pixel", -1);
	bit = ffs(bpp) - 1;
	if (bpp == (1 << bit))
		priv->log2_bpp = bit;
	else
		priv->log2_bpp = bpp;
	if (bpp == -1) {
		debug("%s: Pixel bpp parameters missing\n", __func__);
		return -EINVAL;
	}

	if (fdtdec_parse_phandle_with_args(blob, panel_node, "nvidia,pwm",
					   "#pwm-cells", 0, 0, &args)) {
		debug("%s: Unable to decode PWM\n", __func__);
		return -EINVAL;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_PWM, args.node, &priv->pwm);
	if (ret) {
		debug("%s: Unable to find PWM\n", __func__);
		return -EINVAL;
	}
	priv->pwm_channel = args.args[0];

	priv->cache_type = fdtdec_get_int(blob, panel_node, "nvidia,cache-type",
					  FDT_LCD_CACHE_WRITE_BACK_FLUSH);

	/* These GPIOs are all optional */
	gpio_request_by_name_nodev(blob, panel_node,
				   "nvidia,backlight-enable-gpios", 0,
				   &priv->backlight_en, GPIOD_IS_OUT);
	gpio_request_by_name_nodev(blob, panel_node,
				   "nvidia,lvds-shutdown-gpios", 0,
				   &priv->lvds_shutdown, GPIOD_IS_OUT);
	gpio_request_by_name_nodev(blob, panel_node,
				   "nvidia,backlight-vdd-gpios", 0,
				   &priv->backlight_vdd, GPIOD_IS_OUT);
	gpio_request_by_name_nodev(blob, panel_node,
				   "nvidia,panel-vdd-gpios", 0,
				   &priv->panel_vdd, GPIOD_IS_OUT);

	if (fdtdec_get_int_array(blob, panel_node, "nvidia,panel-timings",
				 priv->panel_timings, FDT_LCD_TIMINGS))
		return -EINVAL;

	return 0;
}

static int tegra_lcd_bind(struct udevice *dev)
{
	struct video_uc_platdata *plat = dev_get_uclass_platdata(dev);

	plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
		(1 << LCD_MAX_LOG2_BPP) / 8;

	return 0;
}

static const struct video_ops tegra_lcd_ops = {
};

static const struct udevice_id tegra_lcd_ids[] = {
	{ .compatible = "nvidia,tegra20-dc" },
	{ }
};

U_BOOT_DRIVER(tegra_lcd) = {
	.name	= "tegra_lcd",
	.id	= UCLASS_VIDEO,
	.of_match = tegra_lcd_ids,
	.ops	= &tegra_lcd_ops,
	.bind	= tegra_lcd_bind,
	.probe	= tegra_lcd_probe,
	.ofdata_to_platdata	= tegra_lcd_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(struct tegra_lcd_priv),
};
