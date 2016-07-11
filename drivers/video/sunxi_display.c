/*
 * Display driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/display.h>
#include <asm/arch/gpio.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <axp_pmic.h>
#include <errno.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <i2c.h>
#include <malloc.h>
#include <video_fb.h>
#include "videomodes.h"
#include "anx9804.h"
#include "hitachi_tx18d42vm_lcd.h"
#include "ssd2828.h"

#ifdef CONFIG_VIDEO_LCD_BL_PWM_ACTIVE_LOW
#define PWM_ON 0
#define PWM_OFF 1
#else
#define PWM_ON 1
#define PWM_OFF 0
#endif

DECLARE_GLOBAL_DATA_PTR;

enum sunxi_monitor {
	sunxi_monitor_none,
	sunxi_monitor_dvi,
	sunxi_monitor_hdmi,
	sunxi_monitor_lcd,
	sunxi_monitor_vga,
	sunxi_monitor_composite_pal,
	sunxi_monitor_composite_ntsc,
	sunxi_monitor_composite_pal_m,
	sunxi_monitor_composite_pal_nc,
};
#define SUNXI_MONITOR_LAST sunxi_monitor_composite_pal_nc

struct sunxi_display {
	GraphicDevice graphic_device;
	enum sunxi_monitor monitor;
	unsigned int depth;
	unsigned int fb_addr;
	unsigned int fb_size;
} sunxi_display;

const struct ctfb_res_modes composite_video_modes[2] = {
	/*  x     y  hz  pixclk ps/kHz   le   ri  up  lo   hs vs  s  vmode */
	{ 720,  576, 50, 37037,  27000, 137,   5, 20, 27,   2, 2, 0, FB_VMODE_INTERLACED },
	{ 720,  480, 60, 37037,  27000, 116,  20, 16, 27,   2, 2, 0, FB_VMODE_INTERLACED },
};

#ifdef CONFIG_VIDEO_HDMI

/*
 * Wait up to 200ms for value to be set in given part of reg.
 */
static int await_completion(u32 *reg, u32 mask, u32 val)
{
	unsigned long tmo = timer_get_us() + 200000;

	while ((readl(reg) & mask) != val) {
		if (timer_get_us() > tmo) {
			printf("DDC: timeout reading EDID\n");
			return -ETIME;
		}
	}
	return 0;
}

static int sunxi_hdmi_hpd_detect(int hpd_delay)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	unsigned long tmo = timer_get_us() + hpd_delay * 1000;

	/* Set pll3 to 300MHz */
	clock_set_pll3(300000000);

	/* Set hdmi parent to pll3 */
	clrsetbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_PLL_MASK,
			CCM_HDMI_CTRL_PLL3);

	/* Set ahb gating to pass */
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);

	/* Clock on */
	setbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);

	writel(SUNXI_HDMI_CTRL_ENABLE, &hdmi->ctrl);
	writel(SUNXI_HDMI_PAD_CTRL0_HDP, &hdmi->pad_ctrl0);

	while (timer_get_us() < tmo) {
		if (readl(&hdmi->hpd) & SUNXI_HDMI_HPD_DETECT)
			return 1;
	}

	return 0;
}

static void sunxi_hdmi_shutdown(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;

	clrbits_le32(&hdmi->ctrl, SUNXI_HDMI_CTRL_ENABLE);
	clrbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);
	clrbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	clrbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	clock_set_pll3(0);
}

static int sunxi_hdmi_ddc_do_command(u32 cmnd, int offset, int n)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;

	setbits_le32(&hdmi->ddc_fifo_ctrl, SUNXI_HDMI_DDC_FIFO_CTRL_CLEAR);
	writel(SUNXI_HMDI_DDC_ADDR_EDDC_SEGMENT(offset >> 8) |
	       SUNXI_HMDI_DDC_ADDR_EDDC_ADDR |
	       SUNXI_HMDI_DDC_ADDR_OFFSET(offset) |
	       SUNXI_HMDI_DDC_ADDR_SLAVE_ADDR, &hdmi->ddc_addr);
#ifndef CONFIG_MACH_SUN6I
	writel(n, &hdmi->ddc_byte_count);
	writel(cmnd, &hdmi->ddc_cmnd);
#else
	writel(n << 16 | cmnd, &hdmi->ddc_cmnd);
#endif
	setbits_le32(&hdmi->ddc_ctrl, SUNXI_HMDI_DDC_CTRL_START);

	return await_completion(&hdmi->ddc_ctrl, SUNXI_HMDI_DDC_CTRL_START, 0);
}

static int sunxi_hdmi_ddc_read(int offset, u8 *buf, int count)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	int i, n;

	while (count > 0) {
		if (count > 16)
			n = 16;
		else
			n = count;

		if (sunxi_hdmi_ddc_do_command(
				SUNXI_HDMI_DDC_CMND_EXPLICIT_EDDC_READ,
				offset, n))
			return -ETIME;

		for (i = 0; i < n; i++)
			*buf++ = readb(&hdmi->ddc_fifo_data);

		offset += n;
		count -= n;
	}

	return 0;
}

static int sunxi_hdmi_edid_get_block(int block, u8 *buf)
{
	int r, retries = 2;

	do {
		r = sunxi_hdmi_ddc_read(block * 128, buf, 128);
		if (r)
			continue;
		r = edid_check_checksum(buf);
		if (r) {
			printf("EDID block %d: checksum error%s\n",
			       block, retries ? ", retrying" : "");
		}
	} while (r && retries--);

	return r;
}

static int sunxi_hdmi_edid_get_mode(struct ctfb_res_modes *mode)
{
	struct edid1_info edid1;
	struct edid_cea861_info cea681[4];
	struct edid_detailed_timing *t =
		(struct edid_detailed_timing *)edid1.monitor_details.timing;
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int i, r, ext_blocks = 0;

	/* SUNXI_HDMI_CTRL_ENABLE & PAD_CTRL0 are already set by hpd_detect */
	writel(SUNXI_HDMI_PAD_CTRL1 | SUNXI_HDMI_PAD_CTRL1_HALVE,
	       &hdmi->pad_ctrl1);
	writel(SUNXI_HDMI_PLL_CTRL | SUNXI_HDMI_PLL_CTRL_DIV(15),
	       &hdmi->pll_ctrl);
	writel(SUNXI_HDMI_PLL_DBG0_PLL3, &hdmi->pll_dbg0);

	/* Reset i2c controller */
	setbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_DDC_GATE);
	writel(SUNXI_HMDI_DDC_CTRL_ENABLE |
	       SUNXI_HMDI_DDC_CTRL_SDA_ENABLE |
	       SUNXI_HMDI_DDC_CTRL_SCL_ENABLE |
	       SUNXI_HMDI_DDC_CTRL_RESET, &hdmi->ddc_ctrl);
	if (await_completion(&hdmi->ddc_ctrl, SUNXI_HMDI_DDC_CTRL_RESET, 0))
		return -EIO;

	writel(SUNXI_HDMI_DDC_CLOCK, &hdmi->ddc_clock);
#ifndef CONFIG_MACH_SUN6I
	writel(SUNXI_HMDI_DDC_LINE_CTRL_SDA_ENABLE |
	       SUNXI_HMDI_DDC_LINE_CTRL_SCL_ENABLE, &hdmi->ddc_line_ctrl);
