// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 MediaTek Inc.
 * Copyright (C) 2025 Collabora Ltd.
 * Copyright (C) 2026 BayLibre, SAS
 *
 * Authors:
 * - Guillaume La Roque <glaroque@baylibre.com>
 * - Jitao Shi <jitao.shi@mediatek.com>
 * - David Lechner <dlechner@baylibre.com>
 */

#include <asm-generic/gpio.h>
#include <backlight.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <power/regulator.h>

/* Register definitions from the Linux panel-himax-hx8279.c driver */
#define HX8279_REG_PAGE			0xb0

/* Page 0 - Driver/Module Configuration */
#define HX8279_P0_VGHS			0xbf
#define HX8279_P0_VGLS			0xc0
#define HX8279_P0_VGPHS			0xc2
#define HX8279_P0_VGNHS			0xc4
#define HX8279_VGH_MIN_MV		8700
#define HX8279_VGH_STEP_MV		300
#define HX8279_VGL_MIN_MV		6700
#define HX8279_VGL_STEP_MV		300
#define HX8279_VGPNH_MIN_MV		4000
#define HX8279_VGPNX_STEP_MV		50
#define HX8279_VGH_VOLT_SEL(x)		(((x) - HX8279_VGH_MIN_MV) / HX8279_VGH_STEP_MV)
#define HX8279_VGL_VOLT_SEL(x)		(((x) - HX8279_VGL_MIN_MV) / HX8279_VGL_STEP_MV)
#define HX8279_VGPN_VOLT_SEL(x)		(((x) - HX8279_VGPNH_MIN_MV) / HX8279_VGPNX_STEP_MV)

/* Page 1 - Gate driver On Array (GOA) Mux Config */
#define HX8279_P1_REG_GOA_L		0xc0
#define HX8279_P1_REG_GOUTL(x)		(HX8279_P1_REG_GOA_L + (x))
#define HX8279_P1_REG_GOA_R		0xd4
#define HX8279_P1_REG_GOUTR(x)		(HX8279_P1_REG_GOA_R + (x))

/* Page 3 - Gate driver On Array (GOA) Configuration */
#define HX8279_P3_REG_UNKNOWN_BA	0xba
#define HX8279_P3_REG_GOA_STVL		0xc8
#define HX8279_P3_GOA_STV_LEAD		GENMASK(4, 0)
#define HX8279_P3_REG_GOA_CKVL		0xc9
#define HX8279_P3_GOA_CKV_LEAD		GENMASK(4, 0)
#define HX8279_P3_REG_GOA_CKVD		0xca
#define HX8279_P3_GOA_CKV_RESERVED	BIT(6)
#define HX8279_P3_GOA_CKV_DUMMY		GENMASK(5, 0)
#define HX8279_P3_REG_GOA_CLR1_W_ADJ	0xd2
#define HX8279_P3_REG_GOA_CLR234_W_ADJ	0xd3
#define HX8279_P3_REG_GOA_CLR1_CFG	0xd4
#define HX8279_P3_REG_GOA_CLR_CFG(x)	(HX8279_P3_REG_GOA_CLR1_CFG + (x))
#define HX8279_P3_GOA_CLR_CFG_POLARITY	BIT(7)
#define HX8279_P3_GOA_CLR_CFG_STARTPOS	GENMASK(6, 0)
#define HX8279_P3_REG_UNKNOWN_E4	0xe4
#define HX8279_P3_REG_UNKNOWN_E5	0xe5

/* Page 5 - MIPI Configuration */
#define HX8279_P5_REG_TIMING		0xb3
#define HX8279_P5_TIMING_THS_SETTLE	GENMASK(7, 5)
#define HX8279_P5_TIMING_LHS_SETTLE	BIT(4)
#define HX8279_P5_TIMING_TLPX		GENMASK(3, 0)
#define HX8279_P5_REG_UNKNOWN_B8	0xb8
#define HX8279_P5_REG_UNKNOWN_BC	0xbc
#define HX8279_P5_REG_UNKNOWN_D6	0xd6

