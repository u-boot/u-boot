// SPDX-License-Identifier: GPL-2.0-only
/*
 * Qualcomm QCS8300 pinctrl
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
#define MSM_PIN_FUNCTION_COUNT 12

static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

typedef unsigned int msm_pin_function[MSM_PIN_FUNCTION_COUNT];

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11)\
	{\
		msm_mux_gpio, /* gpio mode */\
		msm_mux_##f1,\
		msm_mux_##f2,\
		msm_mux_##f3,\
		msm_mux_##f4,\
		msm_mux_##f5,\
		msm_mux_##f6,\
		msm_mux_##f7,\
		msm_mux_##f8,\
		msm_mux_##f9,\
		msm_mux_##f10,\
		msm_mux_##f11 /* egpio mode */\
	}

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)\
	{\
		.name = pg_name,\
		.ctl_reg = ctl,\
		.io_reg = 0,\
		.pull_bit = pull,\
		.drv_bit = drv,\
		.oe_bit = -1,\
		.in_bit = -1,\
		.out_bit = -1,\
	}

#define UFS_RESET(pg_name, ctl)\
	{\
		.name = pg_name,\
		.ctl_reg = ctl,\
		.io_reg = ctl + 0x4,\
		.pull_bit = 3,\
		.drv_bit = 0,\
		.oe_bit = -1,\
		.in_bit = -1,\
		.out_bit = 0,\
	}

enum qcs8300_functions {
	msm_mux_gpio,
	msm_mux_aoss_cti,
	msm_mux_atest_char,
	msm_mux_atest_usb2,
	msm_mux_audio_ref,
	msm_mux_cam_mclk,
	msm_mux_cci_async,
	msm_mux_cci_i2c_scl,
	msm_mux_cci_i2c_sda,
	msm_mux_cci_timer,
	msm_mux_cri_trng,
	msm_mux_dbg_out,
	msm_mux_ddr_bist,
	msm_mux_ddr_pxi0,
	msm_mux_ddr_pxi1,
	msm_mux_ddr_pxi2,
	msm_mux_ddr_pxi3,
	msm_mux_edp0_hot,
	msm_mux_edp0_lcd,
	msm_mux_edp1_lcd,
	msm_mux_egpio,
	msm_mux_emac0_mcg0,
	msm_mux_emac0_mcg1,
	msm_mux_emac0_mcg2,
	msm_mux_emac0_mcg3,
	msm_mux_emac0_mdc,
	msm_mux_emac0_mdio,
	msm_mux_emac0_ptp_aux,
	msm_mux_emac0_ptp_pps,
	msm_mux_gcc_gp1,
	msm_mux_gcc_gp2,
	msm_mux_gcc_gp3,
	msm_mux_gcc_gp4,
	msm_mux_gcc_gp5,
	msm_mux_hs0_mi2s,
	msm_mux_hs1_mi2s,
	msm_mux_hs2_mi2s,
	msm_mux_ibi_i3c,
	msm_mux_jitter_bist,
	msm_mux_mdp0_vsync0,
	msm_mux_mdp0_vsync1,
	msm_mux_mdp0_vsync3,
	msm_mux_mdp0_vsync6,
	msm_mux_mdp0_vsync7,
	msm_mux_mdp_vsync,
	msm_mux_mi2s1_data0,
	msm_mux_mi2s1_data1,
	msm_mux_mi2s1_sck,
	msm_mux_mi2s1_ws,
	msm_mux_mi2s2_data0,
	msm_mux_mi2s2_data1,
	msm_mux_mi2s2_sck,
	msm_mux_mi2s2_ws,
	msm_mux_mi2s_mclk0,
	msm_mux_mi2s_mclk1,
	msm_mux_pcie0_clkreq,
	msm_mux_pcie1_clkreq,
	msm_mux_phase_flag,
	msm_mux_pll_bist,
	msm_mux_pll_clk,
	msm_mux_prng_rosc0,
	msm_mux_prng_rosc1,
	msm_mux_prng_rosc2,
	msm_mux_prng_rosc3,
	msm_mux_qdss_cti,
	msm_mux_qdss_gpio,
	msm_mux_qup0_se0,
	msm_mux_qup0_se1,
	msm_mux_qup0_se2,
	msm_mux_qup0_se3,
	msm_mux_qup0_se4,
	msm_mux_qup0_se5,
	msm_mux_qup0_se6,
	msm_mux_qup0_se7,
	msm_mux_qup1_se0,
	msm_mux_qup1_se1,
	msm_mux_qup1_se2,
	msm_mux_qup1_se3,
	msm_mux_qup1_se4,
	msm_mux_qup1_se5,
	msm_mux_qup1_se6,
	msm_mux_qup1_se7,
	msm_mux_qup2_se0,
	msm_mux_sailss_emac0,
	msm_mux_sailss_ospi,
	msm_mux_sgmii_phy,
	msm_mux_tb_trig,
	msm_mux_tgu_ch0,
	msm_mux_tgu_ch1,
	msm_mux_tgu_ch2,
	msm_mux_tgu_ch3,
	msm_mux_tsense_pwm1,
	msm_mux_tsense_pwm2,
	msm_mux_tsense_pwm3,
	msm_mux_tsense_pwm4,
	msm_mux_usb2phy_ac,
	msm_mux_vsense_trigger,
	msm_mux__,
};

