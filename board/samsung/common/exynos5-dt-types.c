// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <adc.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/s2mps11.h>
#include <samsung/exynos5-dt-types.h>
#include <samsung/misc.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id board_ids[] = {
	{ .compatible = "samsung,odroidxu3", .data = EXYNOS5_BOARD_ODROID_XU3 },
	{ .compatible = "samsung,exynos5", .data = EXYNOS5_BOARD_GENERIC },
	{ },
};

/**
 * Odroix XU3/XU4/HC1/HC2 board revisions (from HC1+_HC2_MAIN_REV0.1_20171017.pdf):
 * Rev   ADCmax  Board
 * 0.1     0     XU3 0.1
 * 0.2   372     XU3 0.2 | XU3L - no DISPLAYPORT (probe I2C0:0x40 / INA231)
 * 0.3  1280     XU4 0.1
 * 0.4   739     XU4 0.2
 * 0.5  1016     XU4+Air0.1 (Passive cooling)
 * 0.6  1309     XU4-HC1 0.1
 * 0.7  1470     XU4-HC1+ 0.1 (HC2)
 * Use +1% for ADC value tolerance in the array below, the code loops until
 * the measured ADC value is lower than then ADCmax from the array.
 */
struct odroid_rev_info odroid_info[] = {
	{ EXYNOS5_BOARD_ODROID_XU3_REV01, 1, 10, "xu3" },
	{ EXYNOS5_BOARD_ODROID_XU3_REV02, 2, 375, "xu3" },
	{ EXYNOS5_BOARD_ODROID_XU4_REV01, 1, 1293, "xu4" },
	{ EXYNOS5_BOARD_ODROID_HC1_REV01, 1, 1322, "hc1" },
	{ EXYNOS5_BOARD_ODROID_HC2_REV01, 1, 1484, "hc1" },
	{ EXYNOS5_BOARD_ODROID_UNKNOWN, 0, 4095, "unknown" },
};

static unsigned int odroid_get_rev(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(odroid_info); i++) {
		if (odroid_info[i].board_type == gd->board_type)
			return odroid_info[i].board_rev;
	}

	return 0;
}

static int odroid_get_board_type(void)
{
	unsigned int adcval;
	int ret, i;

	ret = adc_channel_single_shot("adc", CONFIG_ODROID_REV_AIN, &adcval);
	if (ret)
		goto rev_default;

	for (i = 0; i < ARRAY_SIZE(odroid_info); i++) {
		/* ADC tolerance: +1% */
		if (adcval < odroid_info[i].adc_val)
			return odroid_info[i].board_type;
	}

rev_default:
	return EXYNOS5_BOARD_ODROID_XU3;
}

/**
 * odroid_get_type_str - returns pointer to one of the board type string.
 * Board types: "xu3", "xu3-lite", "xu4". However the "xu3lite" can be
 * detected only when the i2c controller is ready to use. Fortunately,
 * XU3 and XU3L are compatible, and the information about board lite
 * revision is needed before booting the linux, to set proper environment
 * variable: $fdtfile.
 */
static const char *odroid_get_type_str(void)
{
	const char *type_xu3l = "xu3-lite";
	struct udevice *dev, *chip;
	int i, ret;

	if (gd->board_type != EXYNOS5_BOARD_ODROID_XU3_REV02)
		goto exit;

	ret = pmic_get("s2mps11", &dev);
	if (ret)
		goto exit;

	/* Enable LDO26: 3.0V */
	ret = pmic_reg_write(dev, S2MPS11_REG_L26CTRL,
			     S2MPS11_LDO26_ENABLE);
	if (ret)
		goto exit;

	/* Check XU3Lite by probe INA231 I2C0:0x40 */
	ret = uclass_get_device(UCLASS_I2C, 0, &dev);
	if (ret)
		goto exit;

	ret = dm_i2c_probe(dev, 0x40, 0x0, &chip);
	if (ret)
		return type_xu3l;

exit:
	for (i = 0; i < ARRAY_SIZE(odroid_info); i++) {
		if (odroid_info[i].board_type == gd->board_type)
			return odroid_info[i].name;
	}

	return NULL;
}

bool board_is_odroidxu3(void)
{
	if (gd->board_type >= EXYNOS5_BOARD_ODROID_XU3 &&
	    gd->board_type <= EXYNOS5_BOARD_ODROID_XU3_REV02)
		return true;

	return false;
}

bool board_is_odroidxu4(void)
{
	if (gd->board_type == EXYNOS5_BOARD_ODROID_XU4_REV01)
		return true;

	return false;
}

bool board_is_odroidhc1(void)
{
	if (gd->board_type == EXYNOS5_BOARD_ODROID_HC1_REV01)
		return true;

	return false;
}

bool board_is_odroidhc2(void)
{
	if (gd->board_type == EXYNOS5_BOARD_ODROID_HC2_REV01)
		return true;

	return false;
}

bool board_is_generic(void)
{
	if (gd->board_type == EXYNOS5_BOARD_GENERIC)
		return true;

	return false;
}

/**
 * get_board_rev() - return detected board revision.
 *
 * @return:  return board revision number for XU3 or 0 for generic
 */
u32 get_board_rev(void)
{
	if (board_is_generic())
		return 0;

	return odroid_get_rev();
}

/**
 * get_board_type() - returns board type string.
 *
 * @return:  return board type string for XU3 or empty string for generic
 */
const char *get_board_type(void)
{
	const char *generic = "";

	if (board_is_generic())
		return generic;

	return odroid_get_type_str();
}

/**
 * set_board_type() - set board type in gd->board_type.
 * As default type set EXYNOS5_BOARD_GENERIC, if detect Odroid,
 * then set its proper type.
 */
void set_board_type(void)
{
	const struct udevice_id *of_match = board_ids;
	int ret;

	gd->board_type = EXYNOS5_BOARD_GENERIC;

	while (of_match->compatible) {
		ret = fdt_node_check_compatible(gd->fdt_blob, 0,
						of_match->compatible);
		if (ret)
			of_match++;

		gd->board_type = of_match->data;
		break;
	}

	/* If Odroid, then check its revision */
	if (board_is_odroidxu3())
		gd->board_type = odroid_get_board_type();
}
