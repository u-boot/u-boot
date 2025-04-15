// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-common.h"
#include "clk-cv1800b.h"
#include "clk-ip.h"
#include "clk-pll.h"

static const char *const clk_cam_parents[] = {
	"clk_cam0pll",
	"clk_cam0pll_d2",
	"clk_cam0pll_d3",
	"clk_mipimpll_d3"
};

static const char *const clk_tpu_parents[] = {
	"clk_tpll",
	"clk_a0pll",
	"clk_mipimpll",
	"clk_fpll"
};

static const char *const clk_axi4_parents[] = { "clk_fpll", "clk_disppll" };
static const char *const clk_aud_parents[] = { "clk_a0pll", "clk_a24m" };
static const char *const clk_cam0_200_parents[] = { "osc", "clk_disppll" };

static const char *const clk_vip_sys_parents[] = {
	"clk_mipimpll",
	"clk_cam0pll",
	"clk_disppll",
	"clk_fpll"
};

static const char *const clk_axi_video_codec_parents[] = {
	"clk_a0pll",
	"clk_mipimpll",
	"clk_cam1pll",
	"clk_fpll"
};

static const char *const clk_vc_src0_parents[] = {
	"clk_disppll",
	"clk_mipimpll",
	"clk_cam1pll",
	"clk_fpll"
};

static const struct cv1800b_mmux_parent_info clk_c906_0_parents[] = {
	{ "clk_tpll", 0, 0 },
	{ "clk_a0pll", 0, 1 },
	{ "clk_mipimpll", 0, 2 },
	{ "clk_mpll", 0, 3 },
	{ "clk_fpll", 1, 0 },
};

static const struct cv1800b_mmux_parent_info clk_c906_1_parents[] = {
	{ "clk_tpll", 0, 0 },
	{ "clk_a0pll", 0, 1 },
	{ "clk_disppll", 0, 2 },
	{ "clk_mpll", 0, 3 },
	{ "clk_fpll", 1, 0 },
};

static const struct cv1800b_mmux_parent_info clk_a53_parents[] = {
	{ "clk_tpll", 0, 0 },
	{ "clk_a0pll", 0, 1 },
	{ "clk_mipimpll", 0, 2 },
	{ "clk_mpll", 0, 3 },
	{ "clk_fpll", 1, 0 },
};

