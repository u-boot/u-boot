// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Chris Morgan <macromorgan@hotmail.com>
 */

#include <abuf.h>
#include <adc.h>
#include <asm/io.h>
#include <display.h>
#include <dm.h>
#include <dm/lists.h>
#include <env.h>
#include <fdt_support.h>
#include <i2c.h>
#include <linux/delay.h>
#include <mipi_dsi.h>
#include <mmc.h>
#include <panel.h>
#include <pwm.h>
#include <stdlib.h>
#include <video_bridge.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO0_BASE		0xfdd60000
#define GPIO4_BASE		0xfe770000
#define GPIO_SWPORT_DR_L	0x0000
#define GPIO_SWPORT_DR_H	0x0004
#define GPIO_SWPORT_DDR_L	0x0008
#define GPIO_SWPORT_DDR_H	0x000c
#define GPIO_A0			BIT(0)
#define GPIO_C5			BIT(5)
#define GPIO_C6			BIT(6)
#define GPIO_C7			BIT(7)

#define GPIO_WRITEMASK(bits)	((bits) << 16)

#define DTB_DIR			"rockchip/"

struct rg3xx_model {
	const u16 adc_value;
	const char *board;
	const char *board_name;
	const char *fdtfile;
	const bool detect_panel;
	const bool detect_regulator;
	const bool uart_con;
};

enum rgxx3_device_id {
	RG353M = 1,
	RG353P,
	RG353V,
	RG503,
	RGB30,
	RK2023,
	RGARCD,
	RGB10MAX3,
	/* Devices with duplicate ADC value */
	RG353PS,
	RG353VS,
	RGARCS,
};

static const struct rg3xx_model rg3xx_model_details[] = {
	[RG353M] = {
		.adc_value = 517, /* Observed average from device */
		.board = "rk3566-anbernic-rg353m",
		.board_name = "Anbernic RG353M",
		/* Device is identical to RG353P. */
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353p.dtb",
		.detect_panel = 1,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RG353P] = {
		.adc_value = 860, /* Documented value of 860 */
		.board = "rk3566-anbernic-rg353p",
		.board_name = "Anbernic RG353P",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353p.dtb",
		.detect_panel = 1,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RG353V] = {
		.adc_value = 695, /* Observed average from device */
		.board = "rk3566-anbernic-rg353v",
		.board_name = "Anbernic RG353V",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353v.dtb",
		.detect_panel = 1,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RG503] = {
		.adc_value = 1023, /* Observed average from device */
		.board = "rk3566-anbernic-rg503",
		.board_name = "Anbernic RG503",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg503.dtb",
		.detect_panel = 0,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RGB30] = {
		.adc_value = 383, /* Gathered from second hand information */
		.board = "rk3566-powkiddy-rgb30",
		.board_name = "Powkiddy RGB30",
		.fdtfile = DTB_DIR "rk3566-powkiddy-rgb30.dtb",
		.detect_panel = 0,
		.detect_regulator = 1,
		.uart_con = 0,
	},
	[RK2023] = {
		.adc_value = 635, /* Observed average from device */
		.board = "rk3566-powkiddy-rk2023",
		.board_name = "Powkiddy RK2023",
		.fdtfile = DTB_DIR "rk3566-powkiddy-rk2023.dtb",
		.detect_panel = 0,
		.detect_regulator = 1,
		.uart_con = 0,
	},
	[RGARCD] = {
		.adc_value = 183, /* Observed average from device */
		.board = "rk3566-anbernic-rg-arc-d",
		.board_name = "Anbernic RG ARC-D",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg-arc-d.dtb",
		.detect_panel = 0,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RGB10MAX3] = {
		.adc_value = 765, /* Observed average from device */
		.board = "rk3566-powkiddy-rgb10max3",
		.board_name = "Powkiddy RGB10MAX3",
		.fdtfile = DTB_DIR "rk3566-powkiddy-rgb10max3.dtb",
		.detect_panel = 0,
		.detect_regulator = 1,
		.uart_con = 0,
	},
	/* Devices with duplicate ADC value */
	[RG353PS] = {
		.adc_value = 860, /* Observed average from device */
		.board = "rk3566-anbernic-rg353ps",
		.board_name = "Anbernic RG353PS",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353ps.dtb",
		.detect_panel = 1,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RG353VS] = {
		.adc_value = 695, /* Gathered from second hand information */
		.board = "rk3566-anbernic-rg353vs",
		.board_name = "Anbernic RG353VS",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg353vs.dtb",
		.detect_panel = 1,
		.detect_regulator = 0,
		.uart_con = 1,
	},
	[RGARCS] = {
		.adc_value = 183, /* Observed average from device */
		.board = "rk3566-anbernic-rg-arc-s",
		.board_name = "Anbernic RG ARC-S",
		.fdtfile = DTB_DIR "rk3566-anbernic-rg-arc-s.dtb",
		.detect_panel = 0,
		.detect_regulator = 0,
		.uart_con = 1,
	},
};

