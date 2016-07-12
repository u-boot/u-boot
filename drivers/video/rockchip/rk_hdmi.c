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
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/hdmi_rk3288.h>
#include <power/regulator.h>

struct tmds_n_cts {
	u32 tmds;
	u32 cts;
	u32 n;
};

struct rk_hdmi_priv {
	struct rk3288_hdmi *regs;
	struct rk3288_grf *grf;
};

static const struct tmds_n_cts n_cts_table[] = {
	{
		.tmds = 25175, .n = 6144, .cts = 25175,
	}, {
		.tmds = 25200, .n = 6144, .cts = 25200,
	}, {
		.tmds = 27000, .n = 6144, .cts = 27000,
	}, {
		.tmds = 27027, .n = 6144, .cts = 27027,
	}, {
		.tmds = 40000, .n = 6144, .cts = 40000,
	}, {
		.tmds = 54000, .n = 6144, .cts = 54000,
	}, {
		.tmds = 54054, .n = 6144, .cts = 54054,
	}, {
		.tmds = 65000, .n = 6144, .cts = 65000,
	}, {
		.tmds = 74176, .n = 11648, .cts = 140625,
	}, {
		.tmds = 74250, .n = 6144, .cts = 74250,
	}, {
		.tmds = 83500, .n = 6144, .cts = 83500,
	}, {
		.tmds = 106500, .n = 6144, .cts = 106500,
	}, {
		.tmds = 108000, .n = 6144, .cts = 108000,
	}, {
		.tmds = 148352, .n = 5824, .cts = 140625,
	}, {
		.tmds = 148500, .n = 6144, .cts = 148500,
	}, {
		.tmds = 297000, .n = 5120, .cts = 247500,
	}
};

struct hdmi_mpll_config {
	u64 mpixelclock;
	/* Mode of Operation and PLL Dividers Control Register */
	u32 cpce;
	/* PLL Gmp Control Register */
	u32 gmp;
	/* PLL Current COntrol Register */
	u32 curr;
};

struct hdmi_phy_config {
	u64 mpixelclock;
	u32 sym_ctr;    /* clock symbol and transmitter control */
	u32 term;       /* transmission termination value */
	u32 vlev_ctr;   /* voltage level control */
};

static const struct hdmi_phy_config rockchip_phy_config[] = {
	{
		.mpixelclock = 74250,
		.sym_ctr = 0x8009, .term = 0x0004, .vlev_ctr = 0x0272,
	}, {
		.mpixelclock = 148500,
		.sym_ctr = 0x802b, .term = 0x0004, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = 297000,
		.sym_ctr = 0x8039, .term = 0x0005, .vlev_ctr = 0x028d,
	}, {
		.mpixelclock = ~0ul,
		.sym_ctr = 0x0000, .term = 0x0000, .vlev_ctr = 0x0000,
	}
};

static const struct hdmi_mpll_config rockchip_mpll_cfg[] = {
	{
		.mpixelclock = 40000,
		.cpce = 0x00b3, .gmp = 0x0000, .curr = 0x0018,
	}, {
		.mpixelclock = 65000,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 66000,
		.cpce = 0x013e, .gmp = 0x0003, .curr = 0x0038,
	}, {
		.mpixelclock = 83500,
		.cpce = 0x0072, .gmp = 0x0001, .curr = 0x0028,
	}, {
		.mpixelclock = 146250,
		.cpce = 0x0051, .gmp = 0x0002, .curr = 0x0038,
	}, {
		.mpixelclock = 148500,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}, {
		.mpixelclock = ~0ul,
		.cpce = 0x0051, .gmp = 0x0003, .curr = 0x0000,
	}
};

static const u32 csc_coeff_default[3][4] = {
	{ 0x2000, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x2000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x2000, 0x0000 }
};