#define MSM_PIN_FUNCTION(fname)\
	[msm_mux_##fname] = {#fname, msm_mux_##fname}

static const struct pinctrl_function msm_pinctrl_functions[] = {
	MSM_PIN_FUNCTION(gpio),
	MSM_PIN_FUNCTION(aoss_cti),
	MSM_PIN_FUNCTION(atest_char),
	MSM_PIN_FUNCTION(atest_usb2),
	MSM_PIN_FUNCTION(audio_ref),
	MSM_PIN_FUNCTION(cam_mclk),
	MSM_PIN_FUNCTION(cci_async),
	MSM_PIN_FUNCTION(cci_i2c_scl),
	MSM_PIN_FUNCTION(cci_i2c_sda),
	MSM_PIN_FUNCTION(cci_timer),
	MSM_PIN_FUNCTION(cri_trng),
	MSM_PIN_FUNCTION(dbg_out),
	MSM_PIN_FUNCTION(ddr_bist),
	MSM_PIN_FUNCTION(ddr_pxi0),
	MSM_PIN_FUNCTION(ddr_pxi1),
	MSM_PIN_FUNCTION(ddr_pxi2),
	MSM_PIN_FUNCTION(ddr_pxi3),
	MSM_PIN_FUNCTION(edp0_hot),
	MSM_PIN_FUNCTION(edp0_lcd),
	MSM_PIN_FUNCTION(edp1_lcd),
	MSM_PIN_FUNCTION(egpio),
	MSM_PIN_FUNCTION(emac0_mcg0),
	MSM_PIN_FUNCTION(emac0_mcg1),
	MSM_PIN_FUNCTION(emac0_mcg2),
	MSM_PIN_FUNCTION(emac0_mcg3),
	MSM_PIN_FUNCTION(emac0_mdc),
	MSM_PIN_FUNCTION(emac0_mdio),
	MSM_PIN_FUNCTION(emac0_ptp_aux),
	MSM_PIN_FUNCTION(emac0_ptp_pps),
	MSM_PIN_FUNCTION(gcc_gp1),
	MSM_PIN_FUNCTION(gcc_gp2),
	MSM_PIN_FUNCTION(gcc_gp3),
	MSM_PIN_FUNCTION(gcc_gp4),
	MSM_PIN_FUNCTION(gcc_gp5),
	MSM_PIN_FUNCTION(hs0_mi2s),
	MSM_PIN_FUNCTION(hs1_mi2s),
	MSM_PIN_FUNCTION(hs2_mi2s),
	MSM_PIN_FUNCTION(ibi_i3c),
	MSM_PIN_FUNCTION(jitter_bist),
	MSM_PIN_FUNCTION(mdp0_vsync0),
	MSM_PIN_FUNCTION(mdp0_vsync1),
	MSM_PIN_FUNCTION(mdp0_vsync3),
	MSM_PIN_FUNCTION(mdp0_vsync6),
	MSM_PIN_FUNCTION(mdp0_vsync7),
	MSM_PIN_FUNCTION(mdp_vsync),
	MSM_PIN_FUNCTION(mi2s1_data0),
	MSM_PIN_FUNCTION(mi2s1_data1),
	MSM_PIN_FUNCTION(mi2s1_sck),
	MSM_PIN_FUNCTION(mi2s1_ws),
	MSM_PIN_FUNCTION(mi2s2_data0),
	MSM_PIN_FUNCTION(mi2s2_data1),
	MSM_PIN_FUNCTION(mi2s2_sck),
	MSM_PIN_FUNCTION(mi2s2_ws),
	MSM_PIN_FUNCTION(mi2s_mclk0),
	MSM_PIN_FUNCTION(mi2s_mclk1),
	MSM_PIN_FUNCTION(pcie0_clkreq),
	MSM_PIN_FUNCTION(pcie1_clkreq),
	MSM_PIN_FUNCTION(phase_flag),
	MSM_PIN_FUNCTION(pll_bist),
	MSM_PIN_FUNCTION(pll_clk),
	MSM_PIN_FUNCTION(prng_rosc0),
	MSM_PIN_FUNCTION(prng_rosc1),
	MSM_PIN_FUNCTION(prng_rosc2),
	MSM_PIN_FUNCTION(prng_rosc3),
	MSM_PIN_FUNCTION(qdss_cti),
	MSM_PIN_FUNCTION(qdss_gpio),
	MSM_PIN_FUNCTION(qup0_se0),
	MSM_PIN_FUNCTION(qup0_se1),
	MSM_PIN_FUNCTION(qup0_se2),
	MSM_PIN_FUNCTION(qup0_se3),
	MSM_PIN_FUNCTION(qup0_se4),
	MSM_PIN_FUNCTION(qup0_se5),
	MSM_PIN_FUNCTION(qup0_se6),
	MSM_PIN_FUNCTION(qup0_se7),
	MSM_PIN_FUNCTION(qup1_se0),
	MSM_PIN_FUNCTION(qup1_se1),
	MSM_PIN_FUNCTION(qup1_se2),
	MSM_PIN_FUNCTION(qup1_se3),
	MSM_PIN_FUNCTION(qup1_se4),
	MSM_PIN_FUNCTION(qup1_se5),
	MSM_PIN_FUNCTION(qup1_se6),
	MSM_PIN_FUNCTION(qup1_se7),
	MSM_PIN_FUNCTION(qup2_se0),
	MSM_PIN_FUNCTION(sailss_emac0),
	MSM_PIN_FUNCTION(sailss_ospi),
	MSM_PIN_FUNCTION(sgmii_phy),
	MSM_PIN_FUNCTION(tb_trig),
	MSM_PIN_FUNCTION(tgu_ch0),
	MSM_PIN_FUNCTION(tgu_ch1),
	MSM_PIN_FUNCTION(tgu_ch2),
	MSM_PIN_FUNCTION(tgu_ch3),
	MSM_PIN_FUNCTION(tsense_pwm1),
	MSM_PIN_FUNCTION(tsense_pwm2),
	MSM_PIN_FUNCTION(tsense_pwm3),
	MSM_PIN_FUNCTION(tsense_pwm4),
	MSM_PIN_FUNCTION(usb2phy_ac),
	MSM_PIN_FUNCTION(vsense_trigger),
};

static const msm_pin_function qcs8300_pin_functions[] = {
	[0] = PINGROUP(0, _, _, _, _, _, _, _, _, _, _, _),
	[1] = PINGROUP(1, pcie0_clkreq, _, _, _, _, _, _, _, _, _, _),
	[2] = PINGROUP(2, _, _, _, _, _, _, _, _, _, _, _),
	[3] = PINGROUP(3, phase_flag, _, _, _, _, _, _, _, _, _, _),
	[4] = PINGROUP(4, sgmii_phy, qdss_cti, _, _, _, _, _, _, _, _, _),
	[5] = PINGROUP(5, emac0_mdc, qdss_cti, _, _, _, _, _, _, _, _, _),
	[6] = PINGROUP(6, emac0_mdio, _, _, _, _, _, _, _, _, _, _),
	[7] = PINGROUP(7, usb2phy_ac, _, _, _, _, _, _, _, _, _, _),
	[8] = PINGROUP(8, usb2phy_ac, _, _, _, _, _, _, _, _, _, _),
	[9] = PINGROUP(9, phase_flag, _, _, _, _, _, _, _, _, _, _),
	[10] = PINGROUP(10, qup2_se0, emac0_mcg0, _, _, _, _, _, _, _, _, _),
	[11] = PINGROUP(11, qup2_se0, emac0_mcg1, _, _, _, _, _, _, _, _, _),
	[12] = PINGROUP(12, qup2_se0, _, _, _, _, _, _, _, _, _, _),
	[13] = PINGROUP(13, qup2_se0, _, _, _, _, _, _, _, _, _, _),
	[14] = PINGROUP(14, qup2_se0, tb_trig, _, _, _, _, _, _, _, _, _),
	[15] = PINGROUP(15, qup2_se0, _, sailss_ospi, sailss_emac0, _, _, _, _, _, _, _),
	[16] = PINGROUP(16, qup2_se0, _, _, sailss_ospi, sailss_emac0, _, _, _, _, _, _),
	[17] = PINGROUP(17, qup0_se0, qup0_se1, ibi_i3c, _, _, _, _, _, _, _, _),
	[18] = PINGROUP(18, qup0_se0, qup0_se1, ibi_i3c, _, _, _, _, _, _, _, _),
	[19] = PINGROUP(19, qup0_se1, qup0_se0, cci_timer, ibi_i3c, _, _, _, _, _, _, _),
	[20] = PINGROUP(20, qup0_se1, qup0_se0, cci_timer, ibi_i3c, _, _, _, _, _, _, _),
	[21] = PINGROUP(21, qup0_se5, cci_timer, _, _, _, _, _, _, _, _, _),
	[22] = PINGROUP(22, pcie1_clkreq, qup0_se5, cci_timer, _, _, _, _, _, _, _, _),
	[23] = PINGROUP(23, qup0_se5, cci_timer, qdss_cti, _, _, _, _, _, _, _, _),
	[24] = PINGROUP(24, qup0_se5, emac0_ptp_aux, emac0_ptp_pps, qdss_cti, emac0_mcg2, _, _, _, _, _, _),
	[25] = PINGROUP(25, qup0_se3, phase_flag, _, _, _, _, _, _, _, _, _),
	[26] = PINGROUP(26, qup0_se3, phase_flag, _, _, _, _, _, _, _, _, _),
	[27] = PINGROUP(27, qup0_se3, _, _, _, _, _, _, _, _, _, _),
	[28] = PINGROUP(28, qup0_se3, phase_flag, _, _, _, _, _, _, _, _, _),
	[29] = PINGROUP(29, qup0_se4, cci_i2c_sda, cci_async, emac0_ptp_pps, tgu_ch2, _, _, _, _, _, _),
	[30] = PINGROUP(30, qup0_se4, cci_i2c_scl, cci_async, emac0_ptp_pps, tgu_ch3, _, _, _, _, _, _),
	[31] = PINGROUP(31, qup0_se4, cci_i2c_sda, cci_async, emac0_ptp_aux, _, _, _, _, _, _, _),
	[32] = PINGROUP(32, qup0_se4, cci_i2c_scl, emac0_ptp_aux, mdp_vsync, _, _, _, _, _, _, _),
	[33] = PINGROUP(33, qup0_se2, qdss_gpio, _, _, _, _, _, _, _, _, _),
	[34] = PINGROUP(34, qup0_se2, qdss_gpio, _, _, _, _, _, _, _, _, _),
	[35] = PINGROUP(35, qup0_se2, gcc_gp1, _, _, _, _, _, _, _, _, _),
	[36] = PINGROUP(36, qup0_se2, gcc_gp2, qdss_gpio, _, _, _, _, _, _, _, _),
	[37] = PINGROUP(37, qup1_se0, ibi_i3c, qup1_se1, aoss_cti, _, _, _, _, _, _, _),
	[38] = PINGROUP(38, qup1_se0, ibi_i3c, qup1_se1, aoss_cti, _, _, _, _, _, _, _),
	[39] = PINGROUP(39, qup1_se1, ibi_i3c, qup1_se0, aoss_cti, _, _, _, _, _, _, _),
	[40] = PINGROUP(40, qup1_se1, ibi_i3c, qup1_se0, aoss_cti, _, _, _, _, _, _, _),
	[41] = PINGROUP(41, qup1_se3, _, _, _, _, _, _, _, _, _, _),
	[42] = PINGROUP(42, qup1_se3, _, mdp_vsync, _, _, _, _, _, _, _, _),
	[43] = PINGROUP(43, qup0_se7, _, tgu_ch0, _, _, _, _, _, _, _, _),
	[44] = PINGROUP(44, qup0_se7, _, tgu_ch1, _, _, _, _, _, _, _, _),
	[45] = PINGROUP(45, qup1_se4, hs1_mi2s, _, _, _, _, _, _, _, _, _),
	[46] = PINGROUP(46, qup1_se4, hs1_mi2s, _, _, _, _, _, _, _, _, _),
	[47] = PINGROUP(47, qup1_se4, hs1_mi2s, _, _, _, _, _, _, _, _, _),
	[48] = PINGROUP(48, qup1_se4, hs1_mi2s, edp0_lcd, _, _, _, _, _, _, _, _),
	[49] = PINGROUP(49, qup1_se5, hs2_mi2s, cci_timer, qdss_cti, edp1_lcd, ddr_pxi1, _, _, _, _, _),
	[50] = PINGROUP(50, qup1_se5, hs2_mi2s, cci_timer, qdss_cti, _, ddr_pxi1, _, _, _, _, _),
	[51] = PINGROUP(51, qup1_se5, hs2_mi2s, qdss_cti, _, _, _, _, _, _, _, _),
	[52] = PINGROUP(52, qup1_se5, hs2_mi2s, qdss_cti, mdp_vsync, ddr_pxi2, _, _, _, _, _, _),
	[53] = PINGROUP(53, ddr_bist, _, _, _, _, _, _, _, _, _, _),
	[54] = PINGROUP(54, cci_i2c_sda, phase_flag, ddr_bist, _, _, _, _, _, _, _, _),
	[55] = PINGROUP(55, cci_i2c_scl, phase_flag, ddr_bist, _, _, _, _, _, _, _, _),
	[56] = PINGROUP(56, phase_flag, ddr_bist, _, _, _, _, _, _, _, _, _),
	[57] = PINGROUP(57, cci_i2c_sda, prng_rosc0, qdss_gpio, _, _, _, _, _, _, _, _),
	[58] = PINGROUP(58, cci_i2c_scl, prng_rosc1, qdss_gpio, _, _, _, _, _, _, _, _),
	[59] = PINGROUP(59, cci_i2c_sda, prng_rosc2, qdss_gpio, _, _, _, _, _, _, _, _),
	[60] = PINGROUP(60, cci_i2c_scl, prng_rosc3, qdss_gpio, _, _, _, _, _, _, _, _),
	[61] = PINGROUP(61, cci_i2c_sda, qdss_gpio, _, _, _, _, _, _, _, _, _),
	[62] = PINGROUP(62, cci_i2c_scl, qdss_gpio, _, _, _, _, _, _, _, _, _),
	[63] = PINGROUP(63, cci_timer, cci_async, qdss_gpio, atest_usb2, _, _, _, _, _, _, _),
	[64] = PINGROUP(64, cci_timer, cci_async, qdss_gpio, atest_usb2, _, _, _, _, _, _, _),
	[65] = PINGROUP(65, cci_timer, cci_async, qdss_gpio, atest_usb2, _, _, _, _, _, _, _),
	[66] = PINGROUP(66, phase_flag, _, atest_char, _, _, _, _, _, _, _, _),
	[67] = PINGROUP(67, cam_mclk, vsense_trigger, atest_usb2, _, _, _, _, _, _, _, _),
	[68] = PINGROUP(68, cam_mclk, gcc_gp4, atest_usb2, ddr_pxi0, _, _, _, _, _, _, _),
	[69] = PINGROUP(69, cam_mclk, gcc_gp3, atest_usb2, ddr_pxi0, _, _, _, _, _, _, _),
	[70] = PINGROUP(70, qdss_gpio, atest_char, _, _, _, _, _, _, _, _, _),
	[71] = PINGROUP(71, qdss_gpio, atest_char, _, _, _, _, _, _, _, _, _),
	[72] = PINGROUP(72, qdss_gpio, atest_char, _, _, _, _, _, _, _, _, _),
	[73] = PINGROUP(73, _, qdss_gpio, _, _, _, _, _, _, _, _, _),
	[74] = PINGROUP(74, pll_clk, qdss_gpio, atest_usb2, _, _, _, _, _, _, _, _),
	[75] = PINGROUP(75, _, dbg_out, qdss_gpio, atest_usb2, _, _, _, _, _, _, _),
	[76] = PINGROUP(76, gcc_gp5, tsense_pwm4, _, _, _, _, _, _, _, _, _),
	[77] = PINGROUP(77, gcc_gp5, tsense_pwm3, _, _, _, _, _, _, _, _, _),
	[78] = PINGROUP(78, tsense_pwm2, _, _, _, _, _, _, _, _, _, _),
	[79] = PINGROUP(79, emac0_ptp_aux, emac0_ptp_pps, emac0_mcg3, _, tsense_pwm1, _, _, _, _, _, _),
	[80] = PINGROUP(80, qup0_se6, mdp0_vsync6, _, atest_usb2, ddr_pxi3, _, _, _, _, _, _),
	[81] = PINGROUP(81, qup0_se6, mdp0_vsync7, gcc_gp2, _, atest_usb2, ddr_pxi3, _, _, _, _, _),
	[82] = PINGROUP(82, qup0_se6, gcc_gp3, _, _, _, _, _, _, _, _, _),
	[83] = PINGROUP(83, qup0_se6, gcc_gp4, _, atest_usb2, ddr_pxi2, _, _, _, _, _, _),
	[84] = PINGROUP(84, qup1_se2, gcc_gp1, _, atest_usb2, _, _, _, _, _, _, _),
	[85] = PINGROUP(85, qup1_se2, _, atest_usb2, _, _, _, _, _, _, _, _),
	[86] = PINGROUP(86, qup1_se2, _, _, _, _, _, _, _, _, _, _),
	[87] = PINGROUP(87, qup1_se2, _, atest_usb2, _, _, _, _, _, _, _, _),
	[88] = PINGROUP(88, qup1_se2, _, _, _, _, _, _, _, _, _, _),
	[89] = PINGROUP(89, qup1_se6, qup1_se7, mdp0_vsync0, _, _, _, _, _, _, _, _),
	[90] = PINGROUP(90, qup1_se6, qup1_se7, mdp0_vsync1, cri_trng, _, _, _, _, _, _, _),
	[91] = PINGROUP(91, qup1_se7, qup1_se6, mdp0_vsync3, cri_trng, _, _, _, _, _, _, _),
	[92] = PINGROUP(92, qup1_se7, qup1_se6, cri_trng, _, atest_usb2, _, _, _, _, _, _),
	[93] = PINGROUP(93, atest_char, _, _, _, _, _, _, _, _, _, _),
	[94] = PINGROUP(94, edp0_hot, _, _, _, _, _, _, _, _, _, _),
	[95] = PINGROUP(95, _, _, _, _, _, _, _, _, _, _, _),
	[96] = PINGROUP(96, _, _, _, _, _, _, _, _, _, _, _),
	[97] = PINGROUP(97, mi2s_mclk0, jitter_bist, qdss_gpio, _, _, _, _, _, _, _, _),
	[98] = PINGROUP(98, mi2s1_sck, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[99] = PINGROUP(99, mi2s1_ws, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[100] = PINGROUP(100, mi2s1_data0, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[101] = PINGROUP(101, mi2s1_data1, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[102] = PINGROUP(102, mi2s2_sck, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[103] = PINGROUP(103, mi2s2_ws, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[104] = PINGROUP(104, mi2s2_data0, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[105] = PINGROUP(105, mi2s2_data1, audio_ref, phase_flag, _, qdss_gpio, _, _, _, _, _, _),
	[106] = PINGROUP(106, hs0_mi2s, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[107] = PINGROUP(107, hs0_mi2s, pll_bist, phase_flag, _, qdss_gpio, _, _, _, _, _, _),
	[108] = PINGROUP(108, hs0_mi2s, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _),
	[109] = PINGROUP(109, hs0_mi2s, mi2s_mclk1, qdss_gpio, _, _, _, _, _, _, _, _),
	[110] = PINGROUP(110, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _, egpio),
	[111] = PINGROUP(111, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _, egpio),
	[112] = PINGROUP(112, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _, egpio),
	[113] = PINGROUP(113, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _, egpio),
	[114] = PINGROUP(114, phase_flag, _, qdss_gpio, _, _, _, _, _, _, _, egpio),
	[115] = PINGROUP(115, _, _, _, _, _, _, _, _, _, _, egpio),
	[116] = PINGROUP(116, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[117] = PINGROUP(117, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[118] = PINGROUP(118, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[119] = PINGROUP(119, _, _, _, _, _, _, _, _, _, _, egpio),
	[120] = PINGROUP(120, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[121] = PINGROUP(121, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[122] = PINGROUP(122, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[123] = PINGROUP(123, _, _, _, _, _, _, _, _, _, _, egpio),
	[124] = PINGROUP(124, _, _, _, _, _, _, _, _, _, _, egpio),
	[125] = PINGROUP(125, phase_flag, _, _, _, _, _, _, _, _, _, egpio),
	[126] = PINGROUP(126, _, _, _, _, _, _, _, _, _, _, egpio),
	[127] = PINGROUP(127, _, _, _, _, _, _, _, _, _, _, egpio),
	[128] = PINGROUP(128, _, _, _, _, _, _, _, _, _, _, egpio),
	[129] = PINGROUP(129, _, _, _, _, _, _, _, _, _, _, egpio),
	[130] = PINGROUP(130, _, _, _, _, _, _, _, _, _, _, egpio),
	[131] = PINGROUP(131, _, _, _, _, _, _, _, _, _, _, egpio),
	[132] = PINGROUP(132, _, _, _, _, _, _, _, _, _, _, egpio),
};

static const struct msm_special_pin_data qcs8300_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0x92000),
	[1] = SDC_QDSD_PINGROUP("sdc1_rclk", 0x89000, 15, 0),
	[2] = SDC_QDSD_PINGROUP("sdc1_clk", 0x89000, 13, 6),
	[3] = SDC_QDSD_PINGROUP("sdc1_cmd", 0x89000, 11, 3),
	[4] = SDC_QDSD_PINGROUP("sdc1_data", 0x89000, 9, 0),
};

static const char *qcs8300_get_function_name(struct udevice *dev,
					     unsigned int selector)
{
	return msm_pinctrl_functions[selector].name;
}

static const char *qcs8300_get_pin_name(struct udevice *dev,
					unsigned int selector)
{
	if (selector >= 133 && selector <= 137)
		snprintf(pin_name, MAX_PIN_NAME_LEN, "%s",
			 qcs8300_special_pins_data[selector - 133].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int qcs8300_get_function_mux(__maybe_unused unsigned int pin,
				    unsigned int selector)
{
	unsigned int i;
	const msm_pin_function *func = qcs8300_pin_functions + pin;

	for (i = 0; i < MSM_PIN_FUNCTION_COUNT; i++)
		if ((*func)[i] == selector)
			return i;

	pr_err("Can't find requested function for pin %u\n", pin);

	return -EINVAL;
}

static const struct msm_pinctrl_data qcs8300_data = {
	.pin_data = {
		.pin_count = 138,
		.special_pins_start = 133,
		.special_pins_data = qcs8300_special_pins_data,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = qcs8300_get_function_name,
	.get_function_mux = qcs8300_get_function_mux,
	.get_pin_name = qcs8300_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{
		.compatible = "qcom,qcs8300-tlmm",
		.data = (ulong)&qcs8300_data
	},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pinctrl_qcs8300) = {
	.name		= "pinctrl_qcs8300",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops		= &msm_pinctrl_ops,
	.bind		= msm_pinctrl_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
