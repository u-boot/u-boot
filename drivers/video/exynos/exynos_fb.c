/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <lcd.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/system.h>
#include <asm/gpio.h>
#include <asm-generic/errno.h>

#include "exynos_fb.h"

DECLARE_GLOBAL_DATA_PTR;

struct vidinfo panel_info  = {
	/*
	 * Insert a value here so that we don't end up in the BSS
	 * Reference: drivers/video/tegra.c
	 */
	.vl_col = -1,
};

static void exynos_fimd_set_dualrgb(struct vidinfo *pvid, unsigned int enabled)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	if (enabled) {
		cfg = EXYNOS_DUALRGB_BYPASS_DUAL | EXYNOS_DUALRGB_LINESPLIT |
			EXYNOS_DUALRGB_VDEN_EN_ENABLE;

		/* in case of Line Split mode, MAIN_CNT doesn't neet to set. */
		cfg |= EXYNOS_DUALRGB_SUB_CNT(pvid->vl_col / 2) |
			EXYNOS_DUALRGB_MAIN_CNT(0);
	}

	writel(cfg, &fimd_ctrl->dualrgb);
}

static void exynos_fimd_set_dp_clkcon(struct vidinfo *pvid,
				      unsigned int enabled)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	if (enabled)
		cfg = EXYNOS_DP_CLK_ENABLE;

	writel(cfg, &fimd_ctrl->dp_mie_clkcon);
}

static void exynos_fimd_set_par(struct vidinfo *pvid, unsigned int win_id)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	/* set window control */
	cfg = readl((unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg &= ~(EXYNOS_WINCON_BITSWP_ENABLE | EXYNOS_WINCON_BYTESWP_ENABLE |
		EXYNOS_WINCON_HAWSWP_ENABLE | EXYNOS_WINCON_WSWP_ENABLE |
		EXYNOS_WINCON_BURSTLEN_MASK | EXYNOS_WINCON_BPPMODE_MASK |
		EXYNOS_WINCON_INRGB_MASK | EXYNOS_WINCON_DATAPATH_MASK);

	/* DATAPATH is DMA */
	cfg |= EXYNOS_WINCON_DATAPATH_DMA;

	cfg |= EXYNOS_WINCON_HAWSWP_ENABLE;

	/* dma burst is 16 */
	cfg |= EXYNOS_WINCON_BURSTLEN_16WORD;

	switch (pvid->vl_bpix) {
	case 4:
		cfg |= EXYNOS_WINCON_BPPMODE_16BPP_565;
		break;
	default:
		cfg |= EXYNOS_WINCON_BPPMODE_24BPP_888;
		break;
	}

	writel(cfg, (unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));

	/* set window position to x=0, y=0*/
	cfg = EXYNOS_VIDOSD_LEFT_X(0) | EXYNOS_VIDOSD_TOP_Y(0);
	writel(cfg, (unsigned int)&fimd_ctrl->vidosd0a +
			EXYNOS_VIDOSD(win_id));

	cfg = EXYNOS_VIDOSD_RIGHT_X(pvid->vl_col - 1) |
		EXYNOS_VIDOSD_BOTTOM_Y(pvid->vl_row - 1) |
		EXYNOS_VIDOSD_RIGHT_X_E(1) |
		EXYNOS_VIDOSD_BOTTOM_Y_E(0);

	writel(cfg, (unsigned int)&fimd_ctrl->vidosd0b +
			EXYNOS_VIDOSD(win_id));

	/* set window size for window0*/
	cfg = EXYNOS_VIDOSD_SIZE(pvid->vl_col * pvid->vl_row);
	writel(cfg, (unsigned int)&fimd_ctrl->vidosd0c +
			EXYNOS_VIDOSD(win_id));
}

static void exynos_fimd_set_buffer_address(struct vidinfo *pvid,
					   unsigned int win_id,
					   ulong lcd_base_addr)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned long start_addr, end_addr;

	start_addr = lcd_base_addr;
	end_addr = start_addr + ((pvid->vl_col * (NBITS(pvid->vl_bpix) / 8)) *
				pvid->vl_row);

	writel(start_addr, (unsigned int)&fimd_ctrl->vidw00add0b0 +
			EXYNOS_BUFFER_OFFSET(win_id));
	writel(end_addr, (unsigned int)&fimd_ctrl->vidw00add1b0 +
			EXYNOS_BUFFER_OFFSET(win_id));
}

