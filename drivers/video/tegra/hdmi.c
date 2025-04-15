// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 NVIDIA Corporation
 * Copyright (c) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <clk.h>
#include <dm.h>
#include <edid.h>
#include <i2c.h>
#include <log.h>
#include <misc.h>
#include <panel.h>
#include <reset.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/time.h>
#include <power/regulator.h>
#include <video_bridge.h>

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

#include "dc.h"
#include "hdmi.h"

#define DDCCI_ENTRY_ADDR	0x37
#define DDCCI_SOURSE_ADDR	0x51
#define DDCCI_COMMAND_WRITE	0x03
#define DDCCI_CTRL_BRIGHTNESS	0x10

#define HDMI_EDID_I2C_ADDR	0x50
#define HDMI_REKEY_DEFAULT	56

static const char * const hdmi_supplies[] = {
	"hdmi-supply", "pll-supply", "vdd-supply"
};

struct tmds_config {
	unsigned int pclk;
	u32 pll0;
	u32 pll1;
	u32 pe_current;
	u32 drive_current;
	u32 peak_current;
};

struct tegra_hdmi_config {
	const struct tmds_config *tmds;
	unsigned int num_tmds;
	unsigned int max_pclk;

	/* to be filled */
};

struct tegra_hdmi_priv {
	struct hdmi_ctlr *hdmi_regmap;

	struct udevice *supplies[ARRAY_SIZE(hdmi_supplies)];
	struct udevice *hdmi_ddc;

	struct gpio_desc hpd; /* hotplug detection gpio */
	struct display_timing timing;

	struct clk *clk;
	struct clk *clk_parent;

	int panel_bits_per_colourp;
	const struct tegra_hdmi_config *config;
};

/* 1280x720p 60hz: EIA/CEA-861-B Format 4 */
static struct display_timing default_720p_timing = {
	.pixelclock.typ		= 74250000,
	.hactive.typ		= 1280,
	.hfront_porch.typ	= 110,
	.hback_porch.typ	= 220,
	.hsync_len.typ		= 40,
	.vactive.typ		= 720,
	.vfront_porch.typ	= 5,
	.vback_porch.typ	= 20,
	.vsync_len.typ		= 5,
	.flags			= DISPLAY_FLAGS_HSYNC_HIGH |
				  DISPLAY_FLAGS_VSYNC_HIGH,
};

