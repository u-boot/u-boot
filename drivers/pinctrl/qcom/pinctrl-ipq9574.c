// SPDX-License-Identifier: GPL-2.0
/*
 * pinctrl driver for Qualcomm ipq9574
 *
 * (C) Copyright 2025 Linaro Ltd.
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

enum ipq9574_functions {
	msm_mux_blsp0_spi,
	msm_mux_blsp0_uart,
	msm_mux_blsp1_i2c,
	msm_mux_blsp1_spi,
	msm_mux_blsp1_uart,
	msm_mux_blsp2_i2c,
	msm_mux_blsp2_spi,
	msm_mux_blsp2_uart,
	msm_mux_blsp3_i2c,
	msm_mux_blsp3_spi,
	msm_mux_blsp3_uart,
	msm_mux_blsp4_i2c,
	msm_mux_blsp4_spi,
	msm_mux_blsp4_uart,
	msm_mux_blsp5_i2c,
	msm_mux_blsp5_uart,
	msm_mux_gpio,
	msm_mux_mdc,
	msm_mux_mdio,
	msm_mux_pcie0_clk,
	msm_mux_pcie0_wake,
	msm_mux_pcie1_clk,
	msm_mux_pcie1_wake,
	msm_mux_pcie2_clk,
	msm_mux_pcie2_wake,
	msm_mux_pcie3_clk,
	msm_mux_pcie3_wake,
	msm_mux_qspi_data,
	msm_mux_qspi_clk,
	msm_mux_qspi_cs,
	msm_mux_sdc_data,
	msm_mux_sdc_clk,
	msm_mux_sdc_cmd,
	msm_mux_sdc_rclk,
	msm_mux_NA,
};

#define MSM_PIN_FUNCTION(fname)				\
	[msm_mux_##fname] = {#fname, msm_mux_##fname}

static const struct pinctrl_function msm_pinctrl_functions[] = {
	MSM_PIN_FUNCTION(blsp0_spi),
	MSM_PIN_FUNCTION(blsp0_uart),
	MSM_PIN_FUNCTION(blsp1_i2c),
	MSM_PIN_FUNCTION(blsp1_spi),
	MSM_PIN_FUNCTION(blsp1_uart),
	MSM_PIN_FUNCTION(blsp2_i2c),
	MSM_PIN_FUNCTION(blsp2_spi),
	MSM_PIN_FUNCTION(blsp2_uart),
	MSM_PIN_FUNCTION(blsp3_i2c),
	MSM_PIN_FUNCTION(blsp3_spi),
	MSM_PIN_FUNCTION(blsp3_uart),
	MSM_PIN_FUNCTION(blsp4_i2c),
	MSM_PIN_FUNCTION(blsp4_spi),
	MSM_PIN_FUNCTION(blsp4_uart),
	MSM_PIN_FUNCTION(blsp5_i2c),
	MSM_PIN_FUNCTION(blsp5_uart),
	MSM_PIN_FUNCTION(gpio),
	MSM_PIN_FUNCTION(mdc),
	MSM_PIN_FUNCTION(mdio),
	MSM_PIN_FUNCTION(pcie0_clk),
	MSM_PIN_FUNCTION(pcie0_wake),
	MSM_PIN_FUNCTION(pcie1_clk),
	MSM_PIN_FUNCTION(pcie1_wake),
	MSM_PIN_FUNCTION(pcie2_clk),
	MSM_PIN_FUNCTION(pcie2_wake),
	MSM_PIN_FUNCTION(pcie3_clk),
	MSM_PIN_FUNCTION(pcie3_wake),
	MSM_PIN_FUNCTION(qspi_data),
	MSM_PIN_FUNCTION(qspi_clk),
	MSM_PIN_FUNCTION(qspi_cs),
	MSM_PIN_FUNCTION(sdc_data),
	MSM_PIN_FUNCTION(sdc_clk),
	MSM_PIN_FUNCTION(sdc_cmd),
	MSM_PIN_FUNCTION(sdc_rclk),
};

typedef unsigned int msm_pin_function[10];

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9) \
	[id] = {        msm_mux_gpio, /* gpio mode */	\
			msm_mux_##f1,			\
			msm_mux_##f2,			\
			msm_mux_##f3,			\
			msm_mux_##f4,			\
			msm_mux_##f5,			\
			msm_mux_##f6,			\
			msm_mux_##f7,			\
			msm_mux_##f8,			\
			msm_mux_##f9,			\
	}

