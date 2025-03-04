// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016-2018,2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

enum ipq5424_functions {
	msm_mux_atest_char,
	msm_mux_atest_char0,
	msm_mux_atest_char1,
	msm_mux_atest_char2,
	msm_mux_atest_char3,
	msm_mux_atest_tic,
	msm_mux_audio_pri,
	msm_mux_audio_pri0,
	msm_mux_audio_pri1,
	msm_mux_audio_sec,
	msm_mux_audio_sec0,
	msm_mux_audio_sec1,
	msm_mux_core_voltage,
	msm_mux_cri_trng0,
	msm_mux_cri_trng1,
	msm_mux_cri_trng2,
	msm_mux_cri_trng3,
	msm_mux_cxc_clk,
	msm_mux_cxc_data,
	msm_mux_dbg_out,
	msm_mux_gcc_plltest,
	msm_mux_gcc_tlmm,
	msm_mux_gpio,
	msm_mux_i2c0_scl,
	msm_mux_i2c0_sda,
	msm_mux_i2c1_scl,
	msm_mux_i2c1_sda,
	msm_mux_i2c11,
	msm_mux_mac0,
	msm_mux_mac1,
	msm_mux_mdc_mst,
	msm_mux_mdc_slv,
	msm_mux_mdio_mst,
	msm_mux_mdio_slv,
	msm_mux_pcie0_clk,
	msm_mux_pcie0_wake,
	msm_mux_pcie1_clk,
	msm_mux_pcie1_wake,
	msm_mux_pcie2_clk,
	msm_mux_pcie2_wake,
	msm_mux_pcie3_clk,
	msm_mux_pcie3_wake,
	msm_mux_pll_test,
	msm_mux_prng_rosc0,
	msm_mux_prng_rosc1,
	msm_mux_prng_rosc2,
	msm_mux_prng_rosc3,
	msm_mux_PTA0_0,
	msm_mux_PTA0_1,
	msm_mux_PTA0_2,
	msm_mux_PTA10,
	msm_mux_PTA11,
	msm_mux_pwm0,
	msm_mux_pwm1,
	msm_mux_pwm2,
	msm_mux_qdss_cti_trig_in_a0,
	msm_mux_qdss_cti_trig_out_a0,
	msm_mux_qdss_cti_trig_in_a1,
	msm_mux_qdss_cti_trig_out_a1,
	msm_mux_qdss_cti_trig_in_b0,
	msm_mux_qdss_cti_trig_out_b0,
	msm_mux_qdss_cti_trig_in_b1,
	msm_mux_qdss_cti_trig_out_b1,
	msm_mux_qdss_traceclk_a,
	msm_mux_qdss_tracectl_a,
	msm_mux_qdss_tracedata_a,
	msm_mux_qspi_clk,
	msm_mux_qspi_cs,
	msm_mux_qspi_data,
	msm_mux_resout,
	msm_mux_rx0,
	msm_mux_rx1,
	msm_mux_rx2,
	msm_mux_sdc_clk,
	msm_mux_sdc_cmd,
	msm_mux_sdc_data,
	msm_mux_spi0_clk,
	msm_mux_spi0_cs,
	msm_mux_spi0_miso,
	msm_mux_spi0_mosi,
	msm_mux_spi1,
	msm_mux_spi10,
	msm_mux_spi11,
	msm_mux_tsens_max,
	msm_mux_uart0,
	msm_mux_uart1,
	msm_mux_wci_txd,
	msm_mux_wci_rxd,
	msm_mux_wsi_clk,
	msm_mux_wsi_data,
	msm_mux__,
};