static void exynos_fimd_set_clock(struct vidinfo *pvid)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0, div = 0, remainder, remainder_div;
	unsigned long pixel_clock;
	unsigned long long src_clock;

	if (pvid->dual_lcd_enabled) {
		pixel_clock = pvid->vl_freq *
				(pvid->vl_hspw + pvid->vl_hfpd +
				 pvid->vl_hbpd + pvid->vl_col / 2) *
				(pvid->vl_vspw + pvid->vl_vfpd +
				 pvid->vl_vbpd + pvid->vl_row);
	} else if (pvid->interface_mode == FIMD_CPU_INTERFACE) {
		pixel_clock = pvid->vl_freq *
				pvid->vl_width * pvid->vl_height *
				(pvid->cs_setup + pvid->wr_setup +
				 pvid->wr_act + pvid->wr_hold + 1);
	} else {
		pixel_clock = pvid->vl_freq *
				(pvid->vl_hspw + pvid->vl_hfpd +
				 pvid->vl_hbpd + pvid->vl_col) *
				(pvid->vl_vspw + pvid->vl_vfpd +
				 pvid->vl_vbpd + pvid->vl_row);
	}

	cfg = readl(&fimd_ctrl->vidcon0);
	cfg &= ~(EXYNOS_VIDCON0_CLKSEL_MASK | EXYNOS_VIDCON0_CLKVALUP_MASK |
		EXYNOS_VIDCON0_CLKVAL_F(0xFF) | EXYNOS_VIDCON0_VCLKEN_MASK |
		EXYNOS_VIDCON0_CLKDIR_MASK);
	cfg |= (EXYNOS_VIDCON0_CLKSEL_SCLK | EXYNOS_VIDCON0_CLKVALUP_ALWAYS |
		EXYNOS_VIDCON0_VCLKEN_NORMAL | EXYNOS_VIDCON0_CLKDIR_DIVIDED);

	src_clock = (unsigned long long) get_lcd_clk();

	/* get quotient and remainder. */
	remainder = do_div(src_clock, pixel_clock);
	div = src_clock;

	remainder *= 10;
	remainder_div = remainder / pixel_clock;

	/* round about one places of decimals. */
	if (remainder_div >= 5)
		div++;

	/* in case of dual lcd mode. */
	if (pvid->dual_lcd_enabled)
		div--;

	cfg |= EXYNOS_VIDCON0_CLKVAL_F(div - 1);
	writel(cfg, &fimd_ctrl->vidcon0);
}

void exynos_set_trigger(struct vidinfo *pvid)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	cfg = readl(&fimd_ctrl->trigcon);

	cfg |= (EXYNOS_I80SOFT_TRIG_EN | EXYNOS_I80START_TRIG);

	writel(cfg, &fimd_ctrl->trigcon);
}

int exynos_is_i80_frame_done(struct vidinfo *pvid)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;
	int status;

	cfg = readl(&fimd_ctrl->trigcon);

	/* frame done func is valid only when TRIMODE[0] is set to 1. */
	status = (cfg & EXYNOS_I80STATUS_TRIG_DONE) ==
			EXYNOS_I80STATUS_TRIG_DONE;

	return status;
}

static void exynos_fimd_lcd_on(struct vidinfo *pvid)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	/* display on */
	cfg = readl(&fimd_ctrl->vidcon0);
	cfg |= (EXYNOS_VIDCON0_ENVID_ENABLE | EXYNOS_VIDCON0_ENVID_F_ENABLE);
	writel(cfg, &fimd_ctrl->vidcon0);
}