static void hdmi_set_clock_regenerator(struct rk3288_hdmi *regs, u32 n, u32 cts)
{
	u8 cts3;
	u8 n3;

	/* first set ncts_atomic_write (if present) */
	n3 = HDMI_AUD_N3_NCTS_ATOMIC_WRITE;
	writel(n3, &regs->aud_n3);

	/* set cts_manual (if present) */
	cts3 = HDMI_AUD_CTS3_CTS_MANUAL;

	cts3 |= HDMI_AUD_CTS3_N_SHIFT_1 << HDMI_AUD_CTS3_N_SHIFT_OFFSET;
	cts3 |= (cts >> 16) & HDMI_AUD_CTS3_AUDCTS19_16_MASK;

	/* write cts values; cts3 must be written first */
	writel(cts3, &regs->aud_cts3);
	writel((cts >> 8) & 0xff, &regs->aud_cts2);
	writel(cts & 0xff, &regs->aud_cts1);

	/* write n values; n1 must be written last */
	n3 |= (n >> 16) & HDMI_AUD_N3_AUDN19_16_MASK;
	writel(n3, &regs->aud_n3);
	writel((n >> 8) & 0xff, &regs->aud_n2);
	writel(n & 0xff, &regs->aud_n1);

	writel(HDMI_AUD_INPUTCLKFS_128, &regs->aud_inputclkfs);
}

static int hdmi_lookup_n_cts(u32 pixel_clk)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(n_cts_table); i++)
		if (pixel_clk <= n_cts_table[i].tmds)
			break;

	if (i >= ARRAY_SIZE(n_cts_table))
		return -1;

	return i;
}

static void hdmi_audio_set_samplerate(struct rk3288_hdmi *regs, u32 pixel_clk)
{
	u32 clk_n, clk_cts;
	int index;

	index = hdmi_lookup_n_cts(pixel_clk);
	if (index == -1) {
		debug("audio not supported for pixel clk %d\n", pixel_clk);
		return;
	}

	clk_n = n_cts_table[index].n;
	clk_cts = n_cts_table[index].cts;
	hdmi_set_clock_regenerator(regs, clk_n, clk_cts);
}

/*
 * this submodule is responsible for the video data synchronization.
 * for example, for rgb 4:4:4 input, the data map is defined as
 *			pin{47~40} <==> r[7:0]
 *			pin{31~24} <==> g[7:0]
 *			pin{15~8}  <==> b[7:0]
 */
static void hdmi_video_sample(struct rk3288_hdmi *regs)
{
	u32 color_format = 0x01;
	u8 val;

	val = HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE |
	      ((color_format << HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET) &
	      HDMI_TX_INVID0_VIDEO_MAPPING_MASK);

	writel(val, &regs->tx_invid0);

	/* enable tx stuffing: when de is inactive, fix the output data to 0 */
	val = HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE;
	writel(val, &regs->tx_instuffing);
	writel(0x0, &regs->tx_gydata0);
	writel(0x0, &regs->tx_gydata1);
	writel(0x0, &regs->tx_rcrdata0);
	writel(0x0, &regs->tx_rcrdata1);
	writel(0x0, &regs->tx_bcbdata0);
	writel(0x0, &regs->tx_bcbdata1);
}

static void hdmi_update_csc_coeffs(struct rk3288_hdmi *regs)
{
	u32 i, j;
	u32 csc_scale = 1;

	/* the csc registers are sequential, alternating msb then lsb */
	for (i = 0; i < ARRAY_SIZE(csc_coeff_default); i++) {
		for (j = 0; j < ARRAY_SIZE(csc_coeff_default[0]); j++) {
			u32 coeff = csc_coeff_default[i][j];
			writel(coeff >> 8, &regs->csc_coef[i][j].msb);
			writel(coeff && 0xff, &regs->csc_coef[i][j].lsb);
		}
	}

	clrsetbits_le32(&regs->csc_scale, HDMI_CSC_SCALE_CSCSCALE_MASK,
			csc_scale);
}

static void hdmi_video_csc(struct rk3288_hdmi *regs)
{
	u32 color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP;
	u32 interpolation = HDMI_CSC_CFG_INTMODE_DISABLE;

	/* configure the csc registers */
	writel(interpolation, &regs->csc_cfg);
	clrsetbits_le32(&regs->csc_scale,
			HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK, color_depth);

	hdmi_update_csc_coeffs(regs);
}