static struct cv1800b_clk_gate cv1800b_gate_info[] = {
	CV1800B_GATE(CLK_XTAL_AP, "clk_xtal_ap", "osc", REG_CLK_EN_0, 3, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_RTC_25M, "clk_rtc_25m", "osc", REG_CLK_EN_0, 8, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TEMPSEN, "clk_tempsen", "osc", REG_CLK_EN_0, 9, 0),
	CV1800B_GATE(CLK_SARADC, "clk_saradc", "osc", REG_CLK_EN_0, 10, 0),
	CV1800B_GATE(CLK_EFUSE, "clk_efuse", "osc", REG_CLK_EN_0, 11, 0),
	CV1800B_GATE(CLK_APB_EFUSE, "clk_apb_efuse", "osc", REG_CLK_EN_0, 12, 0),
	CV1800B_GATE(CLK_DEBUG, "clk_debug", "osc", REG_CLK_EN_0, 13, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_XTAL_MISC, "clk_xtal_misc", "osc", REG_CLK_EN_0, 14, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_APB_WDT, "clk_apb_wdt", "osc", REG_CLK_EN_1, 7, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_WGN, "clk_wgn", "osc", REG_CLK_EN_3, 22, 0),
	CV1800B_GATE(CLK_WGN0, "clk_wgn0", "osc", REG_CLK_EN_3, 23, 0),
	CV1800B_GATE(CLK_WGN1, "clk_wgn1", "osc", REG_CLK_EN_3, 24, 0),
	CV1800B_GATE(CLK_WGN2, "clk_wgn2", "osc", REG_CLK_EN_3, 25, 0),
	CV1800B_GATE(CLK_KEYSCAN, "clk_keyscan", "osc", REG_CLK_EN_3, 26, 0),
	CV1800B_GATE(CLK_TPU_FAB, "clk_tpu_fab", "clk_mipimpll", REG_CLK_EN_0, 5, 0),
	CV1800B_GATE(CLK_AHB_ROM, "clk_ahb_rom", "clk_axi4", REG_CLK_EN_0, 6, 0),
	CV1800B_GATE(CLK_AXI4_EMMC, "clk_axi4_emmc", "clk_axi4", REG_CLK_EN_0, 15, 0),
	CV1800B_GATE(CLK_AXI4_SD0, "clk_axi4_sd0", "clk_axi4", REG_CLK_EN_0, 18, 0),
	CV1800B_GATE(CLK_AXI4_SD1, "clk_axi4_sd1", "clk_axi4", REG_CLK_EN_0, 21, 0),
	CV1800B_GATE(CLK_AXI4_ETH0, "clk_axi4_eth0", "clk_axi4", REG_CLK_EN_0, 26, 0),
	CV1800B_GATE(CLK_AXI4_ETH1, "clk_axi4_eth1", "clk_axi4", REG_CLK_EN_0, 28, 0),
	CV1800B_GATE(CLK_AHB_SF, "clk_ahb_sf", "clk_axi4", REG_CLK_EN_1, 0, 0),
	CV1800B_GATE(CLK_SDMA_AXI, "clk_sdma_axi", "clk_axi4", REG_CLK_EN_1, 1, 0),
	CV1800B_GATE(CLK_APB_I2C, "clk_apb_i2c", "clk_axi4", REG_CLK_EN_1, 6, 0),
	CV1800B_GATE(CLK_APB_SPI0, "clk_apb_spi0", "clk_axi4", REG_CLK_EN_1, 9, 0),
	CV1800B_GATE(CLK_APB_SPI1, "clk_apb_spi1", "clk_axi4", REG_CLK_EN_1, 10, 0),
	CV1800B_GATE(CLK_APB_SPI2, "clk_apb_spi2", "clk_axi4", REG_CLK_EN_1, 11, 0),
	CV1800B_GATE(CLK_APB_SPI3, "clk_apb_spi3", "clk_axi4", REG_CLK_EN_1, 12, 0),
	CV1800B_GATE(CLK_APB_UART0, "clk_apb_uart0", "clk_axi4", REG_CLK_EN_1, 15, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_APB_UART1, "clk_apb_uart1", "clk_axi4", REG_CLK_EN_1, 17, 0),
	CV1800B_GATE(CLK_APB_UART2, "clk_apb_uart2", "clk_axi4", REG_CLK_EN_1, 19, 0),
	CV1800B_GATE(CLK_APB_UART3, "clk_apb_uart3", "clk_axi4", REG_CLK_EN_1, 21, 0),
	CV1800B_GATE(CLK_APB_UART4, "clk_apb_uart4", "clk_axi4", REG_CLK_EN_1, 23, 0),
	CV1800B_GATE(CLK_APB_I2S0, "clk_apb_i2s0", "clk_axi4", REG_CLK_EN_1, 24, 0),
	CV1800B_GATE(CLK_APB_I2S1, "clk_apb_i2s1", "clk_axi4", REG_CLK_EN_1, 25, 0),
	CV1800B_GATE(CLK_APB_I2S2, "clk_apb_i2s2", "clk_axi4", REG_CLK_EN_1, 26, 0),
	CV1800B_GATE(CLK_APB_I2S3, "clk_apb_i2s3", "clk_axi4", REG_CLK_EN_1, 27, 0),
	CV1800B_GATE(CLK_AXI4_USB, "clk_axi4_usb", "clk_axi4", REG_CLK_EN_1, 28, 0),
	CV1800B_GATE(CLK_APB_USB, "clk_apb_usb", "clk_axi4", REG_CLK_EN_1, 29, 0),
	CV1800B_GATE(CLK_APB_I2C0, "clk_apb_i2c0", "clk_axi4", REG_CLK_EN_3, 17, 0),
	CV1800B_GATE(CLK_APB_I2C1, "clk_apb_i2c1", "clk_axi4", REG_CLK_EN_3, 18, 0),
	CV1800B_GATE(CLK_APB_I2C2, "clk_apb_i2c2", "clk_axi4", REG_CLK_EN_3, 19, 0),
	CV1800B_GATE(CLK_APB_I2C3, "clk_apb_i2c3", "clk_axi4", REG_CLK_EN_3, 20, 0),
	CV1800B_GATE(CLK_APB_I2C4, "clk_apb_i2c4", "clk_axi4", REG_CLK_EN_3, 21, 0),
	CV1800B_GATE(CLK_AHB_SF1, "clk_ahb_sf1", "clk_axi4", REG_CLK_EN_3, 27, 0),
	CV1800B_GATE(CLK_APB_AUDSRC, "clk_apb_audsrc", "clk_axi4", REG_CLK_EN_4, 2, 0),
	CV1800B_GATE(CLK_DDR_AXI_REG, "clk_ddr_axi_reg", "clk_axi6", REG_CLK_EN_0, 7,
		     CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_APB_GPIO, "clk_apb_gpio", "clk_axi6", REG_CLK_EN_0, 29, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_APB_GPIO_INTR, "clk_apb_gpio_intr", "clk_axi6", REG_CLK_EN_0, 30,
		     CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_APB_JPEG, "clk_apb_jpeg", "clk_axi6", REG_CLK_EN_2, 13, CLK_IGNORE_UNUSED),
	CV1800B_GATE(CLK_APB_H264C, "clk_apb_h264c", "clk_axi6", REG_CLK_EN_2, 14, 0),
	CV1800B_GATE(CLK_APB_H265C, "clk_apb_h265c", "clk_axi6", REG_CLK_EN_2, 15, 0),
	CV1800B_GATE(CLK_PM, "clk_pm", "clk_axi6", REG_CLK_EN_3, 8, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_CFG_REG_VIP, "clk_cfg_reg_vip", "clk_axi6", REG_CLK_EN_3, 31, 0),
	CV1800B_GATE(CLK_CFG_REG_VC, "clk_cfg_reg_vc", "clk_axi6", REG_CLK_EN_4, 0,
		     CLK_IGNORE_UNUSED),
	CV1800B_GATE(CLK_PWM, "clk_pwm", "clk_pwm_src", REG_CLK_EN_1, 8, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_UART0, "clk_uart0", "clk_cam0_200", REG_CLK_EN_1, 14, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_UART1, "clk_uart1", "clk_cam0_200", REG_CLK_EN_1, 16, 0),
	CV1800B_GATE(CLK_UART2, "clk_uart2", "clk_cam0_200", REG_CLK_EN_1, 18, 0),
	CV1800B_GATE(CLK_UART3, "clk_uart3", "clk_cam0_200", REG_CLK_EN_1, 20, 0),
	CV1800B_GATE(CLK_UART4, "clk_uart4", "clk_cam0_200", REG_CLK_EN_1, 22, 0),
	CV1800B_GATE(CLK_H264C, "clk_h264c", "clk_axi_video_codec", REG_CLK_EN_2, 10, 0),
	CV1800B_GATE(CLK_H265C, "clk_h265c", "clk_axi_video_codec", REG_CLK_EN_2, 11, 0),
	CV1800B_GATE(CLK_JPEG, "clk_jpeg", "clk_axi_video_codec", REG_CLK_EN_2, 12,
		     CLK_IGNORE_UNUSED),
	CV1800B_GATE(CLK_CSI_MAC0_VIP, "clk_csi_mac0_vip", "clk_axi_vip", REG_CLK_EN_2, 18, 0),
	CV1800B_GATE(CLK_CSI_MAC1_VIP, "clk_csi_mac1_vip", "clk_axi_vip", REG_CLK_EN_2, 19, 0),
	CV1800B_GATE(CLK_ISP_TOP_VIP, "clk_isp_top_vip", "clk_axi_vip", REG_CLK_EN_2, 20, 0),
	CV1800B_GATE(CLK_IMG_D_VIP, "clk_img_d_vip", "clk_axi_vip", REG_CLK_EN_2, 21, 0),
	CV1800B_GATE(CLK_IMG_V_VIP, "clk_img_v_vip", "clk_axi_vip", REG_CLK_EN_2, 22, 0),
	CV1800B_GATE(CLK_SC_TOP_VIP, "clk_sc_top_vip", "clk_axi_vip", REG_CLK_EN_2, 23, 0),
	CV1800B_GATE(CLK_SC_D_VIP, "clk_sc_d_vip", "clk_axi_vip", REG_CLK_EN_2, 24, 0),
	CV1800B_GATE(CLK_SC_V1_VIP, "clk_sc_v1_vip", "clk_axi_vip", REG_CLK_EN_2, 25, 0),
	CV1800B_GATE(CLK_SC_V2_VIP, "clk_sc_v2_vip", "clk_axi_vip", REG_CLK_EN_2, 26, 0),
	CV1800B_GATE(CLK_SC_V3_VIP, "clk_sc_v3_vip", "clk_axi_vip", REG_CLK_EN_2, 27, 0),
	CV1800B_GATE(CLK_DWA_VIP, "clk_dwa_vip", "clk_axi_vip", REG_CLK_EN_2, 28, 0),
	CV1800B_GATE(CLK_BT_VIP, "clk_bt_vip", "clk_axi_vip", REG_CLK_EN_2, 29, 0),
	CV1800B_GATE(CLK_DISP_VIP, "clk_disp_vip", "clk_axi_vip", REG_CLK_EN_2, 30, 0),
	CV1800B_GATE(CLK_DSI_MAC_VIP, "clk_dsi_mac_vip", "clk_axi_vip", REG_CLK_EN_2, 31, 0),
	CV1800B_GATE(CLK_LVDS0_VIP, "clk_lvds0_vip", "clk_axi_vip", REG_CLK_EN_3, 0, 0),
	CV1800B_GATE(CLK_LVDS1_VIP, "clk_lvds1_vip", "clk_axi_vip", REG_CLK_EN_3, 1, 0),
	CV1800B_GATE(CLK_CSI0_RX_VIP, "clk_csi0_rx_vip", "clk_axi_vip", REG_CLK_EN_3, 2, 0),
	CV1800B_GATE(CLK_CSI1_RX_VIP, "clk_csi1_rx_vip", "clk_axi_vip", REG_CLK_EN_3, 3, 0),
	CV1800B_GATE(CLK_PAD_VI_VIP, "clk_pad_vi_vip", "clk_axi_vip", REG_CLK_EN_3, 4, 0),
	CV1800B_GATE(CLK_PAD_VI1_VIP, "clk_pad_vi1_vip", "clk_axi_vip", REG_CLK_EN_3, 30, 0),
	CV1800B_GATE(CLK_PAD_VI2_VIP, "clk_pad_vi2_vip", "clk_axi_vip", REG_CLK_EN_4, 7, 0),
	CV1800B_GATE(CLK_CSI_BE_VIP, "clk_csi_be_vip", "clk_axi_vip", REG_CLK_EN_4, 8, 0),
	CV1800B_GATE(CLK_VIP_IP0, "clk_vip_ip0", "clk_axi_vip", REG_CLK_EN_4, 9, 0),
	CV1800B_GATE(CLK_VIP_IP1, "clk_vip_ip1", "clk_axi_vip", REG_CLK_EN_4, 10, 0),
	CV1800B_GATE(CLK_VIP_IP2, "clk_vip_ip2", "clk_axi_vip", REG_CLK_EN_4, 11, 0),
	CV1800B_GATE(CLK_VIP_IP3, "clk_vip_ip3", "clk_axi_vip", REG_CLK_EN_4, 12, 0),
	CV1800B_GATE(CLK_IVE_VIP, "clk_ive_vip", "clk_axi_vip", REG_CLK_EN_4, 17, 0),
	CV1800B_GATE(CLK_RAW_VIP, "clk_raw_vip", "clk_axi_vip", REG_CLK_EN_4, 18, 0),
	CV1800B_GATE(CLK_OSDC_VIP, "clk_osdc_vip", "clk_axi_vip", REG_CLK_EN_4, 19, 0),
	CV1800B_GATE(CLK_CSI_MAC2_VIP, "clk_csi_mac2_vip", "clk_axi_vip", REG_CLK_EN_4, 20, 0),
	CV1800B_GATE(CLK_CAM0_VIP, "clk_cam0_vip", "clk_axi_vip", REG_CLK_EN_4, 21, 0),
	CV1800B_GATE(CLK_TIMER0, "clk_timer0", "clk_xtal_misc", REG_CLK_EN_3, 9, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER1, "clk_timer1", "clk_xtal_misc", REG_CLK_EN_3, 10, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER2, "clk_timer2", "clk_xtal_misc", REG_CLK_EN_3, 11, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER3, "clk_timer3", "clk_xtal_misc", REG_CLK_EN_3, 12, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER4, "clk_timer4", "clk_xtal_misc", REG_CLK_EN_3, 13, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER5, "clk_timer5", "clk_xtal_misc", REG_CLK_EN_3, 14, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER6, "clk_timer6", "clk_xtal_misc", REG_CLK_EN_3, 15, CLK_IS_CRITICAL),
	CV1800B_GATE(CLK_TIMER7, "clk_timer7", "clk_xtal_misc", REG_CLK_EN_3, 16, CLK_IS_CRITICAL),
};