/* Page 6 - ENG/Gamma Configuration */
#define HX8279_P6_REG_ENGINEER_PWD	0xb8
#define HX8279_P6_REG_INHOUSE_FUNC	0xc0
#define HX8279_P6_ENG_UNLOCK_WORD	0xa5
#define HX8279_P6_REG_GAMMA_CHOPPER	0xbc
#define HX8279_P6_GAMMA_POCGM_CTL	GENMASK(6, 4)
#define HX8279_P6_GAMMA_POGCMD_CTL	GENMASK(2, 0)
#define HX8279_P6_REG_DLY_TIME_ADJ	0xd5

struct stk078_panel {
	struct gpio_desc *enable_gpio;
	struct gpio_desc *reset_gpio;
	struct mipi_dsi_device *dsi;
	struct udevice *iovcc;
	struct udevice *vdd;
	struct udevice *vddp;
	struct udevice *backlight;
};

static const struct drm_display_mode default_mode = {
	.clock = 156458,
	.hdisplay = 1200,
	.hsync_start = 1200 + 50,
	.hsync_end = 1200 + 50 + 24,
	.htotal = 1200 + 50 + 24 + 66,
	.vdisplay = 1920,
	.vsync_start = 1920 + 14,
	.vsync_end = 1920 + 14 + 2,
	.vtotal = 1920 + 14 + 2 + 10,
};

static const u8 stk078_gout_l[] = {
	0x0d, 0x0d, 0x06, 0x06, 0x08, 0x08, 0x0a, 0x0a, 0x0c, 0x0c,
	0x00, 0x00, 0x0e, 0x0e, 0x01, 0x01, 0x04, 0x04, 0x00, 0x00,
};

static const u8 stk078_gout_r[] = {
	0x0d, 0x0d, 0x05, 0x05, 0x07, 0x07, 0x09, 0x09, 0x0b, 0x0b,
	0x00, 0x00, 0x0e, 0x0e, 0x01, 0x01, 0x03, 0x03, 0x00, 0x00,
};

