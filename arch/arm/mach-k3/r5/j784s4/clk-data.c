// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * J784S4 specific clock platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Bryan Brattlof <bb@ti.com>.
 *
 * Copyright (C) 2020-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/clk-provider.h>
#include "k3-clk.h"

static const char * const gluelogic_hfosc0_clkout_parents[] = {
	"osc_19_2_mhz",
	"osc_20_mhz",
	"osc_24_mhz",
	"osc_25_mhz",
	"osc_26_mhz",
	"osc_27_mhz",
};

static const char * const mcu_ospi0_iclk_sel_out0_parents[] = {
	"board_0_mcu_ospi0_dqs_out",
	"fss_mcu_0_ospi_0_ospi_oclk_clk",
};

static const char * const mcu_ospi1_iclk_sel_out0_parents[] = {
	"board_0_mcu_ospi1_dqs_out",
	"fss_mcu_0_ospi_1_ospi_oclk_clk",
};

static const char * const wkup_fref_clksel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"j7am_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
};

static const char * const k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents[] = {
	"wkup_fref_clksel_out0",
	"hsdiv1_16fft_mcu_0_hsdivout0_clk",
};

static const char * const mcu_ospi_ref_clk_sel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
};

static const char * const mcu_ospi_ref_clk_sel_out1_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout4_clk",
	"hsdiv4_16fft_mcu_2_hsdivout4_clk",
};

static const char * const wkup_gpio0_clksel_out0_parents[] = {
	"k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk",
	"k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk",
	"j7am_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk",
	"j7am_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk",
};

static const char * const mcu_usart_clksel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"postdiv3_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_i2c_mcupll_bypass_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"gluelogic_hfosc0_clkout",
};

static const char * const wkup_usart_clksel_out0_parents[] = {
	"hsdiv4_16fft_mcu_1_hsdivout3_clk",
	"postdiv3_16fft_main_1_hsdivout5_clk",
};

static const char * const wkup_usart_mcupll_bypass_out0_parents[] = {
	"wkup_usart_clksel_out0",
	"gluelogic_hfosc0_clkout",
};

static const char * const main_pll_hfosc_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out1_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out12_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out19_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out2_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out26_0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out27_0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out28_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out3_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out7_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const main_pll_hfosc_sel_out8_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const usb0_refclk_sel_out0_parents[] = {
	"gluelogic_hfosc0_clkout",
	"board_0_hfosc1_clk_out",
};

static const char * const emmcsd1_lb_clksel_out0_parents[] = {
	"board_0_mmc1_clklb_out",
	"board_0_mmc1_clk_out",
};

static const char * const mcu_clkout_mux_out0_parents[] = {
	"hsdiv4_16fft_mcu_2_hsdivout0_clk",
	"hsdiv4_16fft_mcu_2_hsdivout1_clk",
};

static const char * const k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents[] = {
	"main_pll_hfosc_sel_out0",
	"hsdiv4_16fft_main_0_hsdivout0_clk",
};

static const char * const dpi0_ext_clksel_out0_parents[] = {
	"hsdiv1_16fft_main_19_hsdivout0_clk",
	"board_0_vout0_extpclkin_out",
};

static const char * const emmcsd_refclk_sel_out0_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const emmcsd_refclk_sel_out1_parents[] = {
	"hsdiv4_16fft_main_0_hsdivout2_clk",
	"hsdiv4_16fft_main_1_hsdivout2_clk",
	"hsdiv4_16fft_main_2_hsdivout2_clk",
	"hsdiv4_16fft_main_3_hsdivout2_clk",
};

static const char * const gtc_clk_mux_out0_parents[] = {
	"hsdiv4_16fft_main_3_hsdivout1_clk",
	"postdiv3_16fft_main_0_hsdivout6_clk",
	"board_0_mcu_cpts0_rft_clk_out",
	"board_0_cpts0_rft_clk_out",
	"board_0_mcu_ext_refclk0_out",
	"board_0_ext_refclk1_out",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"hsdiv4_16fft_mcu_2_hsdivout1_clk",
	"k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk",
};