#endif

	r = sunxi_hdmi_edid_get_block(0, (u8 *)&edid1);
	if (r == 0) {
		r = edid_check_info(&edid1);
		if (r) {
			printf("EDID: invalid EDID data\n");
			r = -EINVAL;
		}
	}
	if (r == 0) {
		ext_blocks = edid1.extension_flag;
		if (ext_blocks > 4)
			ext_blocks = 4;
		for (i = 0; i < ext_blocks; i++) {
			if (sunxi_hdmi_edid_get_block(1 + i,
						(u8 *)&cea681[i]) != 0) {
				ext_blocks = i;
				break;
			}
		}
	}

	/* Disable DDC engine, no longer needed */
	clrbits_le32(&hdmi->ddc_ctrl, SUNXI_HMDI_DDC_CTRL_ENABLE);
	clrbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_DDC_GATE);

	if (r)
		return r;

	/* We want version 1.3 or 1.2 with detailed timing info */
	if (edid1.version != 1 || (edid1.revision < 3 &&
			!EDID1_INFO_FEATURE_PREFERRED_TIMING_MODE(edid1))) {
		printf("EDID: unsupported version %d.%d\n",
		       edid1.version, edid1.revision);
		return -EINVAL;
	}

	/* Take the first usable detailed timing */
	for (i = 0; i < 4; i++, t++) {
		r = video_edid_dtd_to_ctfb_res_modes(t, mode);
		if (r == 0)
			break;
	}
	if (i == 4) {
		printf("EDID: no usable detailed timing found\n");
		return -ENOENT;
	}

	/* Check for basic audio support, if found enable hdmi output */
	sunxi_display.monitor = sunxi_monitor_dvi;
	for (i = 0; i < ext_blocks; i++) {
		if (cea681[i].extension_tag != EDID_CEA861_EXTENSION_TAG ||
		    cea681[i].revision < 2)
			continue;

		if (EDID_CEA861_SUPPORTS_BASIC_AUDIO(cea681[i]))
			sunxi_display.monitor = sunxi_monitor_hdmi;
	}

	return 0;
}

#endif /* CONFIG_VIDEO_HDMI */

#ifdef CONFIG_MACH_SUN4I
/*
 * Testing has shown that on sun4i the display backend engine does not have
 * deep enough fifo-s causing flickering / tearing in full-hd mode due to
 * fifo underruns. So on sun4i we use the display frontend engine to do the
 * dma from memory, as the frontend does have deep enough fifo-s.
 */

static const u32 sun4i_vert_coef[32] = {
	0x00004000, 0x000140ff, 0x00033ffe, 0x00043ffd,
	0x00063efc, 0xff083dfc, 0x000a3bfb, 0xff0d39fb,
	0xff0f37fb, 0xff1136fa, 0xfe1433fb, 0xfe1631fb,
	0xfd192ffb, 0xfd1c2cfb, 0xfd1f29fb, 0xfc2127fc,
	0xfc2424fc, 0xfc2721fc, 0xfb291ffd, 0xfb2c1cfd,
	0xfb2f19fd, 0xfb3116fe, 0xfb3314fe, 0xfa3611ff,
	0xfb370fff, 0xfb390dff, 0xfb3b0a00, 0xfc3d08ff,
	0xfc3e0600, 0xfd3f0400, 0xfe3f0300, 0xff400100,
};

static const u32 sun4i_horz_coef[64] = {
	0x40000000, 0x00000000, 0x40fe0000, 0x0000ff03,
	0x3ffd0000, 0x0000ff05, 0x3ffc0000, 0x0000ff06,
	0x3efb0000, 0x0000ff08, 0x3dfb0000, 0x0000ff09,
	0x3bfa0000, 0x0000fe0d, 0x39fa0000, 0x0000fe0f,
	0x38fa0000, 0x0000fe10, 0x36fa0000, 0x0000fe12,
	0x33fa0000, 0x0000fd16, 0x31fa0000, 0x0000fd18,
	0x2ffa0000, 0x0000fd1a, 0x2cfa0000, 0x0000fc1e,
	0x29fa0000, 0x0000fc21, 0x27fb0000, 0x0000fb23,
	0x24fb0000, 0x0000fb26, 0x21fb0000, 0x0000fb29,
	0x1ffc0000, 0x0000fa2b, 0x1cfc0000, 0x0000fa2e,
	0x19fd0000, 0x0000fa30, 0x16fd0000, 0x0000fa33,
	0x14fd0000, 0x0000fa35, 0x11fe0000, 0x0000fa37,
	0x0ffe0000, 0x0000fa39, 0x0dfe0000, 0x0000fa3b,
	0x0afe0000, 0x0000fa3e, 0x08ff0000, 0x0000fb3e,
	0x06ff0000, 0x0000fb40, 0x05ff0000, 0x0000fc40,
	0x03ff0000, 0x0000fd41, 0x01ff0000, 0x0000fe42,
};

static void sunxi_frontend_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_de_fe_reg * const de_fe =
		(struct sunxi_de_fe_reg *)SUNXI_DE_FE0_BASE;
	int i;

	/* Clocks on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DE_FE0);
	setbits_le32(&ccm->dram_clk_gate, 1 << CCM_DRAM_GATE_OFFSET_DE_FE0);
	clock_set_de_mod_clock(&ccm->fe0_clk_cfg, 300000000);

	setbits_le32(&de_fe->enable, SUNXI_DE_FE_ENABLE_EN);

	for (i = 0; i < 32; i++) {
		writel(sun4i_horz_coef[2 * i], &de_fe->ch0_horzcoef0[i]);
		writel(sun4i_horz_coef[2 * i + 1], &de_fe->ch0_horzcoef1[i]);
		writel(sun4i_vert_coef[i], &de_fe->ch0_vertcoef[i]);
		writel(sun4i_horz_coef[2 * i], &de_fe->ch1_horzcoef0[i]);
		writel(sun4i_horz_coef[2 * i + 1], &de_fe->ch1_horzcoef1[i]);
		writel(sun4i_vert_coef[i], &de_fe->ch1_vertcoef[i]);
	}

	setbits_le32(&de_fe->frame_ctrl, SUNXI_DE_FE_FRAME_CTRL_COEF_RDY);
}

static void sunxi_frontend_mode_set(const struct ctfb_res_modes *mode,
				    unsigned int address)
{
	struct sunxi_de_fe_reg * const de_fe =
		(struct sunxi_de_fe_reg *)SUNXI_DE_FE0_BASE;

	setbits_le32(&de_fe->bypass, SUNXI_DE_FE_BYPASS_CSC_BYPASS);
	writel(CONFIG_SYS_SDRAM_BASE + address, &de_fe->ch0_addr);
	writel(mode->xres * 4, &de_fe->ch0_stride);
	writel(SUNXI_DE_FE_INPUT_FMT_ARGB8888, &de_fe->input_fmt);
	writel(SUNXI_DE_FE_OUTPUT_FMT_ARGB8888, &de_fe->output_fmt);

	writel(SUNXI_DE_FE_HEIGHT(mode->yres) | SUNXI_DE_FE_WIDTH(mode->xres),
	       &de_fe->ch0_insize);
	writel(SUNXI_DE_FE_HEIGHT(mode->yres) | SUNXI_DE_FE_WIDTH(mode->xres),
	       &de_fe->ch0_outsize);
	writel(SUNXI_DE_FE_FACTOR_INT(1), &de_fe->ch0_horzfact);
	writel(SUNXI_DE_FE_FACTOR_INT(1), &de_fe->ch0_vertfact);

	writel(SUNXI_DE_FE_HEIGHT(mode->yres) | SUNXI_DE_FE_WIDTH(mode->xres),
	       &de_fe->ch1_insize);
	writel(SUNXI_DE_FE_HEIGHT(mode->yres) | SUNXI_DE_FE_WIDTH(mode->xres),
	       &de_fe->ch1_outsize);
	writel(SUNXI_DE_FE_FACTOR_INT(1), &de_fe->ch1_horzfact);
	writel(SUNXI_DE_FE_FACTOR_INT(1), &de_fe->ch1_vertfact);

	setbits_le32(&de_fe->frame_ctrl, SUNXI_DE_FE_FRAME_CTRL_REG_RDY);
}

static void sunxi_frontend_enable(void)
{
	struct sunxi_de_fe_reg * const de_fe =
		(struct sunxi_de_fe_reg *)SUNXI_DE_FE0_BASE;

	setbits_le32(&de_fe->frame_ctrl, SUNXI_DE_FE_FRAME_CTRL_FRM_START);
}
#else
static void sunxi_frontend_init(void) {}
static void sunxi_frontend_mode_set(const struct ctfb_res_modes *mode,
				    unsigned int address) {}
static void sunxi_frontend_enable(void) {}
#endif

static bool sunxi_is_composite(void)
{
	switch (sunxi_display.monitor) {
	case sunxi_monitor_none:
	case sunxi_monitor_dvi:
	case sunxi_monitor_hdmi:
	case sunxi_monitor_lcd:
	case sunxi_monitor_vga:
		return false;
	case sunxi_monitor_composite_pal:
	case sunxi_monitor_composite_ntsc:
	case sunxi_monitor_composite_pal_m:
	case sunxi_monitor_composite_pal_nc:
		return true;
	}

	return false; /* Never reached */
}