static int stk078_panel_init(struct stk078_panel *stk)
{
	struct mipi_dsi_device *dsi = stk->dsi;
	int i, ret;

	/* Page 5 - MIPI Configuration */
	mipi_dsi_generic_write_seq(dsi, HX8279_REG_PAGE, 5);
	mipi_dsi_generic_write_seq(dsi, HX8279_P5_REG_TIMING,
				   FIELD_PREP_CONST(HX8279_P5_TIMING_THS_SETTLE, 2) |
				   HX8279_P5_TIMING_LHS_SETTLE |
				   FIELD_PREP_CONST(HX8279_P5_TIMING_TLPX, 2));
	mipi_dsi_generic_write_seq(dsi, HX8279_P5_REG_UNKNOWN_B8, 0x7f);
	mipi_dsi_generic_write_seq(dsi, HX8279_P5_REG_UNKNOWN_BC, 0x20);
	mipi_dsi_generic_write_seq(dsi, HX8279_P5_REG_UNKNOWN_D6, 0x7f);

	/* Page 1 - GOA Mux Config */
	mipi_dsi_generic_write_seq(dsi, HX8279_REG_PAGE, 1);
	for (i = 0; i < ARRAY_SIZE(stk078_gout_l); i++) {
		const u8 buf[] = { HX8279_P1_REG_GOUTL(i), stk078_gout_l[i] };

		ret = mipi_dsi_generic_write(dsi, buf, ARRAY_SIZE(buf));
		if (ret < 0)
			return ret;
	}
	for (i = 0; i < ARRAY_SIZE(stk078_gout_r); i++) {
		const u8 buf[] = { HX8279_P1_REG_GOUTR(i), stk078_gout_r[i] };

		ret = mipi_dsi_generic_write(dsi, buf, ARRAY_SIZE(buf));
		if (ret < 0)
			return ret;
	}

	/* Page 3 - GOA Configuration */
	mipi_dsi_generic_write_seq(dsi, HX8279_REG_PAGE, 3);
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_UNKNOWN_BA, 0xf0);
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_STVL,
				   FIELD_PREP_CONST(HX8279_P3_GOA_STV_LEAD, 7));
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CKVL,
				   FIELD_PREP_CONST(HX8279_P3_GOA_CKV_LEAD, 3));
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CKVD,
				   HX8279_P3_GOA_CKV_RESERVED |
				   FIELD_PREP_CONST(HX8279_P3_GOA_CKV_DUMMY, 1));
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CLR1_W_ADJ, 1);
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CLR234_W_ADJ, 5);
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CLR_CFG(0),
				   FIELD_PREP_CONST(HX8279_P3_GOA_CLR_CFG_STARTPOS, 5));
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_GOA_CLR_CFG(1),
				   HX8279_P3_GOA_CLR_CFG_POLARITY |
				   FIELD_PREP_CONST(HX8279_P3_GOA_CLR_CFG_STARTPOS, 10));
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_UNKNOWN_E4, 0xc0);
	mipi_dsi_generic_write_seq(dsi, HX8279_P3_REG_UNKNOWN_E5, 0x00);

	/* Page 0 - Driver/Module Configuration */
	mipi_dsi_generic_write_seq(dsi, HX8279_REG_PAGE, 0);
	mipi_dsi_generic_write_seq(dsi, HX8279_P0_VGHS,
				   HX8279_VGH_VOLT_SEL(18000));
	mipi_dsi_generic_write_seq(dsi, HX8279_P0_VGLS,
				   HX8279_VGL_VOLT_SEL(12100));
	mipi_dsi_generic_write_seq(dsi, HX8279_P0_VGPHS,
				   HX8279_VGPN_VOLT_SEL(5500));
	mipi_dsi_generic_write_seq(dsi, HX8279_P0_VGNHS,
				   HX8279_VGPN_VOLT_SEL(5500));

	/* Page 6 - ENG/Gamma Configuration */
	mipi_dsi_generic_write_seq(dsi, HX8279_REG_PAGE, 6);
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_ENGINEER_PWD,
				   HX8279_P6_ENG_UNLOCK_WORD);
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_INHOUSE_FUNC,
				   HX8279_P6_ENG_UNLOCK_WORD);
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_GAMMA_CHOPPER,
				   FIELD_PREP_CONST(HX8279_P6_GAMMA_POCGM_CTL, 1) |
				   FIELD_PREP_CONST(HX8279_P6_GAMMA_POGCMD_CTL, 1));
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_DLY_TIME_ADJ, 72);
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_ENGINEER_PWD, 0x00);
	mipi_dsi_generic_write_seq(dsi, HX8279_P6_REG_INHOUSE_FUNC, 0x00);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	mdelay(130);

	return mipi_dsi_dcs_set_display_on(dsi);
}