static const struct clk_data clk_list[] = {
	CLK_FIXED_RATE("osc_27_mhz", 27000000, 0),
	CLK_FIXED_RATE("osc_26_mhz", 26000000, 0),
	CLK_FIXED_RATE("osc_25_mhz", 25000000, 0),
	CLK_FIXED_RATE("osc_24_mhz", 24000000, 0),
	CLK_FIXED_RATE("osc_20_mhz", 20000000, 0),
	CLK_FIXED_RATE("osc_19_2_mhz", 19200000, 0),
	CLK_MUX("gluelogic_hfosc0_clkout", gluelogic_hfosc0_clkout_parents, 6, 0x43000030, 0, 3, 0),
	CLK_FIXED_RATE("board_0_hfosc1_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ospi0_dqs_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ospi1_dqs_out", 0, 0),
	CLK_FIXED_RATE("board_0_wkup_i2c0_scl_out", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_ospi_0_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("fss_mcu_0_ospi_1_ospi_oclk_clk", 0, 0),
	CLK_FIXED_RATE("j7am_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk", 12500000, 0),
	CLK_FIXED_RATE("j7am_wakeup_16ff_wkup_0_wkup_rcosc_32k_clk", 32000, 0),
	CLK_MUX("mcu_ospi0_iclk_sel_out0", mcu_ospi0_iclk_sel_out0_parents, 2, 0x40f08030, 4, 1, 0),
	CLK_MUX("mcu_ospi1_iclk_sel_out0", mcu_ospi1_iclk_sel_out0_parents, 2, 0x40f08034, 4, 1, 0),
	CLK_FIXED_RATE("mshsi2c_wkup_0_porscl", 0, 0),
	CLK_MUX("wkup_fref_clksel_out0", wkup_fref_clksel_out0_parents, 2, 0x43008050, 8, 1, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_mcu_0_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d00000, 0),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d01000, 0, 2400000000),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", "wkup_fref_clksel_out0", 0x40d02000, 0, 2000000000),
	CLK_DIV("hsdiv1_16fft_mcu_0_hsdivout0_clk", "pllfracf2_ssmod_16fft_mcu_0_foutvcop_clk", 0x40d00080, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_mcu_1_hsdivout3_clk", "pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d0108c, 0, 7, 0, 0, 96000000),
	CLK_DIV("hsdiv4_16fft_mcu_1_hsdivout4_clk", "pllfracf2_ssmod_16fft_mcu_1_foutvcop_clk", 0x40d01090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_mcu_2_hsdivout4_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02090, 0, 7, 0, 0, 166666666),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", k3_pll_ctrl_wrap_wkup_0_sysclkout_clk_parents, 2, 0x42010000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x42010118, 0, 5, 0, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out0", mcu_ospi_ref_clk_sel_out0_parents, 2, 0x40f08030, 0, 1, 0),
	CLK_MUX("mcu_ospi_ref_clk_sel_out1", mcu_ospi_ref_clk_sel_out1_parents, 2, 0x40f08034, 0, 1, 0),
	CLK_MUX("wkup_gpio0_clksel_out0", wkup_gpio0_clksel_out0_parents, 4, 0x43008070, 0, 2, 0),
	CLK_MUX("mcu_usart_clksel_out0", mcu_usart_clksel_out0_parents, 2, 0x40f081c0, 0, 1, 0),
	CLK_MUX("wkup_i2c_mcupll_bypass_out0", wkup_i2c_mcupll_bypass_out0_parents, 2, 0x43008060, 0, 1, 0),
	CLK_MUX("wkup_usart_clksel_out0", wkup_usart_clksel_out0_parents, 2, 0x43008064, 0, 1, 0),
	CLK_MUX("wkup_usart_mcupll_bypass_out0", wkup_usart_mcupll_bypass_out0_parents, 2, 0x43008060, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out0", main_pll_hfosc_sel_out0_parents, 2, 0x43008080, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out1", main_pll_hfosc_sel_out1_parents, 2, 0x43008084, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out12", main_pll_hfosc_sel_out12_parents, 2, 0x430080b0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out19", main_pll_hfosc_sel_out19_parents, 2, 0x430080cc, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out2", main_pll_hfosc_sel_out2_parents, 2, 0x43008088, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out26_0", main_pll_hfosc_sel_out26_0_parents, 2, 0x430080e8, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out27_0", main_pll_hfosc_sel_out27_0_parents, 2, 0x430080ec, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out28", main_pll_hfosc_sel_out28_parents, 2, 0x430080f0, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out3", main_pll_hfosc_sel_out3_parents, 2, 0x4300808c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out7", main_pll_hfosc_sel_out7_parents, 2, 0x4300809c, 0, 1, 0),
	CLK_MUX("main_pll_hfosc_sel_out8", main_pll_hfosc_sel_out8_parents, 2, 0x430080a0, 0, 1, 0),
	CLK_MUX("usb0_refclk_sel_out0", usb0_refclk_sel_out0_parents, 2, 0x1080e0, 0, 1, 0),
	CLK_FIXED_RATE("board_0_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_ext_refclk1_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_cpts0_rft_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_mcu_ext_refclk0_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clklb_out", 0, 0),
	CLK_FIXED_RATE("board_0_mmc1_clk_out", 0, 0),
	CLK_FIXED_RATE("board_0_tck_out", 0, 0),
	CLK_FIXED_RATE("board_0_vout0_extpclkin_out", 0, 0),
	CLK_FIXED_RATE("emmcsd4ss_main_0_emmcsdss_io_clk_o", 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout0_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_mcu_2_hsdivout1_clk", "pllfracf2_ssmod_16fft_mcu_2_foutvcop_clk", 0x40d02084, 0, 7, 0, 0),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_main_0_foutvcop_clk", "main_pll_hfosc_sel_out0", 0x680000, 0, 2000000000),
	CLK_DIV("pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk_subdiv", 0x680038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_main_1_foutvcop_clk", "main_pll_hfosc_sel_out1", 0x681000, 0, 1920000000),
	CLK_DIV("pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681038, 16, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_DIV("pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk_subdiv", 0x681038, 24, 3, 0, CLK_DIVIDER_ONE_BASED),
	CLK_PLL("pllfracf2_ssmod_16fft_main_12_foutvcop_clk", "main_pll_hfosc_sel_out12", 0x68c000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_19_foutvcop_clk", "main_pll_hfosc_sel_out19", 0x693000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_2_foutvcop_clk", "main_pll_hfosc_sel_out2", 0x682000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_26_foutvcop_clk", "main_pll_hfosc_sel_out26_0", 0x69a000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_27_foutvcop_clk", "main_pll_hfosc_sel_out27_0", 0x69b000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_28_foutvcop_clk", "main_pll_hfosc_sel_out28", 0x69c000, 0),
	CLK_PLL_DEFFREQ("pllfracf2_ssmod_16fft_main_3_foutvcop_clk", "main_pll_hfosc_sel_out3", 0x683000, 0, 2000000000),
	CLK_PLL("pllfracf2_ssmod_16fft_main_7_foutvcop_clk", "main_pll_hfosc_sel_out7", 0x687000, 0),
	CLK_PLL("pllfracf2_ssmod_16fft_main_8_foutvcop_clk", "main_pll_hfosc_sel_out8", 0x688000, 0),
	CLK_DIV("postdiv3_16fft_main_0_hsdivout6_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", 0x680098, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_0_hsdivout8_clk", "pllfracf2_ssmod_16fft_main_0_foutpostdiv_clk", 0x6800a0, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout5_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", 0x681094, 0, 7, 0, 0),
	CLK_DIV("postdiv3_16fft_main_1_hsdivout7_clk", "pllfracf2_ssmod_16fft_main_1_foutpostdiv_clk", 0x68109c, 0, 7, 0, 0),
	CLK_MUX("emmcsd1_lb_clksel_out0", emmcsd1_lb_clksel_out0_parents, 2, 0x1080b4, 16, 1, 0),
	CLK_MUX("mcu_clkout_mux_out0", mcu_clkout_mux_out0_parents, 2, 0x40f08010, 0, 1, 0),
	CLK_DIV("hsdiv0_16fft_main_12_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_12_foutvcop_clk", 0x68c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_26_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_26_foutvcop_clk", 0x69a080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_27_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_27_foutvcop_clk", 0x69b080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_28_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_28_foutvcop_clk", 0x69c080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_7_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_7_foutvcop_clk", 0x687080, 0, 7, 0, 0),
	CLK_DIV("hsdiv0_16fft_main_8_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_8_foutvcop_clk", 0x688080, 0, 7, 0, 0),
	CLK_DIV("hsdiv1_16fft_main_19_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_19_foutvcop_clk", 0x693080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680080, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout3_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x68008c, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_0_hsdivout4_clk", "pllfracf2_ssmod_16fft_main_0_foutvcop_clk", 0x680090, 0, 7, 0, 0),
	CLK_DIV_DEFFREQ("hsdiv4_16fft_main_1_hsdivout0_clk", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681080, 0, 7, 0, 0, 192000000),
	CLK_DIV("hsdiv4_16fft_main_1_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_1_foutvcop_clk", 0x681088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_2_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_2_foutvcop_clk", 0x682088, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout1_clk", "pllfracf2_ssmod_16fft_main_3_foutvcop_clk", 0x683084, 0, 7, 0, 0),
	CLK_DIV("hsdiv4_16fft_main_3_hsdivout2_clk", "pllfracf2_ssmod_16fft_main_3_foutvcop_clk", 0x683088, 0, 7, 0, 0),
	CLK_MUX_PLLCTRL("k3_pll_ctrl_wrap_main_0_sysclkout_clk", k3_pll_ctrl_wrap_main_0_sysclkout_clk_parents, 2, 0x410000, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x410118, 0, 5, 0, 0),
	CLK_MUX("dpi0_ext_clksel_out0", dpi0_ext_clksel_out0_parents, 2, 0x108300, 0, 1, 0),
	CLK_MUX("emmcsd_refclk_sel_out0", emmcsd_refclk_sel_out0_parents, 4, 0x1080b0, 0, 2, 0),
	CLK_MUX("emmcsd_refclk_sel_out1", emmcsd_refclk_sel_out1_parents, 4, 0x1080b4, 0, 2, 0),
	CLK_MUX("gtc_clk_mux_out0", gtc_clk_mux_out0_parents, 16, 0x108030, 0, 4, 0),
	CLK_DIV_DEFFREQ("usart_programmable_clock_divider_out0", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081c0, 0, 2, 0, 0, 48000000),
	CLK_DIV("usart_programmable_clock_divider_out5", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081d4, 0, 2, 0, 0),
	CLK_DIV("usart_programmable_clock_divider_out8", "hsdiv4_16fft_main_1_hsdivout0_clk", 0x1081e0, 0, 2, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_main_0_sysclkout_clk", 0x41011c, 0, 5, 0, 0),
	CLK_DIV("k3_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk", "k3_pll_ctrl_wrap_wkup_0_sysclkout_clk", 0x4201011c, 0, 5, 0, 0),
};

static const struct dev_clk soc_dev_clk_data[] = {
	DEV_CLK(198, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(198, 3, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(198, 4, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(202, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(203, 0, "hsdiv0_16fft_main_8_hsdivout0_clk"),
	DEV_CLK(61, 0, "gtc_clk_mux_out0"),
	DEV_CLK(61, 1, "hsdiv4_16fft_main_3_hsdivout1_clk"),
	DEV_CLK(61, 2, "postdiv3_16fft_main_0_hsdivout6_clk"),
	DEV_CLK(61, 3, "board_0_mcu_cpts0_rft_clk_out"),
	DEV_CLK(61, 4, "board_0_cpts0_rft_clk_out"),
	DEV_CLK(61, 5, "board_0_mcu_ext_refclk0_out"),
	DEV_CLK(61, 6, "board_0_ext_refclk1_out"),
	DEV_CLK(61, 15, "hsdiv4_16fft_mcu_2_hsdivout1_clk"),
	DEV_CLK(61, 16, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(61, 17, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(78, 0, "postdiv3_16fft_main_0_hsdivout8_clk"),
	DEV_CLK(78, 1, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(78, 2, "hsdiv4_16fft_main_0_hsdivout3_clk"),
	DEV_CLK(78, 3, "hsdiv4_16fft_main_0_hsdivout4_clk"),
	DEV_CLK(78, 4, "gluelogic_hfosc0_clkout"),
	DEV_CLK(78, 5, "board_0_hfosc1_clk_out"),
	DEV_CLK(78, 6, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(78, 8, "gluelogic_hfosc0_clkout"),
	DEV_CLK(78, 9, "board_0_hfosc1_clk_out"),
	DEV_CLK(78, 10, "j7am_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(78, 11, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(78, 12, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(140, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(140, 2, "emmcsd_refclk_sel_out0"),
	DEV_CLK(140, 3, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(140, 4, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(140, 5, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(140, 6, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(141, 0, "emmcsd1_lb_clksel_out0"),
	DEV_CLK(141, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(141, 4, "emmcsd_refclk_sel_out1"),
	DEV_CLK(141, 5, "hsdiv4_16fft_main_0_hsdivout2_clk"),
	DEV_CLK(141, 6, "hsdiv4_16fft_main_1_hsdivout2_clk"),
	DEV_CLK(141, 7, "hsdiv4_16fft_main_2_hsdivout2_clk"),
	DEV_CLK(141, 8, "hsdiv4_16fft_main_3_hsdivout2_clk"),
	DEV_CLK(146, 0, "usart_programmable_clock_divider_out0"),
	DEV_CLK(146, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(149, 0, "mcu_usart_clksel_out0"),
	DEV_CLK(149, 1, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(149, 2, "postdiv3_16fft_main_1_hsdivout5_clk"),
	DEV_CLK(149, 5, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(157, 174, "mcu_clkout_mux_out0"),
	DEV_CLK(157, 175, "hsdiv4_16fft_mcu_2_hsdivout0_clk"),
	DEV_CLK(157, 176, "hsdiv4_16fft_mcu_2_hsdivout1_clk"),
	DEV_CLK(157, 179, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_p"),
	DEV_CLK(157, 180, "fss_mcu_0_hyperbus1p0_0_hpb_out_clk_n"),
	DEV_CLK(157, 224, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 226, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(157, 228, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(157, 230, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(157, 239, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(157, 243, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 245, "emmcsd4ss_main_0_emmcsdss_io_clk_o"),
	DEV_CLK(157, 354, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(157, 359, "dpi0_ext_clksel_out0"),
	DEV_CLK(157, 360, "mshsi2c_wkup_0_porscl"),
	DEV_CLK(160, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(160, 2, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(160, 4, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(160, 6, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(160, 8, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(161, 0, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(161, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(161, 2, "mcu_ospi0_iclk_sel_out0"),
	DEV_CLK(161, 3, "board_0_mcu_ospi0_dqs_out"),
	DEV_CLK(161, 4, "fss_mcu_0_ospi_0_ospi_oclk_clk"),
	DEV_CLK(161, 6, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(161, 7, "mcu_ospi_ref_clk_sel_out0"),
	DEV_CLK(161, 8, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(161, 9, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(162, 0, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(162, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(162, 2, "mcu_ospi1_iclk_sel_out0"),
	DEV_CLK(162, 3, "board_0_mcu_ospi1_dqs_out"),
	DEV_CLK(162, 4, "fss_mcu_0_ospi_1_ospi_oclk_clk"),
	DEV_CLK(162, 6, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(162, 7, "mcu_ospi_ref_clk_sel_out1"),
	DEV_CLK(162, 8, "hsdiv4_16fft_mcu_1_hsdivout4_clk"),
	DEV_CLK(162, 9, "hsdiv4_16fft_mcu_2_hsdivout4_clk"),
	DEV_CLK(167, 0, "wkup_gpio0_clksel_out0"),
	DEV_CLK(178, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(178, 1, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(188, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(188, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(191, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(191, 1, "hsdiv0_16fft_main_12_hsdivout0_clk"),
	DEV_CLK(191, 4, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(191, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(192, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(192, 1, "hsdiv0_16fft_main_26_hsdivout0_clk"),
	DEV_CLK(192, 4, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(192, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(193, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(193, 1, "hsdiv0_16fft_main_27_hsdivout0_clk"),
	DEV_CLK(193, 4, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(193, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(194, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(194, 1, "hsdiv0_16fft_main_28_hsdivout0_clk"),
	DEV_CLK(194, 4, "hsdiv0_16fft_main_7_hsdivout0_clk"),
	DEV_CLK(194, 5, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(201, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(201, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(243, 0, "j7am_wakeup_16ff_wkup_0_wkup_rcosc_12p5m_clk"),
	DEV_CLK(243, 1, "gluelogic_hfosc0_clkout"),
	DEV_CLK(243, 2, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(279, 0, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(279, 1, "board_0_wkup_i2c0_scl_out"),
	DEV_CLK(279, 2, "wkup_i2c_mcupll_bypass_out0"),
	DEV_CLK(279, 3, "hsdiv4_16fft_mcu_1_hsdivout3_clk"),
	DEV_CLK(279, 4, "gluelogic_hfosc0_clkout"),
	DEV_CLK(392, 0, "usart_programmable_clock_divider_out5"),
	DEV_CLK(392, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(395, 0, "usart_programmable_clock_divider_out8"),
	DEV_CLK(395, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(397, 0, "wkup_usart_mcupll_bypass_out0"),
	DEV_CLK(397, 1, "wkup_usart_clksel_out0"),
	DEV_CLK(397, 2, "gluelogic_hfosc0_clkout"),
	DEV_CLK(397, 7, "k3_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk"),
	DEV_CLK(398, 0, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(398, 1, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(398, 2, "postdiv3_16fft_main_1_hsdivout7_clk"),
	DEV_CLK(398, 3, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(398, 20, "k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk"),
	DEV_CLK(398, 21, "usb0_refclk_sel_out0"),
	DEV_CLK(398, 22, "gluelogic_hfosc0_clkout"),
	DEV_CLK(398, 23, "board_0_hfosc1_clk_out"),
	DEV_CLK(398, 28, "board_0_tck_out"),
};

const struct ti_k3_clk_platdata j784s4_clk_platdata = {
	.clk_list = clk_list,
	.clk_list_cnt = ARRAY_SIZE(clk_list),
	.soc_dev_clk_data = soc_dev_clk_data,
	.soc_dev_clk_data_cnt = ARRAY_SIZE(soc_dev_clk_data),
};