static void exynos_fimd_window_on(struct vidinfo *pvid, unsigned int win_id)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	/* enable window */
	cfg = readl((unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));
	cfg |= EXYNOS_WINCON_ENWIN_ENABLE;
	writel(cfg, (unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg = readl(&fimd_ctrl->winshmap);
	cfg |= EXYNOS_WINSHMAP_CH_ENABLE(win_id);
	writel(cfg, &fimd_ctrl->winshmap);
}

void exynos_fimd_lcd_off(struct vidinfo *pvid)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	cfg = readl(&fimd_ctrl->vidcon0);
	cfg &= (EXYNOS_VIDCON0_ENVID_DISABLE | EXYNOS_VIDCON0_ENVID_F_DISABLE);
	writel(cfg, &fimd_ctrl->vidcon0);
}

void exynos_fimd_window_off(struct vidinfo *pvid, unsigned int win_id)
{
	struct exynos_fb *fimd_ctrl = pvid->fimd_ctrl;
	unsigned int cfg = 0;

	cfg = readl((unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));
	cfg &= EXYNOS_WINCON_ENWIN_DISABLE;
	writel(cfg, (unsigned int)&fimd_ctrl->wincon0 +
			EXYNOS_WINCON(win_id));

	cfg = readl(&fimd_ctrl->winshmap);
	cfg &= ~EXYNOS_WINSHMAP_CH_DISABLE(win_id);
	writel(cfg, &fimd_ctrl->winshmap);
}

/*
* The reset value for FIMD SYSMMU register MMU_CTRL is 3
* on Exynos5420 and newer versions.
* This means FIMD SYSMMU is on by default on Exynos5420
* and newer versions.
* Since in u-boot we don't use SYSMMU, we should disable
* those FIMD SYSMMU.
* Note that there are 2 SYSMMU for FIMD: m0 and m1.
* m0 handles windows 0 and 4, and m1 handles windows 1, 2 and 3.
* We disable both of them here.
*/
void exynos_fimd_disable_sysmmu(void)
{
	u32 *sysmmufimd;
	unsigned int node;
	int node_list[2];
	int count;
	int i;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "fimd",
				COMPAT_SAMSUNG_EXYNOS_SYSMMU, node_list, 2);
	for (i = 0; i < count; i++) {
		node = node_list[i];
		if (node <= 0) {
			debug("Can't get device node for fimd sysmmu\n");
			return;
		}

		sysmmufimd = (u32 *)fdtdec_get_addr(gd->fdt_blob, node, "reg");
		if (!sysmmufimd) {
			debug("Can't get base address for sysmmu fimdm0");
			return;
		}

		writel(0x0, sysmmufimd);
	}
}