/*
 * This is the entity that mixes and matches the different layers and inputs.
 * Allwinner calls it the back-end, but i like composer better.
 */
static void sunxi_composer_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
	int i;

	sunxi_frontend_init();

#ifdef CONFIG_SUNXI_GEN_SUN6I
	/* Reset off */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DE_BE0);
#endif

	/* Clocks on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DE_BE0);
#ifndef CONFIG_MACH_SUN4I /* On sun4i the frontend does the dma */
	setbits_le32(&ccm->dram_clk_gate, 1 << CCM_DRAM_GATE_OFFSET_DE_BE0);
#endif
	clock_set_de_mod_clock(&ccm->be0_clk_cfg, 300000000);

	/* Engine bug, clear registers after reset */
	for (i = 0x0800; i < 0x1000; i += 4)
		writel(0, SUNXI_DE_BE0_BASE + i);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_ENABLE);
}

static u32 sunxi_rgb2yuv_coef[12] = {
	0x00000107, 0x00000204, 0x00000064, 0x00000108,
	0x00003f69, 0x00003ed6, 0x000001c1, 0x00000808,
	0x000001c1, 0x00003e88, 0x00003fb8, 0x00000808
};

static void sunxi_composer_mode_set(const struct ctfb_res_modes *mode,
				    unsigned int address)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
	int i;

	sunxi_frontend_mode_set(mode, address);

	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->disp_size);
	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->layer0_size);
#ifndef CONFIG_MACH_SUN4I /* On sun4i the frontend does the dma */
	writel(SUNXI_DE_BE_LAYER_STRIDE(mode->xres), &de_be->layer0_stride);
	writel(address << 3, &de_be->layer0_addr_low32b);
	writel(address >> 29, &de_be->layer0_addr_high4b);
#else
	writel(SUNXI_DE_BE_LAYER_ATTR0_SRC_FE0, &de_be->layer0_attr0_ctrl);
#endif
	writel(SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888, &de_be->layer0_attr1_ctrl);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_LAYER0_ENABLE);
	if (mode->vmode == FB_VMODE_INTERLACED)
		setbits_le32(&de_be->mode,
#ifndef CONFIG_MACH_SUN5I
			     SUNXI_DE_BE_MODE_DEFLICKER_ENABLE |
#endif
			     SUNXI_DE_BE_MODE_INTERLACE_ENABLE);

	if (sunxi_is_composite()) {
		writel(SUNXI_DE_BE_OUTPUT_COLOR_CTRL_ENABLE,
		       &de_be->output_color_ctrl);
		for (i = 0; i < 12; i++)
			writel(sunxi_rgb2yuv_coef[i],
			       &de_be->output_color_coef[i]);
	}
}

static void sunxi_composer_enable(void)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;

	sunxi_frontend_enable();

	setbits_le32(&de_be->reg_ctrl, SUNXI_DE_BE_REG_CTRL_LOAD_REGS);
	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_START);
}

/*
 * LCDC, what allwinner calls a CRTC, so timing controller and serializer.
 */
static void sunxi_lcdc_pll_set(int tcon, int dotclock,
			       int *clk_div, int *clk_double)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int value, n, m, min_m, max_m, diff;
	int best_n = 0, best_m = 0, best_diff = 0x0FFFFFFF;
	int best_double = 0;
	bool use_mipi_pll = false;

	if (tcon == 0) {
#ifdef CONFIG_VIDEO_LCD_IF_PARALLEL
		min_m = 6;
		max_m = 127;
#endif
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
		min_m = max_m = 7;
#endif
	} else {
		min_m = 1;
		max_m = 15;
	}

	/*
	 * Find the lowest divider resulting in a matching clock, if there
	 * is no match, pick the closest lower clock, as monitors tend to
	 * not sync to higher frequencies.
	 */
	for (m = min_m; m <= max_m; m++) {
		n = (m * dotclock) / 3000;

		if ((n >= 9) && (n <= 127)) {
			value = (3000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 0;
			}
		}

		/* These are just duplicates */
		if (!(m & 1))
			continue;

		n = (m * dotclock) / 6000;
		if ((n >= 9) && (n <= 127)) {
			value = (6000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 1;
			}
		}
	}

#ifdef CONFIG_MACH_SUN6I
	/*
	 * Use the MIPI pll if we've been unable to find any matching setting
	 * for PLL3, this happens with high dotclocks because of min_m = 6.
	 */
	if (tcon == 0 && best_n == 0) {
		use_mipi_pll = true;
		best_m = 6;  /* Minimum m for tcon0 */
	}

	if (use_mipi_pll) {
		clock_set_pll3(297000000); /* Fix the video pll at 297 MHz */
		clock_set_mipi_pll(best_m * dotclock * 1000);
		debug("dotclock: %dkHz = %dkHz via mipi pll\n",
		      dotclock, clock_get_mipi_pll() / best_m / 1000);
	} else
#endif
	{
		clock_set_pll3(best_n * 3000000);
		debug("dotclock: %dkHz = %dkHz: (%d * 3MHz * %d) / %d\n",
		      dotclock,
		      (best_double + 1) * clock_get_pll3() / best_m / 1000,
		      best_double + 1, best_n, best_m);
	}

	if (tcon == 0) {
		u32 pll;

		if (use_mipi_pll)
			pll = CCM_LCD_CH0_CTRL_MIPI_PLL;
		else if (best_double)
			pll = CCM_LCD_CH0_CTRL_PLL3_2X;
		else
			pll = CCM_LCD_CH0_CTRL_PLL3;

		writel(CCM_LCD_CH0_CTRL_GATE | CCM_LCD_CH0_CTRL_RST | pll,
		       &ccm->lcd0_ch0_clk_cfg);
	} else {
		writel(CCM_LCD_CH1_CTRL_GATE |
		       (best_double ? CCM_LCD_CH1_CTRL_PLL3_2X :
				      CCM_LCD_CH1_CTRL_PLL3) |
		       CCM_LCD_CH1_CTRL_M(best_m), &ccm->lcd0_ch1_clk_cfg);
		if (sunxi_is_composite())
			setbits_le32(&ccm->lcd0_ch1_clk_cfg,
				     CCM_LCD_CH1_CTRL_HALF_SCLK1);
	}

	*clk_div = best_m;
	*clk_double = best_double;
}

static void sunxi_lcdc_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;

	/* Reset off */
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);
#else
	setbits_le32(&ccm->lcd0_ch0_clk_cfg, CCM_LCD_CH0_CTRL_RST);
#endif

	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset2_cfg, 1 << AHB_RESET_OFFSET_LVDS);
#else
	setbits_le32(&ccm->lvds_clk_cfg, CCM_LVDS_CTRL_RST);
