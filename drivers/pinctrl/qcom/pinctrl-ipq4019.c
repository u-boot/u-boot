// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm IPQ40xx pinctrl
 *
 * Copyright (c) 2019 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

enum ipq4019_functions {
	qca_mux_gpio,
	qca_mux_aud_pin,
	qca_mux_audio_pwm,
	qca_mux_blsp_i2c0,
	qca_mux_blsp_i2c1,
	qca_mux_blsp_spi0,
	qca_mux_blsp_spi1,
	qca_mux_blsp_uart0,
	qca_mux_blsp_uart1,
	qca_mux_chip_rst,
	qca_mux_i2s_rx,
	qca_mux_i2s_spdif_in,
	qca_mux_i2s_spdif_out,
	qca_mux_i2s_td,
	qca_mux_i2s_tx,
	qca_mux_jtag,
	qca_mux_led0,
	qca_mux_led1,
	qca_mux_led2,
	qca_mux_led3,
	qca_mux_led4,
	qca_mux_led5,
	qca_mux_led6,
	qca_mux_led7,
	qca_mux_led8,
	qca_mux_led9,
	qca_mux_led10,
	qca_mux_led11,
	qca_mux_mdc,
	qca_mux_mdio,
	qca_mux_pcie,
	qca_mux_pmu,
	qca_mux_prng_rosc,
	qca_mux_qpic,
	qca_mux_rgmii,
	qca_mux_rmii,
	qca_mux_sdio,
	qca_mux_smart0,
	qca_mux_smart1,
	qca_mux_smart2,
	qca_mux_smart3,
	qca_mux_tm,
	qca_mux_wifi0,
	qca_mux_wifi1,
	qca_mux_NA,
};

#define QCA_PIN_FUNCTION(fname)				\
	[qca_mux_##fname] = {#fname, qca_mux_##fname}

static const struct pinctrl_function msm_pinctrl_functions[] = {
	QCA_PIN_FUNCTION(aud_pin),
	QCA_PIN_FUNCTION(audio_pwm),
	QCA_PIN_FUNCTION(blsp_i2c0),
	QCA_PIN_FUNCTION(blsp_i2c1),
	QCA_PIN_FUNCTION(blsp_spi0),
	QCA_PIN_FUNCTION(blsp_spi1),
	QCA_PIN_FUNCTION(blsp_uart0),
	QCA_PIN_FUNCTION(blsp_uart1),
	QCA_PIN_FUNCTION(chip_rst),
	QCA_PIN_FUNCTION(gpio),
	QCA_PIN_FUNCTION(i2s_rx),
	QCA_PIN_FUNCTION(i2s_spdif_in),
	QCA_PIN_FUNCTION(i2s_spdif_out),
	QCA_PIN_FUNCTION(i2s_td),
	QCA_PIN_FUNCTION(i2s_tx),
	QCA_PIN_FUNCTION(jtag),
	QCA_PIN_FUNCTION(led0),
	QCA_PIN_FUNCTION(led1),
	QCA_PIN_FUNCTION(led2),
	QCA_PIN_FUNCTION(led3),
	QCA_PIN_FUNCTION(led4),
	QCA_PIN_FUNCTION(led5),
	QCA_PIN_FUNCTION(led6),
	QCA_PIN_FUNCTION(led7),
	QCA_PIN_FUNCTION(led8),
	QCA_PIN_FUNCTION(led9),
	QCA_PIN_FUNCTION(led10),
	QCA_PIN_FUNCTION(led11),
	QCA_PIN_FUNCTION(mdc),
	QCA_PIN_FUNCTION(mdio),
	QCA_PIN_FUNCTION(pcie),
	QCA_PIN_FUNCTION(pmu),
	QCA_PIN_FUNCTION(prng_rosc),
	QCA_PIN_FUNCTION(qpic),
	QCA_PIN_FUNCTION(rgmii),
	QCA_PIN_FUNCTION(rmii),
	QCA_PIN_FUNCTION(sdio),
	QCA_PIN_FUNCTION(smart0),
	QCA_PIN_FUNCTION(smart1),
	QCA_PIN_FUNCTION(smart2),
	QCA_PIN_FUNCTION(smart3),
	QCA_PIN_FUNCTION(tm),
	QCA_PIN_FUNCTION(wifi0),
	QCA_PIN_FUNCTION(wifi1),
};

typedef unsigned int msm_pin_function[15];

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) \
	[id] = {        qca_mux_gpio, /* gpio mode */	\
			qca_mux_##f1,			\
			qca_mux_##f2,			\
			qca_mux_##f3,			\
			qca_mux_##f4,			\
			qca_mux_##f5,			\
			qca_mux_##f6,			\
			qca_mux_##f7,			\
			qca_mux_##f8,			\
			qca_mux_##f9,			\
			qca_mux_##f10,			\
			qca_mux_##f11,			\
			qca_mux_##f12,			\
			qca_mux_##f13,			\
			qca_mux_##f14			\
	}