void exynos_fimd_lcd_init(struct vidinfo *pvid, ulong lcd_base_address)
{
	struct exynos_fb *fimd_ctrl;
	unsigned int cfg = 0, rgb_mode;
	unsigned int offset;
	unsigned int node;

	node = fdtdec_next_compatible(gd->fdt_blob,
					0, COMPAT_SAMSUNG_EXYNOS_FIMD);
	if (node <= 0)
		debug("exynos_fb: Can't get device node for fimd\n");

	fimd_ctrl = (struct exynos_fb *)fdtdec_get_addr(gd->fdt_blob, node,
							"reg");
	if (fimd_ctrl == NULL)
		debug("Can't get the FIMD base address\n");
	pvid->fimd_ctrl = fimd_ctrl;

	if (fdtdec_get_bool(gd->fdt_blob, node, "samsung,disable-sysmmu"))
		exynos_fimd_disable_sysmmu();

	offset = exynos_fimd_get_base_offset();

	rgb_mode = pvid->rgb_mode;

	if (pvid->interface_mode == FIMD_RGB_INTERFACE) {
		cfg |= EXYNOS_VIDCON0_VIDOUT_RGB;
		writel(cfg, &fimd_ctrl->vidcon0);

		cfg = readl(&fimd_ctrl->vidcon2);
		cfg &= ~(EXYNOS_VIDCON2_WB_MASK |
			EXYNOS_VIDCON2_TVFORMATSEL_MASK |
			EXYNOS_VIDCON2_TVFORMATSEL_YUV_MASK);
		cfg |= EXYNOS_VIDCON2_WB_DISABLE;
		writel(cfg, &fimd_ctrl->vidcon2);

		/* set polarity */
		cfg = 0;
		if (!pvid->vl_clkp)
			cfg |= EXYNOS_VIDCON1_IVCLK_RISING_EDGE;
		if (!pvid->vl_hsp)
			cfg |= EXYNOS_VIDCON1_IHSYNC_INVERT;
		if (!pvid->vl_vsp)
			cfg |= EXYNOS_VIDCON1_IVSYNC_INVERT;
		if (!pvid->vl_dp)
			cfg |= EXYNOS_VIDCON1_IVDEN_INVERT;

		writel(cfg, (unsigned int)&fimd_ctrl->vidcon1 + offset);

		/* set timing */
		cfg = EXYNOS_VIDTCON0_VFPD(pvid->vl_vfpd - 1);
		cfg |= EXYNOS_VIDTCON0_VBPD(pvid->vl_vbpd - 1);
		cfg |= EXYNOS_VIDTCON0_VSPW(pvid->vl_vspw - 1);
		writel(cfg, (unsigned int)&fimd_ctrl->vidtcon0 + offset);

		cfg = EXYNOS_VIDTCON1_HFPD(pvid->vl_hfpd - 1);
		cfg |= EXYNOS_VIDTCON1_HBPD(pvid->vl_hbpd - 1);
		cfg |= EXYNOS_VIDTCON1_HSPW(pvid->vl_hspw - 1);

		writel(cfg, (unsigned int)&fimd_ctrl->vidtcon1 + offset);

		/* set lcd size */
		cfg = EXYNOS_VIDTCON2_HOZVAL(pvid->vl_col - 1) |
			EXYNOS_VIDTCON2_LINEVAL(pvid->vl_row - 1) |
			EXYNOS_VIDTCON2_HOZVAL_E(pvid->vl_col - 1) |
			EXYNOS_VIDTCON2_LINEVAL_E(pvid->vl_row - 1);

		writel(cfg, (unsigned int)&fimd_ctrl->vidtcon2 + offset);
	}

	/* set display mode */
	cfg = readl(&fimd_ctrl->vidcon0);
	cfg &= ~EXYNOS_VIDCON0_PNRMODE_MASK;
	cfg |= (rgb_mode << EXYNOS_VIDCON0_PNRMODE_SHIFT);
	writel(cfg, &fimd_ctrl->vidcon0);

	/* set par */
	exynos_fimd_set_par(pvid, pvid->win_id);

	/* set memory address */
	exynos_fimd_set_buffer_address(pvid, pvid->win_id, lcd_base_address);

	/* set buffer size */
	cfg = EXYNOS_VIDADDR_PAGEWIDTH(pvid->vl_col *
			NBITS(pvid->vl_bpix) / 8) |
		EXYNOS_VIDADDR_PAGEWIDTH_E(pvid->vl_col *
			NBITS(pvid->vl_bpix) / 8) |
		EXYNOS_VIDADDR_OFFSIZE(0) |
		EXYNOS_VIDADDR_OFFSIZE_E(0);

	writel(cfg, (unsigned int)&fimd_ctrl->vidw00add2 +
					EXYNOS_BUFFER_SIZE(pvid->win_id));

	/* set clock */
	exynos_fimd_set_clock(pvid);

	/* set rgb mode to dual lcd. */
	exynos_fimd_set_dualrgb(pvid, pvid->dual_lcd_enabled);

	/* display on */
	exynos_fimd_lcd_on(pvid);

	/* window on */
	exynos_fimd_window_on(pvid, pvid->win_id);

	exynos_fimd_set_dp_clkcon(pvid, pvid->dp_enabled);
}

unsigned long exynos_fimd_calc_fbsize(struct vidinfo *pvid)
{
	return pvid->vl_col * pvid->vl_row * (NBITS(pvid->vl_bpix) / 8);
}

ushort *configuration_get_cmap(void)
{
#if defined(CONFIG_LCD_LOGO)
	return bmp_logo_palette;
#else
	return NULL;
#endif
}

static void exynos_lcd_init(struct vidinfo *vid, ulong lcd_base)
{
	exynos_fimd_lcd_init(vid, lcd_base);

	/* Enable flushing after LCD writes if requested */
	lcd_set_flush_dcache(1);
}

