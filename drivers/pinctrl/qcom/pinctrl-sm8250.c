// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm sm8250 pinctrl
 *
 * (C) Copyright 2024 Linaro Ltd.
 *
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

static const unsigned int sm8250_pin_offsets[] = {
	[0] = SOUTH,   [1] = SOUTH,   [2] = SOUTH,   [3] = SOUTH,   [4] = NORTH,   [5] = NORTH,
	[6] = NORTH,   [7] = NORTH,   [8] = NORTH,   [9] = NORTH,   [10] = NORTH,  [11] = NORTH,
	[12] = NORTH,  [13] = NORTH,  [14] = NORTH,  [15] = NORTH,  [16] = NORTH,  [17] = NORTH,
	[18] = NORTH,  [19] = NORTH,  [20] = NORTH,  [21] = NORTH,  [22] = NORTH,  [23] = NORTH,
	[24] = SOUTH,  [25] = SOUTH,  [26] = SOUTH,  [27] = SOUTH,  [28] = NORTH,  [29] = NORTH,
	[30] = NORTH,  [31] = NORTH,  [32] = SOUTH,  [33] = SOUTH,  [34] = SOUTH,  [35] = SOUTH,
	[36] = SOUTH,  [37] = SOUTH,  [38] = SOUTH,  [39] = SOUTH,  [40] = SOUTH,  [41] = SOUTH,
	[42] = SOUTH,  [43] = SOUTH,  [44] = SOUTH,  [45] = SOUTH,  [46] = SOUTH,  [47] = SOUTH,
	[48] = SOUTH,  [49] = SOUTH,  [50] = SOUTH,  [51] = SOUTH,  [52] = SOUTH,  [53] = SOUTH,
	[54] = SOUTH,  [55] = SOUTH,  [56] = SOUTH,  [57] = SOUTH,  [58] = SOUTH,  [59] = SOUTH,
	[60] = SOUTH,  [61] = SOUTH,  [62] = SOUTH,  [63] = SOUTH,  [64] = SOUTH,  [65] = SOUTH,
	[66] = NORTH,  [67] = NORTH,  [68] = NORTH,  [69] = SOUTH,  [70] = SOUTH,  [71] = SOUTH,
	[72] = SOUTH,  [73] = SOUTH,  [74] = SOUTH,  [75] = SOUTH,  [76] = SOUTH,  [77] = NORTH,
	[78] = NORTH,  [79] = NORTH,  [80] = NORTH,  [81] = NORTH,  [82] = NORTH,  [83] = NORTH,
	[84] = NORTH,  [85] = SOUTH,  [86] = SOUTH,  [87] = SOUTH,  [88] = SOUTH,  [89] = SOUTH,
	[90] = SOUTH,  [91] = SOUTH,  [92] = NORTH,  [93] = NORTH,  [94] = NORTH,  [95] = NORTH,
	[96] = NORTH,  [97] = NORTH,  [98] = NORTH,  [99] = NORTH,  [100] = NORTH, [101] = NORTH,
	[102] = NORTH, [103] = NORTH, [104] = NORTH, [105] = NORTH, [106] = NORTH, [107] = NORTH,
	[108] = NORTH, [109] = NORTH, [110] = NORTH, [111] = NORTH, [112] = NORTH, [113] = NORTH,
	[114] = NORTH, [115] = NORTH, [116] = NORTH, [117] = NORTH, [118] = NORTH, [119] = NORTH,
	[120] = NORTH, [121] = NORTH, [122] = NORTH, [123] = NORTH, [124] = NORTH, [125] = SOUTH,
	[126] = SOUTH, [127] = SOUTH, [128] = SOUTH, [129] = SOUTH, [130] = SOUTH, [131] = SOUTH,
	[132] = SOUTH, [133] = WEST,  [134] = WEST,  [135] = WEST,  [136] = WEST,  [137] = WEST,
	[138] = WEST,  [139] = WEST,  [140] = WEST,  [141] = WEST,  [142] = WEST,  [143] = WEST,
	[144] = WEST,  [145] = WEST,  [146] = WEST,  [147] = WEST,  [148] = WEST,  [149] = WEST,
	[150] = WEST,  [151] = WEST,  [152] = WEST,  [153] = WEST,  [154] = WEST,  [155] = WEST,
	[156] = WEST,  [157] = WEST,  [158] = WEST,  [159] = WEST,  [160] = WEST,  [161] = WEST,
	[162] = WEST,  [163] = WEST,  [164] = WEST,  [165] = WEST,  [166] = WEST,  [167] = WEST,
	[168] = WEST,  [169] = WEST,  [170] = WEST,  [171] = WEST,  [172] = WEST,  [173] = WEST,
	[174] = WEST,  [175] = WEST,  [176] = WEST,  [177] = WEST,  [178] = WEST,  [179] = WEST,
	[180] = 0,     [181] = 0,     [182] = 0,     [183] = 0,
};

static const char *sm8250_get_function_name(struct udevice *dev, unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm8250_get_pin_name(struct udevice *dev, unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static unsigned int sm8250_get_function_mux(__maybe_unused unsigned int pin, unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm8250_data = {
	.pin_data = {
		.pin_offsets = sm8250_pin_offsets,
		.pin_count = ARRAY_SIZE(sm8250_pin_offsets),
		.special_pins_start = 180,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm8250_get_function_name,
	.get_function_mux = sm8250_get_function_mux,
	.get_pin_name = sm8250_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,sm8250-pinctrl",
		.data = (ulong)&sm8250_data
	},
	{ /* Sentinel */ } };

U_BOOT_DRIVER(pinctrl_sm8250) = {
	.name = "pinctrl_sm8250",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
