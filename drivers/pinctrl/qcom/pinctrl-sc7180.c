// SPDX-License-Identifier: BSD-3-Clause
/*
 * Pinctrl driver for Qualcomm SC7180
 *
 * (C) Copyright 2024 Nikroks <nikroksm@mail.ru>
 *
 * Based on Linux Kernel driver
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define WEST 0x00000000
#define SOUTH 0x00400000
#define NORTH 0x00800000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = { { "qup12", 1 },
								 { "gpio", 0 },
								 { "sdc2_clk", 0 } };

static const unsigned int sc7180_pin_offsets[] = {
	[0] = SOUTH,   [1] = SOUTH,   [2] = SOUTH,   [3] = SOUTH,   [4] = NORTH,
	[5] = NORTH,   [6] = NORTH,   [7] = NORTH,   [8] = NORTH,   [9] = NORTH,
	[10] = NORTH,  [11] = NORTH,  [12] = SOUTH,  [13] = SOUTH,  [14] = SOUTH,
	[15] = SOUTH,  [16] = SOUTH,  [17] = SOUTH,  [18] = SOUTH,  [19] = SOUTH,
	[20] = SOUTH,  [21] = NORTH,  [22] = NORTH,  [23] = SOUTH,  [24] = SOUTH,
	[25] = SOUTH,  [26] = SOUTH,  [27] = SOUTH,  [28] = SOUTH,  [29] = NORTH,
	[30] = SOUTH,  [31] = NORTH,  [32] = NORTH,  [33] = NORTH,  [34] = SOUTH,
	[35] = SOUTH,  [36] = SOUTH,  [37] = SOUTH,  [38] = SOUTH,  [39] = SOUTH,
	[40] = SOUTH,  [41] = SOUTH,  [42] = NORTH,  [43] = NORTH,  [44] = NORTH,
	[45] = NORTH,  [46] = NORTH,  [47] = NORTH,  [48] = NORTH,  [49] = WEST,
	[50] = WEST,   [51] = WEST,   [52] = WEST,   [53] = WEST,   [54] = WEST,
	[55] = WEST,   [56] = WEST,   [57] = WEST,   [58] = WEST,   [59] = NORTH,
	[60] = NORTH,  [61] = NORTH,  [62] = NORTH,  [63] = NORTH,  [64] = NORTH,
	[65] = NORTH,  [66] = NORTH,  [67] = NORTH,  [68] = NORTH,  [69] = WEST,
	[70] = NORTH,  [71] = NORTH,  [72] = NORTH,  [73] = WEST,   [74] = WEST,
	[75] = WEST,   [76] = WEST,   [77] = WEST,   [78] = WEST,   [79] = WEST,
	[80] = WEST,   [81] = WEST,   [82] = WEST,   [83] = WEST,   [84] = WEST,
	[85] = WEST,   [86] = NORTH,  [87] = NORTH,  [88] = NORTH,  [89] = NORTH,
	[90] = NORTH,  [91] = NORTH,  [92] = NORTH,  [93] = NORTH,  [94] = SOUTH,
	[95] = WEST,   [96] = WEST,   [97] = WEST,   [98] = WEST,   [99] = WEST,
	[100] = WEST,  [101] = NORTH, [102] = NORTH, [103] = NORTH, [104] = WEST,
	[105] = NORTH, [106] = NORTH, [107] = WEST,  [108] = SOUTH, [109] = SOUTH,
	[110] = NORTH, [111] = NORTH, [112] = NORTH, [113] = NORTH, [114] = NORTH,
	[115] = WEST,  [116] = WEST,  [117] = WEST,  [118] = WEST,  [119] = 0,
	[120] = 0,     [121] = 0,     [122] = 0,     [123] = 0,     [124] = 0,
	[125] = 0,     [126] = 0,
};

static const char *sc7180_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sc7180_get_pin_name(struct udevice *dev, unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static int sc7180_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sc7180_data = {
	.pin_data = {
		.pin_offsets = sc7180_pin_offsets,
		.pin_count = ARRAY_SIZE(sc7180_pin_offsets),
		.special_pins_start = 120,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sc7180_get_function_name,
	.get_function_mux = sc7180_get_function_mux,
	.get_pin_name = sc7180_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,sc7180-pinctrl",
		.data = (ulong)&sc7180_data
	},
	{ /* Sentinel */ } };

U_BOOT_DRIVER(pinctrl_sc7180) = {
	.name = "pinctrl_sc7180",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