__weak void exynos_cfg_lcd_gpio(void)
{
}

__weak void exynos_backlight_on(unsigned int onoff)
{
}

__weak void exynos_reset_lcd(void)
{
}

__weak void exynos_lcd_power_on(void)
{
}

__weak void exynos_cfg_ldo(void)
{
}

__weak void exynos_enable_ldo(unsigned int onoff)
{
}

__weak void exynos_backlight_reset(void)
{
}

__weak int exynos_lcd_misc_init(struct vidinfo *vid)
{
	return 0;
}

static void lcd_panel_on(struct vidinfo *vid)
{
	struct gpio_desc pwm_out_gpio;
	struct gpio_desc bl_en_gpio;
	unsigned int node;

	udelay(vid->init_delay);

	exynos_backlight_reset();

	exynos_cfg_lcd_gpio();

	exynos_lcd_power_on();

	udelay(vid->power_on_delay);

	if (vid->dp_enabled)
		exynos_init_dp();

	exynos_reset_lcd();

	udelay(vid->reset_delay);

	exynos_backlight_on(1);

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
						COMPAT_SAMSUNG_EXYNOS_FIMD);
	if (node <= 0) {
		debug("FIMD: Can't get device node for FIMD\n");
		return;
	}
	gpio_request_by_name_nodev(gd->fdt_blob, node, "samsung,pwm-out-gpio",
				   0, &pwm_out_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	gpio_request_by_name_nodev(gd->fdt_blob, node, "samsung,bl-en-gpio", 0,
				   &bl_en_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	exynos_cfg_ldo();

	exynos_enable_ldo(1);

	if (vid->mipi_enabled)
		exynos_mipi_dsi_init(panel_info.dsim_platform_data_dt);
}

int exynos_lcd_early_init(const void *blob)
{
	unsigned int node;
	node = fdtdec_next_compatible(blob, 0, COMPAT_SAMSUNG_EXYNOS_FIMD);
	if (node <= 0) {
		debug("exynos_fb: Can't get device node for fimd\n");
		return -ENODEV;
	}

	panel_info.vl_col = fdtdec_get_int(blob, node, "samsung,vl-col", 0);
	if (panel_info.vl_col == 0) {
		debug("Can't get XRES\n");
		return -ENXIO;
	}

	panel_info.vl_row = fdtdec_get_int(blob, node, "samsung,vl-row", 0);
	if (panel_info.vl_row == 0) {
		debug("Can't get YRES\n");
		return -ENXIO;
	}

	panel_info.vl_width = fdtdec_get_int(blob, node,
						"samsung,vl-width", 0);

	panel_info.vl_height = fdtdec_get_int(blob, node,
						"samsung,vl-height", 0);

	panel_info.vl_freq = fdtdec_get_int(blob, node, "samsung,vl-freq", 0);
	if (panel_info.vl_freq == 0) {
		debug("Can't get refresh rate\n");
		return -ENXIO;
	}

	if (fdtdec_get_bool(blob, node, "samsung,vl-clkp"))
		panel_info.vl_clkp = CONFIG_SYS_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-oep"))
		panel_info.vl_oep = CONFIG_SYS_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-hsp"))
		panel_info.vl_hsp = CONFIG_SYS_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-vsp"))
		panel_info.vl_vsp = CONFIG_SYS_LOW;

	if (fdtdec_get_bool(blob, node, "samsung,vl-dp"))
		panel_info.vl_dp = CONFIG_SYS_LOW;

	panel_info.vl_bpix = fdtdec_get_int(blob, node, "samsung,vl-bpix", 0);
	if (panel_info.vl_bpix == 0) {
		debug("Can't get bits per pixel\n");
		return -ENXIO;
	}

	panel_info.vl_hspw = fdtdec_get_int(blob, node, "samsung,vl-hspw", 0);
	if (panel_info.vl_hspw == 0) {
		debug("Can't get hsync width\n");
		return -ENXIO;
	}

	panel_info.vl_hfpd = fdtdec_get_int(blob, node, "samsung,vl-hfpd", 0);
	if (panel_info.vl_hfpd == 0) {
		debug("Can't get right margin\n");
		return -ENXIO;
	}

	panel_info.vl_hbpd = (u_char)fdtdec_get_int(blob, node,
							"samsung,vl-hbpd", 0);
	if (panel_info.vl_hbpd == 0) {
		debug("Can't get left margin\n");
		return -ENXIO;
	}

	panel_info.vl_vspw = (u_char)fdtdec_get_int(blob, node,
							"samsung,vl-vspw", 0);
	if (panel_info.vl_vspw == 0) {
		debug("Can't get vsync width\n");
		return -ENXIO;
	}

	panel_info.vl_vfpd = fdtdec_get_int(blob, node,
							"samsung,vl-vfpd", 0);
	if (panel_info.vl_vfpd == 0) {
		debug("Can't get lower margin\n");
		return -ENXIO;
	}

	panel_info.vl_vbpd = fdtdec_get_int(blob, node, "samsung,vl-vbpd", 0);
	if (panel_info.vl_vbpd == 0) {
		debug("Can't get upper margin\n");
		return -ENXIO;
	}

	panel_info.vl_cmd_allow_len = fdtdec_get_int(blob, node,
						"samsung,vl-cmd-allow-len", 0);

	panel_info.win_id = fdtdec_get_int(blob, node, "samsung,winid", 0);
	panel_info.init_delay = fdtdec_get_int(blob, node,
						"samsung,init-delay", 0);
	panel_info.power_on_delay = fdtdec_get_int(blob, node,
						"samsung,power-on-delay", 0);
	panel_info.reset_delay = fdtdec_get_int(blob, node,
						"samsung,reset-delay", 0);
	panel_info.interface_mode = fdtdec_get_int(blob, node,
						"samsung,interface-mode", 0);
	panel_info.mipi_enabled = fdtdec_get_int(blob, node,
						"samsung,mipi-enabled", 0);
	panel_info.dp_enabled = fdtdec_get_int(blob, node,
						"samsung,dp-enabled", 0);
	panel_info.cs_setup = fdtdec_get_int(blob, node,
						"samsung,cs-setup", 0);
	panel_info.wr_setup = fdtdec_get_int(blob, node,
						"samsung,wr-setup", 0);
	panel_info.wr_act = fdtdec_get_int(blob, node, "samsung,wr-act", 0);
	panel_info.wr_hold = fdtdec_get_int(blob, node, "samsung,wr-hold", 0);

	panel_info.logo_on = fdtdec_get_int(blob, node, "samsung,logo-on", 0);
	if (panel_info.logo_on) {
		panel_info.logo_width = fdtdec_get_int(blob, node,
						"samsung,logo-width", 0);
		panel_info.logo_height = fdtdec_get_int(blob, node,
						"samsung,logo-height", 0);
		panel_info.logo_addr = fdtdec_get_int(blob, node,
						"samsung,logo-addr", 0);
	}

	panel_info.rgb_mode = fdtdec_get_int(blob, node,
						"samsung,rgb-mode", 0);
	panel_info.pclk_name = fdtdec_get_int(blob, node,
						"samsung,pclk-name", 0);
	panel_info.sclk_div = fdtdec_get_int(blob, node,
						"samsung,sclk-div", 0);
	panel_info.dual_lcd_enabled = fdtdec_get_int(blob, node,
						"samsung,dual-lcd-enabled", 0);

	return 0;
}

void lcd_ctrl_init(void *lcdbase)
{
	set_system_display_ctrl();
	set_lcd_clk();

#ifdef CONFIG_EXYNOS_MIPI_DSIM
	exynos_init_dsim_platform_data(&panel_info);
#endif
	exynos_lcd_misc_init(&panel_info);

	exynos_lcd_init(&panel_info, (ulong)lcdbase);
}

void lcd_enable(void)
{
	if (panel_info.logo_on) {
		memset((void *)gd->fb_base, 0,
		       panel_info.vl_width * panel_info.vl_height *
				(NBITS(panel_info.vl_bpix) >> 3));
	}

	lcd_panel_on(&panel_info);
}

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}