#endif
#endif

	/* Init lcdc */
	writel(0, &lcdc->ctrl); /* Disable tcon */
	writel(0, &lcdc->int0); /* Disable all interrupts */

	/* Disable tcon0 dot clock */
	clrbits_le32(&lcdc->tcon0_dclk, SUNXI_LCDC_TCON0_DCLK_ENABLE);

	/* Set all io lines to tristate */
	writel(0xffffffff, &lcdc->tcon0_io_tristate);
	writel(0xffffffff, &lcdc->tcon1_io_tristate);
}

static void sunxi_lcdc_enable(void)
{
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;

	setbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_TCON_ENABLE);
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
	setbits_le32(&lcdc->tcon0_lvds_intf, SUNXI_LCDC_TCON0_LVDS_INTF_ENABLE);
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_EN_MB);
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVC);
	if (sunxi_display.depth == 18)
		setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVD(0x7));
	else
		setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVD(0xf));
#else
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_UPDATE);
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana1, SUNXI_LCDC_LVDS_ANA1_INIT1);
	udelay(1); /* delay at least 120 ns */
	setbits_le32(&lcdc->lvds_ana1, SUNXI_LCDC_LVDS_ANA1_INIT2);
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_UPDATE);
#endif
#endif
}

static void sunxi_lcdc_panel_enable(void)
{
	int pin, reset_pin;

	/*
	 * Start with backlight disabled to avoid the screen flashing to
	 * white while the lcd inits.
	 */
	pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_BL_EN);
	if (pin >= 0) {
		gpio_request(pin, "lcd_backlight_enable");
		gpio_direction_output(pin, 0);
	}

	pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_BL_PWM);
	if (pin >= 0) {
		gpio_request(pin, "lcd_backlight_pwm");
		gpio_direction_output(pin, PWM_OFF);
	}

	reset_pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_RESET);
	if (reset_pin >= 0) {
		gpio_request(reset_pin, "lcd_reset");
		gpio_direction_output(reset_pin, 0); /* Assert reset */
	}

	/* Give the backlight some time to turn off and power up the panel. */
	mdelay(40);
	pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_POWER);
	if (pin >= 0) {
		gpio_request(pin, "lcd_power");
		gpio_direction_output(pin, 1);
	}

	if (reset_pin >= 0)
		gpio_direction_output(reset_pin, 1); /* De-assert reset */
}

static void sunxi_lcdc_backlight_enable(void)
{
	int pin;

	/*
	 * We want to have scanned out at least one frame before enabling the
	 * backlight to avoid the screen flashing to white when we enable it.
	 */
	mdelay(40);

	pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_BL_EN);
	if (pin >= 0)
		gpio_direction_output(pin, 1);

	pin = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_BL_PWM);
	if (pin >= 0)
		gpio_direction_output(pin, PWM_ON);
}

static int sunxi_lcdc_get_clk_delay(const struct ctfb_res_modes *mode, int tcon)
{
	int delay;

	delay = mode->lower_margin + mode->vsync_len + mode->upper_margin;
	if (mode->vmode == FB_VMODE_INTERLACED)
		delay /= 2;
	if (tcon == 1)
		delay -= 2;

	return (delay > 30) ? 30 : delay;
}

static void sunxi_lcdc_tcon0_mode_set(const struct ctfb_res_modes *mode,
				      bool for_ext_vga_dac)
{
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	int bp, clk_delay, clk_div, clk_double, pin, total, val;

#if defined CONFIG_MACH_SUN8I && defined CONFIG_VIDEO_LCD_IF_LVDS
	for (pin = SUNXI_GPD(18); pin <= SUNXI_GPD(27); pin++) {
#else
	for (pin = SUNXI_GPD(0); pin <= SUNXI_GPD(27); pin++) {
#endif
#ifdef CONFIG_VIDEO_LCD_IF_PARALLEL
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPD_LCD0);
#endif
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPD_LVDS0);
#endif
#ifdef CONFIG_VIDEO_LCD_PANEL_EDP_4_LANE_1620M_VIA_ANX9804
		sunxi_gpio_set_drv(pin, 3);
#endif
	}

	sunxi_lcdc_pll_set(0, mode->pixclock_khz, &clk_div, &clk_double);

	/* Use tcon0 */
	clrsetbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_IO_MAP_MASK,
			SUNXI_LCDC_CTRL_IO_MAP_TCON0);

	clk_delay = sunxi_lcdc_get_clk_delay(mode, 0);
	writel(SUNXI_LCDC_TCON0_CTRL_ENABLE |
	       SUNXI_LCDC_TCON0_CTRL_CLK_DELAY(clk_delay), &lcdc->tcon0_ctrl);

	writel(SUNXI_LCDC_TCON0_DCLK_ENABLE |
	       SUNXI_LCDC_TCON0_DCLK_DIV(clk_div), &lcdc->tcon0_dclk);

	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(mode->yres),
	       &lcdc->tcon0_timing_active);

	bp = mode->hsync_len + mode->left_margin;
	total = mode->xres + mode->right_margin + bp;
	writel(SUNXI_LCDC_TCON0_TIMING_H_TOTAL(total) |
	       SUNXI_LCDC_TCON0_TIMING_H_BP(bp), &lcdc->tcon0_timing_h);

	bp = mode->vsync_len + mode->upper_margin;
	total = mode->yres + mode->lower_margin + bp;
	writel(SUNXI_LCDC_TCON0_TIMING_V_TOTAL(total) |
	       SUNXI_LCDC_TCON0_TIMING_V_BP(bp), &lcdc->tcon0_timing_v);

#ifdef CONFIG_VIDEO_LCD_IF_PARALLEL
	writel(SUNXI_LCDC_X(mode->hsync_len) | SUNXI_LCDC_Y(mode->vsync_len),
	       &lcdc->tcon0_timing_sync);

	writel(0, &lcdc->tcon0_hv_intf);
	writel(0, &lcdc->tcon0_cpu_intf);
#endif
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
	val = (sunxi_display.depth == 18) ? 1 : 0;
	writel(SUNXI_LCDC_TCON0_LVDS_INTF_BITWIDTH(val) |
	       SUNXI_LCDC_TCON0_LVDS_CLK_SEL_TCON0, &lcdc->tcon0_lvds_intf);
#endif

	if (sunxi_display.depth == 18 || sunxi_display.depth == 16) {
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[0]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[1]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[2]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[3]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[4]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[5]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB0, &lcdc->tcon0_frm_table[0]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB1, &lcdc->tcon0_frm_table[1]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB2, &lcdc->tcon0_frm_table[2]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB3, &lcdc->tcon0_frm_table[3]);
		writel(((sunxi_display.depth == 18) ?
			SUNXI_LCDC_TCON0_FRM_CTRL_RGB666 :
			SUNXI_LCDC_TCON0_FRM_CTRL_RGB565),
		       &lcdc->tcon0_frm_ctrl);
	}

	val = SUNXI_LCDC_TCON0_IO_POL_DCLK_PHASE(CONFIG_VIDEO_LCD_DCLK_PHASE);
	if (!(mode->sync & FB_SYNC_HOR_HIGH_ACT))
		val |= SUNXI_LCDC_TCON_HSYNC_MASK;
	if (!(mode->sync & FB_SYNC_VERT_HIGH_ACT))
		val |= SUNXI_LCDC_TCON_VSYNC_MASK;

#ifdef CONFIG_VIDEO_VGA_VIA_LCD_FORCE_SYNC_ACTIVE_HIGH
	if (for_ext_vga_dac)
		val = 0;
#endif
	writel(val, &lcdc->tcon0_io_polarity);

	writel(0, &lcdc->tcon0_io_tristate);
}