struct cv1800b_clk_div cv1800b_div_info[] = {
	CV1800B_DIV(CLK_1M, "clk_1m", "osc", REG_CLK_EN_3, 5,
		    REG_DIV_CLK_1M, 16, 6, 25, CLK_IS_CRITICAL),
	CV1800B_DIV(CLK_EMMC_100K, "clk_emmc_100k", "clk_1m", REG_CLK_EN_0, 17,
		    REG_DIV_CLK_EMMC_100K, 16, 8, 10, 0),
	CV1800B_DIV(CLK_SD0_100K, "clk_sd0_100k", "clk_1m", REG_CLK_EN_0, 20,
		    REG_DIV_CLK_SD0_100K, 16, 8, 10, 0),
	CV1800B_DIV(CLK_SD1_100K, "clk_sd1_100k", "clk_1m", REG_CLK_EN_0, 23,
		    REG_DIV_CLK_SD1_100K, 16, 8, 10, 0),
	CV1800B_DIV(CLK_GPIO_DB, "clk_gpio_db", "clk_1m", REG_CLK_EN_0, 31,
		    REG_DIV_CLK_GPIO_DB, 16, 16, 10, CLK_IS_CRITICAL)
};

struct cv1800b_clk_bypass_div cv1800b_bypass_div_info[] = {
	CV1800B_BYPASS_DIV(CLK_AP_DEBUG, "clk_ap_debug", "clk_fpll", REG_CLK_EN_4, 5,
			   REG_DIV_CLK_AP_DEBUG, 16, 4, 5, REG_CLK_BYP_1, 4, CLK_IS_CRITICAL),
	CV1800B_BYPASS_DIV(CLK_SRC_RTC_SYS_0, "clk_src_rtc_sys_0", "clk_fpll", REG_CLK_EN_4, 6,
			   REG_DIV_CLK_RTCSYS_SRC_0, 16, 4, 5, REG_CLK_BYP_1, 5, CLK_IS_CRITICAL),
	CV1800B_BYPASS_DIV(CLK_CPU_GIC, "clk_cpu_gic", "clk_fpll", REG_CLK_EN_0, 2,
			   REG_DIV_CLK_CPU_GIC, 16, 4, 5, REG_CLK_BYP_0, 2, CLK_IS_CRITICAL),
	CV1800B_BYPASS_DIV(CLK_ETH0_500M, "clk_eth0_500m", "clk_fpll", REG_CLK_EN_0, 25,
			   REG_DIV_CLK_GPIO_DB, 16, 4, 3, REG_CLK_BYP_0, 9, 0),
	CV1800B_BYPASS_DIV(CLK_ETH1_500M, "clk_eth1_500m", "clk_fpll", REG_CLK_EN_0, 27,
			   REG_DIV_CLK_GPIO_DB, 16, 4, 3, REG_CLK_BYP_0, 10, 0),
	CV1800B_BYPASS_DIV(CLK_AXI6, "clk_axi6", "clk_fpll", REG_CLK_EN_2, 2, REG_DIV_CLK_AXI6, 16,
			   4, 15, REG_CLK_BYP_0, 20, CLK_IS_CRITICAL),
	CV1800B_BYPASS_DIV(CLK_SPI, "clk_spi", "clk_fpll", REG_CLK_EN_3, 6, REG_DIV_CLK_SPI, 16, 6,
			   8, REG_CLK_BYP_0, 30, 0),
	CV1800B_BYPASS_DIV(CLK_DISP_SRC_VIP, "clk_disp_src_vip", "clk_disppll", REG_CLK_EN_2, 7,
			   REG_DIV_CLK_DISP_SRC_VIP, 16, 4, 8, REG_CLK_BYP_0, 25, 0),
	CV1800B_BYPASS_DIV(CLK_CPU_AXI0, "clk_cpu_axi0", "clk_axi4", REG_CLK_EN_0, 1,
			   REG_DIV_CLK_CPU_AXI0, 16, 4, 3, REG_CLK_BYP_0, 1, CLK_IS_CRITICAL),
	CV1800B_BYPASS_DIV(CLK_DSI_ESC, "clk_dsi_esc", "clk_axi6", REG_CLK_EN_2, 3,
			   REG_DIV_CLK_DSI_ESC, 16, 4, 5, REG_CLK_BYP_0, 21, 0),
	CV1800B_BYPASS_DIV(CLK_I2C, "clk_i2c", "clk_axi6", REG_CLK_EN_3, 7, REG_DIV_CLK_I2C, 16, 4,
			   1, REG_CLK_BYP_0, 31, 0),
};