#define MSM_PIN_FUNCTION(fname)				\
	[msm_mux_##fname] = {#fname, msm_mux_##fname}

static const struct pinctrl_function ipq5424_functions[] = {
	MSM_PIN_FUNCTION(atest_char),
	MSM_PIN_FUNCTION(atest_char0),
	MSM_PIN_FUNCTION(atest_char1),
	MSM_PIN_FUNCTION(atest_char2),
	MSM_PIN_FUNCTION(atest_char3),
	MSM_PIN_FUNCTION(atest_tic),
	MSM_PIN_FUNCTION(audio_pri),
	MSM_PIN_FUNCTION(audio_pri0),
	MSM_PIN_FUNCTION(audio_pri1),
	MSM_PIN_FUNCTION(audio_sec),
	MSM_PIN_FUNCTION(audio_sec0),
	MSM_PIN_FUNCTION(audio_sec1),
	MSM_PIN_FUNCTION(core_voltage),
	MSM_PIN_FUNCTION(cri_trng0),
	MSM_PIN_FUNCTION(cri_trng1),
	MSM_PIN_FUNCTION(cri_trng2),
	MSM_PIN_FUNCTION(cri_trng3),
	MSM_PIN_FUNCTION(cxc_clk),
	MSM_PIN_FUNCTION(cxc_data),
	MSM_PIN_FUNCTION(dbg_out),
	MSM_PIN_FUNCTION(gcc_plltest),
	MSM_PIN_FUNCTION(gcc_tlmm),
	MSM_PIN_FUNCTION(gpio),
	MSM_PIN_FUNCTION(i2c0_scl),
	MSM_PIN_FUNCTION(i2c0_sda),
	MSM_PIN_FUNCTION(i2c1_scl),
	MSM_PIN_FUNCTION(i2c1_sda),
	MSM_PIN_FUNCTION(i2c11),
	MSM_PIN_FUNCTION(mac0),
	MSM_PIN_FUNCTION(mac1),
	MSM_PIN_FUNCTION(mdc_mst),
	MSM_PIN_FUNCTION(mdc_slv),
	MSM_PIN_FUNCTION(mdio_mst),
	MSM_PIN_FUNCTION(mdio_slv),
	MSM_PIN_FUNCTION(pcie0_clk),
	MSM_PIN_FUNCTION(pcie0_wake),
	MSM_PIN_FUNCTION(pcie1_clk),
	MSM_PIN_FUNCTION(pcie1_wake),
	MSM_PIN_FUNCTION(pcie2_clk),
	MSM_PIN_FUNCTION(pcie2_wake),
	MSM_PIN_FUNCTION(pcie3_clk),
	MSM_PIN_FUNCTION(pcie3_wake),
	MSM_PIN_FUNCTION(pll_test),
	MSM_PIN_FUNCTION(prng_rosc0),
	MSM_PIN_FUNCTION(prng_rosc1),
	MSM_PIN_FUNCTION(prng_rosc2),
	MSM_PIN_FUNCTION(prng_rosc3),
	MSM_PIN_FUNCTION(PTA0_0),
	MSM_PIN_FUNCTION(PTA0_1),
	MSM_PIN_FUNCTION(PTA0_2),
	MSM_PIN_FUNCTION(PTA10),
	MSM_PIN_FUNCTION(PTA11),
	MSM_PIN_FUNCTION(pwm0),
	MSM_PIN_FUNCTION(pwm1),
	MSM_PIN_FUNCTION(pwm2),
	MSM_PIN_FUNCTION(qdss_cti_trig_in_a0),
	MSM_PIN_FUNCTION(qdss_cti_trig_out_a0),
	MSM_PIN_FUNCTION(qdss_cti_trig_in_a1),
	MSM_PIN_FUNCTION(qdss_cti_trig_out_a1),
	MSM_PIN_FUNCTION(qdss_cti_trig_in_b0),
	MSM_PIN_FUNCTION(qdss_cti_trig_out_b0),
	MSM_PIN_FUNCTION(qdss_cti_trig_in_b1),
	MSM_PIN_FUNCTION(qdss_cti_trig_out_b1),
	MSM_PIN_FUNCTION(qdss_traceclk_a),
	MSM_PIN_FUNCTION(qdss_tracectl_a),
	MSM_PIN_FUNCTION(qdss_tracedata_a),
	MSM_PIN_FUNCTION(qspi_clk),
	MSM_PIN_FUNCTION(qspi_cs),
	MSM_PIN_FUNCTION(qspi_data),
	MSM_PIN_FUNCTION(resout),
	MSM_PIN_FUNCTION(rx0),
	MSM_PIN_FUNCTION(rx1),
	MSM_PIN_FUNCTION(rx2),
	MSM_PIN_FUNCTION(sdc_clk),
	MSM_PIN_FUNCTION(sdc_cmd),
	MSM_PIN_FUNCTION(sdc_data),
	MSM_PIN_FUNCTION(spi0_clk),
	MSM_PIN_FUNCTION(spi0_cs),
	MSM_PIN_FUNCTION(spi0_miso),
	MSM_PIN_FUNCTION(spi0_mosi),
	MSM_PIN_FUNCTION(spi1),
	MSM_PIN_FUNCTION(spi10),
	MSM_PIN_FUNCTION(spi11),
	MSM_PIN_FUNCTION(tsens_max),
	MSM_PIN_FUNCTION(uart0),
	MSM_PIN_FUNCTION(uart1),
	MSM_PIN_FUNCTION(wci_txd),
	MSM_PIN_FUNCTION(wci_rxd),
	MSM_PIN_FUNCTION(wsi_clk),
	MSM_PIN_FUNCTION(wsi_data),
};

typedef unsigned int msm_pin_function[10];

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9)\
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