static int stk078_panel_enable_backlight(struct udevice *dev)
{
	struct stk078_panel *stk = dev_get_priv(dev);
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	stk->dsi = dsi;
	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "mipi_dsi_attach failed: %d\n", ret);
		return ret;
	}

	dm_gpio_set_value(stk->reset_gpio, 0);
	if (stk->enable_gpio)
		dm_gpio_set_value(stk->enable_gpio, 0);
	mdelay(10);

	/*
	 * VDDP and IOVCC must both be up at least 1 ms before DCDC_EN (the
	 * enable GPIO). VDD only powers the backlight, so it comes last.
	 * Boards that switch all three supplies from a single regulator simply
	 * see the later regulator_enable() calls as reference counts.
	 */
	if (stk->vddp) {
		ret = regulator_enable(stk->vddp);
		if (ret < 0) {
			dev_err(dev, "enable vddp failed: %d\n", ret);
			goto out_detach;
		}
	}

	ret = regulator_enable(stk->iovcc);
	if (ret < 0) {
		dev_err(dev, "enable iovcc failed: %d\n", ret);
		goto out_vddp;
	}

	mdelay(10);

	ret = regulator_enable(stk->vdd);
	if (ret < 0) {
		dev_err(dev, "enable vdd failed: %d\n", ret);
		goto out_iovcc;
	}

	mdelay(15);
	if (stk->enable_gpio)
		dm_gpio_set_value(stk->enable_gpio, 1);
	mdelay(10);
	dm_gpio_set_value(stk->reset_gpio, 1);
	mdelay(140);

	ret = stk078_panel_init(stk);
	if (ret < 0) {
		dev_err(dev, "panel init sequence failed: %d\n", ret);
		goto out_vdd;
	}

	ret = backlight_enable(stk->backlight);
	if (ret < 0) {
		dev_err(dev, "enable backlight failed: %d\n", ret);
		goto out_vdd;
	}

	return 0;

out_vdd:
	dm_gpio_set_value(stk->reset_gpio, 0);
	udelay(100);
	if (stk->enable_gpio)
		dm_gpio_set_value(stk->enable_gpio, 0);
	regulator_disable(stk->vdd);
out_iovcc:
	regulator_disable(stk->iovcc);
out_vddp:
	if (stk->vddp)
		regulator_disable(stk->vddp);
out_detach:
	mipi_dsi_detach(dsi);

	return ret;
}

static int stk078_panel_add(struct udevice *dev)
{
	struct stk078_panel *stk = dev_get_priv(dev);
	int ret;

	ret = device_get_supply_regulator(dev, "iovcc-supply", &stk->iovcc);
	if (ret) {
		dev_err(dev, "Failed to get iovcc regulator: %d\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vdd-supply", &stk->vdd);
	if (ret) {
		dev_err(dev, "Failed to get vdd regulator: %d\n", ret);
		return ret;
	}

	/*
	 * vddp-supply is optional only for compatibility with device trees
	 * that predate it.
	 */
	ret = device_get_supply_regulator(dev, "vddp-supply", &stk->vddp);
	if (ret == -ENOENT) {
		stk->vddp = NULL;
	} else if (ret) {
		dev_err(dev, "Failed to get vddp regulator: %d\n", ret);
		return ret;
	}

	stk->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_IS_OUT);
	if (IS_ERR(stk->reset_gpio)) {
		ret = PTR_ERR(stk->reset_gpio);
		dev_err(dev, "cannot get reset-gpios %d\n", ret);
		return ret;
	}

	stk->enable_gpio = devm_gpiod_get_optional(dev, "enable", GPIOD_IS_OUT);

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &stk->backlight);
	if (ret) {
		dev_err(dev, "failed to get backlight: %d\n", ret);
		return ret;
	}

	return 0;
}

static int stk078_panel_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			   MIPI_DSI_MODE_LPM |
			   MIPI_DSI_CLOCK_NON_CONTINUOUS;

	return stk078_panel_add(dev);
}

static int stk078_panel_get_modes(struct udevice *dev,
				  const struct drm_display_mode **modes)
{
	*modes = &default_mode;

	return 1;
}

static const struct panel_ops stk078_panel_ops = {
	.enable_backlight = stk078_panel_enable_backlight,
	.get_modes = stk078_panel_get_modes,
};

static const struct udevice_id stk078_of_match[] = {
	{ .compatible = "startek,kd070fhfid078" },
	{ }
};

U_BOOT_DRIVER(stk078_panel_driver) = {
	.name		= "panel-startek-kd070fhfid078",
	.id		= UCLASS_PANEL,
	.of_match	= stk078_of_match,
	.ops		= &stk078_panel_ops,
	.probe		= stk078_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct stk078_panel),
};