struct cv1800b_clk_fixed_div cv1800b_fixed_div_info[] = {
	CV1800B_FIXED_DIV(CLK_CAM0PLL_D2, "clk_cam0pll_d2", "clk_cam0pll",
			  REG_CAM0PLL_CLK_CSR, 1, 2,
			  CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED),
	CV1800B_FIXED_DIV(CLK_CAM0PLL_D3, "clk_cam0pll_d3", "clk_cam0pll",
			  REG_CAM0PLL_CLK_CSR, 2, 3,
			  CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED),
	CV1800B_FIXED_DIV(CLK_MIPIMPLL_D3, "clk_mipimpll_d3", "clk_mipimpll",
			  REG_MIPIMPLL_CLK_CSR, 2, 3,
			  CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED),
	CV1800B_FIXED_DIV(CLK_USB_33K, "clk_usb_33k", "clk_1m",
			  REG_CLK_EN_1, 31, 3,
			  0),
};

struct cv1800b_clk_bypass_fixed_div cv1800b_bypass_fixed_div_info[] = {
	CV1800B_BYPASS_FIXED_DIV(CLK_USB_125M, "clk_usb_125m", "clk_fpll",
				 REG_CLK_EN_1, 30, 12,
				 REG_CLK_BYP_0, 17,
				 CLK_SET_RATE_PARENT),
	CV1800B_BYPASS_FIXED_DIV(CLK_USB_12M, "clk_usb_12m", "clk_fpll",
				 REG_CLK_EN_2, 0, 125,
				 REG_CLK_BYP_0, 18,
				 CLK_SET_RATE_PARENT),
	CV1800B_BYPASS_FIXED_DIV(CLK_VC_SRC1, "clk_vc_src1", "clk_fpll",
				 REG_CLK_EN_3, 28, 2,
				 REG_CLK_BYP_1, 0,
				 CLK_SET_RATE_PARENT),
	CV1800B_BYPASS_FIXED_DIV(CLK_VC_SRC2, "clk_vc_src2", "clk_fpll",
				 REG_CLK_EN_4, 3, 3,
				 REG_CLK_BYP_1, 3,
				 CLK_SET_RATE_PARENT),
};