#if defined CONFIG_VIDEO_HDMI || defined CONFIG_VIDEO_VGA || defined CONFIG_VIDEO_COMPOSITE
static void sunxi_lcdc_tcon1_mode_set(const struct ctfb_res_modes *mode,
				      int *clk_div, int *clk_double,
				      bool use_portd_hvsync)
{
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	int bp, clk_delay, total, val, yres;

	/* Use tcon1 */
	clrsetbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_IO_MAP_MASK,
			SUNXI_LCDC_CTRL_IO_MAP_TCON1);

	clk_delay = sunxi_lcdc_get_clk_delay(mode, 1);
	writel(SUNXI_LCDC_TCON1_CTRL_ENABLE |
	       ((mode->vmode == FB_VMODE_INTERLACED) ?
			SUNXI_LCDC_TCON1_CTRL_INTERLACE_ENABLE : 0) |
	       SUNXI_LCDC_TCON1_CTRL_CLK_DELAY(clk_delay), &lcdc->tcon1_ctrl);

	yres = mode->yres;
	if (mode->vmode == FB_VMODE_INTERLACED)
		yres /= 2;
	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_source);
	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_scale);
	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_out);

	bp = mode->hsync_len + mode->left_margin;
	total = mode->xres + mode->right_margin + bp;
	writel(SUNXI_LCDC_TCON1_TIMING_H_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_H_BP(bp), &lcdc->tcon1_timing_h);

	bp = mode->vsync_len + mode->upper_margin;
	total = mode->yres + mode->lower_margin + bp;
	if (mode->vmode == FB_VMODE_NONINTERLACED)
		total *= 2;
	writel(SUNXI_LCDC_TCON1_TIMING_V_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_V_BP(bp), &lcdc->tcon1_timing_v);

	writel(SUNXI_LCDC_X(mode->hsync_len) | SUNXI_LCDC_Y(mode->vsync_len),
	       &lcdc->tcon1_timing_sync);

	if (use_portd_hvsync) {
		sunxi_gpio_set_cfgpin(SUNXI_GPD(26), SUNXI_GPD_LCD0);
		sunxi_gpio_set_cfgpin(SUNXI_GPD(27), SUNXI_GPD_LCD0);

		val = 0;
		if (mode->sync & FB_SYNC_HOR_HIGH_ACT)
			val |= SUNXI_LCDC_TCON_HSYNC_MASK;
		if (mode->sync & FB_SYNC_VERT_HIGH_ACT)
			val |= SUNXI_LCDC_TCON_VSYNC_MASK;
		writel(val, &lcdc->tcon1_io_polarity);

		clrbits_le32(&lcdc->tcon1_io_tristate,
			     SUNXI_LCDC_TCON_VSYNC_MASK |
			     SUNXI_LCDC_TCON_HSYNC_MASK);
	}

#ifdef CONFIG_MACH_SUN5I
	if (sunxi_is_composite())
		clrsetbits_le32(&lcdc->mux_ctrl, SUNXI_LCDC_MUX_CTRL_SRC0_MASK,
				SUNXI_LCDC_MUX_CTRL_SRC0(1));
#endif

	sunxi_lcdc_pll_set(1, mode->pixclock_khz, clk_div, clk_double);
}
#endif /* CONFIG_VIDEO_HDMI || defined CONFIG_VIDEO_VGA || CONFIG_VIDEO_COMPOSITE */

#ifdef CONFIG_VIDEO_HDMI

static void sunxi_hdmi_setup_info_frames(const struct ctfb_res_modes *mode)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	u8 checksum = 0;
	u8 avi_info_frame[17] = {
		0x82, 0x02, 0x0d, 0x00, 0x12, 0x00, 0x88, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00
	};
	u8 vendor_info_frame[19] = {
		0x81, 0x01, 0x06, 0x29, 0x03, 0x0c, 0x00, 0x40,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00
	};
	int i;

	if (mode->pixclock_khz <= 27000)
		avi_info_frame[5] = 0x40; /* SD-modes, ITU601 colorspace */
	else
		avi_info_frame[5] = 0x80; /* HD-modes, ITU709 colorspace */

	if (mode->xres * 100 / mode->yres < 156)
		avi_info_frame[5] |= 0x18; /* 4 : 3 */
	else
		avi_info_frame[5] |= 0x28; /* 16 : 9 */

	for (i = 0; i < ARRAY_SIZE(avi_info_frame); i++)
		checksum += avi_info_frame[i];

	avi_info_frame[3] = 0x100 - checksum;

	for (i = 0; i < ARRAY_SIZE(avi_info_frame); i++)
		writeb(avi_info_frame[i], &hdmi->avi_info_frame[i]);

	writel(SUNXI_HDMI_QCP_PACKET0, &hdmi->qcp_packet0);
	writel(SUNXI_HDMI_QCP_PACKET1, &hdmi->qcp_packet1);

	for (i = 0; i < ARRAY_SIZE(vendor_info_frame); i++)
		writeb(vendor_info_frame[i], &hdmi->vendor_info_frame[i]);

	writel(SUNXI_HDMI_PKT_CTRL0, &hdmi->pkt_ctrl0);
	writel(SUNXI_HDMI_PKT_CTRL1, &hdmi->pkt_ctrl1);

	setbits_le32(&hdmi->video_ctrl, SUNXI_HDMI_VIDEO_CTRL_HDMI);
}

static void sunxi_hdmi_mode_set(const struct ctfb_res_modes *mode,
				int clk_div, int clk_double)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	int x, y;

	/* Write clear interrupt status bits */
	writel(SUNXI_HDMI_IRQ_STATUS_BITS, &hdmi->irq);

	if (sunxi_display.monitor == sunxi_monitor_hdmi)
		sunxi_hdmi_setup_info_frames(mode);

	/* Set input sync enable */
	writel(SUNXI_HDMI_UNKNOWN_INPUT_SYNC, &hdmi->unknown);

	/* Init various registers, select pll3 as clock source */
	writel(SUNXI_HDMI_VIDEO_POL_TX_CLK, &hdmi->video_polarity);
	writel(SUNXI_HDMI_PAD_CTRL0_RUN, &hdmi->pad_ctrl0);
	writel(SUNXI_HDMI_PAD_CTRL1, &hdmi->pad_ctrl1);
	writel(SUNXI_HDMI_PLL_CTRL, &hdmi->pll_ctrl);
	writel(SUNXI_HDMI_PLL_DBG0_PLL3, &hdmi->pll_dbg0);

	/* Setup clk div and doubler */
	clrsetbits_le32(&hdmi->pll_ctrl, SUNXI_HDMI_PLL_CTRL_DIV_MASK,
			SUNXI_HDMI_PLL_CTRL_DIV(clk_div));
	if (!clk_double)
		setbits_le32(&hdmi->pad_ctrl1, SUNXI_HDMI_PAD_CTRL1_HALVE);

	/* Setup timing registers */
	writel(SUNXI_HDMI_Y(mode->yres) | SUNXI_HDMI_X(mode->xres),
	       &hdmi->video_size);

	x = mode->hsync_len + mode->left_margin;
	y = mode->vsync_len + mode->upper_margin;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_bp);

	x = mode->right_margin;
	y = mode->lower_margin;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_fp);

	x = mode->hsync_len;
	y = mode->vsync_len;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_spw);

	if (mode->sync & FB_SYNC_HOR_HIGH_ACT)
		setbits_le32(&hdmi->video_polarity, SUNXI_HDMI_VIDEO_POL_HOR);

	if (mode->sync & FB_SYNC_VERT_HIGH_ACT)
		setbits_le32(&hdmi->video_polarity, SUNXI_HDMI_VIDEO_POL_VER);
}

static void sunxi_hdmi_enable(void)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;

	udelay(100);
	setbits_le32(&hdmi->video_ctrl, SUNXI_HDMI_VIDEO_CTRL_ENABLE);
}

#endif /* CONFIG_VIDEO_HDMI */

#if defined CONFIG_VIDEO_VGA || defined CONFIG_VIDEO_COMPOSITE

