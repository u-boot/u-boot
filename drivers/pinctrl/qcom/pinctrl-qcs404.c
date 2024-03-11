// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm QCS404 pinctrl
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <dm.h>

#include "pinctrl-qcom.h"

#define NORTH	0x00300000
#define SOUTH	0x00000000
#define EAST	0x06b00000

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");
static const char * const msm_pinctrl_pins[] = {
	"sdc1_rclk",
	"sdc1_clk",
	"sdc1_cmd",
	"sdc1_data",
	"sdc2_clk",
	"sdc2_cmd",
	"sdc2_data",
};

static const struct pinctrl_function msm_pinctrl_functions[] = {
	{"gpio", 0},
	{"rgmii_int", 1},
	{"rgmii_ck", 1},
	{"rgmii_tx", 1},
	{"rgmii_ctl", 1},
	{"rgmii_rx", 1},
	{"rgmii_mdio", 1},
	{"rgmii_mdc", 1},
	{"blsp_i2c0", 3},
	{"blsp_i2c1", 2},
	{"blsp_i2c_sda_a2", 3},
	{"blsp_i2c_scl_a2", 3},
	{"blsp_i2c3", 2},
	{"blsp_i2c4", 1},
	{"blsp_uart_tx_a2", 1},
	{"blsp_uart_rx_a2", 1},
};

static const unsigned int qcs404_pin_offsets[] = {
	[0] = SOUTH,    [1] = SOUTH,    [2] = SOUTH,    [3] = SOUTH,    [4] = SOUTH,
	[5] = SOUTH,   [6] = SOUTH,   [7] = SOUTH,   [8] = SOUTH,    [9] = SOUTH,
	[10] = SOUTH,   [11] = SOUTH,   [12] = SOUTH,  [13] = SOUTH,  [14] = SOUTH,
	[15] = SOUTH,  [16] = SOUTH,  [17] = NORTH,  [18] = NORTH,  [19] = NORTH,
	[20] = NORTH,  [21] = SOUTH,  [22] = NORTH,  [23] = NORTH,  [24] = NORTH,
	[25] = NORTH,  [26] = EAST,  [27] = EAST,   [28] = EAST,   [29] = EAST,
	[30] = NORTH,   [31] = NORTH,  [32] = NORTH,  [33] = NORTH,  [34] = SOUTH,
	[35] = SOUTH,  [36] = NORTH,  [37] = NORTH,  [38] = NORTH,  [39] = EAST,
	[40] = EAST,  [41] = EAST,   [42] = EAST,   [43] = EAST,   [44] = EAST,
	[45] = EAST,   [46] = EAST,   [47] = EAST,   [48] = EAST,   [49] = EAST,
	[50] = EAST,  [51] = EAST,  [52] = EAST,  [53] = EAST,  [54] = EAST,
	[55] = EAST,  [56] = EAST,  [57] = EAST,  [58] = EAST,  [59] = EAST,
	[60] = NORTH,  [61] = NORTH,  [62] = NORTH,  [63] = NORTH,  [64] = NORTH,
	[65] = NORTH,  [66] = NORTH,  [67] = NORTH,  [68] = NORTH,  [69] = NORTH,
	[70] = NORTH,   [71] = NORTH,   [72] = NORTH,   [73] = NORTH,   [74] = NORTH,
	[75] = NORTH,   [76] = NORTH,   [77] = NORTH,   [78] = EAST,   [79] = EAST,
	[80] = EAST,  [81] = EAST,  [82] = NORTH,  [83] = NORTH,  [84] = NORTH,
	[85] = NORTH,   [86] = EAST,   [87] = EAST,   [88] = EAST,   [89] = EAST,
	[90] = EAST,  [91] = EAST,  [92] = EAST,  [93] = EAST,  [94] = EAST,
	[95] = EAST,  [96] = EAST,  [97] = EAST,  [98] = EAST,  [99] = EAST,
	[100] = EAST, [101] = EAST, [102] = EAST, [103] = EAST, [104] = EAST,
	[105] = EAST, [106] = EAST, [107] = EAST, [108] = EAST, [109] = EAST,
	[110] = EAST, [111] = EAST, [112] = EAST, [113] = EAST, [114] = EAST,
	[115] = EAST, [116] = EAST, [117] = NORTH, [118] = NORTH, [119] = EAST,
	/*
	 * There's 126 pins but the last ones are special and have non-standard registers
	 * so we leave them out here. The pinctrl and GPIO drivers both currently ignore
	 * these pins.
	 */
};

static const char *qcs404_get_function_name(struct udevice *dev,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *qcs404_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector < 120) {
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
		return pin_name;
	} else {
		return msm_pinctrl_pins[selector - 120];
	}
}

static unsigned int qcs404_get_function_mux(__maybe_unused unsigned int pin,
					    unsigned int selector)
{
	return msm_pinctrl_functions[selector].val;
}

static const struct msm_pinctrl_data qcs404_data = {
	.pin_data = {
		.pin_count = 126,
		.pin_offsets = qcs404_pin_offsets,
		.special_pins_start = 120,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = qcs404_get_function_name,
	.get_function_mux = qcs404_get_function_mux,
	.get_pin_name = qcs404_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,qcs404-pinctrl", .data = (ulong)&qcs404_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_qcs404) = {
	.name		= "pinctrl_qcs404",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
};
