// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <dm.h>

#include "pinctrl-qcom.h"

#define MAX_PIN_NAME_LEN 32
static char pin_name[MAX_PIN_NAME_LEN] __section(".data");

typedef unsigned int msm_pin_function[10];

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9)\
	{					        \
		msm_mux_gpio, /* gpio mode */		\
		msm_mux_##f1,				\
		msm_mux_##f2,				\
		msm_mux_##f3,				\
		msm_mux_##f4,				\
		msm_mux_##f5,				\
		msm_mux_##f6,				\
		msm_mux_##f7,				\
		msm_mux_##f8,				\
		msm_mux_##f9				\
	}

#define SDC_QDSD_PINGROUP(pg_name, ctl, pull, drv)	\
	{						\
		.name = pg_name,			\
		.ctl_reg = ctl,				\
		.io_reg = 0,				\
		.pull_bit = pull,			\
		.drv_bit = drv,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
	}

#define UFS_RESET(pg_name, ctl)			\
	{					\
		.name = pg_name,		\
		.ctl_reg = ctl,			\
		.io_reg = ctl + 0x4,		\
		.pull_bit = 3,			\
		.drv_bit = 0,			\
		.oe_bit = -1,			\
		.in_bit = -1,			\
		.out_bit = 0,			\
	}

#define EAST 0x000000
#define SOUTH 0xc00000
#define WEST 0x400000

enum qcs615_functions {
	msm_mux_gpio,
	msm_mux_adsp_ext,
	msm_mux_agera_pll,
	msm_mux_aoss_cti,
	msm_mux_atest_char,
	msm_mux_atest_tsens,
	msm_mux_atest_usb,
	msm_mux_cam_mclk,
	msm_mux_cci_async,
	msm_mux_cci_i2c,
	msm_mux_cci_timer,
	msm_mux_copy_gp,
	msm_mux_copy_phase,
	msm_mux_cri_trng,
	msm_mux_dbg_out_clk,
	msm_mux_ddr_bist,
	msm_mux_ddr_pxi,
	msm_mux_dp_hot,
	msm_mux_edp_hot,
	msm_mux_edp_lcd,
	msm_mux_emac_gcc,
	msm_mux_emac_phy_intr,
	msm_mux_forced_usb,
	msm_mux_gcc_gp,
	msm_mux_gp_pdm,
	msm_mux_gps_tx,
	msm_mux_hs0_mi2s,
	msm_mux_hs1_mi2s,
	msm_mux_jitter_bist,
	msm_mux_ldo_en,
	msm_mux_ldo_update,
	msm_mux_m_voc,
	msm_mux_mclk1,
	msm_mux_mclk2,
	msm_mux_mdp_vsync,
	msm_mux_mdp_vsync0_out,
	msm_mux_mdp_vsync1_out,
	msm_mux_mdp_vsync2_out,
	msm_mux_mdp_vsync3_out,
	msm_mux_mdp_vsync4_out,
	msm_mux_mdp_vsync5_out,
	msm_mux_mi2s_1,
	msm_mux_mss_lte,
	msm_mux_nav_pps_in,
	msm_mux_nav_pps_out,
	msm_mux_pa_indicator_or,
	msm_mux_pcie_clk_req,
	msm_mux_pcie_ep_rst,
	msm_mux_phase_flag,
	msm_mux_pll_bist,
	msm_mux_pll_bypassnl,
	msm_mux_pll_reset_n,
	msm_mux_prng_rosc,
	msm_mux_qdss_cti,
	msm_mux_qdss_gpio,
	msm_mux_qlink_enable,
	msm_mux_qlink_request,
	msm_mux_qspi,
	msm_mux_qup0,
	msm_mux_qup1,
	msm_mux_rgmii,
	msm_mux_sd_write_protect,
	msm_mux_sp_cmu,
	msm_mux_ter_mi2s,
	msm_mux_tgu_ch,
	msm_mux_uim1,
	msm_mux_uim2,
	msm_mux_usb0_hs,
	msm_mux_usb1_hs,
	msm_mux_usb_phy_ps,
	msm_mux_vfr_1,
	msm_mux_vsense_trigger_mirnat,
	msm_mux_wlan,
	msm_mux_wsa_clk,
	msm_mux_wsa_data,
	msm_mux__,
};