static void hdmi_video_packetize(struct rk3288_hdmi *regs)
{
	u32 output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
	u32 remap_size = HDMI_VP_REMAP_YCC422_16BIT;
	u32 color_depth = 0;
	u8 val, vp_conf;

	/* set the packetizer registers */
	val = ((color_depth << HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET) &
		HDMI_VP_PR_CD_COLOR_DEPTH_MASK) |
		((0 << HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET) &
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK);
	writel(val, &regs->vp_pr_cd);

	clrsetbits_le32(&regs->vp_stuff, HDMI_VP_STUFF_PR_STUFFING_MASK,
			HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE);

	/* data from pixel repeater block */
	vp_conf = HDMI_VP_CONF_PR_EN_DISABLE |
		  HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER;

	clrsetbits_le32(&regs->vp_conf, HDMI_VP_CONF_PR_EN_MASK |
			HDMI_VP_CONF_BYPASS_SELECT_MASK, vp_conf);

	clrsetbits_le32(&regs->vp_stuff, HDMI_VP_STUFF_IDEFAULT_PHASE_MASK,
			1 << HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET);

	writel(remap_size, &regs->vp_remap);

	vp_conf = HDMI_VP_CONF_BYPASS_EN_ENABLE |
		  HDMI_VP_CONF_PP_EN_DISABLE |
		  HDMI_VP_CONF_YCC422_EN_DISABLE;

	clrsetbits_le32(&regs->vp_conf, HDMI_VP_CONF_BYPASS_EN_MASK |
			HDMI_VP_CONF_PP_EN_ENMASK | HDMI_VP_CONF_YCC422_EN_MASK,
			vp_conf);

	clrsetbits_le32(&regs->vp_stuff, HDMI_VP_STUFF_PP_STUFFING_MASK |
			HDMI_VP_STUFF_YCC422_STUFFING_MASK,
			HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE |
			HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE);

	clrsetbits_le32(&regs->vp_conf, HDMI_VP_CONF_OUTPUT_SELECTOR_MASK,
			output_select);
}

static inline void hdmi_phy_test_clear(struct rk3288_hdmi *regs, u8 bit)
{
	clrsetbits_le32(&regs->phy_tst0, HDMI_PHY_TST0_TSTCLR_MASK,
			bit << HDMI_PHY_TST0_TSTCLR_OFFSET);
}

static int hdmi_phy_wait_i2c_done(struct rk3288_hdmi *regs, u32 msec)
{
	ulong start;
	u32 val;

	start = get_timer(0);
	do {
		val = readl(&regs->ih_i2cmphy_stat0);
		if (val & 0x3) {
			writel(val, &regs->ih_i2cmphy_stat0);
			return 0;
		}

		udelay(100);
	} while (get_timer(start) < msec);

	return 1;
}

static void hdmi_phy_i2c_write(struct rk3288_hdmi *regs, uint data, uint addr)
{
	writel(0xff, &regs->ih_i2cmphy_stat0);
	writel(addr, &regs->phy_i2cm_address_addr);
	writel((u8)(data >> 8), &regs->phy_i2cm_datao_1_addr);
	writel((u8)(data >> 0), &regs->phy_i2cm_datao_0_addr);
	writel(HDMI_PHY_I2CM_OPERATION_ADDR_WRITE,
	       &regs->phy_i2cm_operation_addr);

	hdmi_phy_wait_i2c_done(regs, 1000);
}

static void hdmi_phy_enable_power(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0, HDMI_PHY_CONF0_PDZ_MASK,
			enable << HDMI_PHY_CONF0_PDZ_OFFSET);
}

static void hdmi_phy_enable_tmds(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0, HDMI_PHY_CONF0_ENTMDS_MASK,
			enable << HDMI_PHY_CONF0_ENTMDS_OFFSET);
}

static void hdmi_phy_enable_spare(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0, HDMI_PHY_CONF0_SPARECTRL_MASK,
			enable << HDMI_PHY_CONF0_SPARECTRL_OFFSET);
}