struct rg353_panel {
	const u16 id;
	const char *panel_compat[2];
};

static const struct rg353_panel rg353_panel_details[] = {
	{
		.id = 0x3052,
		.panel_compat[0] = "anbernic,rg353p-panel",
		.panel_compat[1] = "newvision,nv3051d",
	},
	{
		.id = 0x3821,
		.panel_compat[0] = "anbernic,rg353v-panel-v2",
		.panel_compat[1] = NULL,
	},
};

struct powkiddy_regulators {
	const u8 addr;
	const char *regulator_compat;
};

static const struct powkiddy_regulators regulator_details[] = {
	{
		.addr = 0x1c,
		.regulator_compat = "tcs,tcs4525",
	},
	{
		.addr = 0x40,
		.regulator_compat = "fcs,fan53555",
	},
};

/*
 * Start LED very early so user knows device is on. Set color
 * to red.
 */
void spl_board_init(void)
{
	/* Set GPIO0_C5, GPIO0_C6, and GPIO0_C7 to output. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) |
	       (GPIO_C7 | GPIO_C6 | GPIO_C5),
	       (GPIO0_BASE + GPIO_SWPORT_DDR_H));
	/* Set GPIO0_C5 and GPIO_C6 to 0 and GPIO0_C7 to 1. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C7,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));
}

/*
 * Buzz the buzzer so the user knows something is going on. Make it
 * optional in case PWM is disabled or if CONFIG_DM_PWM is not
 * enabled.
 */
void __maybe_unused startup_buzz(void)
{
	struct udevice *dev;
	int err;

	if (!IS_ENABLED(CONFIG_DM_PWM))
		return;

	/* Probe the PWM controller. */
	err = uclass_get_device_by_name(UCLASS_PWM,
					"pwm@fe6e0010", &dev);
	if (err)
		return;

	pwm_set_enable(dev, 0, 1);
	mdelay(200);
	pwm_set_enable(dev, 0, 0);
}

/*
 * Provide the bare minimum to identify the panel for the RG353
 * series. Since we don't have a working framebuffer device, no
 * need to init the panel; just identify it and provide the
 * clocks so we know what to set the different clock values to.
 */

static const struct display_timing rg353_default_timing = {
	.pixelclock.typ		= 24150000,
	.hactive.typ		= 640,
	.hfront_porch.typ	= 40,
	.hback_porch.typ	= 80,
	.hsync_len.typ		= 2,
	.vactive.typ		= 480,
	.vfront_porch.typ	= 18,
	.vback_porch.typ	= 28,
	.vsync_len.typ		= 2,
	.flags			= DISPLAY_FLAGS_HSYNC_HIGH |
				  DISPLAY_FLAGS_VSYNC_HIGH,
};

static int anbernic_rg353_panel_get_timing(struct udevice *dev,
					   struct display_timing *timings)
{
	memcpy(timings, &rg353_default_timing, sizeof(*timings));

	return 0;
}

static int anbernic_rg353_panel_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

	plat->lanes = 4;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO |
			   MIPI_DSI_MODE_VIDEO_BURST |
			   MIPI_DSI_MODE_EOT_PACKET |
			   MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct panel_ops anbernic_rg353_panel_ops = {
	.get_display_timing = anbernic_rg353_panel_get_timing,
};

U_BOOT_DRIVER(anbernic_rg353_panel) = {
	.name		= "anbernic_rg353_panel",
	.id		= UCLASS_PANEL,
	.ops		= &anbernic_rg353_panel_ops,
	.probe		= anbernic_rg353_panel_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
};

/*
 * The Anbernic 353 series shipped with 2 distinct displays requiring
 * 2 distinct drivers, with no way for a user to know which panel is
 * which. This function queries the DSI panel for the panel ID to
 * determine which panel is present so the device-tree can be corrected
 * automatically.
 */
int rgxx3_detect_display(void)
{
	struct udevice *dev;
	struct mipi_dsi_device *dsi;
	struct mipi_dsi_panel_plat *mplat;
	const struct rg353_panel *panel;
	int ret = 0;
	int i;
	u8 panel_id[2];

	/*
	 * Take panel out of reset status.
	 * Set GPIO4_A0 to output.
	 */
	writel(GPIO_WRITEMASK(GPIO_A0) | GPIO_A0,
	       (GPIO4_BASE + GPIO_SWPORT_DDR_L));
	/* Set GPIO4_A0 to 1. */
	writel(GPIO_WRITEMASK(GPIO_A0) | GPIO_A0,
	       (GPIO4_BASE + GPIO_SWPORT_DR_L));

	/* Probe the DSI controller. */
	ret = uclass_get_device_by_name(UCLASS_VIDEO_BRIDGE,
					"dsi@fe060000", &dev);
	if (ret) {
		printf("DSI host not probed: %d\n", ret);
		return ret;
	}

	/* Probe the DSI panel. */
	ret = device_bind_driver_to_node(dev, "anbernic_rg353_panel",
					 "anbernic_rg353_panel",
					 dev_ofnode(dev), NULL);
	if (ret) {
		printf("Failed to probe RG353 panel: %d\n", ret);
		return ret;
	}

	/*
	 * Attach the DSI controller which will also probe and attach
	 * the DSIDPHY.
	 */
	ret = video_bridge_attach(dev);
	if (ret) {
		printf("Failed to attach DSI controller: %d\n", ret);
		return ret;
	}

	/*
	 * Get the panel which should have already been probed by the
	 * video_bridge_attach() function.
	 */
	ret = uclass_first_device_err(UCLASS_PANEL, &dev);
	if (ret) {
		printf("Panel device error: %d\n", ret);
		return ret;
	}

	/* Now call the panel via DSI commands to get the panel ID. */
	mplat = dev_get_plat(dev);
	dsi = mplat->device;
	mipi_dsi_set_maximum_return_packet_size(dsi, sizeof(panel_id));
	ret = mipi_dsi_dcs_read(dsi, MIPI_DCS_GET_DISPLAY_ID, &panel_id,
				sizeof(panel_id));
	if (ret < 0) {
		printf("Unable to read panel ID: %d\n", ret);
		return ret;
	}

	/* Get the correct panel compatible from the table. */
	for (i = 0; i < ARRAY_SIZE(rg353_panel_details); i++) {
		if (rg353_panel_details[i].id == ((panel_id[0] << 8) |
						 panel_id[1])) {
			panel = &rg353_panel_details[i];
			break;
		}
	}

	if (!panel) {
		printf("Unable to identify panel_id %x\n",
		       (panel_id[0] << 8) | panel_id[1]);
		return -EINVAL;
	}

	env_set("panel", panel->panel_compat[0]);

	return 0;
}

/*
 * Some of the Powkiddy devices switched the CPU regulator, but users
 * are not able to determine this by looking at their hardware.
 * Attempt to auto-detect this situation and fixup the device-tree.
 */
int rgxx3_detect_regulator(void)
{
	struct udevice *bus;
	struct udevice *chip;
	u8 val;
	int ret;

	/* Get the correct i2c bus (i2c0). */
	ret = uclass_get_device_by_name(UCLASS_I2C,
					"i2c@fdd40000", &bus);
	if (ret)
		return ret;

	/*
	 * Check for all vdd_cpu regulators and read an arbitrary
	 * register to confirm it's present.
	 */
	for (int i = 0; i < ARRAY_SIZE(regulator_details); i++) {
		ret = i2c_get_chip(bus, regulator_details[i].addr,
				   1, &chip);
		if (ret)
			return ret;

		ret = dm_i2c_read(chip, 0, &val, 1);
		if (!ret) {
			env_set("vdd_cpu", regulator_details[i].regulator_compat);
			break;
		}
	}

	return 0;
}

int rgxx3_read_board_id(void)
{
	u32 adc_info;
	int ret;

	ret = adc_channel_single_shot("saradc@fe720000", 1, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/*
	 * Get the correct device from the table. The ADC value is
	 * determined by a resistor on ADC channel 0. The hardware
	 * design calls for no more than a 1% variance on the
	 * resistor, so assume a +- value of 15 should be enough.
	 */
	for (int i = 0; i < ARRAY_SIZE(rg3xx_model_details); i++) {
		u32 adc_min = rg3xx_model_details[i].adc_value - 15;
		u32 adc_max = rg3xx_model_details[i].adc_value + 15;

		if (adc_min < adc_info && adc_max > adc_info)
			return i;
	}

	return -ENODEV;
}

/* Detect which Anbernic RGXX3 device we are using so as to load the
 * correct devicetree for Linux. Set an environment variable once
 * found. The detection depends on the value of ADC channel 1 and the
 * presence of an eMMC on mmc0.
 */
int rgxx3_detect_device(void)
{
	int ret;
	int board_id;
	struct mmc *mmc;

	board_id = rgxx3_read_board_id();
	if (board_id < 0)
		return board_id;

	/*
	 * Try to access the eMMC on an RG353V, RG353P, or RG Arc D.
	 * If it's missing, it's an RG353VS, RG353PS, or RG Arc S.
	 * Note we could also check for a touchscreen at 0x1a on i2c2.
	 */
	if (board_id == RG353V || board_id == RG353P || board_id == RGARCD) {
		mmc = find_mmc_device(0);
		if (mmc) {
			ret = mmc_init(mmc);
			if (ret) {
				if (board_id == RG353V)
					board_id = RG353VS;
				else if (board_id == RG353P)
					board_id = RG353PS;
				else
					board_id = RGARCS;
			}
		}
	}

	return board_id;
}

/*
 * Check the loaded device tree to set the correct gd->board_type.
 * Disable the console if the board doesn't support a console.
 */
int set_gd_value(void)
{
	const char *model;

	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	for (int i = 0; i < ARRAY_SIZE(rg3xx_model_details); i++) {
		if (strcmp(rg3xx_model_details[i].board_name, model) == 0) {
			gd->board_type = i;
			if (!rg3xx_model_details[i].uart_con)
				gd->flags |= GD_FLG_SILENT |
					     GD_FLG_DISABLE_CONSOLE;
			return 0;
		}
	}

	return -ENODEV;
}

int rk_board_late_init(void)
{
	int ret;

	ret = set_gd_value();
	if (ret) {
		printf("Unable to auto-detect device\n");
		goto end;
	}

	/*
	 * Change the model number on the RG353M since it uses the same
	 * tree as the RG353P.
	 */
	if (gd->board_type == RG353P) {
		ret = rgxx3_read_board_id();
		if (ret > 0)
			gd->board_type = ret;
	}

	env_set("board", rg3xx_model_details[gd->board_type].board);
	env_set("board_name",
		rg3xx_model_details[gd->board_type].board_name);
	env_set("fdtfile", rg3xx_model_details[gd->board_type].fdtfile);

	/*
	 * Skip panel detection if not needed. Warn but don't fail for
	 * errors in auto-detection of the panel.
	 */
	if (rg3xx_model_details[gd->board_type].detect_panel) {
		ret = rgxx3_detect_display();
		if (ret)
			printf("Failed to detect panel type\n");
	}

	/*
	 * Skip vdd_cpu regulator detection if not needed. Warn but
	 * don't fail for errors in auto-detection of regulator.
	 */
	if (rg3xx_model_details[gd->board_type].detect_regulator) {
		ret = rgxx3_detect_regulator();
		if (ret)
			printf("Unable to detect vdd_cpu regulator\n");
	}

end:
	/* Turn off red LED and turn on orange LED. */
	writel(GPIO_WRITEMASK(GPIO_C7 | GPIO_C6 | GPIO_C5) | GPIO_C6,
	       (GPIO0_BASE + GPIO_SWPORT_DR_H));

	startup_buzz();

	return 0;
}

int rgxx3_panel_fixup(void *blob)
{
	const struct rg353_panel *panel = NULL;
	int node, ret;
	char *env;

	env = env_get("panel");
	if (!env) {
		printf("Can't get panel env\n");
		return -EINVAL;
	}

	/*
	 * Check if the environment variable doesn't equal the panel.
	 * If it doesn't, update the devicetree to the correct panel.
	 */
	node = fdt_path_offset(blob, "/dsi@fe060000/panel@0");
	if (!(node > 0)) {
		printf("Can't find the DSI node\n");
		return -ENODEV;
	}

	ret = fdt_node_check_compatible(blob, node, env);
	if (ret < 0)
		return -ENODEV;

	/* Panels match, return 0. */
	if (!ret)
		return 0;

	/* Panels don't match, search by first compatible value. */
	for (int i = 0; i < ARRAY_SIZE(rg353_panel_details); i++) {
		if (!strcmp(env, rg353_panel_details[i].panel_compat[0])) {
			panel = &rg353_panel_details[i];
			break;
		}
	}

	if (!panel) {
		printf("Unable to identify panel by compat string\n");
		return -ENODEV;
	}

	/* Set the compatible with the auto-detected values */
	fdt_setprop_string(blob, node, "compatible", panel->panel_compat[0]);
	if (panel->panel_compat[1])
		fdt_appendprop_string(blob, node, "compatible",
				      panel->panel_compat[1]);

	return 0;
}

int rgxx3_regulator_fixup(void *blob)
{
	const struct powkiddy_regulators *vdd_cpu = NULL;
	int node, ret, i;
	char path[] = "/i2c@fdd40000/regulator@00";
	char name[] = "regulator@00";
	char *env;

	env = env_get("vdd_cpu");
	if (!env) {
		printf("Can't get vdd_cpu env\n");
		return -EINVAL;
	}

	/*
	 * Find the device we have in our tree, which may or may not
	 * be present.
	 */
	for (i = 0; i < ARRAY_SIZE(regulator_details); i++) {
		sprintf(path, "/i2c@fdd40000/regulator@%02x",
			regulator_details[i].addr);
		node = fdt_path_offset(blob, path);
		if (node > 0)
			break;

		printf("Unable to find vdd_cpu\n");
		return -ENODEV;
	}

	node = fdt_path_offset(blob, path);
	if (!(node > 0)) {
		printf("Can't find the vdd_cpu node\n");
		return -ENODEV;
	}

	ret = fdt_node_check_compatible(blob, node, env);
	if (ret < 0)
		return -ENODEV;

	/* vdd_cpu regulators match, return 0. */
	if (!ret)
		return 0;

	/* Regulators don't match, search by first compatible value. */
	for (i = 0; i < ARRAY_SIZE(regulator_details); i++) {
		if (!strcmp(env, regulator_details[i].regulator_compat)) {
			vdd_cpu = &regulator_details[i];
			break;
		}
	}

	if (!vdd_cpu) {
		printf("Unable to identify vdd_cpu by compat string\n");
		return -ENODEV;
	}

	/* Set the compatible and reg with the auto-detected values */
	fdt_setprop_string(blob, node, "compatible", vdd_cpu->regulator_compat);
	fdt_setprop_u32(blob, node, "reg", vdd_cpu->addr);
	sprintf(name, "regulator@%02x", vdd_cpu->addr);
	fdt_set_name(blob, node, name);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;

	if (gd->board_type == RG353M)
		fdt_setprop(blob, 0, "model",
			    rg3xx_model_details[RG353M].board_name,
			    sizeof(rg3xx_model_details[RG353M].board_name));

	if (rg3xx_model_details[gd->board_type].detect_panel) {
		ret = rgxx3_panel_fixup(blob);
		if (ret)
			printf("Unable to update panel compat\n");
	}

	if (rg3xx_model_details[gd->board_type].detect_regulator) {
		ret = rgxx3_regulator_fixup(blob);
		if (ret)
			printf("Unable to update vdd_cpu compat\n");
	}

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	int ret;

	if (gd->board_type == 0) {
		ret = rgxx3_detect_device();
		if (ret < 0)
			return ret;
		gd->board_type = ret;
	}

	if (strcmp(name, rg3xx_model_details[gd->board_type].fdtfile) == 0)
		return 0;

	return -ENXIO;
}