static void sunxi_tvencoder_mode_set(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_tve_reg * const tve =
		(struct sunxi_tve_reg *)SUNXI_TVE0_BASE;

	/* Reset off */
	setbits_le32(&ccm->lcd0_ch0_clk_cfg, CCM_LCD_CH0_CTRL_TVE_RST);
	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_TVE0);

	switch (sunxi_display.monitor) {
	case sunxi_monitor_vga:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_VGA, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_VGA, &tve->dac_cfg0);
		writel(SUNXI_TVE_UNKNOWN1_VGA, &tve->unknown1);
		break;
	case sunxi_monitor_composite_pal_nc:
		writel(SUNXI_TVE_CHROMA_FREQ_PAL_NC, &tve->chroma_freq);
		/* Fall through */
	case sunxi_monitor_composite_pal:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(3, 4), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_PAL, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_COMPOSITE, &tve->dac_cfg0);
		writel(SUNXI_TVE_FILTER_COMPOSITE, &tve->filter);
		writel(SUNXI_TVE_PORCH_NUM_PAL, &tve->porch_num);
		writel(SUNXI_TVE_LINE_NUM_PAL, &tve->line_num);
		writel(SUNXI_TVE_BLANK_BLACK_LEVEL_PAL, &tve->blank_black_level);
		writel(SUNXI_TVE_UNKNOWN1_COMPOSITE, &tve->unknown1);
		writel(SUNXI_TVE_CBR_LEVEL_PAL, &tve->cbr_level);
		writel(SUNXI_TVE_BURST_WIDTH_COMPOSITE, &tve->burst_width);
		writel(SUNXI_TVE_UNKNOWN2_PAL, &tve->unknown2);
		writel(SUNXI_TVE_ACTIVE_NUM_COMPOSITE, &tve->active_num);
		writel(SUNXI_TVE_CHROMA_BW_GAIN_COMP, &tve->chroma_bw_gain);
		writel(SUNXI_TVE_NOTCH_WIDTH_COMPOSITE, &tve->notch_width);
		writel(SUNXI_TVE_RESYNC_NUM_PAL, &tve->resync_num);
		writel(SUNXI_TVE_SLAVE_PARA_COMPOSITE, &tve->slave_para);
		break;
	case sunxi_monitor_composite_pal_m:
		writel(SUNXI_TVE_CHROMA_FREQ_PAL_M, &tve->chroma_freq);
		writel(SUNXI_TVE_COLOR_BURST_PAL_M, &tve->color_burst);
		/* Fall through */
	case sunxi_monitor_composite_ntsc:
		writel(SUNXI_TVE_GCTRL_DAC_INPUT(0, 1) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(1, 2) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(2, 3) |
		       SUNXI_TVE_GCTRL_DAC_INPUT(3, 4), &tve->gctrl);
		writel(SUNXI_TVE_CFG0_NTSC, &tve->cfg0);
		writel(SUNXI_TVE_DAC_CFG0_COMPOSITE, &tve->dac_cfg0);
		writel(SUNXI_TVE_FILTER_COMPOSITE, &tve->filter);
		writel(SUNXI_TVE_PORCH_NUM_NTSC, &tve->porch_num);
		writel(SUNXI_TVE_LINE_NUM_NTSC, &tve->line_num);
		writel(SUNXI_TVE_BLANK_BLACK_LEVEL_NTSC, &tve->blank_black_level);
		writel(SUNXI_TVE_UNKNOWN1_COMPOSITE, &tve->unknown1);
		writel(SUNXI_TVE_CBR_LEVEL_NTSC, &tve->cbr_level);
		writel(SUNXI_TVE_BURST_PHASE_NTSC, &tve->burst_phase);
		writel(SUNXI_TVE_BURST_WIDTH_COMPOSITE, &tve->burst_width);
		writel(SUNXI_TVE_UNKNOWN2_NTSC, &tve->unknown2);
		writel(SUNXI_TVE_SYNC_VBI_LEVEL_NTSC, &tve->sync_vbi_level);
		writel(SUNXI_TVE_ACTIVE_NUM_COMPOSITE, &tve->active_num);
		writel(SUNXI_TVE_CHROMA_BW_GAIN_COMP, &tve->chroma_bw_gain);
		writel(SUNXI_TVE_NOTCH_WIDTH_COMPOSITE, &tve->notch_width);
		writel(SUNXI_TVE_RESYNC_NUM_NTSC, &tve->resync_num);
		writel(SUNXI_TVE_SLAVE_PARA_COMPOSITE, &tve->slave_para);
		break;
	case sunxi_monitor_none:
	case sunxi_monitor_dvi:
	case sunxi_monitor_hdmi:
	case sunxi_monitor_lcd:
		break;
	}
}

static void sunxi_tvencoder_enable(void)
{
	struct sunxi_tve_reg * const tve =
		(struct sunxi_tve_reg *)SUNXI_TVE0_BASE;

	setbits_le32(&tve->gctrl, SUNXI_TVE_GCTRL_ENABLE);
}

#endif /* CONFIG_VIDEO_VGA || defined CONFIG_VIDEO_COMPOSITE */

static void sunxi_drc_init(void)
{
#ifdef CONFIG_SUNXI_GEN_SUN6I
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* On sun6i the drc must be clocked even when in pass-through mode */
#ifdef CONFIG_MACH_SUN8I_A33
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_SAT);
#endif
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DRC0);
	clock_set_de_mod_clock(&ccm->iep_drc0_clk_cfg, 300000000);
#endif
}

#ifdef CONFIG_VIDEO_VGA_VIA_LCD
static void sunxi_vga_external_dac_enable(void)
{
	int pin;

	pin = sunxi_name_to_gpio(CONFIG_VIDEO_VGA_EXTERNAL_DAC_EN);
	if (pin >= 0) {
		gpio_request(pin, "vga_enable");
		gpio_direction_output(pin, 1);
	}
}
#endif /* CONFIG_VIDEO_VGA_VIA_LCD */

#ifdef CONFIG_VIDEO_LCD_SSD2828
static int sunxi_ssd2828_init(const struct ctfb_res_modes *mode)
{
	struct ssd2828_config cfg = {
		.csx_pin = name_to_gpio(CONFIG_VIDEO_LCD_SPI_CS),
		.sck_pin = name_to_gpio(CONFIG_VIDEO_LCD_SPI_SCLK),
		.sdi_pin = name_to_gpio(CONFIG_VIDEO_LCD_SPI_MOSI),
		.sdo_pin = name_to_gpio(CONFIG_VIDEO_LCD_SPI_MISO),
		.reset_pin = name_to_gpio(CONFIG_VIDEO_LCD_SSD2828_RESET),
		.ssd2828_tx_clk_khz  = CONFIG_VIDEO_LCD_SSD2828_TX_CLK * 1000,
		.ssd2828_color_depth = 24,
#ifdef CONFIG_VIDEO_LCD_PANEL_MIPI_4_LANE_513_MBPS_VIA_SSD2828
		.mipi_dsi_number_of_data_lanes           = 4,
		.mipi_dsi_bitrate_per_data_lane_mbps     = 513,
		.mipi_dsi_delay_after_exit_sleep_mode_ms = 100,
		.mipi_dsi_delay_after_set_display_on_ms  = 200
#else
#error MIPI LCD panel needs configuration parameters
#endif
	};

	if (cfg.csx_pin == -1 || cfg.sck_pin == -1 || cfg.sdi_pin == -1) {
		printf("SSD2828: SPI pins are not properly configured\n");
		return 1;
	}
	if (cfg.reset_pin == -1) {
		printf("SSD2828: Reset pin is not properly configured\n");
		return 1;
	}

	return ssd2828_init(&cfg, mode);
}
#endif /* CONFIG_VIDEO_LCD_SSD2828 */

static void sunxi_engines_init(void)
{
	sunxi_composer_init();
	sunxi_lcdc_init();
	sunxi_drc_init();
}