static void hdmi_phy_gen2_pddq(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0, HDMI_PHY_CONF0_GEN2_PDDQ_MASK,
			enable << HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET);
}

static void hdmi_phy_gen2_txpwron(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0,
			HDMI_PHY_CONF0_GEN2_TXPWRON_MASK,
			enable << HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET);
}

static void hdmi_phy_sel_data_en_pol(struct rk3288_hdmi *regs, uint enable)
{
	clrsetbits_le32(&regs->phy_conf0,
			HDMI_PHY_CONF0_SELDATAENPOL_MASK,
			enable << HDMI_PHY_CONF0_SELDATAENPOL_OFFSET);
}

static void hdmi_phy_sel_interface_control(struct rk3288_hdmi *regs,
					   uint enable)
{
	clrsetbits_le32(&regs->phy_conf0, HDMI_PHY_CONF0_SELDIPIF_MASK,
			enable << HDMI_PHY_CONF0_SELDIPIF_OFFSET);
}

static int hdmi_phy_configure(struct rk3288_hdmi *regs, u32 mpixelclock)
{
	ulong start;
	u8 i, val;

	writel(HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS,
	       &regs->mc_flowctrl);

	/* gen2 tx power off */
	hdmi_phy_gen2_txpwron(regs, 0);

	/* gen2 pddq */
	hdmi_phy_gen2_pddq(regs, 1);

	/* phy reset */
	writel(HDMI_MC_PHYRSTZ_DEASSERT, &regs->mc_phyrstz);
	writel(HDMI_MC_PHYRSTZ_ASSERT, &regs->mc_phyrstz);
	writel(HDMI_MC_HEACPHY_RST_ASSERT, &regs->mc_heacphy_rst);

	hdmi_phy_test_clear(regs, 1);
	writel(HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2, &regs->phy_i2cm_slave_addr);
	hdmi_phy_test_clear(regs, 0);

	/* pll/mpll cfg - always match on final entry */
	for (i = 0; rockchip_mpll_cfg[i].mpixelclock != (~0ul); i++)
		if (mpixelclock <= rockchip_mpll_cfg[i].mpixelclock)
			break;

	hdmi_phy_i2c_write(regs, rockchip_mpll_cfg[i].cpce, PHY_OPMODE_PLLCFG);
	hdmi_phy_i2c_write(regs, rockchip_mpll_cfg[i].gmp, PHY_PLLGMPCTRL);
	hdmi_phy_i2c_write(regs, rockchip_mpll_cfg[i].curr, PHY_PLLCURRCTRL);

	hdmi_phy_i2c_write(regs, 0x0000, PHY_PLLPHBYCTRL);
	hdmi_phy_i2c_write(regs, 0x0006, PHY_PLLCLKBISTPHASE);

	for (i = 0; rockchip_phy_config[i].mpixelclock != (~0ul); i++)
		if (mpixelclock <= rockchip_phy_config[i].mpixelclock)
			break;

	/*
	 * resistance term 133ohm cfg
	 * preemp cgf 0.00
	 * tx/ck lvl 10
	 */
	hdmi_phy_i2c_write(regs, rockchip_phy_config[i].term, PHY_TXTERM);
	hdmi_phy_i2c_write(regs, rockchip_phy_config[i].sym_ctr,
			   PHY_CKSYMTXCTRL);
	hdmi_phy_i2c_write(regs, rockchip_phy_config[i].vlev_ctr, PHY_VLEVCTRL);

	/* remove clk term */
	hdmi_phy_i2c_write(regs, 0x8000, PHY_CKCALCTRL);

	hdmi_phy_enable_power(regs, 1);

	/* toggle tmds enable */
	hdmi_phy_enable_tmds(regs, 0);
	hdmi_phy_enable_tmds(regs, 1);

	/* gen2 tx power on */
	hdmi_phy_gen2_txpwron(regs, 1);
	hdmi_phy_gen2_pddq(regs, 0);

	hdmi_phy_enable_spare(regs, 1);

	/* wait for phy pll lock */
	start = get_timer(0);
	do {
		val = readl(&regs->phy_stat0);
		if (!(val & HDMI_PHY_TX_PHY_LOCK))
			return 0;

		udelay(100);
	} while (get_timer(start) < 5);

	return -1;
}