#define MSM_PIN_FUNCTION(fname)                         \
	[msm_mux_##fname] = {#fname, msm_mux_##fname}

static const struct pinctrl_function msm_pinctrl_functions[] = {
	MSM_PIN_FUNCTION(gpio),
	MSM_PIN_FUNCTION(adsp_ext),
	MSM_PIN_FUNCTION(agera_pll),
	MSM_PIN_FUNCTION(aoss_cti),
	MSM_PIN_FUNCTION(atest_char),
	MSM_PIN_FUNCTION(atest_tsens),
	MSM_PIN_FUNCTION(atest_usb),
	MSM_PIN_FUNCTION(cam_mclk),
	MSM_PIN_FUNCTION(cci_async),
	MSM_PIN_FUNCTION(cci_i2c),
	MSM_PIN_FUNCTION(cci_timer),
	MSM_PIN_FUNCTION(copy_gp),
	MSM_PIN_FUNCTION(copy_phase),
	MSM_PIN_FUNCTION(cri_trng),
	MSM_PIN_FUNCTION(dbg_out_clk),
	MSM_PIN_FUNCTION(ddr_bist),
	MSM_PIN_FUNCTION(ddr_pxi),
	MSM_PIN_FUNCTION(dp_hot),
	MSM_PIN_FUNCTION(edp_hot),
	MSM_PIN_FUNCTION(edp_lcd),
	MSM_PIN_FUNCTION(emac_gcc),
	MSM_PIN_FUNCTION(emac_phy_intr),
	MSM_PIN_FUNCTION(forced_usb),
	MSM_PIN_FUNCTION(gcc_gp),
	MSM_PIN_FUNCTION(gp_pdm),
	MSM_PIN_FUNCTION(gps_tx),
	MSM_PIN_FUNCTION(hs0_mi2s),
	MSM_PIN_FUNCTION(hs1_mi2s),
	MSM_PIN_FUNCTION(jitter_bist),
	MSM_PIN_FUNCTION(ldo_en),
	MSM_PIN_FUNCTION(ldo_update),
	MSM_PIN_FUNCTION(m_voc),
	MSM_PIN_FUNCTION(mclk1),
	MSM_PIN_FUNCTION(mclk2),
	MSM_PIN_FUNCTION(mdp_vsync),
	MSM_PIN_FUNCTION(mdp_vsync0_out),
	MSM_PIN_FUNCTION(mdp_vsync1_out),
	MSM_PIN_FUNCTION(mdp_vsync2_out),
	MSM_PIN_FUNCTION(mdp_vsync3_out),
	MSM_PIN_FUNCTION(mdp_vsync4_out),
	MSM_PIN_FUNCTION(mdp_vsync5_out),
	MSM_PIN_FUNCTION(mi2s_1),
	MSM_PIN_FUNCTION(mss_lte),
	MSM_PIN_FUNCTION(nav_pps_in),
	MSM_PIN_FUNCTION(nav_pps_out),
	MSM_PIN_FUNCTION(pa_indicator_or),
	MSM_PIN_FUNCTION(pcie_clk_req),
	MSM_PIN_FUNCTION(pcie_ep_rst),
	MSM_PIN_FUNCTION(phase_flag),
	MSM_PIN_FUNCTION(pll_bist),
	MSM_PIN_FUNCTION(pll_bypassnl),
	MSM_PIN_FUNCTION(pll_reset_n),
	MSM_PIN_FUNCTION(prng_rosc),
	MSM_PIN_FUNCTION(qdss_cti),
	MSM_PIN_FUNCTION(qdss_gpio),
	MSM_PIN_FUNCTION(qlink_enable),
	MSM_PIN_FUNCTION(qlink_request),
	MSM_PIN_FUNCTION(qspi),
	MSM_PIN_FUNCTION(qup0),
	MSM_PIN_FUNCTION(qup1),
	MSM_PIN_FUNCTION(rgmii),
	MSM_PIN_FUNCTION(sd_write_protect),
	MSM_PIN_FUNCTION(sp_cmu),
	MSM_PIN_FUNCTION(ter_mi2s),
	MSM_PIN_FUNCTION(tgu_ch),
	MSM_PIN_FUNCTION(uim1),
	MSM_PIN_FUNCTION(uim2),
	MSM_PIN_FUNCTION(usb0_hs),
	MSM_PIN_FUNCTION(usb1_hs),
	MSM_PIN_FUNCTION(usb_phy_ps),
	MSM_PIN_FUNCTION(vfr_1),
	MSM_PIN_FUNCTION(vsense_trigger_mirnat),
	MSM_PIN_FUNCTION(wlan),
	MSM_PIN_FUNCTION(wsa_clk),
	MSM_PIN_FUNCTION(wsa_data),
};

static const msm_pin_function qcs615_pin_functions[] = {
	[0] = PINGROUP(0, qup0, _, qdss_gpio, _, _, _, _, _, _),
	[1] = PINGROUP(1, qup0, _, qdss_gpio, _, _, _, _, _, _),
	[2] = PINGROUP(2, qup0, _, qdss_gpio, _, _, _, _, _, _),
	[3] = PINGROUP(3, qup0, _, qdss_gpio, _, _, _, _, _, _),
	[4] = PINGROUP(4, qup0, _, _, _, _, _, _, _, _),
	[5] = PINGROUP(5, qup0, _, _, _, _, _, _, _, _),
	[6] = PINGROUP(6, qup1, qdss_gpio, ddr_pxi, _, _, _, _, _, _),
	[7] = PINGROUP(7, qup1, ddr_bist, qdss_gpio, atest_tsens,
			vsense_trigger_mirnat, atest_usb, ddr_pxi, _, _),
	[8] = PINGROUP(8, qup1, gp_pdm, ddr_bist, qdss_gpio, _, _, _, _, _),
	[9] = PINGROUP(9, qup1, ddr_bist, qdss_gpio, _, _, _, _, _, _),
	[10] = PINGROUP(10, qup1, ddr_bist, _, phase_flag, atest_usb, ddr_pxi, _, _, _),
	[11] = PINGROUP(11, qup1, dbg_out_clk, atest_usb, ddr_pxi, _, _, _, _, _),
	[12] = PINGROUP(12, qup1, jitter_bist, ddr_pxi, _, _, _, _, _, _),
	[13] = PINGROUP(13, qup1, pll_bypassnl, _, ddr_pxi, _, _, _, _, _),
	[14] = PINGROUP(14, qup1, pll_reset_n, _, qdss_gpio, _, _, _, _, _),
	[15] = PINGROUP(15, qup1, qdss_gpio, _, _, _, _, _, _, _),
	[16] = PINGROUP(16, qup0, _, wlan, _, _, _, _, _, _),
	[17] = PINGROUP(17, qup0, _, wlan, _, _, _, _, _, _),
	[18] = PINGROUP(18, qup0, _, phase_flag, _, _, _, _, _, _),
	[19] = PINGROUP(19, qup0, _, phase_flag, _, _, _, _, _, _),
	[20] = PINGROUP(20, qup1, _, phase_flag, qdss_gpio, _, _, _, _, _),
	[21] = PINGROUP(21, qup1, gcc_gp, _, qdss_gpio, _, _, _, _, _),
	[22] = PINGROUP(22, qup1, gcc_gp, _, _, _, _, _, _, _),
	[23] = PINGROUP(23, qup1, _, phase_flag, _, _, _, _, _, _),
	[24] = PINGROUP(24, hs1_mi2s, sd_write_protect, _, phase_flag, _, _, _, _, _),
	[25] = PINGROUP(25, hs1_mi2s, _, phase_flag, _, _, _, _, _, _),
	[26] = PINGROUP(26, cci_async, hs1_mi2s, jitter_bist, _, _, _, _, _, _),
	[27] = PINGROUP(27, hs1_mi2s, pll_bist, _, _, _, _, _, _, _),
	[28] = PINGROUP(28, cam_mclk, agera_pll, qdss_gpio, _, _, _, _, _, _),
	[29] = PINGROUP(29, cam_mclk, _, qdss_gpio, atest_tsens, _, _, _, _, _),
	[30] = PINGROUP(30, cam_mclk, qdss_gpio, _, _, _, _, _, _, _),
	[31] = PINGROUP(31, cam_mclk, _, qdss_gpio, _, _, _, _, _, _),
	[32] = PINGROUP(32, cci_i2c, _, qdss_gpio, _, _, _, _, _, _),
	[33] = PINGROUP(33, cci_i2c, _, qdss_gpio, _, _, _, _, _, _),
	[34] = PINGROUP(34, cci_i2c, _, qdss_gpio, _, _, _, _, _, _),
	[35] = PINGROUP(35, cci_i2c, _, qdss_gpio, _, _, _, _, _, _),
	[36] = PINGROUP(36, hs0_mi2s, _, _, _, _, _, _, _, _),
	[37] = PINGROUP(37, cci_timer, hs0_mi2s, _, _, _, _, _, _, _),
	[38] = PINGROUP(38, cci_timer, hs0_mi2s, _, phase_flag, _, _, _, _, _),
	[39] = PINGROUP(39, cci_timer, hs0_mi2s, _, _, _, _, _, _, _),
	[40] = PINGROUP(40, _, phase_flag, _, _, _, _, _, _, _),
	[41] = PINGROUP(41, cci_async, cci_timer, _, phase_flag, _, _, _, _, _),
	[42] = PINGROUP(42, cci_async, cci_timer, _, phase_flag, _, _, _, _, _),
	[43] = PINGROUP(43, _, phase_flag, forced_usb, _, _, _, _, _, _),
	[44] = PINGROUP(44, qspi, _, phase_flag, qdss_gpio, _, _, _, _, _),
	[45] = PINGROUP(45, qspi, _, phase_flag, qdss_gpio, _, _, _, _, _),
	[46] = PINGROUP(46, qspi, _, qdss_gpio, _, _, _, _, _, _),
	[47] = PINGROUP(47, qspi, _, qdss_gpio, wlan, _, _, _, _, _),
	[48] = PINGROUP(48, qspi, _, wlan, _, _, _, _, _, _),
	[49] = PINGROUP(49, qspi, _, _, _, _, _, _, _, _),
	[50] = PINGROUP(50, qspi, _, _, _, _, _, _, _, _),
	[51] = PINGROUP(51, qlink_request, _, _, _, _, _, _, _, _),
	[52] = PINGROUP(52, qlink_enable, _, _, _, _, _, _, _, _),
	[53] = PINGROUP(53, pa_indicator_or, nav_pps_in, nav_pps_out, gps_tx, _,
			phase_flag, _, _, _),
	[54] = PINGROUP(54, _, gps_tx, gp_pdm, _, phase_flag, atest_usb, ddr_pxi, _, _),
	[55] = PINGROUP(55, _, _, phase_flag, atest_usb, ddr_pxi, _, _, _, _),
	[56] = PINGROUP(56, _, nav_pps_in, nav_pps_out, gps_tx, _, _, _, _, _),
	[57] = PINGROUP(57, _, nav_pps_in, gps_tx, nav_pps_out, gcc_gp, _, _, _, _),
	[58] = PINGROUP(58, _, gcc_gp, _, _, _, _, _, _, _),
	[59] = PINGROUP(59, _, nav_pps_in, nav_pps_out, gps_tx, gcc_gp, _, _, _, _),
	[60] = PINGROUP(60, _, nav_pps_in, nav_pps_out, gps_tx, cri_trng, _, _, _, _),
	[61] = PINGROUP(61, _, cri_trng, _, _, _, _, _, _, _),
	[62] = PINGROUP(62, _, cri_trng, _, _, _, _, _, _, _),
	[63] = PINGROUP(63, _, _, gp_pdm, _, _, _, _, _, _),
	[64] = PINGROUP(64, _, sp_cmu, _, _, _, _, _, _, _),
	[65] = PINGROUP(65, _, _, _, _, _, _, _, _, _),
	[66] = PINGROUP(66, _, gp_pdm, _, _, _, _, _, _, _),
	[67] = PINGROUP(67, _, _, _, phase_flag, atest_usb, _, _, _, _),
	[68] = PINGROUP(68, _, _, _, phase_flag, atest_usb, _, _, _, _),
	[69] = PINGROUP(69, _, _, _, _, _, _, _, _, _),
	[70] = PINGROUP(70, _, _, _, _, _, _, _, _, _),
	[71] = PINGROUP(71, _, _, _, _, _, _, _, _, _),
	[72] = PINGROUP(72, _, _, _, _, _, _, _, _, _),
	[73] = PINGROUP(73, uim2, _, _, _, _, _, _, _, _),
	[74] = PINGROUP(74, uim2, _, _, _, _, _, _, _, _),
	[75] = PINGROUP(75, uim2, _, phase_flag, atest_usb, _, _, _, _, _),
	[76] = PINGROUP(76, uim2, _, phase_flag, atest_usb, aoss_cti, _, _, _, _),
	[77] = PINGROUP(77, uim1, _, phase_flag, atest_usb, _, _, _, _, _),
	[78] = PINGROUP(78, uim1, gcc_gp, _, phase_flag, _, _, _, _, _),
	[79] = PINGROUP(79, uim1, gp_pdm, _, phase_flag, _, _, _, _, _),
	[80] = PINGROUP(80, uim1, _, phase_flag, _, _, _, _, _, _),
	[81] = PINGROUP(81, rgmii, mdp_vsync, _, qdss_gpio, _, _, _, _, _),
	[82] = PINGROUP(82, rgmii, mdp_vsync, _, phase_flag, qdss_gpio, _, _, _, _),
	[83] = PINGROUP(83, rgmii, mdp_vsync, _, qdss_cti, _, _, _, _, _),
	[84] = PINGROUP(84, _, phase_flag, atest_char, _, _, _, _, _, _),
	[85] = PINGROUP(85, _, atest_char, _, _, _, _, _, _, _),
	[86] = PINGROUP(86, copy_gp, _, atest_char, _, _, _, _, _, _),
	[87] = PINGROUP(87, _, atest_char, _, _, _, _, _, _, _),
	[88] = PINGROUP(88, _, usb0_hs, _, _, _, _, _, _, _),
	[89] = PINGROUP(89, emac_phy_intr, pcie_ep_rst, tgu_ch, usb1_hs, _, _, _, _, _),
	[90] = PINGROUP(90, mdp_vsync, mdp_vsync0_out, mdp_vsync1_out,
			mdp_vsync2_out, mdp_vsync3_out, mdp_vsync4_out, mdp_vsync5_out,
			pcie_clk_req, tgu_ch),
	[91] = PINGROUP(91, rgmii, tgu_ch, _, _, _, _, _, _, _),
	[92] = PINGROUP(92, rgmii, vfr_1, tgu_ch, _, phase_flag, qdss_gpio, _, _, _),
	[93] = PINGROUP(93, rgmii, qdss_gpio, _, _, _, _, _, _, _),
	[94] = PINGROUP(94, rgmii, qdss_gpio, _, _, _, _, _, _, _),
	[95] = PINGROUP(95, rgmii, gp_pdm, qdss_gpio, _, _, _, _, _, _),
	[96] = PINGROUP(96, rgmii, qdss_cti, _, _, _, _, _, _, _),
	[97] = PINGROUP(97, rgmii, mdp_vsync, ldo_en, qdss_cti, _, _, _, _, _),
	[98] = PINGROUP(98, mdp_vsync, ldo_update, qdss_cti, _, _, _, _, _, _),
	[99] = PINGROUP(99, prng_rosc, _, _, _, _, _, _, _, _),
	[100] = PINGROUP(100, _, _, _, _, _, _, _, _, _),
	[101] = PINGROUP(101, emac_gcc, _, _, _, _, _, _, _, _),
	[102] = PINGROUP(102, rgmii, dp_hot, emac_gcc, prng_rosc, _, _, _, _, _),
	[103] = PINGROUP(103, rgmii, dp_hot, copy_phase, qdss_cti, _, _, _, _, _),
	[104] = PINGROUP(104,  usb_phy_ps, _, qdss_cti, dp_hot, _, _, _, _, _),
	[105] = PINGROUP(105, _, _, _, _, _, _, _, _, _),
	[106] = PINGROUP(106, mss_lte, _, _, _, _, _, _, _, _),
	[107] = PINGROUP(107, mss_lte, _, _, _, _, _, _, _, _),
	[108] = PINGROUP(108, mi2s_1, _, qdss_gpio, _, _, _, _, _, _),
	[109] = PINGROUP(109, mi2s_1, _, qdss_gpio, _, _, _, _, _, _),
	[110] = PINGROUP(110, wsa_data, mi2s_1, _, _, _, _, _, _, _),
	[111] = PINGROUP(111, wsa_clk, mi2s_1, _, _, _, _, _, _, _),
	[112] = PINGROUP(112, rgmii, _, qdss_cti, _, _, _, _, _, _),
	[113] = PINGROUP(113, rgmii, edp_hot, _, qdss_cti, _, _, _, _, _),
	[114] = PINGROUP(114, rgmii, _, _, _, _, _, _, _, _),
	[115] = PINGROUP(115,  ter_mi2s, atest_char, _, _, _, _, _, _, _),
	[116] = PINGROUP(116, ter_mi2s, _, phase_flag, _, _, _, _, _, _),
	[117] = PINGROUP(117, ter_mi2s, _, phase_flag, qdss_gpio, atest_char, _, _, _, _),
	[118] = PINGROUP(118, ter_mi2s, adsp_ext, _, phase_flag, qdss_gpio, atest_char,
			_, _, _),
	[119] = PINGROUP(119, edp_lcd, _, phase_flag, qdss_gpio, atest_char, _, _, _, _),
	[120] = PINGROUP(120, m_voc, qdss_gpio, atest_char, _, _, _, _, _, _),
	[121] = PINGROUP(121, mclk1, atest_char, _, _, _, _, _, _, _),
	[122] = PINGROUP(122, mclk2, _, _, _, _, _, _, _, _),
};

static const struct msm_special_pin_data qcs615_special_pins_data[] = {
	[0] = UFS_RESET("ufs_reset", 0x9f000 + WEST),
	[1] = SDC_QDSD_PINGROUP("sdc1_rclk", 0x9a000 + WEST, 15, 0),
	[2] = SDC_QDSD_PINGROUP("sdc1_clk", 0x9a000 + WEST, 13, 6),
	[3] = SDC_QDSD_PINGROUP("sdc1_cmd", 0x9a000 + WEST, 11, 3),
	[4] = SDC_QDSD_PINGROUP("sdc1_data", 0x9a000 + WEST, 9, 0),
	[5] = SDC_QDSD_PINGROUP("sdc2_clk", 0x98000 + SOUTH, 14, 6),
	[6] = SDC_QDSD_PINGROUP("sdc2_cmd", 0x98000 + SOUTH, 11, 3),
	[7] = SDC_QDSD_PINGROUP("sdc2_data", 0x98000 + SOUTH, 9, 0),
};

static const unsigned int qcs615_pin_offsets[] = {
	[0] = WEST, [1] = WEST, [2] = WEST,
	[3] = WEST, [4] = WEST, [5] = WEST,
	[6] = EAST, [7] = EAST, [8] = EAST,
	[9] = EAST, [10] = EAST, [11] = EAST,
	[12] = EAST, [13] = EAST, [14] = EAST,
	[15] = EAST, [16] = WEST, [17] = WEST,
	[18] = WEST, [19] = WEST, [20] = SOUTH,
	[21] = SOUTH, [22] = SOUTH, [23] = SOUTH,
	[24] = EAST, [25] = EAST, [26] = EAST,
	[27] = EAST, [28] = EAST, [29] = EAST,
	[30] = EAST, [31] = EAST, [32] = EAST,
	[33] = EAST, [34] = EAST, [35] = EAST,
	[36] = EAST, [37] = EAST, [38] = EAST,
	[39] = EAST, [40] = EAST, [41] = EAST,
	[42] = EAST, [43] = SOUTH, [44] = EAST,
	[45] = EAST, [46] = EAST, [47] = EAST,
	[48] = EAST, [49] = EAST, [50] = EAST,
	[51] = SOUTH, [52] = SOUTH, [53] = SOUTH,
	[54] = SOUTH, [55] = SOUTH, [56] = SOUTH,
	[57] = SOUTH, [58] = SOUTH, [59] = SOUTH,
	[60] = SOUTH, [61] = SOUTH, [62] = SOUTH,
	[63] = SOUTH, [64] = SOUTH, [65] = SOUTH,
	[66] = SOUTH, [67] = SOUTH, [68] = SOUTH,
	[69] = SOUTH, [70] = SOUTH, [71] = SOUTH,
	[72] = SOUTH, [73] = SOUTH, [74] = SOUTH,
	[75] = SOUTH, [76] = SOUTH, [77] = SOUTH,
	[78] = SOUTH, [79] = SOUTH, [80] = SOUTH,
	[81] = WEST, [82] = WEST, [83] = WEST,
	[84] = SOUTH, [85] = SOUTH, [86] = SOUTH,
	[87] = SOUTH, [88] = WEST, [89] = WEST,
	[90] = WEST, [91] = WEST, [92] = WEST,
	[93] = WEST, [94] = WEST, [95] = WEST,
	[96] = WEST, [97] = WEST, [98] = WEST,
	[99] = EAST, [100] = WEST, [101] = WEST,
	[102] = WEST, [103] = WEST, [104] = WEST,
	[105] = SOUTH, [106] = EAST, [107] = EAST,
	[108] = SOUTH, [109] = SOUTH, [110] = SOUTH,
	[111] = SOUTH, [112] = WEST, [113] = WEST,
	[114] = WEST, [115] = SOUTH, [116] = SOUTH,
	[117] = SOUTH, [118] = SOUTH, [119] = SOUTH,
	[120] = SOUTH, [121] = SOUTH, [122] = SOUTH,
};

static const char *qcs615_get_function_name(struct udevice *dev,
					    unsigned int selector)

{
	return msm_pinctrl_functions[selector].name;
}

static const char *qcs615_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	struct msm_pinctrl_data *data = (struct msm_pinctrl_data *)dev_get_driver_data(dev);
	unsigned int special_pins_start = data->pin_data.special_pins_start;

	if (selector > (data->pin_data.pin_count - 1))
		snprintf(pin_name, MAX_PIN_NAME_LEN, "unknown");
	else if (selector >= special_pins_start)

		snprintf(pin_name, MAX_PIN_NAME_LEN,
			 qcs615_special_pins_data[selector - special_pins_start].name);
	else
		snprintf(pin_name, MAX_PIN_NAME_LEN, "gpio%u", selector);

	return pin_name;
}