static const msm_pin_function ipq5424_pin_functions[] = {
	PINGROUP(0, sdc_data, qspi_data, pwm2, wci_txd, wci_rxd, _, _, _, _),
	PINGROUP(1, sdc_data, qspi_data, pwm2, wci_txd, wci_rxd, _, _, _, _),
	PINGROUP(2, sdc_data, qspi_data, pwm2, _, _, _, _, _, _),
	PINGROUP(3, sdc_data, qspi_data, pwm2, _, _, _, _, _, _),
	PINGROUP(4, sdc_cmd, qspi_cs, _, _, _, _, _, _, _),
	PINGROUP(5, sdc_clk, qspi_clk, _, _, _, _, _, _, _),
	PINGROUP(6, spi0_clk, pwm1, _, cri_trng0, qdss_tracedata_a, _, _, _, _),
	PINGROUP(7, spi0_cs, pwm1, _, cri_trng1, qdss_tracedata_a, _, _, _, _),
	PINGROUP(8, spi0_miso, pwm1, wci_txd, wci_rxd, _, cri_trng2, qdss_tracedata_a, _, _),
	PINGROUP(9, spi0_mosi, pwm1, _, cri_trng3, qdss_tracedata_a, _, _, _, _),
	PINGROUP(10, uart0, pwm0, spi11, _, wci_txd, wci_rxd, _, qdss_tracedata_a, _),
	PINGROUP(11, uart0, pwm0, spi1, _, wci_txd, wci_rxd, _, qdss_tracedata_a, _),
	PINGROUP(12, uart0, pwm0, spi11, _, prng_rosc0, qdss_tracedata_a, _, _, _),
	PINGROUP(13, uart0, pwm0, spi11, _, prng_rosc1, qdss_tracedata_a, _, _, _),
	PINGROUP(14, i2c0_scl, tsens_max, _, prng_rosc2, qdss_tracedata_a, _, _, _, _),
	PINGROUP(15, i2c0_sda, _, prng_rosc3, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(16, core_voltage, i2c1_scl, _, _, _, _, _, _, _),
	PINGROUP(17, core_voltage, i2c1_sda, _, _, _, _, _, _, _),
	PINGROUP(18, _, _, _, _, _, _, _, _, _),
	PINGROUP(19, _, _, _, _, _, _, _, _, _),
	PINGROUP(20, mdc_slv, atest_char0, _, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(21, mdio_slv, atest_char1, _, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(22, mdc_mst, atest_char2, _, _, _, _, _, _, _),
	PINGROUP(23, mdio_mst, atest_char3, _, _, _, _, _, _, _),
	PINGROUP(24, pcie0_clk, PTA10, mac0, _, wsi_clk, _, atest_char, qdss_cti_trig_out_a0, _),
	PINGROUP(25, _, _, _, _, _, _, _, _, _),
	PINGROUP(26, pcie0_wake, PTA10, mac0, _, wsi_data, _, qdss_cti_trig_in_a0, _, _),
	PINGROUP(27, pcie1_clk, i2c11, PTA10, wsi_clk, qdss_cti_trig_out_a1, _, _, _, _),
	PINGROUP(28, _, _, _, _, _, _, _, _, _),
	PINGROUP(29, pcie1_wake, i2c11, wsi_data, qdss_cti_trig_in_a1, _, _, _, _, _),
	PINGROUP(30, pcie2_clk, PTA11, mac1, qdss_cti_trig_out_b0, _, _, _, _, _),
	PINGROUP(31, _, _, _, _, _, _, _, _, _),
	PINGROUP(32, pcie2_wake, PTA11, mac1, audio_pri0, audio_pri0, qdss_cti_trig_in_b0, _, _, _),
	PINGROUP(33, pcie3_clk, PTA11, audio_pri1, audio_pri1, qdss_cti_trig_out_b1, _, _, _, _),
	PINGROUP(34, _, _, _, _, _, _, _, _, _),
	PINGROUP(35, pcie3_wake, audio_sec1, audio_sec1, qdss_cti_trig_in_b1, _, _, _, _, _),
	PINGROUP(36, audio_pri, spi1, audio_sec0, audio_sec0, qdss_tracedata_a, _, _, _, _),
	PINGROUP(37, audio_pri, spi1, rx2, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(38, audio_pri, spi1, pll_test, rx1, qdss_tracedata_a, _, _, _, _),
	PINGROUP(39, audio_pri, rx0, _, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(40, PTA0_0, wci_txd, wci_rxd, _, atest_tic, _, _, _, _),
	PINGROUP(41, PTA0_1, wci_txd, wci_rxd, cxc_data, _, _, _, _, _),
	PINGROUP(42, PTA0_2, cxc_clk, _, _, _, _, _, _, _),
	PINGROUP(43, uart1, gcc_plltest, _, _, _, _, _, _, _),
	PINGROUP(44, uart1, gcc_tlmm, _, _, _, _, _, _, _),
	PINGROUP(45, spi10, rx2, audio_sec, gcc_plltest, _, qdss_traceclk_a, _, _, _),
	PINGROUP(46, spi1, rx1, audio_sec, dbg_out, qdss_tracectl_a, _, _, _, _),
	PINGROUP(47, spi10, rx0, audio_sec, _, _, _, _, _, _),
	PINGROUP(48, spi10, audio_sec, _, _, _, _, _, _, _),
	PINGROUP(49, resout, _, _, _, _, _, _, _, _),
};

static const char *ipq5424_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return ipq5424_functions[selector].name;
}

static const char *ipq5424_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);
	return pin_name;
}

static int ipq5424_get_function_mux(unsigned int pin, unsigned int selector)
{
	unsigned int i;
	const msm_pin_function *func = ipq5424_pin_functions + pin;

	for (i = 0; i < 10; i++)
		if ((*func)[i] == selector)
			return i;

	debug("Can't find requested function for pin:selector %u:%u\n",
	      pin, selector);

	return -EINVAL;
}

static const struct msm_pinctrl_data ipq5424_data = {
	.pin_data = {
		.pin_count = 49,
	},
	.functions_count = ARRAY_SIZE(ipq5424_functions),
	.get_function_name = ipq5424_get_function_name,
	.get_function_mux = ipq5424_get_function_mux,
	.get_pin_name = ipq5424_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,ipq5424-tlmm", .data = (ulong)&ipq5424_data },
	{ /* Sentinal */ }
};

U_BOOT_DRIVER(pinctrl_ipq5424) = {
	.name		= "pinctrl_ipq5424",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
	.flags = DM_FLAG_PRE_RELOC,
};