static int hdmi_phy_init(struct rk3288_hdmi *regs, uint mpixelclock)
{
	int i, ret;

	/* hdmi phy spec says to do the phy initialization sequence twice */
	for (i = 0; i < 2; i++) {
		hdmi_phy_sel_data_en_pol(regs, 1);
		hdmi_phy_sel_interface_control(regs, 0);
		hdmi_phy_enable_tmds(regs, 0);
		hdmi_phy_enable_power(regs, 0);

		/* enable csc */
		ret = hdmi_phy_configure(regs, mpixelclock);
		if (ret) {
			debug("hdmi phy config failure %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static void hdmi_av_composer(struct rk3288_hdmi *regs,
			     const struct display_timing *edid)
{
	u8 mdataenablepolarity = 1;
	u8 inv_val;
	uint hbl;
	uint vbl;

	hbl = edid->hback_porch.typ + edid->hfront_porch.typ +
			edid->hsync_len.typ;
	vbl = edid->vback_porch.typ + edid->vfront_porch.typ +
			edid->vsync_len.typ;

	/* set up hdmi_fc_invidconf */
	inv_val = HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE;

	inv_val |= (edid->flags & DISPLAY_FLAGS_HSYNC_HIGH ?
		   HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (edid->flags & DISPLAY_FLAGS_VSYNC_HIGH ?
		   HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (mdataenablepolarity ?
		   HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH :
		   HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW);

	/*
	 * TODO(sjg@chromium.org>: Need to check for HDMI / DVI
	 * inv_val |= (edid->hdmi_monitor_detected ?
	 *	   HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE :
	 *	   HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE);
	 */
	inv_val |= HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE;

	inv_val |= HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW;

	inv_val |= HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE;

	writel(inv_val, &regs->fc_invidconf);

	/* set up horizontal active pixel width */
	writel(edid->hactive.typ >> 8, &regs->fc_inhactv1);
	writel(edid->hactive.typ, &regs->fc_inhactv0);

	/* set up vertical active lines */
	writel(edid->vactive.typ >> 8, &regs->fc_invactv1);
	writel(edid->vactive.typ, &regs->fc_invactv0);

	/* set up horizontal blanking pixel region width */
	writel(hbl >> 8, &regs->fc_inhblank1);
	writel(hbl, &regs->fc_inhblank0);

	/* set up vertical blanking pixel region width */
	writel(vbl, &regs->fc_invblank);

	/* set up hsync active edge delay width (in pixel clks) */
	writel(edid->hfront_porch.typ >> 8, &regs->fc_hsyncindelay1);
	writel(edid->hfront_porch.typ, &regs->fc_hsyncindelay0);

	/* set up vsync active edge delay (in lines) */
	writel(edid->vfront_porch.typ, &regs->fc_vsyncindelay);

	/* set up hsync active pulse width (in pixel clks) */
	writel(edid->hsync_len.typ >> 8, &regs->fc_hsyncinwidth1);
	writel(edid->hsync_len.typ, &regs->fc_hsyncinwidth0);

	/* set up vsync active edge delay (in lines) */
	writel(edid->vsync_len.typ, &regs->fc_vsyncinwidth);
}

/* hdmi initialization step b.4 */
static void hdmi_enable_video_path(struct rk3288_hdmi *regs)
{
	u8 clkdis;

	/* control period minimum duration */
	writel(12, &regs->fc_ctrldur);
	writel(32, &regs->fc_exctrldur);
	writel(1, &regs->fc_exctrlspac);

	/* set to fill tmds data channels */
	writel(0x0b, &regs->fc_ch0pream);
	writel(0x16, &regs->fc_ch1pream);
	writel(0x21, &regs->fc_ch2pream);

	/* enable pixel clock and tmds data path */
	clkdis = 0x7f;
	clkdis &= ~HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
	writel(clkdis, &regs->mc_clkdis);

	clkdis &= ~HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
	writel(clkdis, &regs->mc_clkdis);

	clkdis &= ~HDMI_MC_CLKDIS_AUDCLK_DISABLE;
	writel(clkdis, &regs->mc_clkdis);
}

/* workaround to clear the overflow condition */
static void hdmi_clear_overflow(struct rk3288_hdmi *regs)
{
	u8 val, count;

	/* tmds software reset */
	writel((u8)~HDMI_MC_SWRSTZ_TMDSSWRST_REQ, &regs->mc_swrstz);

	val = readl(&regs->fc_invidconf);

	for (count = 0; count < 4; count++)
		writel(val, &regs->fc_invidconf);
}

static void hdmi_audio_set_format(struct rk3288_hdmi *regs)
{
	writel(HDMI_AUD_CONF0_I2S_SELECT | HDMI_AUD_CONF0_I2S_IN_EN_0,
	       &regs->aud_conf0);


	writel(HDMI_AUD_CONF1_I2S_MODE_STANDARD_MODE |
	       HDMI_AUD_CONF1_I2S_WIDTH_16BIT, &regs->aud_conf1);

	writel(0x00, &regs->aud_conf2);
}

static void hdmi_audio_fifo_reset(struct rk3288_hdmi *regs)
{
	writel((u8)~HDMI_MC_SWRSTZ_II2SSWRST_REQ, &regs->mc_swrstz);
	writel(HDMI_AUD_CONF0_SW_AUDIO_FIFO_RST, &regs->aud_conf0);

	writel(0x00, &regs->aud_int);
	writel(0x00, &regs->aud_int1);
}

static void hdmi_init_interrupt(struct rk3288_hdmi *regs)
{
	u8 ih_mute;

	/*
	 * boot up defaults are:
	 * hdmi_ih_mute   = 0x03 (disabled)
	 * hdmi_ih_mute_* = 0x00 (enabled)
	 *
	 * disable top level interrupt bits in hdmi block
	 */
	ih_mute = readl(&regs->ih_mute) |
		  HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT |
		  HDMI_IH_MUTE_MUTE_ALL_INTERRUPT;

	writel(ih_mute, &regs->ih_mute);

	/* enable i2c master done irq */
	writel(~0x04, &regs->i2cm_int);

	/* enable i2c client nack % arbitration error irq */
	writel(~0x44, &regs->i2cm_ctlint);

	/* enable phy i2cm done irq */
	writel(HDMI_PHY_I2CM_INT_ADDR_DONE_POL, &regs->phy_i2cm_int_addr);

	/* enable phy i2cm nack & arbitration error irq */
	writel(HDMI_PHY_I2CM_CTLINT_ADDR_NAC_POL |
		HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_POL,
		&regs->phy_i2cm_ctlint_addr);

	/* enable cable hot plug irq */
	writel((u8)~HDMI_PHY_HPD, &regs->phy_mask0);

	/* clear hotplug interrupts */
	writel(HDMI_IH_PHY_STAT0_HPD, &regs->ih_phy_stat0);
}

static u8 hdmi_get_plug_in_status(struct rk3288_hdmi *regs)
{
	u8 val = readl(&regs->phy_stat0) & HDMI_PHY_HPD;

	return !!(val);
}

static int hdmi_wait_for_hpd(struct rk3288_hdmi *regs)
{
	ulong start;

	start = get_timer(0);
	do {
		if (hdmi_get_plug_in_status(regs))
			return 0;
		udelay(100);
	} while (get_timer(start) < 300);

	return -1;
}

static int hdmi_ddc_wait_i2c_done(struct rk3288_hdmi *regs, int msec)
{
	u32 val;
	ulong start;

	start = get_timer(0);
	do {
		val = readl(&regs->ih_i2cm_stat0);
		if (val & 0x2) {
			writel(val, &regs->ih_i2cm_stat0);
			return 0;
		}

		udelay(100);
	} while (get_timer(start) < msec);

	return 1;
}

static void hdmi_ddc_reset(struct rk3288_hdmi *regs)
{
	clrbits_le32(&regs->i2cm_softrstz, HDMI_I2CM_SOFTRSTZ);
}

static int hdmi_read_edid(struct rk3288_hdmi *regs, int block, u8 *buff)
{
	int shift = (block % 2) * 0x80;
	int edid_read_err = 0;
	u32 trytime = 5;
	u32 n, j, val;

	/* set ddc i2c clk which devided from ddc_clk to 100khz */
	writel(0x7a, &regs->i2cm_ss_scl_hcnt_0_addr);
	writel(0x8d, &regs->i2cm_ss_scl_lcnt_0_addr);

	/*
	 * TODO(sjg@chromium.org): The above values don't work - these ones
	 * work better, but generate lots of errors in the data.
	 */
	writel(0x0d, &regs->i2cm_ss_scl_hcnt_0_addr);
	writel(0x0d, &regs->i2cm_ss_scl_lcnt_0_addr);
	clrsetbits_le32(&regs->i2cm_div, HDMI_I2CM_DIV_FAST_STD_MODE,
			HDMI_I2CM_DIV_STD_MODE);

	writel(HDMI_I2CM_SLAVE_DDC_ADDR, &regs->i2cm_slave);
	writel(HDMI_I2CM_SEGADDR_DDC, &regs->i2cm_segaddr);
	writel(block >> 1, &regs->i2cm_segptr);

	while (trytime--) {
		edid_read_err = 0;

		for (n = 0; n < HDMI_EDID_BLOCK_SIZE / 8; n++) {
			writel(shift + 8 * n, &regs->i2c_address);

			if (block == 0)
				clrsetbits_le32(&regs->i2cm_operation,
						HDMI_I2CM_OPT_RD8,
						HDMI_I2CM_OPT_RD8);
			else
				clrsetbits_le32(&regs->i2cm_operation,
						HDMI_I2CM_OPT_RD8_EXT,
						HDMI_I2CM_OPT_RD8_EXT);

			if (hdmi_ddc_wait_i2c_done(regs, 10)) {
				hdmi_ddc_reset(regs);
				edid_read_err = 1;
				break;
			}

			for (j = 0; j < 8; j++) {
				val = readl(&regs->i2cm_buf0 + j);
				buff[8 * n + j] = val;
			}
		}

		if (!edid_read_err)
			break;
	}

	return edid_read_err;
}

static u8 pre_buf[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
	0x04, 0x69, 0xfa, 0x23, 0xc8, 0x28, 0x01, 0x00,
	0x10, 0x17, 0x01, 0x03, 0x80, 0x33, 0x1d, 0x78,
	0x2a, 0xd9, 0x45, 0xa2, 0x55, 0x4d, 0xa0, 0x27,
	0x12, 0x50, 0x54, 0xb7, 0xef, 0x00, 0x71, 0x4f,
	0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xb3, 0x00,
	0xd1, 0xc0, 0x81, 0xc0, 0x81, 0x00, 0x02, 0x3a,
	0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
	0x45, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x00, 0xff, 0x00, 0x44, 0x34, 0x4c,
	0x4d, 0x54, 0x46, 0x30, 0x37, 0x35, 0x39, 0x37,
	0x36, 0x0a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x32,
	0x4b, 0x18, 0x53, 0x11, 0x00, 0x0a, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
	0x00, 0x41, 0x53, 0x55, 0x53, 0x20, 0x56, 0x53,
	0x32, 0x33, 0x38, 0x0a, 0x20, 0x20, 0x01, 0xb0,
	0x02, 0x03, 0x22, 0x71, 0x4f, 0x01, 0x02, 0x03,
	0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x0e, 0x0f,
	0x1d, 0x1e, 0x1f, 0x10, 0x23, 0x09, 0x17, 0x07,
	0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00,
	0x10, 0x00, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0,
	0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0xfd, 0x1e,
	0x11, 0x00, 0x00, 0x18, 0x01, 0x1d, 0x00, 0x72,
	0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00,
	0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e, 0x01, 0x1d,
	0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8, 0x28,
	0x55, 0x40, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e,
	0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20,
	0x0c, 0x40, 0x55, 0x00, 0xfd, 0x1e, 0x11, 0x00,
	0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe9,
};

static int rk_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	u32 edid_size = HDMI_EDID_BLOCK_SIZE;
	int ret;

	if (0) {
		edid_size = sizeof(pre_buf);
		memcpy(buf, pre_buf, edid_size);
	} else {
		ret = hdmi_read_edid(priv->regs, 0, buf);
		if (ret) {
			debug("failed to read edid.\n");
			return -1;
		}

		if (buf[0x7e] != 0) {
			hdmi_read_edid(priv->regs, 1,
				       buf + HDMI_EDID_BLOCK_SIZE);
			edid_size += HDMI_EDID_BLOCK_SIZE;
		}
	}

	return edid_size;
}

static int rk_hdmi_enable(struct udevice *dev, int panel_bpp,
			  const struct display_timing *edid)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct rk3288_hdmi *regs = priv->regs;
	int ret;

	debug("hdmi, mode info : clock %d hdis %d vdis %d\n",
	      edid->pixelclock.typ, edid->hactive.typ, edid->vactive.typ);

	hdmi_av_composer(regs, edid);

	ret = hdmi_phy_init(regs, edid->pixelclock.typ);
	if (ret)
		return ret;

	hdmi_enable_video_path(regs);

	hdmi_audio_fifo_reset(regs);
	hdmi_audio_set_format(regs);
	hdmi_audio_set_samplerate(regs, edid->pixelclock.typ);

	hdmi_video_packetize(regs);
	hdmi_video_csc(regs);
	hdmi_video_sample(regs);

	hdmi_clear_overflow(regs);

	return 0;
}

static int rk_hdmi_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_hdmi_priv *priv = dev_get_priv(dev);

	priv->regs = (struct rk3288_hdmi *)dev_get_addr(dev);
	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	return 0;
}

static int rk_hdmi_probe(struct udevice *dev)
{
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	struct rk_hdmi_priv *priv = dev_get_priv(dev);
	struct udevice *reg;
	struct clk clk;
	int ret;
	int vop_id = uc_plat->source_id;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 0);
		clk_free(&clk);
	}
	if (ret) {
		debug("%s: Failed to set EDP clock: ret=%d\n", __func__, ret);
		return ret;
	}

	/*
	 * Configure the maximum clock to permit whatever resolution the
	 * monitor wants
	 */
	ret = clk_get_by_index(uc_plat->src_dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 384000000);
		clk_free(&clk);
	}
	if (ret < 0) {
		debug("%s: Failed to set clock in source device '%s': ret=%d\n",
		      __func__, uc_plat->src_dev->name, ret);
		return ret;
	}

	ret = regulator_get_by_platname("vcc50_hdmi", &reg);
	if (!ret)
		ret = regulator_set_enable(reg, true);
	if (ret)
		debug("%s: Cannot set regulator vcc50_hdmi\n", __func__);

	/* hdmi source select hdmi controller */
	rk_setreg(&priv->grf->soc_con6, 1 << 15);

	/* hdmi data from vop id */
	rk_setreg(&priv->grf->soc_con6, (vop_id == 1) ? (1 << 4) : (1 << 4));

	ret = hdmi_wait_for_hpd(priv->regs);
	if (ret < 0) {
		debug("hdmi can not get hpd signal\n");
		return -1;
	}

	hdmi_init_interrupt(priv->regs);

	return 0;
}

static const struct dm_display_ops rk_hdmi_ops = {
	.read_edid = rk_hdmi_read_edid,
	.enable = rk_hdmi_enable,
};

static const struct udevice_id rk_hdmi_ids[] = {
	{ .compatible = "rockchip,rk3288-dw-hdmi" },
	{ }
};

U_BOOT_DRIVER(hdmi_rockchip) = {
	.name	= "hdmi_rockchip",
	.id	= UCLASS_DISPLAY,
	.of_match = rk_hdmi_ids,
	.ops	= &rk_hdmi_ops,
	.ofdata_to_platdata	= rk_hdmi_ofdata_to_platdata,
	.probe	= rk_hdmi_probe,
	.priv_auto_alloc_size	 = sizeof(struct rk_hdmi_priv),
};