static int qcs615_get_function_mux(__maybe_unused unsigned int pin,
				   unsigned int selector)
{
	unsigned int i;
	const msm_pin_function *func = NULL;

	if (pin >= ARRAY_SIZE(qcs615_pin_functions))
		return -EINVAL;

	func = qcs615_pin_functions + pin;
	for (i = 0; i < 10; i++)
		if ((*func)[i] == selector)
			return i;

	pr_err("Can't find requested function for pin %u pin\n", pin);

	return -EINVAL;
}

static const struct msm_pinctrl_data qcs615_data = {
	.pin_data = {
		.pin_count = 131,
		.special_pins_start = 123,
		.special_pins_data = qcs615_special_pins_data,
		.pin_offsets = qcs615_pin_offsets,
	},
	.functions_count = ARRAY_SIZE(msm_pinctrl_functions),
	.get_function_name = qcs615_get_function_name,
	.get_function_mux = qcs615_get_function_mux,
	.get_pin_name = qcs615_get_pin_name,
};

static const struct udevice_id msm_pinctrl_ids[] = {
	{ .compatible = "qcom,qcs615-tlmm", .data = (ulong)&qcs615_data},
	{ }
};

U_BOOT_DRIVER(qcs615_pinctrl) = {
	.name		= "qcs615_pinctrl",
	.id		= UCLASS_NOP,
	.of_match	= msm_pinctrl_ids,
	.ops            = &msm_pinctrl_ops,
	.bind           = msm_pinctrl_bind,
	.flags          = DM_FLAG_PRE_RELOC,

};