static void sunxi_mode_set(const struct ctfb_res_modes *mode,
			   unsigned int address)
{
	int __maybe_unused clk_div, clk_double;

	switch (sunxi_display.monitor) {
	case sunxi_monitor_none:
		break;
	case sunxi_monitor_dvi:
	case sunxi_monitor_hdmi:
#ifdef CONFIG_VIDEO_HDMI
		sunxi_composer_mode_set(mode, address);
		sunxi_lcdc_tcon1_mode_set(mode, &clk_div, &clk_double, 0);
		sunxi_hdmi_mode_set(mode, clk_div, clk_double);
		sunxi_composer_enable();
		sunxi_lcdc_enable();
		sunxi_hdmi_enable();
#endif
		break;
	case sunxi_monitor_lcd:
		sunxi_lcdc_panel_enable();
		if (IS_ENABLED(CONFIG_VIDEO_LCD_PANEL_EDP_4_LANE_1620M_VIA_ANX9804)) {
			/*
			 * The anx9804 needs 1.8V from eldo3, we do this here
			 * and not via CONFIG_AXP_ELDO3_VOLT from board_init()
			 * to avoid turning this on when using hdmi output.
			 */
			axp_set_eldo(3, 1800);
			anx9804_init(CONFIG_VIDEO_LCD_I2C_BUS, 4,
				     ANX9804_DATA_RATE_1620M,
				     sunxi_display.depth);
		}
		if (IS_ENABLED(CONFIG_VIDEO_LCD_HITACHI_TX18D42VM)) {
			mdelay(50); /* Wait for lcd controller power on */
			hitachi_tx18d42vm_init();
		}
		if (IS_ENABLED(CONFIG_VIDEO_LCD_TL059WV5C0)) {
			unsigned int orig_i2c_bus = i2c_get_bus_num();
			i2c_set_bus_num(CONFIG_VIDEO_LCD_I2C_BUS);
			i2c_reg_write(0x5c, 0x04, 0x42); /* Turn on the LCD */
			i2c_set_bus_num(orig_i2c_bus);
		}
		sunxi_composer_mode_set(mode, address);
		sunxi_lcdc_tcon0_mode_set(mode, false);
		sunxi_composer_enable();
		sunxi_lcdc_enable();
#ifdef CONFIG_VIDEO_LCD_SSD2828
		sunxi_ssd2828_init(mode);
#endif
		sunxi_lcdc_backlight_enable();
		break;
	case sunxi_monitor_vga:
#ifdef CONFIG_VIDEO_VGA
		sunxi_composer_mode_set(mode, address);
		sunxi_lcdc_tcon1_mode_set(mode, &clk_div, &clk_double, 1);
		sunxi_tvencoder_mode_set();
		sunxi_composer_enable();
		sunxi_lcdc_enable();
		sunxi_tvencoder_enable();
#elif defined CONFIG_VIDEO_VGA_VIA_LCD
		sunxi_composer_mode_set(mode, address);
		sunxi_lcdc_tcon0_mode_set(mode, true);
		sunxi_composer_enable();
		sunxi_lcdc_enable();
		sunxi_vga_external_dac_enable();
#endif
		break;
	case sunxi_monitor_composite_pal:
	case sunxi_monitor_composite_ntsc:
	case sunxi_monitor_composite_pal_m:
	case sunxi_monitor_composite_pal_nc:
#ifdef CONFIG_VIDEO_COMPOSITE
		sunxi_composer_mode_set(mode, address);
		sunxi_lcdc_tcon1_mode_set(mode, &clk_div, &clk_double, 0);
		sunxi_tvencoder_mode_set();
		sunxi_composer_enable();
		sunxi_lcdc_enable();
		sunxi_tvencoder_enable();
#endif
		break;
	}
}

static const char *sunxi_get_mon_desc(enum sunxi_monitor monitor)
{
	switch (monitor) {
	case sunxi_monitor_none:		return "none";
	case sunxi_monitor_dvi:			return "dvi";
	case sunxi_monitor_hdmi:		return "hdmi";
	case sunxi_monitor_lcd:			return "lcd";
	case sunxi_monitor_vga:			return "vga";
	case sunxi_monitor_composite_pal:	return "composite-pal";
	case sunxi_monitor_composite_ntsc:	return "composite-ntsc";
	case sunxi_monitor_composite_pal_m:	return "composite-pal-m";
	case sunxi_monitor_composite_pal_nc:	return "composite-pal-nc";
	}
	return NULL; /* never reached */
}

ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_top - CONFIG_SUNXI_MAX_FB_SIZE;
}

static bool sunxi_has_hdmi(void)
{
#ifdef CONFIG_VIDEO_HDMI
	return true;
#else
	return false;
#endif
}

static bool sunxi_has_lcd(void)
{
	char *lcd_mode = CONFIG_VIDEO_LCD_MODE;

	return lcd_mode[0] != 0;
}

static bool sunxi_has_vga(void)
{
#if defined CONFIG_VIDEO_VGA || defined CONFIG_VIDEO_VGA_VIA_LCD
	return true;
#else
	return false;
#endif
}

static bool sunxi_has_composite(void)
{
#ifdef CONFIG_VIDEO_COMPOSITE
	return true;
#else
	return false;
#endif
}

static enum sunxi_monitor sunxi_get_default_mon(bool allow_hdmi)
{
	if (allow_hdmi && sunxi_has_hdmi())
		return sunxi_monitor_dvi;
	else if (sunxi_has_lcd())
		return sunxi_monitor_lcd;
	else if (sunxi_has_vga())
		return sunxi_monitor_vga;
	else if (sunxi_has_composite())
		return sunxi_monitor_composite_pal;
	else
		return sunxi_monitor_none;
}

