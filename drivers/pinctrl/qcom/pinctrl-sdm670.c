// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDM670 pinctrl
 *
 * (C) Copyright 2025 David Wronek <david.wronek@mainlining.org>
 */

#include <dm.h>
#include "pinctrl-qcom.h"

#define NORTH	0x00500000
#define SOUTH	0x00900000
#define WEST	0x00100000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function sdm670_pinctrl_functions[] = {
	{ "gpio", 0 },
	{ "blsp_uart2", 3 }, /* gpio 4 and 5, used for debug uart */
};

static const unsigned int sdm670_pin_offsets[] = {
	[0] = SOUTH,
	[1] = SOUTH,
	[2] = SOUTH,
	[3] = SOUTH,
	[4] = NORTH,
	[5] = NORTH,
	[6] = NORTH,
	[7] = NORTH,
	[8] = WEST,
	[9] = WEST,
	[10] = NORTH,
	[11] = NORTH,
	[12] = SOUTH,
	[13] = WEST,
	[14] = WEST,
	[15] = WEST,
	[16] = WEST,
	[17] = WEST,
	[18] = WEST,
	[19] = WEST,
	[20] = WEST,
	[21] = WEST,
	[22] = WEST,
	[23] = WEST,
	[24] = WEST,
	[25] = WEST,
	[26] = WEST,
	[27] = WEST,
	[28] = WEST,
	[29] = WEST,
	[30] = WEST,
	[31] = WEST,
	[32] = WEST,
	[33] = WEST,
	[34] = WEST,
	[35] = NORTH,
	[36] = NORTH,
	[37] = NORTH,
	[38] = NORTH,
	[39] = NORTH,
	[40] = NORTH,
	[41] = SOUTH,
	[42] = SOUTH,
	[43] = SOUTH,
	[44] = SOUTH,
	[45] = SOUTH,
	[46] = SOUTH,
	[47] = SOUTH,
	[48] = SOUTH,
	[49] = NORTH,
	[50] = NORTH,
	[51] = NORTH,
	[52] = NORTH,
	[53] = NORTH,
	[54] = NORTH,
	[55] = NORTH,
	[56] = NORTH,
	[57] = NORTH,
	[65] = NORTH,
	[66] = NORTH,
	[67] = NORTH,
	[68] = NORTH,
	[75] = NORTH,
	[76] = NORTH,
	[77] = NORTH,
	[78] = NORTH,
	[79] = NORTH,
	[80] = NORTH,
	[81] = NORTH,
	[82] = NORTH,
	[83] = NORTH,
	[84] = NORTH,
	[85] = SOUTH,
	[86] = SOUTH,
	[87] = SOUTH,
	[88] = SOUTH,
	[89] = SOUTH,
	[90] = SOUTH,
	[91] = SOUTH,
	[92] = SOUTH,
	[93] = SOUTH,
	[94] = SOUTH,
	[95] = SOUTH,
	[96] = SOUTH,
	[97] = WEST,
	[98] = WEST,
	[99] = NORTH,
	[100] = WEST,
	[101] = WEST,
	[102] = WEST,
	[103] = WEST,
	[105] = NORTH,
	[106] = NORTH,
	[107] = NORTH,
	[108] = NORTH,
	[109] = NORTH,
	[110] = NORTH,
	[111] = NORTH,
	[112] = NORTH,
	[113] = NORTH,
	[114] = WEST,
	[115] = WEST,
	[116] = SOUTH,
	[117] = NORTH,
	[118] = NORTH,
	[119] = NORTH,
	[120] = NORTH,
	[121] = NORTH,
	[122] = NORTH,
	[123] = NORTH,
	[124] = NORTH,
	[125] = NORTH,
	[126] = NORTH,
	[127] = WEST,
	[128] = WEST,
	[129] = WEST,
	[130] = WEST,
	[131] = WEST,
	[132] = WEST,
	[133] = NORTH,
	[134] = NORTH,
	[135] = WEST,
	[136] = WEST,
	[137] = WEST,
	[138] = WEST,
	[139] = WEST,
	[140] = WEST,
	[141] = WEST,
	[142] = WEST,
	[143] = WEST,
	[144] = SOUTH,
	[145] = SOUTH,
	[146] = WEST,
	[147] = WEST,
	[148] = WEST,
	[149] = WEST,
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
{							\
	.name = pg_name,				\
	.ctl_reg = ctl,					\
	.io_reg = 0,					\
	.pull_bit = pull,				\
	.drv_bit = drv,					\
	.oe_bit = -1,					\
	.in_bit = -1,					\
	.out_bit = -1,					\
}

#define UFS_RESET(pg_name, offset)			\
{							\
	.name = pg_name,				\
	.ctl_reg = offset,				\
	.io_reg = offset + 0x4,				\
	.pull_bit = 3,					\
	.drv_bit = 0,					\
	.oe_bit = -1,					\
	.in_bit = -1,					\
	.out_bit = 0,					\
}

static const struct msm_special_pin_data sdm670_special_pins_data[] = {
	UFS_RESET("ufs_reset", 0x99d000),
	SDC_QDSD_PINGROUP("sdc1_rclk", NORTH + 0x99000, 15, 0),
	SDC_QDSD_PINGROUP("sdc1_clk",  NORTH + 0x99000, 13, 6),
	SDC_QDSD_PINGROUP("sdc1_cmd",  NORTH + 0x99000, 11, 3),
	SDC_QDSD_PINGROUP("sdc1_data", NORTH + 0x99000, 9,  0),
	SDC_QDSD_PINGROUP("sdc2_clk",  NORTH + 0x9a000, 14, 6),
	SDC_QDSD_PINGROUP("sdc2_cmd",  NORTH + 0x9a000, 11, 3),
	SDC_QDSD_PINGROUP("sdc2_data", NORTH + 0x9a000, 9,  0),
};

static const char *sdm670_get_function_name(struct udevice *dev, unsigned int selector)
{
	return sdm670_pinctrl_functions[selector].name;
}

static const char *sdm670_get_pin_name(struct udevice *dev, unsigned int selector)
{
	if (selector >= 150 && selector <= 157)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 sdm670_special_pins_data[selector - 150].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sdm670_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	if (selector >= 0 && selector < ARRAY_SIZE(sdm670_pinctrl_functions))
		return sdm670_pinctrl_functions[selector].val;
	return -EINVAL;
}

struct msm_pinctrl_data sdm670_data = {
	.pin_data = {
		.pin_offsets = sdm670_pin_offsets,
		.pin_count = ARRAY_SIZE(sdm670_pin_offsets) + ARRAY_SIZE(sdm670_special_pins_data),
		.special_pins_start = 150,
		.special_pins_data = sdm670_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(sdm670_pinctrl_functions),
	.get_function_name = sdm670_get_function_name,
	.get_function_mux = sdm670_get_function_mux,
	.get_pin_name = sdm670_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,sdm670-tlmm",
		.data = (ulong)&sdm670_data
	},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_ssdm670) = {
	.name = "pinctrl_sdm670",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