static const struct tmds_config tegra20_tmds_config[] = {
	{ /* slow pixel clock modes */
		.pclk = 27000000,
		.pll0 = SOR_PLL_BG_V17_S(3) | SOR_PLL_ICHPMP(1) |
			SOR_PLL_RESISTORSEL | SOR_PLL_VCOCAP(0) |
			SOR_PLL_TX_REG_LOAD(3),
		.pll1 = SOR_PLL_TMDS_TERM_ENABLE,
		.pe_current = PE_CURRENT0(PE_CURRENT_0_0_mA) |
			PE_CURRENT1(PE_CURRENT_0_0_mA) |
			PE_CURRENT2(PE_CURRENT_0_0_mA) |
			PE_CURRENT3(PE_CURRENT_0_0_mA),
		.drive_current = DRIVE_CURRENT_LANE0(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE1(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE2(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE3(DRIVE_CURRENT_7_125_mA),
	},
	{ /* high pixel clock modes */
		.pclk = UINT_MAX,
		.pll0 = SOR_PLL_BG_V17_S(3) | SOR_PLL_ICHPMP(1) |
			SOR_PLL_RESISTORSEL | SOR_PLL_VCOCAP(1) |
			SOR_PLL_TX_REG_LOAD(3),
		.pll1 = SOR_PLL_TMDS_TERM_ENABLE | SOR_PLL_PE_EN,
		.pe_current = PE_CURRENT0(PE_CURRENT_6_0_mA) |
			PE_CURRENT1(PE_CURRENT_6_0_mA) |
			PE_CURRENT2(PE_CURRENT_6_0_mA) |
			PE_CURRENT3(PE_CURRENT_6_0_mA),
		.drive_current = DRIVE_CURRENT_LANE0(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE1(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE2(DRIVE_CURRENT_7_125_mA) |
			DRIVE_CURRENT_LANE3(DRIVE_CURRENT_7_125_mA),
	},
};

static const struct tmds_config tegra30_tmds_config[] = {
	{ /* 480p modes */
		.pclk = 27000000,
		.pll0 = SOR_PLL_BG_V17_S(3) | SOR_PLL_ICHPMP(1) |
			SOR_PLL_RESISTORSEL | SOR_PLL_VCOCAP(0) |
			SOR_PLL_TX_REG_LOAD(0),
		.pll1 = SOR_PLL_TMDS_TERM_ENABLE,
		.pe_current = PE_CURRENT0(PE_CURRENT_0_0_mA) |
			PE_CURRENT1(PE_CURRENT_0_0_mA) |
			PE_CURRENT2(PE_CURRENT_0_0_mA) |
			PE_CURRENT3(PE_CURRENT_0_0_mA),
		.drive_current = DRIVE_CURRENT_LANE0(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE1(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE2(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE3(DRIVE_CURRENT_5_250_mA),
	}, { /* 720p modes */
		.pclk = 74250000,
		.pll0 = SOR_PLL_BG_V17_S(3) | SOR_PLL_ICHPMP(1) |
			SOR_PLL_RESISTORSEL | SOR_PLL_VCOCAP(1) |
			SOR_PLL_TX_REG_LOAD(0),
		.pll1 = SOR_PLL_TMDS_TERM_ENABLE | SOR_PLL_PE_EN,
		.pe_current = PE_CURRENT0(PE_CURRENT_5_0_mA) |
			PE_CURRENT1(PE_CURRENT_5_0_mA) |
			PE_CURRENT2(PE_CURRENT_5_0_mA) |
			PE_CURRENT3(PE_CURRENT_5_0_mA),
		.drive_current = DRIVE_CURRENT_LANE0(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE1(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE2(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE3(DRIVE_CURRENT_5_250_mA),
	}, { /* 1080p modes */
		.pclk = UINT_MAX,
		.pll0 = SOR_PLL_BG_V17_S(3) | SOR_PLL_ICHPMP(1) |
			SOR_PLL_RESISTORSEL | SOR_PLL_VCOCAP(3) |
			SOR_PLL_TX_REG_LOAD(0),
		.pll1 = SOR_PLL_TMDS_TERM_ENABLE | SOR_PLL_PE_EN,
		.pe_current = PE_CURRENT0(PE_CURRENT_5_0_mA) |
			PE_CURRENT1(PE_CURRENT_5_0_mA) |
			PE_CURRENT2(PE_CURRENT_5_0_mA) |
			PE_CURRENT3(PE_CURRENT_5_0_mA),
		.drive_current = DRIVE_CURRENT_LANE0(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE1(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE2(DRIVE_CURRENT_5_250_mA) |
			DRIVE_CURRENT_LANE3(DRIVE_CURRENT_5_250_mA),
	},
};

static void tegra_dc_enable_controller(struct udevice *dev)
{
	struct tegra_dc_plat *dc_plat = dev_get_plat(dev);
	struct dc_ctlr *dc = dc_plat->dc;
	u32 value;

	value = readl(&dc->disp.disp_win_opt);
	value |= HDMI_ENABLE;
	writel(value, &dc->disp.disp_win_opt);

	writel(GENERAL_UPDATE, &dc->cmd.state_ctrl);
	writel(GENERAL_ACT_REQ, &dc->cmd.state_ctrl);
}

static void tegra_hdmi_setup_tmds(struct tegra_hdmi_priv *priv,
				  const struct tmds_config *tmds)
{
	struct hdmi_ctlr *hdmi = priv->hdmi_regmap;
	u32 value;

	writel(tmds->pll0, &hdmi->nv_pdisp_sor_pll0);
	writel(tmds->pll1, &hdmi->nv_pdisp_sor_pll1);
	writel(tmds->pe_current, &hdmi->nv_pdisp_pe_current);

	writel(tmds->drive_current, &hdmi->nv_pdisp_sor_lane_drive_current);

	value = readl(&hdmi->nv_pdisp_sor_lane_drive_current);
	value |= BIT(31);
	writel(value, &hdmi->nv_pdisp_sor_lane_drive_current);
}

static int tegra_hdmi_encoder_enable(struct udevice *dev)
{
	struct tegra_dc_plat *dc_plat = dev_get_plat(dev);
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);
	struct dc_ctlr *dc = dc_plat->dc;
	struct display_timing *dt = &priv->timing;
	struct hdmi_ctlr *hdmi = priv->hdmi_regmap;
	unsigned long rate, div82;
	unsigned int pulse_start, rekey;
	int retries = 1000;
	u32 value;
	int i;

	/* power up sequence */
	value = readl(&hdmi->nv_pdisp_sor_pll0);
	value &= ~SOR_PLL_PDBG;
	writel(value, &hdmi->nv_pdisp_sor_pll0);

	udelay(20);

	value = readl(&hdmi->nv_pdisp_sor_pll0);
	value &= ~SOR_PLL_PWR;
	writel(value, &hdmi->nv_pdisp_sor_pll0);

	writel(VSYNC_H_POSITION(1), &dc->disp.disp_timing_opt);
	writel(DITHER_CONTROL_DISABLE | BASE_COLOR_SIZE_888,
	       &dc->disp.disp_color_ctrl);

	/* video_preamble uses h_pulse2 */
	pulse_start = 1 + dt->hsync_len.typ + dt->hback_porch.typ - 10;

	writel(H_PULSE2_ENABLE, &dc->disp.disp_signal_opt0);

	value = PULSE_MODE_NORMAL | PULSE_POLARITY_HIGH |
		PULSE_QUAL_VACTIVE | PULSE_LAST_END_A;
	writel(value, &dc->disp.h_pulse[H_PULSE2].h_pulse_ctrl);

	value = PULSE_START(pulse_start) | PULSE_END(pulse_start + 8);
	writel(value, &dc->disp.h_pulse[H_PULSE2].h_pulse_pos[H_PULSE0_POSITION_A]);

	value = VSYNC_WINDOW_END(0x210) | VSYNC_WINDOW_START(0x200) |
		VSYNC_WINDOW_ENABLE;
	writel(value, &hdmi->nv_pdisp_hdmi_vsync_window);

	if (dc_plat->pipe)
		value = HDMI_SRC_DISPLAYB;
	else
		value = HDMI_SRC_DISPLAYA;

	if (dt->hactive.typ == 720 && (dt->vactive.typ == 480 ||
				       dt->vactive.typ == 576))
		writel(value | ARM_VIDEO_RANGE_FULL,
		       &hdmi->nv_pdisp_input_control);
	else
		writel(value | ARM_VIDEO_RANGE_LIMITED,
		       &hdmi->nv_pdisp_input_control);

	rate = clock_get_periph_rate(priv->clk->id, priv->clk_parent->id);
	div82 = rate / USEC_PER_SEC * 4;
	value = SOR_REFCLK_DIV_INT(div82 >> 2) | SOR_REFCLK_DIV_FRAC(div82);
	writel(value, &hdmi->nv_pdisp_sor_refclk);

	rekey = HDMI_REKEY_DEFAULT;
	value = HDMI_CTRL_REKEY(rekey);
	value |= HDMI_CTRL_MAX_AC_PACKET((dt->hsync_len.typ + dt->hback_porch.typ +
					  dt->hfront_porch.typ - rekey - 18) / 32);
	writel(value, &hdmi->nv_pdisp_hdmi_ctrl);

	/* TMDS CONFIG */
	for (i = 0; i < priv->config->num_tmds; i++) {
		if (dt->pixelclock.typ <= priv->config->tmds[i].pclk) {
			tegra_hdmi_setup_tmds(priv, &priv->config->tmds[i]);
			break;
		}
	}

	writel(SOR_SEQ_PU_PC(0) | SOR_SEQ_PU_PC_ALT(0) | SOR_SEQ_PD_PC(8) |
	       SOR_SEQ_PD_PC_ALT(8), &hdmi->nv_pdisp_sor_seq_ctl);

	value = SOR_SEQ_INST_WAIT_TIME(1) | SOR_SEQ_INST_WAIT_UNITS_VSYNC |
		SOR_SEQ_INST_HALT | SOR_SEQ_INST_PIN_A_LOW |
		SOR_SEQ_INST_PIN_B_LOW | SOR_SEQ_INST_DRIVE_PWM_OUT_LO;

	writel(value, &hdmi->nv_pdisp_sor_seq_inst0);
	writel(value, &hdmi->nv_pdisp_sor_seq_inst8);

	value = readl(&hdmi->nv_pdisp_sor_cstm);

	value &= ~SOR_CSTM_ROTCLK(~0);
	value |= SOR_CSTM_ROTCLK(2);
	value |= SOR_CSTM_PLLDIV;
	value &= ~SOR_CSTM_LVDS_ENABLE;
	value &= ~SOR_CSTM_MODE_MASK;
	value |= SOR_CSTM_MODE_TMDS;

	writel(value, &hdmi->nv_pdisp_sor_cstm);

	/* start SOR */
	writel(SOR_PWR_NORMAL_STATE_PU | SOR_PWR_NORMAL_START_NORMAL |
	       SOR_PWR_SAFE_STATE_PD | SOR_PWR_SETTING_NEW_TRIGGER,
	       &hdmi->nv_pdisp_sor_pwr);
	writel(SOR_PWR_NORMAL_STATE_PU | SOR_PWR_NORMAL_START_NORMAL |
	       SOR_PWR_SAFE_STATE_PD | SOR_PWR_SETTING_NEW_DONE,
	       &hdmi->nv_pdisp_sor_pwr);

	do {
		if (--retries < 0)
			return -ETIME;
		value = readl(&hdmi->nv_pdisp_sor_pwr);
	} while (value & SOR_PWR_SETTING_NEW_PENDING);

	value = SOR_STATE_ASY_CRCMODE_COMPLETE |
		SOR_STATE_ASY_OWNER_HEAD0 |
		SOR_STATE_ASY_SUBOWNER_BOTH |
		SOR_STATE_ASY_PROTOCOL_SINGLE_TMDS_A |
		SOR_STATE_ASY_DEPOL_POS;

	/* setup sync polarities */
	if (dt->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		value |= SOR_STATE_ASY_HSYNCPOL_POS;

	if (dt->flags & DISPLAY_FLAGS_HSYNC_LOW)
		value |= SOR_STATE_ASY_HSYNCPOL_NEG;

	if (dt->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		value |= SOR_STATE_ASY_VSYNCPOL_POS;

	if (dt->flags & DISPLAY_FLAGS_VSYNC_LOW)
		value |= SOR_STATE_ASY_VSYNCPOL_NEG;

	writel(value, &hdmi->nv_pdisp_sor_state2);

	value = SOR_STATE_ASY_HEAD_OPMODE_AWAKE | SOR_STATE_ASY_ORMODE_NORMAL;
	writel(value, &hdmi->nv_pdisp_sor_state1);

	writel(0, &hdmi->nv_pdisp_sor_state0);
	writel(SOR_STATE_UPDATE, &hdmi->nv_pdisp_sor_state0);
	writel(value | SOR_STATE_ATTACHED,
	       &hdmi->nv_pdisp_sor_state1);
	writel(0, &hdmi->nv_pdisp_sor_state0);

	tegra_dc_enable_controller(dev);

	return 0;
}

/* DDC/CI backlight control */
static int tegra_hdmi_set_connector(struct udevice *dev, int percent)
{
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);
	struct udevice *ddc_entry;
	struct i2c_msg msg[1];
	u8 checksum = DDCCI_ENTRY_ADDR << 1;
	int i, ret;

	ret = dm_i2c_probe(priv->hdmi_ddc, DDCCI_ENTRY_ADDR, 0, &ddc_entry);
	if (ret) {
		log_debug("%s: cannot probe DDC/CI entry: error %d\n",
			  __func__, ret);
		return 0;
	}

	/*
	 * payload[1] is length: hithest bit OR last 4 bits indicate
	 * the number of following bytes (excluding checksum)
	 */
	u8 payload[7] = { DDCCI_SOURSE_ADDR, BIT(7) | (sizeof(payload) - 3),
			  DDCCI_COMMAND_WRITE, DDCCI_CTRL_BRIGHTNESS,
			  (u8)(percent & 0xff), (u8)(percent & 0xff), 0 };

	/* DDC/CI checksum is a simple XOR of all preceding bytes */
	for (i = 0; i < (sizeof(payload) - 1); i++)
		checksum ^= payload[i];

	payload[6] = checksum;

	msg->addr = DDCCI_ENTRY_ADDR;
	msg->flags = 0;
	msg->len = sizeof(payload);
	msg->buf = payload;

	dm_i2c_xfer(ddc_entry, msg, 1);

	return 0;
}

static int tegra_hdmi_timings(struct udevice *dev,
			      struct display_timing *timing)
{
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static void tegra_hdmi_init_clocks(struct udevice *dev)
{
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);
	u32 n = priv->timing.pixelclock.typ * 2 / USEC_PER_SEC;

	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
	case CLOCK_OSC_FREQ_48_0: /* OSC is 48Mhz */
		clock_set_rate(priv->clk_parent->id, n, 12, 0, 8);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(priv->clk_parent->id, n, 26, 0, 8);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
	case CLOCK_OSC_FREQ_16_8: /* OSC is 16.8Mhz */
		clock_set_rate(priv->clk_parent->id, n, 13, 0, 8);
		break;

	case CLOCK_OSC_FREQ_19_2:
	case CLOCK_OSC_FREQ_38_4:
	default:
		/*
		 * These are not supported.
		 */
		break;
	}

	clock_start_periph_pll(priv->clk->id, priv->clk_parent->id,
			       priv->timing.pixelclock.typ);
}

static bool tegra_hdmi_mode_valid(void *hdmi_priv, const struct display_timing *timing)
{
	struct tegra_hdmi_priv *priv = hdmi_priv;

	if (timing->pixelclock.typ > priv->config->max_pclk)
		return false;

	return true;
}

static int tegra_hdmi_decode_edid(struct udevice *dev)
{
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);
	struct udevice *hdmi_edid;
	uchar edid_buf[EDID_SIZE] = { 0 };
	int i, ret;

	/* Poll for 1 sec in case EDID is not ready right after hpd */
	for (i = 0; i < 10; i++) {
		ret = dm_i2c_probe(priv->hdmi_ddc, HDMI_EDID_I2C_ADDR, 0,
				   &hdmi_edid);
		if (!ret)
			break;

		mdelay(100);
	}
	if (ret) {
		log_debug("%s: cannot probe EDID: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = dm_i2c_read(hdmi_edid, 0, edid_buf, sizeof(edid_buf));
	if (ret) {
		log_debug("%s: cannot dump EDID buffer: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = edid_get_timing_validate(edid_buf, sizeof(edid_buf), &priv->timing,
				       &priv->panel_bits_per_colourp,
				       tegra_hdmi_mode_valid, priv);
	if (ret) {
		log_debug("%s: cannot decode EDID info: error %d\n",
			  __func__, ret);
		return ret;
	}

	return 0;
}

static int tegra_hdmi_wait_hpd(struct tegra_hdmi_priv *priv)
{
	int i;

	/* Poll 1 second for HPD signal */
	for (i = 0; i < 10; i++) {
		if (dm_gpio_get_value(&priv->hpd))
			return 0;

		mdelay(100);
	}

	return -ETIMEDOUT;
}

static int tegra_hdmi_probe(struct udevice *dev)
{
	struct tegra_hdmi_priv *priv = dev_get_priv(dev);
	struct reset_ctl reset_ctl;
	int i, ret;

	priv->hdmi_regmap = (struct hdmi_ctlr *)dev_read_addr_ptr(dev);
	if (!priv->hdmi_regmap) {
		log_debug("%s: no display controller address\n", __func__);
		return -EINVAL;
	}

	priv->config = (struct tegra_hdmi_config *)dev_get_driver_data(dev);

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		log_debug("%s: Could not get HDMI clock: %ld\n",
			  __func__, PTR_ERR(priv->clk));
		return PTR_ERR(priv->clk);
	}

	priv->clk_parent = devm_clk_get(dev, "parent");
	if (IS_ERR(priv->clk_parent)) {
		log_debug("%s: Could not get HDMI clock parent: %ld\n",
			  __func__, PTR_ERR(priv->clk_parent));
		return PTR_ERR(priv->clk_parent);
	}

	for (i = 0; i < ARRAY_SIZE(hdmi_supplies); i++) {
		ret = device_get_supply_regulator(dev, hdmi_supplies[i],
						  &priv->supplies[i]);
		if (ret) {
			log_debug("%s: cannot get %s %d\n", __func__,
				  hdmi_supplies[i], ret);
			if (ret != -ENOENT)
				return log_ret(ret);
		}

		ret = regulator_set_enable_if_allowed(priv->supplies[i], true);
		if (ret && ret != -ENOSYS) {
			log_debug("%s: cannot enable %s: error %d\n",
				  __func__, hdmi_supplies[i], ret);
			return ret;
		}
	}

	ret = reset_get_by_name(dev, "hdmi", &reset_ctl);
	if (ret) {
		log_debug("%s: reset_get_by_name() failed: %d\n",
			  __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_I2C, dev,
					   "nvidia,ddc-i2c-bus",
					   &priv->hdmi_ddc);
	if (ret) {
		log_debug("%s: cannot get hdmi ddc i2c bus: error %d\n",
			  __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "nvidia,hpd-gpio", 0,
				   &priv->hpd, GPIOD_IS_IN);
	if (ret) {
		log_debug("%s: Could not decode hpd-gpios (%d)\n",
			  __func__, ret);
		return ret;
	}

	/* wait for connector */
	ret = tegra_hdmi_wait_hpd(priv);
	if (ret) {
		/* HPD failed, use default timings */
		memcpy(&priv->timing, &default_720p_timing,
		       sizeof(default_720p_timing));
	} else {
		ret = tegra_hdmi_decode_edid(dev);
		if (ret)
			memcpy(&priv->timing, &default_720p_timing,
			       sizeof(default_720p_timing));
	}

	reset_assert(&reset_ctl);
	tegra_hdmi_init_clocks(dev);

	mdelay(2);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct tegra_hdmi_config tegra20_hdmi_config = {
	.tmds = tegra20_tmds_config,
	.num_tmds = ARRAY_SIZE(tegra20_tmds_config),
	.max_pclk = 148500000, /* 1080p */
};

static const struct tegra_hdmi_config tegra30_hdmi_config = {
	.tmds = tegra30_tmds_config,
	.num_tmds = ARRAY_SIZE(tegra30_tmds_config),
	.max_pclk = 148500000, /* 1080p */
};

static const struct video_bridge_ops tegra_hdmi_ops = {
	.attach			= tegra_hdmi_encoder_enable,
	.set_backlight		= tegra_hdmi_set_connector,
	.get_display_timing	= tegra_hdmi_timings,
};

static const struct udevice_id tegra_hdmi_ids[] = {
	{
		.compatible = "nvidia,tegra20-hdmi",
		.data = (ulong)&tegra20_hdmi_config
	}, {
		.compatible = "nvidia,tegra30-hdmi",
		.data = (ulong)&tegra30_hdmi_config
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(tegra_hdmi) = {
	.name		= "tegra_hdmi",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= tegra_hdmi_ids,
	.ops		= &tegra_hdmi_ops,
	.probe		= tegra_hdmi_probe,
	.plat_auto	= sizeof(struct tegra_dc_plat),
	.priv_auto	= sizeof(struct tegra_hdmi_priv),
};