struct cv1800b_clk_mux cv1800b_mux_info[] = {
	CV1800B_MUX(CLK_CAM0, "clk_cam0", clk_cam_parents,
		    REG_CLK_EN_2, 16,
		    REG_CLK_CAM0_SRC_DIV, 16, 6, 0,
		    REG_CLK_CAM0_SRC_DIV, 8, 2,
		    CLK_IGNORE_UNUSED),
	CV1800B_MUX(CLK_CAM1, "clk_cam1", clk_cam_parents,
		    REG_CLK_EN_2, 17,
		    REG_CLK_CAM1_SRC_DIV, 16, 6, 0,
		    REG_CLK_CAM1_SRC_DIV, 8, 2,
		    CLK_IGNORE_UNUSED),
};

struct cv1800b_clk_bypass_mux cv1800b_bypass_mux_info[] = {
	CV1800B_BYPASS_MUX(CLK_TPU, "clk_tpu", clk_tpu_parents,
			   REG_CLK_EN_0, 4,
			   REG_DIV_CLK_TPU, 16, 4, 3,
			   REG_DIV_CLK_TPU, 8, 2,
			   REG_CLK_BYP_0, 3,
			   0),
	CV1800B_BYPASS_MUX(CLK_EMMC, "clk_emmc", clk_axi4_parents,
			   REG_CLK_EN_0, 16,
			   REG_DIV_CLK_EMMC, 16, 5, 15,
			   REG_DIV_CLK_EMMC, 8, 2,
			   REG_CLK_BYP_0, 5,
			   0),
	CV1800B_BYPASS_MUX(CLK_SD0, "clk_sd0", clk_axi4_parents,
			   REG_CLK_EN_0, 19,
			   REG_DIV_CLK_SD0, 16, 5, 15,
			   REG_DIV_CLK_SD0, 8, 2,
			   REG_CLK_BYP_0, 6,
			   0),
	CV1800B_BYPASS_MUX(CLK_SD1, "clk_sd1", clk_axi4_parents,
			   REG_CLK_EN_0, 22,
			   REG_DIV_CLK_SD1, 16, 5, 15,
			   REG_DIV_CLK_SD1, 8, 2,
			   REG_CLK_BYP_0, 7,
			   0),
	CV1800B_BYPASS_MUX(CLK_SPI_NAND, "clk_spi_nand", clk_axi4_parents,
			   REG_CLK_EN_0, 24,
			   REG_DIV_CLK_SPI_NAND, 16, 5, 8,
			   REG_DIV_CLK_SPI_NAND, 8, 2,
			   REG_CLK_BYP_0, 8,
			   0),
	CV1800B_BYPASS_MUX(CLK_AXI4, "clk_axi4", clk_axi4_parents,
			   REG_CLK_EN_2, 1,
			   REG_DIV_CLK_AXI4, 16, 4, 5,
			   REG_DIV_CLK_AXI4, 8, 2,
			   REG_CLK_BYP_0, 19,
			   CLK_IS_CRITICAL),
	CV1800B_BYPASS_MUX(CLK_PWM_SRC, "clk_pwm_src", clk_axi4_parents,
			   REG_CLK_EN_4, 4,
			   REG_DIV_CLK_PWM_SRC_0, 16, 6, 10,
			   REG_DIV_CLK_PWM_SRC_0, 8, 2,
			   REG_CLK_BYP_0, 15,
			   CLK_IS_CRITICAL),
	CV1800B_BYPASS_MUX(CLK_AUDSRC, "clk_audsrc", clk_aud_parents,
			   REG_CLK_EN_4, 1,
			   REG_DIV_CLK_AUDSRC, 16, 8, 18,
			   REG_DIV_CLK_AUDSRC, 8, 2,
			   REG_CLK_BYP_1, 2,
			   0),
	CV1800B_BYPASS_MUX(CLK_SDMA_AUD0, "clk_sdma_aud0", clk_aud_parents,
			   REG_CLK_EN_1, 2,
			   REG_DIV_CLK_SDMA_AUD0, 16, 8, 18,
			   REG_DIV_CLK_SDMA_AUD0, 8, 2,
			   REG_CLK_BYP_0, 11,
			   0),
	CV1800B_BYPASS_MUX(CLK_SDMA_AUD1, "clk_sdma_aud1", clk_aud_parents,
			   REG_CLK_EN_1, 3,
			   REG_DIV_CLK_SDMA_AUD1, 16, 8, 18,
			   REG_DIV_CLK_SDMA_AUD1, 8, 2,
			   REG_CLK_BYP_0, 12,
			   0),
	CV1800B_BYPASS_MUX(CLK_SDMA_AUD2, "clk_sdma_aud2", clk_aud_parents,
			   REG_CLK_EN_1, 3,
			   REG_DIV_CLK_SDMA_AUD2, 16, 8, 18,
			   REG_DIV_CLK_SDMA_AUD2, 8, 2,
			   REG_CLK_BYP_0, 13,
			   0),
	CV1800B_BYPASS_MUX(CLK_SDMA_AUD3, "clk_sdma_aud3", clk_aud_parents,
			   REG_CLK_EN_1, 3,
			   REG_DIV_CLK_SDMA_AUD3, 16, 8, 18,
			   REG_DIV_CLK_SDMA_AUD3, 8, 2,
			   REG_CLK_BYP_0, 14,
			   0),
	CV1800B_BYPASS_MUX(CLK_CAM0_200, "clk_cam0_200", clk_cam0_200_parents,
			   REG_CLK_EN_1, 13,
			   REG_DIV_CLK_CAM0_200, 16, 4, 1,
			   REG_DIV_CLK_CAM0_200, 8, 2,
			   REG_CLK_BYP_0, 16,
			   CLK_IS_CRITICAL),
	CV1800B_BYPASS_MUX(CLK_AXI_VIP, "clk_axi_vip", clk_vip_sys_parents,
			   REG_CLK_EN_2, 4,
			   REG_DIV_CLK_AXI_VIP, 16, 4, 3,
			   REG_DIV_CLK_AXI_VIP, 8, 2,
			   REG_CLK_BYP_0, 22,
			   0),
	CV1800B_BYPASS_MUX(CLK_SRC_VIP_SYS_0, "clk_src_vip_sys_0", clk_vip_sys_parents,
			   REG_CLK_EN_2, 5,
			   REG_DIV_CLK_SRC_VIP_SYS_0, 16, 4, 6,
			   REG_DIV_CLK_SRC_VIP_SYS_0, 8, 2,
			   REG_CLK_BYP_0, 23,
			   0),
	CV1800B_BYPASS_MUX(CLK_SRC_VIP_SYS_1, "clk_src_vip_sys_1", clk_vip_sys_parents,
			   REG_CLK_EN_2, 6,
			   REG_DIV_CLK_SRC_VIP_SYS_1, 16, 4, 6,
			   REG_DIV_CLK_SRC_VIP_SYS_1, 8, 2,
			   REG_CLK_BYP_0, 24,
			   0),
	CV1800B_BYPASS_MUX(CLK_SRC_VIP_SYS_2, "clk_src_vip_sys_2", clk_vip_sys_parents,
			   REG_CLK_EN_3, 29,
			   REG_DIV_CLK_SRC_VIP_SYS_2, 16, 4, 2,
			   REG_DIV_CLK_SRC_VIP_SYS_2, 8, 2,
			   REG_CLK_BYP_1, 1,
			   0),
	CV1800B_BYPASS_MUX(CLK_SRC_VIP_SYS_3, "clk_src_vip_sys_3", clk_vip_sys_parents,
			   REG_CLK_EN_4, 15,
			   REG_DIV_CLK_SRC_VIP_SYS_3, 16, 4, 2,
			   REG_DIV_CLK_SRC_VIP_SYS_3, 8, 2,
			   REG_CLK_BYP_1, 8,
			   0),
	CV1800B_BYPASS_MUX(CLK_SRC_VIP_SYS_4, "clk_src_vip_sys_4", clk_vip_sys_parents,
			   REG_CLK_EN_4, 16,
			   REG_DIV_CLK_SRC_VIP_SYS_4, 16, 4, 3,
			   REG_DIV_CLK_SRC_VIP_SYS_4, 8, 2,
			   REG_CLK_BYP_1, 9,
			   0),
	CV1800B_BYPASS_MUX(CLK_AXI_VIDEO_CODEC, "clk_axi_video_codec", clk_axi_video_codec_parents,
			   REG_CLK_EN_2, 8,
			   REG_DIV_CLK_AXI_VIDEO_CODEC, 16, 4, 2,
			   REG_DIV_CLK_AXI_VIDEO_CODEC, 8, 2,
			   REG_CLK_BYP_0, 26,
			   0),
	CV1800B_BYPASS_MUX(CLK_VC_SRC0, "clk_vc_src0", clk_vc_src0_parents,
			   REG_CLK_EN_2, 9,
			   REG_DIV_CLK_VC_SRC0, 16, 4, 2,
			   REG_DIV_CLK_VC_SRC0, 8, 2,
			   REG_CLK_BYP_0, 27,
			   0),
};