static const msm_pin_function ipq4019_pin_functions[] = {
	PINGROUP(0, jtag, smart0, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(1, jtag, smart0, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(2, jtag, smart0, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(3, jtag, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(4, jtag, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(5, jtag, smart0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(6, mdio, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(7, mdc, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(8, blsp_uart1, NA, NA, smart1, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(9, blsp_uart1, NA, NA, smart1, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(10, blsp_uart1, NA, NA, blsp_i2c0, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(11, blsp_uart1, NA, NA, blsp_i2c0, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(12, blsp_spi0, blsp_i2c1, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(13, blsp_spi0, blsp_i2c1, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(14, blsp_spi0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(15, blsp_spi0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(16, blsp_uart0, led0, smart1, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(17, blsp_uart0, led1, smart1, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(18, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(19, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(20, blsp_i2c0, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(21, blsp_i2c0, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(22, rgmii, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(23, sdio, rgmii, i2s_rx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(24, sdio, rgmii, i2s_tx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(25, sdio, rgmii, i2s_tx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(26, sdio, rgmii, i2s_tx, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(27, sdio, rgmii, i2s_td, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(28, sdio, rgmii, i2s_td, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(29, sdio, rgmii, i2s_td, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(30, sdio, rgmii, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(31, sdio, rgmii, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(32, sdio, rgmii, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(33, rgmii, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(34, blsp_i2c1, i2s_spdif_in, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(35, blsp_i2c1, i2s_spdif_out, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(36, rmii, led2, led0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(37, rmii, wifi0, wifi1, led1, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(38, rmii, led2, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(39, rmii, pcie, led3, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(40, rmii, wifi0, wifi1, smart2, led4, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(41, rmii, wifi0, wifi1, smart2, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(42, rmii, wifi0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(43, rmii, wifi1, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(44, rmii, blsp_spi1, smart0, led5, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(45, rmii, blsp_spi1, blsp_spi0, smart0, led6, NA, NA, NA, NA,
		 NA, NA, NA, NA, NA),
	PINGROUP(46, rmii, blsp_spi1, smart0, led7, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(47, rmii, blsp_spi1, smart0, led8, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(48, rmii, aud_pin, smart2, led9, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(49, rmii, aud_pin, smart2, led10, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(50, rmii, aud_pin, wifi0, wifi1, led11, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(51, rmii, aud_pin, wifi0, wifi1, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(52, qpic, mdc, pcie, i2s_tx, NA, NA, NA, tm, wifi0, wifi1, NA,
		 NA, NA, NA),
	PINGROUP(53, qpic, mdio, i2s_tx, prng_rosc, NA, tm, wifi0, wifi1, NA,
		 NA, NA, NA, NA, NA),
	PINGROUP(54, qpic, blsp_spi0, i2s_td, NA, pmu, NA, NA, NA, tm, NA, NA,
		 NA, NA, NA),
	PINGROUP(55, qpic, blsp_spi0, i2s_td, NA, pmu, NA, NA, NA, tm, NA, NA,
		 NA, NA, NA),
	PINGROUP(56, qpic, blsp_spi0, i2s_td, NA, NA, tm, wifi0, wifi1, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(57, qpic, blsp_spi0, i2s_tx, NA, NA, tm, wifi0, wifi1, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(58, qpic, led2, blsp_i2c0, smart3, smart1, i2s_rx, NA, NA, tm,
		 wifi0, wifi1, NA, NA, NA),
	PINGROUP(59, qpic, blsp_i2c0, smart3, smart1, i2s_spdif_in, NA, NA, NA,
		 NA, NA, tm, NA, NA, NA),
	PINGROUP(60, qpic, blsp_uart0, smart1, smart3, led0, i2s_tx, i2s_rx,
		 NA, NA, NA, NA, NA, tm, NA),
	PINGROUP(61, qpic, blsp_uart0, smart1, smart3, led1, i2s_tx, i2s_rx,
		 NA, NA, NA, NA, NA, tm, NA),
	PINGROUP(62, qpic, chip_rst, NA, NA, i2s_spdif_out, NA, NA, NA, NA, NA,
		 tm, NA, NA, NA),
	PINGROUP(63, qpic, NA, NA, NA, i2s_td, i2s_rx, i2s_spdif_out,
		 i2s_spdif_in, NA, NA, NA, NA, tm, NA),
	PINGROUP(64, qpic, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(65, qpic, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(66, qpic, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(67, qpic, audio_pwm, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(68, qpic, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(69, qpic, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(70, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(71, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(72, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(73, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(74, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(75, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(76, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(77, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(78, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(79, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(80, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(81, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(82, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(83, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(84, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(85, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(86, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(87, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(88, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(89, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(90, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(91, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(92, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(93, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(94, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(95, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(96, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(97, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(98, wifi0, wifi1, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(99, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
};

static const char *ipq4019_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *ipq4019_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static int ipq4019_get_function_mux(unsigned int pin, unsigned int selector)
{
	unsigned int i;
	const msm_pin_function *func = ipq4019_pin_functions + pin;

	for (i = 0; i < 15; i++)
		if ((*func)[i] == selector)
			return i;

	pr_err("Can't find requested function for pin %u pin\n", pin);
	return -EINVAL;
}

static const struct msm_pinctrl_data ipq4019_data = {
	.pin_data = {
		.pin_count = 100,
		.special_pins_start = 100, /* There are no special pins */
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = ipq4019_get_function_name,
	.get_function_mux = ipq4019_get_function_mux,
	.get_pin_name = ipq4019_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,ipq4019-pinctrl", .data = (ulong)&ipq4019_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_ipq4019) = {
	.name		= "pinctrl_ipq4019",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
