/*
 *  (C) Copyright 2010
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/tegra.h>
#include <asm/arch/display.h>
#include <asm/arch/dc.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/timer.h>

static struct fdt_disp_config config;

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

static void write_pair(struct fdt_disp_config *config, int item, u32 *reg)
{
	writel(config->horiz_timing[item] |
			(config->vert_timing[item] << 16), reg);
}

static int update_display_mode(struct dc_disp_reg *disp,
		struct fdt_disp_config *config)
{
	unsigned long val;
	unsigned long rate;
	unsigned long div;

	writel(0x0, &disp->disp_timing_opt);
	write_pair(config, FDT_LCD_TIMING_REF_TO_SYNC, &disp->ref_to_sync);
	write_pair(config, FDT_LCD_TIMING_SYNC_WIDTH, &disp->sync_width);
	write_pair(config, FDT_LCD_TIMING_BACK_PORCH, &disp->back_porch);
	write_pair(config, FDT_LCD_TIMING_FRONT_PORCH, &disp->front_porch);

	writel(config->width | (config->height << 16), &disp->disp_active);

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
	div = ((rate * 2 + config->pixel_clock / 2) / config->pixel_clock) - 2;
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
			struct fdt_disp_config *config)
{
	win->x = 0;
	win->y = 0;
	win->w = config->width;
	win->h = config->height;
	win->out_x = 0;
	win->out_y = 0;
	win->out_w = config->width;
	win->out_h = config->height;
	win->phys_addr = config->frame_buffer;
	win->stride = config->width * (1 << config->log2_bpp) / 8;
	debug("%s: depth = %d\n", __func__, config->log2_bpp);
	switch (config->log2_bpp) {
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

struct fdt_disp_config *tegra_display_get_config(void)
{
	return config.valid ? &config : NULL;
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
 * Decode panel information from the fdt, according to a standard binding
 *
 * @param blob		fdt blob
 * @param node		offset of fdt node to read from
 * @param config	structure to store fdt config into
 * @return 0 if ok, -ve on error
 */
static int tegra_decode_panel(const void *blob, int node,
			      struct fdt_disp_config *config)
{
	int front, back, ref;

	config->width = fdtdec_get_int(blob, node, "xres", -1);
	config->height = fdtdec_get_int(blob, node, "yres", -1);
	config->pixel_clock = fdtdec_get_int(blob, node, "clock", 0);
	if (!config->pixel_clock || config->width == -1 ||
			config->height == -1) {
		debug("%s: Pixel parameters missing\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}

	back = fdtdec_get_int(blob, node, "left-margin", -1);
	front = fdtdec_get_int(blob, node, "right-margin", -1);
	ref = fdtdec_get_int(blob, node, "hsync-len", -1);
	if ((back | front | ref) == -1) {
		debug("%s: Horizontal parameters missing\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}

	/* Use a ref-to-sync of 1 always, and take this from the front porch */
	config->horiz_timing[FDT_LCD_TIMING_REF_TO_SYNC] = 1;
	config->horiz_timing[FDT_LCD_TIMING_SYNC_WIDTH] = ref;
	config->horiz_timing[FDT_LCD_TIMING_BACK_PORCH] = back;
	config->horiz_timing[FDT_LCD_TIMING_FRONT_PORCH] = front -
		config->horiz_timing[FDT_LCD_TIMING_REF_TO_SYNC];
	debug_timing("horiz", config->horiz_timing);

	back = fdtdec_get_int(blob, node, "upper-margin", -1);
	front = fdtdec_get_int(blob, node, "lower-margin", -1);
	ref = fdtdec_get_int(blob, node, "vsync-len", -1);
	if ((back | front | ref) == -1) {
		debug("%s: Vertical parameters missing\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}

	config->vert_timing[FDT_LCD_TIMING_REF_TO_SYNC] = 1;
	config->vert_timing[FDT_LCD_TIMING_SYNC_WIDTH] = ref;
	config->vert_timing[FDT_LCD_TIMING_BACK_PORCH] = back;
	config->vert_timing[FDT_LCD_TIMING_FRONT_PORCH] = front -
		config->vert_timing[FDT_LCD_TIMING_REF_TO_SYNC];
	debug_timing("vert", config->vert_timing);

	return 0;
}

/**
 * Decode the display controller information from the fdt.
 *
 * @param blob		fdt blob
 * @param config	structure to store fdt config into
 * @return 0 if ok, -ve on error
 */
static int tegra_display_decode_config(const void *blob,
				       struct fdt_disp_config *config)
{
	int node, rgb;
	int bpp, bit;

	/* TODO: Support multiple controllers */
	node = fdtdec_next_compatible(blob, 0, COMPAT_NVIDIA_TEGRA20_DC);
	if (node < 0) {
		debug("%s: Cannot find display controller node in fdt\n",
		      __func__);
		return node;
	}
	config->disp = (struct disp_ctlr *)fdtdec_get_addr(blob, node, "reg");
	if (!config->disp) {
		debug("%s: No display controller address\n", __func__);
		return -1;
	}

	rgb = fdt_subnode_offset(blob, node, "rgb");

	config->panel_node = fdtdec_lookup_phandle(blob, rgb, "nvidia,panel");
	if (config->panel_node < 0) {
		debug("%s: Cannot find panel information\n", __func__);
		return -1;
	}

	if (tegra_decode_panel(blob, config->panel_node, config)) {
		debug("%s: Failed to decode panel information\n", __func__);
		return -1;
	}

	bpp = fdtdec_get_int(blob, config->panel_node, "nvidia,bits-per-pixel",
			     -1);
	bit = ffs(bpp) - 1;
	if (bpp == (1 << bit))
		config->log2_bpp = bit;
	else
		config->log2_bpp = bpp;
	if (bpp == -1) {
		debug("%s: Pixel bpp parameters missing\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}
	config->bpp = bpp;

	config->valid = 1;	/* we have a valid configuration */

	return 0;
}

int tegra_display_probe(const void *blob, void *default_lcd_base)
{
	struct disp_ctl_win window;
	struct dc_ctlr *dc;

	if (tegra_display_decode_config(blob, &config))
		return -1;

	config.frame_buffer = (u32)default_lcd_base;

	dc = (struct dc_ctlr *)config.disp;

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

	if (config.pixel_clock)
		update_display_mode(&dc->disp, &config);

	if (setup_window(&window, &config))
		return -1;

	update_window(dc, &window);

	return 0;
}