struct cv1800b_clk_mmux cv1800b_mmux_info[] = {
	CV1800B_MMUX(CLK_C906_0, "clk_c906_0", clk_c906_0_parents,
		     REG_CLK_EN_4, 13,
		     REG_DIV_CLK_C906_0_0, 16, 4, 1,
		     REG_DIV_CLK_C906_0_1, 16, 4, 2,
		     REG_DIV_CLK_C906_0_0, 8, 2,
		     REG_DIV_CLK_C906_0_1, 8, 2,
		     REG_CLK_BYP_1, 6,
		     REG_CLK_SEL_0, 23,
		     CLK_IS_CRITICAL | CLK_GET_RATE_NOCACHE),
	CV1800B_MMUX(CLK_C906_1, "clk_c906_1", clk_c906_1_parents,
		     REG_CLK_EN_4, 14,
		     REG_DIV_CLK_C906_1_0, 16, 4, 2,
		     REG_DIV_CLK_C906_1_1, 16, 4, 3,
		     REG_DIV_CLK_C906_1_0, 8, 2,
		     REG_DIV_CLK_C906_1_1, 8, 2,
		     REG_CLK_BYP_1, 7,
		     REG_CLK_SEL_0, 24,
		     CLK_IS_CRITICAL | CLK_GET_RATE_NOCACHE),
	CV1800B_MMUX(CLK_A53, "clk_a53", clk_a53_parents,
		     REG_CLK_EN_0, 0,
		     REG_DIV_CLK_A53_0, 16, 4, 1,
		     REG_DIV_CLK_A53_1, 16, 4, 2,
		     REG_DIV_CLK_A53_0, 8, 2,
		     REG_DIV_CLK_A53_1, 8, 2,
		     REG_CLK_BYP_0, 0,
		     REG_CLK_SEL_0, 0,
		     CLK_IS_CRITICAL | CLK_GET_RATE_NOCACHE),
};

static struct cv1800b_clk_audio cv1800b_audio_info[] = {
	CV1800B_AUDIO(CLK_A24M, "clk_a24m", "clk_mipimpll",
		      REG_APLL_FRAC_DIV_CTRL, 0,
		      REG_APLL_FRAC_DIV_CTRL, 3,
		      REG_APLL_FRAC_DIV_CTRL, 1,
		      REG_APLL_FRAC_DIV_CTRL, 2,
		      REG_APLL_FRAC_DIV_M, 0, 22,
		      REG_APLL_FRAC_DIV_N, 0, 22,
		      0),
};

