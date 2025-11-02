// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <linux/stddef.h>
#include <adc.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <env.h>
#include <env_internal.h>
#include <stdlib.h>

DECLARE_GLOBAL_DATA_PTR;

#define DTB_DIR			"rockchip/"

struct oga_model {
	const u16 adc_value;
	const char *board;
	const char *board_name;
	const char *fdtfile;
};

enum oga_device_id {
	OGA = 1,
	OGA_V11,
	OGS,
};

/*
 * All ADC values from schematic of Odroid Go Advance Black Edition.
 * Value for OGS is inferred based on schematic and observed values.
 */
static const struct oga_model oga_model_details[] = {
	[OGA] = {
		856,
		"rk3326-odroid-go2",
		"ODROID-GO Advance",
		DTB_DIR "rk3326-odroid-go2.dtb",
	},
	[OGA_V11] = {
		677,
		"rk3326-odroid-go2-v11",
		"ODROID-GO Advance Black Edition",
		DTB_DIR "rk3326-odroid-go2-v11.dtb",
	},
	[OGS] = {
		85,
		"rk3326-odroid-go3",
		"ODROID-GO Super",
		DTB_DIR "rk3326-odroid-go3.dtb",
	},
};

static int oga_read_board_id(void)
{
	u32 adc_info;
	int i, ret;

	ret = adc_channel_single_shot("saradc@ff288000", 0, &adc_info);
	if (ret) {
		printf("Read SARADC failed with error %d\n", ret);
		return ret;
	}

	/*
	 * Get the correct device from the table. The ADC value is
	 * determined by a resistor on ADC channel 0. The manufacturer
	 * accounted for this with a 5% tolerance, so assume a +- value
	 * of 50 should be enough.
	 */
	for (i = 1; i < ARRAY_SIZE(oga_model_details); i++) {
		u32 adc_min = oga_model_details[i].adc_value - 50;
		u32 adc_max = oga_model_details[i].adc_value + 50;

		if (adc_min < adc_info && adc_max > adc_info)
			return i;
	}

	return -ENODEV;
}

/* Detect which Odroid Go Advance device we are using so as to load the
 * correct devicetree for Linux. Set an environment variable once
 * found. The detection depends on the value of ADC channel 0.
 */
static int oga_detect_device(void)
{
	int board_id;

	board_id = oga_read_board_id();
	if (board_id < 0)
		return board_id;
	gd->board_type = board_id;

	env_set("board", oga_model_details[board_id].board);
	env_set("board_name", oga_model_details[board_id].board_name);
	env_set("fdtfile", oga_model_details[board_id].fdtfile);

	return 0;
}

int rk_board_late_init(void)
{
	int ret;

	ret = oga_detect_device();
	if (ret) {
		printf("Unable to detect device type: %d\n", ret);
		return ret;
	}

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	int board_id;

	if (!gd->board_type) {
		board_id = oga_read_board_id();
		if (board_id < 0)
			return board_id;
		gd->board_type = board_id;
	}

	if (!strcmp(name, oga_model_details[gd->board_type].fdtfile))
		return 0;

	return -EINVAL;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	const char *boot_device;
	struct udevice *dev;
	ofnode node;

	if (prio)
		return ENVL_UNKNOWN;

	boot_device = ofnode_read_chosen_string("u-boot,spl-boot-device");
	if (!boot_device) {
		debug("%s: /chosen/u-boot,spl-boot-device not set\n", __func__);
		return ENVL_NOWHERE;
	}

	debug("%s: booted from %s\n", __func__, boot_device);

	node = ofnode_path(boot_device);
	if (!ofnode_valid(node))
		return ENVL_NOWHERE;

	if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH) &&
	    !uclass_find_device_by_ofnode(UCLASS_SPI_FLASH, node, &dev))
		return ENVL_SPI_FLASH;

	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC) &&
	    !uclass_find_device_by_ofnode(UCLASS_MMC, node, &dev))
		return ENVL_MMC;

	return ENVL_NOWHERE;
}