void *video_hw_init(void)
{
	static GraphicDevice *graphic_device = &sunxi_display.graphic_device;
	const struct ctfb_res_modes *mode;
	struct ctfb_res_modes custom;
	const char *options;
#ifdef CONFIG_VIDEO_HDMI
	int ret, hpd, hpd_delay, edid;
#endif
	int i, overscan_offset, overscan_x, overscan_y;
	unsigned int fb_dma_addr;
	char mon[16];
	char *lcd_mode = CONFIG_VIDEO_LCD_MODE;

	memset(&sunxi_display, 0, sizeof(struct sunxi_display));

	video_get_ctfb_res_modes(RES_MODE_1024x768, 24, &mode,
				 &sunxi_display.depth, &options);
#ifdef CONFIG_VIDEO_HDMI
	hpd = video_get_option_int(options, "hpd", 1);
	hpd_delay = video_get_option_int(options, "hpd_delay", 500);
	edid = video_get_option_int(options, "edid", 1);
#endif
	overscan_x = video_get_option_int(options, "overscan_x", -1);
	overscan_y = video_get_option_int(options, "overscan_y", -1);
	sunxi_display.monitor = sunxi_get_default_mon(true);
	video_get_option_string(options, "monitor", mon, sizeof(mon),
				sunxi_get_mon_desc(sunxi_display.monitor));
	for (i = 0; i <= SUNXI_MONITOR_LAST; i++) {
		if (strcmp(mon, sunxi_get_mon_desc(i)) == 0) {
			sunxi_display.monitor = i;
			break;
		}
	}
	if (i > SUNXI_MONITOR_LAST)
		printf("Unknown monitor: '%s', falling back to '%s'\n",
		       mon, sunxi_get_mon_desc(sunxi_display.monitor));

#ifdef CONFIG_VIDEO_HDMI
	/* If HDMI/DVI is selected do HPD & EDID, and handle fallback */
	if (sunxi_display.monitor == sunxi_monitor_dvi ||
	    sunxi_display.monitor == sunxi_monitor_hdmi) {
		/* Always call hdp_detect, as it also enables clocks, etc. */
		ret = sunxi_hdmi_hpd_detect(hpd_delay);
		if (ret) {
			printf("HDMI connected: ");
			if (edid && sunxi_hdmi_edid_get_mode(&custom) == 0)
				mode = &custom;
		} else if (hpd) {
			sunxi_hdmi_shutdown();
			sunxi_display.monitor = sunxi_get_default_mon(false);
		} /* else continue with hdmi/dvi without a cable connected */
	}
#endif

	switch (sunxi_display.monitor) {
	case sunxi_monitor_none:
		return NULL;
	case sunxi_monitor_dvi:
	case sunxi_monitor_hdmi:
		if (!sunxi_has_hdmi()) {
			printf("HDMI/DVI not supported on this board\n");
			sunxi_display.monitor = sunxi_monitor_none;
			return NULL;
		}
		break;
	case sunxi_monitor_lcd:
		if (!sunxi_has_lcd()) {
			printf("LCD not supported on this board\n");
			sunxi_display.monitor = sunxi_monitor_none;
			return NULL;
		}
		sunxi_display.depth = video_get_params(&custom, lcd_mode);
		mode = &custom;
		break;
	case sunxi_monitor_vga:
		if (!sunxi_has_vga()) {
			printf("VGA not supported on this board\n");
			sunxi_display.monitor = sunxi_monitor_none;
			return NULL;
		}
		sunxi_display.depth = 18;
		break;
	case sunxi_monitor_composite_pal:
	case sunxi_monitor_composite_ntsc:
	case sunxi_monitor_composite_pal_m:
	case sunxi_monitor_composite_pal_nc:
		if (!sunxi_has_composite()) {
			printf("Composite video not supported on this board\n");
			sunxi_display.monitor = sunxi_monitor_none;
			return NULL;
		}
		if (sunxi_display.monitor == sunxi_monitor_composite_pal ||
		    sunxi_display.monitor == sunxi_monitor_composite_pal_nc)
			mode = &composite_video_modes[0];
		else
			mode = &composite_video_modes[1];
		sunxi_display.depth = 24;
		break;
	}

	/* Yes these defaults are quite high, overscan on composite sucks... */
	if (overscan_x == -1)
		overscan_x = sunxi_is_composite() ? 32 : 0;
	if (overscan_y == -1)
		overscan_y = sunxi_is_composite() ? 20 : 0;

	sunxi_display.fb_size =
		(mode->xres * mode->yres * 4 + 0xfff) & ~0xfff;
	overscan_offset = (overscan_y * mode->xres + overscan_x) * 4;
	/* We want to keep the fb_base for simplefb page aligned, where as
	 * the sunxi dma engines will happily accept an unaligned address. */
	if (overscan_offset)
		sunxi_display.fb_size += 0x1000;

	if (sunxi_display.fb_size > CONFIG_SUNXI_MAX_FB_SIZE) {
		printf("Error need %dkB for fb, but only %dkB is reserved\n",
		       sunxi_display.fb_size >> 10,
		       CONFIG_SUNXI_MAX_FB_SIZE >> 10);
		return NULL;
	}

	printf("Setting up a %dx%d%s %s console (overscan %dx%d)\n",
	       mode->xres, mode->yres,
	       (mode->vmode == FB_VMODE_INTERLACED) ? "i" : "",
	       sunxi_get_mon_desc(sunxi_display.monitor),
	       overscan_x, overscan_y);

	gd->fb_base = gd->bd->bi_dram[0].start +
		      gd->bd->bi_dram[0].size - sunxi_display.fb_size;
	sunxi_engines_init();

	fb_dma_addr = gd->fb_base - CONFIG_SYS_SDRAM_BASE;
	sunxi_display.fb_addr = gd->fb_base;
	if (overscan_offset) {
		fb_dma_addr += 0x1000 - (overscan_offset & 0xfff);
		sunxi_display.fb_addr += (overscan_offset + 0xfff) & ~0xfff;
		memset((void *)gd->fb_base, 0, sunxi_display.fb_size);
		flush_cache(gd->fb_base, sunxi_display.fb_size);
	}
	sunxi_mode_set(mode, fb_dma_addr);

	/*
	 * These are the only members of this structure that are used. All the
	 * others are driver specific. The pitch is stored in plnSizeX.
	 */
	graphic_device->frameAdrs = sunxi_display.fb_addr;
	graphic_device->gdfIndex = GDF_32BIT_X888RGB;
	graphic_device->gdfBytesPP = 4;
	graphic_device->winSizeX = mode->xres - 2 * overscan_x;
	graphic_device->winSizeY = mode->yres - 2 * overscan_y;
	graphic_device->plnSizeX = mode->xres * graphic_device->gdfBytesPP;

	return graphic_device;
}

/*
 * Simplefb support.
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_VIDEO_DT_SIMPLEFB)
int sunxi_simplefb_setup(void *blob)
{
	static GraphicDevice *graphic_device = &sunxi_display.graphic_device;
	int offset, ret;
	u64 start, size;
	const char *pipeline = NULL;

#ifdef CONFIG_MACH_SUN4I
#define PIPELINE_PREFIX "de_fe0-"
#else
#define PIPELINE_PREFIX
#endif

	switch (sunxi_display.monitor) {
	case sunxi_monitor_none:
		return 0;
	case sunxi_monitor_dvi:
	case sunxi_monitor_hdmi:
		pipeline = PIPELINE_PREFIX "de_be0-lcd0-hdmi";
		break;
	case sunxi_monitor_lcd:
		pipeline = PIPELINE_PREFIX "de_be0-lcd0";
		break;
	case sunxi_monitor_vga:
#ifdef CONFIG_VIDEO_VGA
		pipeline = PIPELINE_PREFIX "de_be0-lcd0-tve0";
#elif defined CONFIG_VIDEO_VGA_VIA_LCD
		pipeline = PIPELINE_PREFIX "de_be0-lcd0";
#endif
		break;
	case sunxi_monitor_composite_pal:
	case sunxi_monitor_composite_ntsc:
	case sunxi_monitor_composite_pal_m:
	case sunxi_monitor_composite_pal_nc:
		pipeline = PIPELINE_PREFIX "de_be0-lcd0-tve0";
		break;
	}

	/* Find a prefilled simpefb node, matching out pipeline config */
	offset = fdt_node_offset_by_compatible(blob, -1,
					       "allwinner,simple-framebuffer");
	while (offset >= 0) {
		ret = fdt_find_string(blob, offset, "allwinner,pipeline",
				      pipeline);
		if (ret == 0)
			break;
		offset = fdt_node_offset_by_compatible(blob, offset,
					       "allwinner,simple-framebuffer");
	}
	if (offset < 0) {
		eprintf("Cannot setup simplefb: node not found\n");
		return 0; /* Keep older kernels working */
	}

	/*
	 * Do not report the framebuffer as free RAM to the OS, note we cannot
	 * use fdt_add_mem_rsv() here, because then it is still seen as RAM,
	 * and e.g. Linux refuses to iomap RAM on ARM, see:
	 * linux/arch/arm/mm/ioremap.c around line 301.
	 */
	start = gd->bd->bi_dram[0].start;
	size = gd->bd->bi_dram[0].size - sunxi_display.fb_size;
	ret = fdt_fixup_memory_banks(blob, &start, &size, 1);
	if (ret) {
		eprintf("Cannot setup simplefb: Error reserving memory\n");
		return ret;
	}

	ret = fdt_setup_simplefb_node(blob, offset, sunxi_display.fb_addr,
			graphic_device->winSizeX, graphic_device->winSizeY,
			graphic_device->plnSizeX, "x8r8g8b8");
	if (ret)
		eprintf("Cannot setup simplefb: Error setting properties\n");

	return ret;
}
#endif /* CONFIG_OF_BOARD_SETUP && CONFIG_VIDEO_DT_SIMPLEFB */