static struct cv1800b_clk_ipll cv1800b_ipll_info[] = {
	CV1800B_IPLL(CLK_FPLL, "clk_fpll", "osc", REG_FPLL_CSR,
		     REG_PLL_G6_CTRL, 8,
		     REG_PLL_G6_STATUS, 2,
		     CLK_IS_CRITICAL),
	CV1800B_IPLL(CLK_MIPIMPLL, "clk_mipimpll", "osc", REG_MIPIMPLL_CSR,
		     REG_PLL_G2_CTRL, 0,
		     REG_PLL_G2_STATUS, 0,
		     CLK_IS_CRITICAL),
};

static struct cv1800b_clk_fpll cv1800b_fpll_info[] = {
	CV1800B_FPLL(CLK_MPLL, "clk_mpll", "osc", REG_MPLL_CSR,
		     REG_PLL_G6_CTRL, 0,
		     REG_PLL_G6_STATUS, 0,
		     REG_PLL_G6_SSC_SYN_CTRL, 2,
		     REG_PLL_G6_SSC_SYN_CTRL, 0,
		     REG_MPLL_SSC_SYN_CTRL, REG_MPLL_SSC_SYN_SET,
		     CLK_IS_CRITICAL),
	CV1800B_FPLL(CLK_TPLL, "clk_tpll", "osc", REG_TPLL_CSR,
		     REG_PLL_G6_CTRL, 4,
		     REG_PLL_G6_STATUS, 1,
		     REG_PLL_G6_SSC_SYN_CTRL, 3,
		     REG_PLL_G6_SSC_SYN_CTRL, 0,
		     REG_TPLL_SSC_SYN_CTRL, REG_TPLL_SSC_SYN_SET,
		     CLK_IS_CRITICAL),
	CV1800B_FPLL(CLK_A0PLL, "clk_a0pll", "clk_mipimpll", REG_A0PLL_CSR,
		     REG_PLL_G2_CTRL, 4,
		     REG_PLL_G2_STATUS, 1,
		     REG_PLL_G2_SSC_SYN_CTRL, 2,
		     REG_PLL_G2_SSC_SYN_CTRL, 0,
		     REG_A0PLL_SSC_SYN_CTRL, REG_A0PLL_SSC_SYN_SET,
		     CLK_IS_CRITICAL),
	CV1800B_FPLL(CLK_DISPPLL, "clk_disppll", "clk_mipimpll", REG_DISPPLL_CSR,
		     REG_PLL_G2_CTRL, 8,
		     REG_PLL_G2_STATUS, 2,
		     REG_PLL_G2_SSC_SYN_CTRL, 3,
		     REG_PLL_G2_SSC_SYN_CTRL, 0,
		     REG_DISPPLL_SSC_SYN_CTRL, REG_DISPPLL_SSC_SYN_SET,
		     CLK_IS_CRITICAL),
	CV1800B_FPLL(CLK_CAM0PLL, "clk_cam0pll", "clk_mipimpll", REG_CAM0PLL_CSR,
		     REG_PLL_G2_CTRL, 12,
		     REG_PLL_G2_STATUS, 3,
		     REG_PLL_G2_SSC_SYN_CTRL, 4,
		     REG_PLL_G2_SSC_SYN_CTRL, 0,
		     REG_CAM0PLL_SSC_SYN_CTRL, REG_CAM0PLL_SSC_SYN_SET,
		     CLK_IGNORE_UNUSED),
	CV1800B_FPLL(CLK_CAM1PLL, "clk_cam1pll", "clk_mipimpll", REG_CAM1PLL_CSR,
		     REG_PLL_G2_CTRL, 16,
		     REG_PLL_G2_STATUS, 4,
		     REG_PLL_G2_SSC_SYN_CTRL, 5,
		     REG_PLL_G2_SSC_SYN_CTRL, 0,
		     REG_CAM1PLL_SSC_SYN_CTRL, REG_CAM1PLL_SSC_SYN_SET,
		     CLK_IS_CRITICAL),
};

