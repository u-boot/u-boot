// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sm6115 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define WEST 0x00000000
#define SOUTH 0x00400000
#define EAST 0x00800000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup4", 1 },
	{ "gpio", 0 },
};

static const unsigned int sm6115_pin_offsets[] = {
	[0] = WEST,
	[1] = WEST,
	[2] = WEST,
	[3] = WEST,
	[4] = WEST,
	[5] = WEST,
	[6] = WEST,
	[7] = WEST,
	[8] = EAST,
	[9] = EAST,
	[10] = EAST,
	[11] = EAST,
	[12] = WEST,
	[13] = WEST,
	[14] = WEST,
	[15] = WEST,
	[16] = WEST,
	[17] = WEST,
	[18] = EAST,
	[19] = EAST,
	[20] = EAST,
	[21] = EAST,
	[22] = EAST,
	[23] = EAST,
	[24] = EAST,
	[25] = EAST,
	[26] = EAST,
	[27] = EAST,
	[28] = EAST,
	[29] = EAST,
	[30] = EAST,
	[31] = EAST,
	[32] = EAST,
	[33] = EAST,
	[34] = EAST,
	[35] = EAST,
	[36] = EAST,
	[37] = EAST,
	[38] = EAST,
	[39] = EAST,
	[40] = EAST,
	[41] = EAST,
	[42] = EAST,
	[43] = EAST,
	[44] = EAST,
	[45] = EAST,
	[46] = EAST,
	[47] = EAST,
	[48] = EAST,
	[49] = EAST,
	[50] = EAST,
	[51] = EAST,
	[52] = EAST,
	[53] = EAST,
	[54] = EAST,
	[55] = EAST,
	[56] = EAST,
	[57] = EAST,
	[58] = EAST,
	[59] = EAST,
	[60] = EAST,
	[61] = EAST,
	[62] = EAST,
	[63] = EAST,
	[64] = EAST,
	[65] = WEST,
	[66] = WEST,
	[67] = WEST,
	[68] = WEST,
	[69] = WEST,
	[70] = WEST,
	[71] = WEST,
	[72] = SOUTH,
	[73] = SOUTH,
	[74] = SOUTH,
	[75] = SOUTH,
	[76] = SOUTH,
	[77] = SOUTH,
	[78] = SOUTH,
	[79] = SOUTH,
	[80] = WEST,
	[81] = WEST,
	[82] = WEST,
	[83] = WEST,
	[84] = WEST,
	[85] = WEST,
	[86] = WEST,
	[87] = EAST,
	[88] = EAST,
	[89] = WEST,
	[90] = EAST,
	[91] = EAST,
	[92] = WEST,
	[93] = WEST,
	[94] = WEST,
	[95] = WEST,
	[96] = WEST,
	[97] = WEST,
	[98] = SOUTH,
	[99] = SOUTH,
	[100] = SOUTH,
	[101] = SOUTH,
	[102] = SOUTH,
	[103] = SOUTH,
	[104] = SOUTH,
	[105] = SOUTH,
	[106] = SOUTH,
	[107] = SOUTH,
	[108] = SOUTH,
	[109] = SOUTH,
	[110] = SOUTH,
	[111] = SOUTH,
	[112] = SOUTH,
	/* Special pins */
	[113] = 0,
	[114] = 0,
	[115] = 0,
	[116] = 0,
	[117] = 0,
	[118] = 0,
	[119] = 0,
	[120] = 0,
};

static const char *sm6115_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm6115_get_pin_name(struct udevice *dev, unsigned int selector)
{
	static const char *special_pins_names[] = {
		"ufs_reset", "sdc1_rclk", "sdc1_clk", "sdc1_cmd",
		"sdc1_data", "sdc2_clk",  "sdc2_cmd", "sdc2_data",
	};

	if (selector >= 113 && selector <= 120)
		snprintf(pin_name, MAX_PIN_NAME_LEN, special_pins_names[selector - 113]);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sm6115_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

struct msm_pinctrl_data sm6115_data = {
	.pin_data = {
		.pin_offsets = sm6115_pin_offsets,
		.pin_count = ARRAY_SIZE(sm6115_pin_offsets),
		.special_pins_start = 113,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm6115_get_function_name,
	.get_function_mux = sm6115_get_function_mux,
	.get_pin_name = sm6115_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,sm6115-tlmm",
		.data = (ulong)&sm6115_data
	},
	{ /* Sentinel */ } };

U_BOOT_DRIVER(pinctrl_sm6115) = {
	.name = "pinctrl_sm6115",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