static const msm_pin_function ipq9574_pin_functions[] = {
	PINGROUP(0, sdc_data, qspi_data, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(1, sdc_data, qspi_data, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(2, sdc_data, qspi_data, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(3, sdc_data, qspi_data, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(4, sdc_cmd, qspi_cs, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(5, sdc_clk, qspi_clk, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(6, sdc_data, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(7, sdc_data, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(8, sdc_data, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(9, sdc_data, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(10, sdc_rclk, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(11, blsp0_spi, blsp0_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(12, blsp0_spi, blsp0_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(13, blsp0_spi, blsp0_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(14, blsp0_spi, blsp0_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(15, blsp3_spi, blsp3_i2c, blsp3_uart, NA, NA, NA, NA, NA, NA),
	PINGROUP(16, blsp3_spi, blsp3_i2c, blsp3_uart, NA, NA, NA, NA, NA, NA),
	PINGROUP(17, blsp3_spi, blsp3_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(18, blsp3_spi, blsp3_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(19, blsp3_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(20, blsp3_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(21, blsp3_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(22, pcie0_clk, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(23, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(24, pcie0_wake, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(25, pcie1_clk, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(26, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(27, pcie1_wake, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(28, pcie2_clk, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(29, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(30, pcie2_wake, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(31, pcie3_clk, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(32, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(33, pcie3_wake, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(34, blsp2_uart, blsp2_i2c, blsp2_spi, blsp1_uart, NA, NA, NA, NA, NA),
	PINGROUP(35, blsp2_uart, blsp2_i2c, blsp2_spi, blsp1_uart, NA, NA, NA, NA, NA),
	PINGROUP(36, blsp1_uart, blsp1_i2c, blsp2_spi, NA, NA, NA, NA, NA, NA),
	PINGROUP(37, blsp1_uart, blsp1_i2c, blsp2_spi, NA, NA, NA, NA, NA, NA),
	PINGROUP(38, mdc, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(39, mdio, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(40, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(41, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(42, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(43, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(44, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(45, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(46, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(47, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(48, blsp5_i2c, blsp5_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(49, blsp5_i2c, blsp5_uart, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(50, blsp4_uart, blsp4_i2c, blsp4_spi, NA, NA, NA, NA, NA, NA),
	PINGROUP(51, blsp4_uart, blsp4_i2c, blsp4_spi, NA, NA, NA, NA, NA, NA),
	PINGROUP(52, blsp4_uart, blsp4_spi, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(53, blsp4_uart, blsp4_spi, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(54, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(55, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(56, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(57, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(58, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(59, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(60, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(61, blsp1_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(62, blsp1_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(63, blsp1_spi, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(64, blsp1_spi, NA, NA, NA, NA, NA, NA, NA, NA),
};

static const char *ipq9574_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *ipq9574_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static int ipq9574_get_function_mux(unsigned int pin, unsigned int selector)
{
	unsigned int i;
	const msm_pin_function *func = ipq9574_pin_functions + pin;

	for (i = 0; i < 10; i++)
		if ((*func)[i] == selector)
			return i;

	debug("Can't find requested function for pin:selector %u:%u\n",
	      pin, selector);

	return -EINVAL;
}

static const struct msm_pinctrl_data ipq9574_data = {
	.pin_data = {
		.pin_count = 65,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = ipq9574_get_function_name,
	.get_function_mux = ipq9574_get_function_mux,
	.get_pin_name = ipq9574_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,ipq9574-tlmm", .data = (ulong)&ipq9574_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_ipq9574) = {
	.name		= "pinctrl_ipq9574",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
	.flags = DM_FLAG_PRE_RELOC,
};