static int cv1800b_register_clk(struct udevice *dev)
{
	struct clk osc;
	ulong osc_rate;
	void *base = devfdt_get_addr_ptr(dev);
	int i, ret;

	ret = clk_get_by_index(dev, 0, &osc);
	if (ret) {
		pr_err("Failed to get clock\n");
		return ret;
	}

	osc_rate = clk_get_rate(&osc);
	clk_dm(CV1800B_CLK_OSC, clk_register_fixed_rate(NULL, "osc", osc_rate));
	clk_dm(CV1800B_CLK_BYPASS, clk_register_fixed_rate(NULL, "bypass", osc_rate));

	for (i = 0; i < ARRAY_SIZE(cv1800b_ipll_info); i++) {
		struct cv1800b_clk_ipll *ipll = &cv1800b_ipll_info[i];

		ipll->base = base;
		ret = clk_register(&ipll->clk, "cv1800b_clk_ipll", ipll->name,
				   ipll->parent_name);
		if (ret) {
			pr_err("Failed to register ipll %s\n", ipll->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_fpll_info); i++) {
		struct cv1800b_clk_fpll *fpll = &cv1800b_fpll_info[i];

		fpll->ipll.base = base;
		ret = clk_register(&fpll->ipll.clk, "cv1800b_clk_fpll",
				   fpll->ipll.name, fpll->ipll.parent_name);
		if (ret) {
			pr_err("Failed to register fpll %s\n", fpll->ipll.name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_div_info); i++) {
		struct cv1800b_clk_div *div = &cv1800b_div_info[i];

		div->base = base;
		ret = clk_register(&div->clk, "cv1800b_clk_div", div->name,
				   div->parent_name);
		if (ret) {
			pr_err("Failed to register div %s\n", div->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_fixed_div_info); i++) {
		struct cv1800b_clk_fixed_div *fixed_div =
			&cv1800b_fixed_div_info[i];

		fixed_div->base = base;
		ret = clk_register(&fixed_div->clk, "cv1800b_clk_fixed_div",
				   fixed_div->name, fixed_div->parent_name);
		if (ret) {
			pr_err("Failed to register fixed div %s\n",
			       fixed_div->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_bypass_fixed_div_info); i++) {
		struct cv1800b_clk_bypass_fixed_div *bypass_fixed_div =
			&cv1800b_bypass_fixed_div_info[i];

		bypass_fixed_div->div.base = base;
		ret = clk_register(&bypass_fixed_div->div.clk,
				   "cv1800b_clk_bypass_fixed_div",
				   bypass_fixed_div->div.name,
				   bypass_fixed_div->div.parent_name);
		if (ret) {
			pr_err("Failed to register bypass fixed div %s\n",
			       bypass_fixed_div->div.name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_mux_info); i++) {
		struct cv1800b_clk_mux *mux = &cv1800b_mux_info[i];
		int parent;

		mux->base = base;
		parent = cv1800b_clk_getfield(base, &mux->mux);
		ret = clk_register(&mux->clk, "cv1800b_clk_mux", mux->name,
				   mux->parent_names[parent]);
		if (ret) {
			pr_err("Failed to register mux %s\n", mux->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_mmux_info); i++) {
		struct cv1800b_clk_mmux *mmux = &cv1800b_mmux_info[i];
		int clk_sel, parent, idx;

		mmux->base = base;
		clk_sel = cv1800b_clk_getbit(base, &mmux->clk_sel) ? 0 : 1;
		parent = cv1800b_clk_getfield(base, &mmux->mux[clk_sel]);
		for (idx = 0; idx < mmux->num_parents; idx++) {
			if (clk_sel == mmux->parent_infos[idx].clk_sel &&
			    parent == mmux->parent_infos[idx].index)
				break;
		}
		ret = clk_register(&mmux->clk, "cv1800b_clk_mmux", mmux->name,
				   mmux->parent_infos[idx].name);
		if (ret) {
			pr_err("Failed to register mmux %s\n", mmux->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_audio_info); i++) {
		struct cv1800b_clk_audio *audio = &cv1800b_audio_info[i];

		audio->base = base;
		ret = clk_register(&audio->clk, "cv1800b_clk_audio",
				   audio->name, audio->parent_name);
		if (ret) {
			pr_err("Failed to register audio %s\n", audio->name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_bypass_mux_info); i++) {
		struct cv1800b_clk_bypass_mux *bypass_mux =
			&cv1800b_bypass_mux_info[i];
		int parent;

		bypass_mux->mux.base = base;
		parent = cv1800b_clk_getfield(base, &bypass_mux->mux.mux);
		ret = clk_register(&bypass_mux->mux.clk,
				   "cv1800b_clk_bypass_mux",
				   bypass_mux->mux.name,
				   bypass_mux->mux.parent_names[parent]);
		if (ret) {
			pr_err("Failed to register bypass mux %s\n",
			       bypass_mux->mux.name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_bypass_div_info); i++) {
		struct cv1800b_clk_bypass_div *bypass_div =
			&cv1800b_bypass_div_info[i];

		bypass_div->div.base = base;
		ret = clk_register(&bypass_div->div.clk,
				   "cv1800b_clk_bypass_div",
				   bypass_div->div.name,
				   bypass_div->div.parent_name);
		if (ret) {
			pr_err("Failed to register bypass div %s\n",
			       bypass_div->div.name);
			return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(cv1800b_gate_info); i++) {
		struct cv1800b_clk_gate *gate = &cv1800b_gate_info[i];

		gate->base = base;
		ret = clk_register(&gate->clk, "cv1800b_clk_gate", gate->name,
				   gate->parent_name);
		if (ret) {
			pr_err("Failed to register gate %s\n", gate->name);
			return ret;
		}
	}
	return 0;
}

static int cv1800b_clk_probe(struct udevice *dev)
{
	return cv1800b_register_clk(dev);
}

static int cv1800b_clk_enable(struct clk *clk)
{
	struct clk *c;
	int err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(clk->id), &c);

	if (err)
		return err;
	return clk_enable(c);
}

static int cv1800b_clk_disable(struct clk *clk)
{
	struct clk *c;
	int err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(clk->id), &c);

	if (err)
		return err;
	return clk_disable(c);
}

static ulong cv1800b_clk_get_rate(struct clk *clk)
{
	struct clk *c;
	int err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(clk->id), &c);

	if (err)
		return err;
	return clk_get_rate(c);
}

static ulong cv1800b_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk *c;
	int err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(clk->id), &c);

	if (err)
		return err;
	return clk_set_rate(c, rate);
}

static int cv1800b_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct clk *c, *p;
	int err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(clk->id), &c);

	if (err)
		return err;
	err = clk_get_by_id(CV1800B_CLK_ID_TRANSFORM(parent->id), &p);
	if (err)
		return err;
	return clk_set_parent(c, p);
}

const struct clk_ops cv1800b_clk_ops = {
	.enable = cv1800b_clk_enable,
	.disable = cv1800b_clk_disable,
	.get_rate = cv1800b_clk_get_rate,
	.set_rate = cv1800b_clk_set_rate,
	.set_parent = cv1800b_clk_set_parent,
};

static const struct udevice_id cv1800b_clk_of_match[] = {
	{ .compatible = "sophgo,cv1800-clk" },
	{ },
};

U_BOOT_DRIVER(sophgo_clk) = {
	.name      = "cv1800b_clk",
	.id        = UCLASS_CLK,
	.of_match  = cv1800b_clk_of_match,
	.probe     = cv1800b_clk_probe,
	.ops       = &cv1800b_clk_ops,
	.flags	   = DM_FLAG_PRE_RELOC,
};
