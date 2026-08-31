// SPDX-License-Identifier: BSD-3-Clause
/*
 * Pinctrl driver for Qualcomm SM6125
 *
 * (C) Copyright 2026 Biswapriyo Nath <nathbappai@gmail.com>
 *
 * Based on Linux Kernel driver
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define TLMM_BASE 0x00500000
#define WEST (0x00500000 - TLMM_BASE) /* 0x0 */
#define SOUTH (0x00900000 - TLMM_BASE) /* 0x400000 */
#define EAST (0x00d00000 - TLMM_BASE) /* 0x800000 */

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{ "qup04", 1 },
	{ "gpio", 0 },
};

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv) \
	{                                          \
		.name = pg_name,                   \
		.ctl_reg = ctl,                    \
		.io_reg = 0,                       \
		.pull_bit = pull,                  \
		.drv_bit = drv,                    \
		.oe_bit = -1,                      \
		.in_bit = -1,                      \
		.out_bit = -1,                     \
	}

#define UFS_RESET(pg_name, offset)      \
	{                               \
		.name = pg_name,        \
		.ctl_reg = offset,      \
		.io_reg = offset + 0x4, \
		.pull_bit = 3,          \
		.drv_bit = 0,           \
		.oe_bit = -1,           \
		.in_bit = -1,           \
		.out_bit = 0,           \
	}

static const struct msm_special_pin_data msm_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0x190000),
	[1] = SDC_QDSD_PINGROUP("sdc1_rclk", WEST + 0x18d000, 15, 0),
	[2] = SDC_QDSD_PINGROUP("sdc1_clk", WEST + 0x18d000, 13, 6),
	[3] = SDC_QDSD_PINGROUP("sdc1_cmd", WEST + 0x18d000, 11, 3),
	[4] = SDC_QDSD_PINGROUP("sdc1_data", WEST + 0x18d000, 9, 0),
	[5] = SDC_QDSD_PINGROUP("sdc2_clk", SOUTH + 0x58b000, 14, 6),
	[6] = SDC_QDSD_PINGROUP("sdc2_cmd", SOUTH + 0x58b000, 11, 3),
	[7] = SDC_QDSD_PINGROUP("sdc2_data", SOUTH + 0x58b000, 9, 0),
};

static const unsigned int sm6125_pin_offsets[] = {
	[0] = WEST,    [1] = WEST,    [2] = WEST,    [3] = WEST,
	[4] = WEST,    [5] = WEST,    [6] = WEST,    [7] = WEST,
	[8] = WEST,    [9] = WEST,    [10] = EAST,   [11] = EAST,
	[12] = EAST,   [13] = EAST,   [14] = WEST,   [15] = WEST,
	[16] = WEST,   [17] = WEST,   [18] = EAST,   [19] = EAST,
	[20] = EAST,   [21] = EAST,   [22] = WEST,   [23] = WEST,
	[24] = WEST,   [25] = WEST,   [26] = WEST,   [27] = WEST,
	[28] = WEST,   [29] = WEST,   [30] = WEST,   [31] = WEST,
	[32] = WEST,   [33] = WEST,   [34] = SOUTH,  [35] = SOUTH,
	[36] = SOUTH,  [37] = SOUTH,  [38] = EAST,   [39] = EAST,
	[40] = EAST,   [41] = EAST,   [42] = EAST,   [43] = EAST,
	[44] = SOUTH,  [45] = SOUTH,  [46] = SOUTH,  [47] = SOUTH,
	[48] = SOUTH,  [49] = SOUTH,  [50] = SOUTH,  [51] = SOUTH,
	[52] = SOUTH,  [53] = SOUTH,  [54] = SOUTH,  [55] = SOUTH,
	[56] = SOUTH,  [57] = SOUTH,  [58] = SOUTH,  [59] = SOUTH,
	[60] = SOUTH,  [61] = SOUTH,  [62] = SOUTH,  [63] = SOUTH,
	[64] = SOUTH,  [65] = SOUTH,  [66] = SOUTH,  [67] = SOUTH,
	[68] = SOUTH,  [69] = SOUTH,  [70] = SOUTH,  [71] = SOUTH,
	[72] = SOUTH,  [73] = SOUTH,  [74] = SOUTH,  [75] = SOUTH,
	[76] = SOUTH,  [77] = SOUTH,  [78] = SOUTH,  [79] = SOUTH,
	[80] = SOUTH,  [81] = SOUTH,  [82] = SOUTH,  [83] = SOUTH,
	[84] = SOUTH,  [85] = SOUTH,  [86] = SOUTH,  [87] = WEST,
	[88] = WEST,   [89] = WEST,   [90] = WEST,   [91] = WEST,
	[92] = WEST,   [93] = WEST,   [94] = SOUTH,  [95] = SOUTH,
	[96] = SOUTH,  [97] = SOUTH,  [98] = SOUTH,  [99] = SOUTH,
	[100] = SOUTH, [101] = SOUTH, [102] = SOUTH, [103] = SOUTH,
	[104] = EAST,  [105] = EAST,  [106] = EAST,  [107] = EAST,
	[108] = EAST,  [109] = EAST,  [110] = EAST,  [111] = EAST,
	[112] = EAST,  [113] = EAST,  [114] = EAST,  [115] = EAST,
	[116] = EAST,  [117] = SOUTH, [118] = SOUTH, [119] = SOUTH,
	[120] = SOUTH, [121] = EAST,  [122] = EAST,  [123] = EAST,
	[124] = EAST,  [125] = EAST,  [126] = EAST,  [127] = EAST,
	[128] = EAST,  [129] = SOUTH, [130] = SOUTH, [131] = SOUTH,
	[132] = SOUTH,
};

static const char *sm6125_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *sm6125_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= 133 && selector <= 140)
		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 msm_special_pins_data[selector - 133].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int sm6125_get_function_mux(__maybe_unused unsigned int pin,
				   unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static struct msm_pinctrl_data sm6125_data = {
	.pin_data = {
		.pin_offsets = sm6125_pin_offsets,
		.pin_count = 141,
		.special_pins_start = 133,
		.special_pins_data = msm_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = sm6125_get_function_name,
	.get_function_mux = sm6125_get_function_mux,
	.get_pin_name = sm6125_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,sm6125-tlmm", .data = (ulong)&sm6125_data },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pinctrl_sm6125) = {
	.name = "pinctrl_sm6125",
	.id = UCLASS_NOP,
	.of_match = msm_pinctrl_ids,
	.ops = &msm_pinctrl_ops,
	.bind = msm_pinctrl_bind,
};
