// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2021 NXP.
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imx93-clock.h>

#include "clk.h"

#define IMX93_CLK_END 207

#define PLAT_IMX93 BIT(0)
#define PLAT_IMX91 BIT(1)

enum clk_sel {
	LOW_SPEED_IO_SEL,
	NON_IO_SEL,
	FAST_SEL,
	AUDIO_SEL,
	VIDEO_SEL,
	TPM_SEL,
	CKO1_SEL,
	CKO2_SEL,
	MISC_SEL,
	MAX_SEL
};

static u32 share_count_sai1;
static u32 share_count_sai2;
static u32 share_count_sai3;
static u32 share_count_mub;

static const char * const a55_core_sels[] = {"a55_alt", "arm_pll"};
static const char *parent_names[MAX_SEL][4] = {
	{"clock-osc-24m", "sys_pll_pfd0_div2", "sys_pll_pfd1_div2", "video_pll"},
	{"clock-osc-24m", "sys_pll_pfd0_div2", "sys_pll_pfd1_div2", "sys_pll_pfd2_div2"},
	{"clock-osc-24m", "sys_pll_pfd0", "sys_pll_pfd1", "sys_pll_pfd2"},
	{"clock-osc-24m", "audio_pll", "video_pll", "clk_ext1"},
	{"clock-osc-24m", "audio_pll", "video_pll", "sys_pll_pfd0"},
	{"clock-osc-24m", "sys_pll_pfd0", "audio_pll", "clk_ext1"},
	{"clock-osc-24m", "sys_pll_pfd0", "sys_pll_pfd1", "audio_pll"},
	{"clock-osc-24m", "sys_pll_pfd0", "sys_pll_pfd1", "video_pll"},
	{"clock-osc-24m", "audio_pll", "video_pll", "sys_pll_pfd2"},
};

static const struct imx93_clk_root {
	u32 clk;
	char *name;
	u32 off;
	enum clk_sel sel;
	unsigned long flags;
	unsigned long plat;
} root_array[] = {
	/* a55/m33/bus critical clk for system run */
	{ IMX93_CLK_A55_PERIPH,		"a55_periph_root",	0x0000,	FAST_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_A55_MTR_BUS,	"a55_mtr_bus_root",	0x0080,	LOW_SPEED_IO_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_A55,		"a55_alt_root",		0x0100,	FAST_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_M33,		"m33_root",		0x0180,	LOW_SPEED_IO_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_BUS_WAKEUP,		"bus_wakeup_root",	0x0280,	LOW_SPEED_IO_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_BUS_AON,		"bus_aon_root",		0x0300,	LOW_SPEED_IO_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_WAKEUP_AXI,		"wakeup_axi_root",	0x0380,	FAST_SEL, CLK_IS_CRITICAL },
	{ IMX93_CLK_SWO_TRACE,		"swo_trace_root",	0x0400,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_M33_SYSTICK,	"m33_systick_root",	0x0480,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_FLEXIO1,		"flexio1_root",		0x0500,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_FLEXIO2,		"flexio2_root",		0x0580,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPTMR1,		"lptmr1_root",		0x0700,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPTMR2,		"lptmr2_root",		0x0780,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_TPM2,		"tpm2_root",		0x0880,	TPM_SEL, },
	{ IMX93_CLK_TPM4,		"tpm4_root",		0x0980,	TPM_SEL, },
	{ IMX93_CLK_TPM5,		"tpm5_root",		0x0a00,	TPM_SEL, },
	{ IMX93_CLK_TPM6,		"tpm6_root",		0x0a80,	TPM_SEL, },
	{ IMX93_CLK_FLEXSPI1,		"flexspi1_root",	0x0b00,	FAST_SEL, },
	{ IMX93_CLK_CAN1,		"can1_root",		0x0b80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_CAN2,		"can2_root",		0x0c00,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART1,		"lpuart1_root",		0x0c80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART2,		"lpuart2_root",		0x0d00,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART3,		"lpuart3_root",		0x0d80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART4,		"lpuart4_root",		0x0e00,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART5,		"lpuart5_root",		0x0e80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART6,		"lpuart6_root",		0x0f00,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART7,		"lpuart7_root",		0x0f80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPUART8,		"lpuart8_root",		0x1000,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C1,		"lpi2c1_root",		0x1080,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C2,		"lpi2c2_root",		0x1100,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C3,		"lpi2c3_root",		0x1180,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C4,		"lpi2c4_root",		0x1200,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C5,		"lpi2c5_root",		0x1280,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C6,		"lpi2c6_root",		0x1300,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C7,		"lpi2c7_root",		0x1380,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPI2C8,		"lpi2c8_root",		0x1400,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI1,		"lpspi1_root",		0x1480,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI2,		"lpspi2_root",		0x1500,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI3,		"lpspi3_root",		0x1580,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI4,		"lpspi4_root",		0x1600,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI5,		"lpspi5_root",		0x1680,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI6,		"lpspi6_root",		0x1700,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI7,		"lpspi7_root",		0x1780,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_LPSPI8,		"lpspi8_root",		0x1800,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_I3C1,		"i3c1_root",		0x1880,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_I3C2,		"i3c2_root",		0x1900,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_USDHC1,		"usdhc1_root",		0x1980,	FAST_SEL, },
	{ IMX93_CLK_USDHC2,		"usdhc2_root",		0x1a00,	FAST_SEL, },
	{ IMX93_CLK_USDHC3,		"usdhc3_root",		0x1a80,	FAST_SEL, },
	{ IMX93_CLK_SAI1,		"sai1_root",		0x1b00,	AUDIO_SEL, },
	{ IMX93_CLK_SAI2,		"sai2_root",		0x1b80,	AUDIO_SEL, },
	{ IMX93_CLK_SAI3,		"sai3_root",		0x1c00,	AUDIO_SEL, },
	{ IMX93_CLK_CCM_CKO1,		"ccm_cko1_root",	0x1c80,	CKO1_SEL, },
	{ IMX93_CLK_CCM_CKO2,		"ccm_cko2_root",	0x1d00,	CKO2_SEL, },
	{ IMX93_CLK_CCM_CKO3,		"ccm_cko3_root",	0x1d80,	CKO1_SEL, },
	{ IMX93_CLK_CCM_CKO4,		"ccm_cko4_root",	0x1e00,	CKO2_SEL, },
	/*
	 * Critical because clk is used for handshake between HSIOMIX and NICMIX when
	 * NICMIX power down/on during system suspend/resume
	 */
	{ IMX93_CLK_HSIO,		"hsio_root",		0x1e80,	LOW_SPEED_IO_SEL, CLK_IS_CRITICAL},
	{ IMX93_CLK_HSIO_USB_TEST_60M,	"hsio_usb_test_60m_root", 0x1f00, LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_HSIO_ACSCAN_80M,	"hsio_acscan_80m_root",	0x1f80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_HSIO_ACSCAN_480M,	"hsio_acscan_480m_root", 0x2000, MISC_SEL, },
	{ IMX93_CLK_NIC_AXI,		"nic_axi_root",		0x2080, FAST_SEL, CLK_IS_CRITICAL, },
	{ IMX93_CLK_ML_APB,		"ml_apb_root",		0x2180,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ML,			"ml_root",		0x2200,	FAST_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_MEDIA_AXI,		"media_axi_root",	0x2280,	FAST_SEL, },
	{ IMX93_CLK_MEDIA_APB,		"media_apb_root",	0x2300,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_MEDIA_LDB,		"media_ldb_root",	0x2380,	VIDEO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_MEDIA_DISP_PIX,	"media_disp_pix_root",	0x2400,	VIDEO_SEL, },
	{ IMX93_CLK_CAM_PIX,		"cam_pix_root",		0x2480,	VIDEO_SEL, },
	{ IMX93_CLK_MIPI_TEST_BYTE,	"mipi_test_byte_root",	0x2500,	VIDEO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_MIPI_PHY_CFG,	"mipi_phy_cfg_root",	0x2580,	VIDEO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ADC,		"adc_root",		0x2700,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_PDM,		"pdm_root",		0x2780,	AUDIO_SEL, },
	{ IMX93_CLK_TSTMR1,		"tstmr1_root",		0x2800,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_TSTMR2,		"tstmr2_root",		0x2880,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_MQS1,		"mqs1_root",		0x2900,	AUDIO_SEL, },
	{ IMX93_CLK_MQS2,		"mqs2_root",		0x2980,	AUDIO_SEL, },
	{ IMX93_CLK_AUDIO_XCVR,		"audio_xcvr_root",	0x2a00,	NON_IO_SEL, },
	{ IMX93_CLK_SPDIF,		"spdif_root",		0x2a80,	AUDIO_SEL, },
	{ IMX93_CLK_ENET,		"enet_root",		0x2b00,	NON_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ENET_TIMER1,	"enet_timer1_root",	0x2b80,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ENET_TIMER2,	"enet_timer2_root",	0x2c00,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ENET_REF,		"enet_ref_root",	0x2c80,	NON_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_ENET_REF_PHY,	"enet_ref_phy_root",	0x2d00,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX91_CLK_ENET1_QOS_TSN,	"enet1_qos_tsn_root",   0x2b00, NON_IO_SEL, 0, PLAT_IMX91, },
	{ IMX91_CLK_ENET_TIMER,		"enet_timer_root",      0x2b80, LOW_SPEED_IO_SEL, 0, PLAT_IMX91, },
	{ IMX91_CLK_ENET2_REGULAR,	"enet2_regular_root",   0x2c80, NON_IO_SEL, 0, PLAT_IMX91, },
	{ IMX93_CLK_I3C1_SLOW,		"i3c1_slow_root",	0x2d80,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_I3C2_SLOW,		"i3c2_slow_root",	0x2e00,	LOW_SPEED_IO_SEL, 0, PLAT_IMX93, },
	{ IMX93_CLK_USB_PHY_BURUNIN,	"usb_phy_root",		0x2e80,	LOW_SPEED_IO_SEL, },
	{ IMX93_CLK_PAL_CAME_SCAN,	"pal_came_scan_root",	0x2f00,	MISC_SEL, }
};

static const struct imx93_clk_ccgr {
	u32 clk;
	char *name;
	char *parent_name;
	u32 off;
	unsigned long flags;
	u32 *shared_count;
	unsigned long plat;
} ccgr_array[] = {
	{ IMX93_CLK_A55_GATE,		"a55_alt",	"a55_alt_root",		0x8000, },
	/* M33 critical clk for system run */
	{ IMX93_CLK_CM33_GATE,		"cm33",		"m33_root",		0x8040, CLK_IS_CRITICAL },
	{ IMX93_CLK_ADC1_GATE,		"adc1",		"adc_root",		0x82c0, },
	{ IMX93_CLK_WDOG1_GATE,		"wdog1",	"clock-osc-24m",	0x8300, },
	{ IMX93_CLK_WDOG2_GATE,		"wdog2",	"clock-osc-24m",	0x8340, },
	{ IMX93_CLK_WDOG3_GATE,		"wdog3",	"clock-osc-24m",	0x8380, },
	{ IMX93_CLK_WDOG4_GATE,		"wdog4",	"clock-osc-24m",	0x83c0, },
	{ IMX93_CLK_WDOG5_GATE,		"wdog5",	"clock-osc-24m",	0x8400, },
	{ IMX93_CLK_SEMA1_GATE,		"sema1",	"bus_aon_root",		0x8440, },
	{ IMX93_CLK_SEMA2_GATE,		"sema2",	"bus_wakeup_root",	0x8480, },
	{ IMX93_CLK_MU1_A_GATE,		"mu1_a",	"bus_aon_root",		0x84c0, CLK_IGNORE_UNUSED },
	{ IMX93_CLK_MU2_A_GATE,		"mu2_a",	"bus_wakeup_root",	0x84c0, CLK_IGNORE_UNUSED },
	{ IMX93_CLK_MU1_B_GATE,		"mu1_b",	"bus_aon_root",		0x8500, 0, &share_count_mub },
	{ IMX93_CLK_MU2_B_GATE,		"mu2_b",	"bus_wakeup_root",	0x8500, 0, &share_count_mub },
	{ IMX93_CLK_EDMA1_GATE,		"edma1",	"m33_root",		0x8540, },
	{ IMX93_CLK_EDMA2_GATE,		"edma2",	"wakeup_axi_root",	0x8580, },
	{ IMX93_CLK_FLEXSPI1_GATE,	"flexspi1",	"flexspi1_root",	0x8640, },
	{ IMX93_CLK_GPIO1_GATE,		"gpio1",	"m33_root",		0x8880, },
	{ IMX93_CLK_GPIO2_GATE,		"gpio2",	"bus_wakeup_root",	0x88c0, },
	{ IMX93_CLK_GPIO3_GATE,		"gpio3",	"bus_wakeup_root",	0x8900, },
	{ IMX93_CLK_GPIO4_GATE,		"gpio4",	"bus_wakeup_root",	0x8940, },
	{ IMX93_CLK_FLEXIO1_GATE,	"flexio1",	"flexio1_root",		0x8980, },
	{ IMX93_CLK_FLEXIO2_GATE,	"flexio2",	"flexio2_root",		0x89c0, },
	{ IMX93_CLK_LPIT1_GATE,		"lpit1",	"bus_aon_root",		0x8a00, },
	{ IMX93_CLK_LPIT2_GATE,		"lpit2",	"bus_wakeup_root",	0x8a40, },
	{ IMX93_CLK_LPTMR1_GATE,	"lptmr1",	"lptmr1_root",		0x8a80, },
	{ IMX93_CLK_LPTMR2_GATE,	"lptmr2",	"lptmr2_root",		0x8ac0, },
	{ IMX93_CLK_TPM1_GATE,		"tpm1",		"bus_aon_root",		0x8b00, },
	{ IMX93_CLK_TPM2_GATE,		"tpm2",		"tpm2_root",		0x8b40, },
	{ IMX93_CLK_TPM3_GATE,		"tpm3",		"bus_wakeup_root",	0x8b80, },
	{ IMX93_CLK_TPM4_GATE,		"tpm4",		"tpm4_root",		0x8bc0, },
	{ IMX93_CLK_TPM5_GATE,		"tpm5",		"tpm5_root",		0x8c00, },
	{ IMX93_CLK_TPM6_GATE,		"tpm6",		"tpm6_root",		0x8c40, },
	{ IMX93_CLK_CAN1_GATE,		"can1",		"can1_root",		0x8c80, },
	{ IMX93_CLK_CAN2_GATE,		"can2",		"can2_root",		0x8cc0, },
	{ IMX93_CLK_LPUART1_GATE,	"lpuart1",	"lpuart1_root",		0x8d00, },
	{ IMX93_CLK_LPUART2_GATE,	"lpuart2",	"lpuart2_root",		0x8d40, },
	{ IMX93_CLK_LPUART3_GATE,	"lpuart3",	"lpuart3_root",		0x8d80, },
	{ IMX93_CLK_LPUART4_GATE,	"lpuart4",	"lpuart4_root",		0x8dc0, },
	{ IMX93_CLK_LPUART5_GATE,	"lpuart5",	"lpuart5_root",		0x8e00, },
	{ IMX93_CLK_LPUART6_GATE,	"lpuart6",	"lpuart6_root",		0x8e40, },
	{ IMX93_CLK_LPUART7_GATE,	"lpuart7",	"lpuart7_root",		0x8e80, },
	{ IMX93_CLK_LPUART8_GATE,	"lpuart8",	"lpuart8_root",		0x8ec0, },
	{ IMX93_CLK_LPI2C1_GATE,	"lpi2c1",	"lpi2c1_root",		0x8f00, },
	{ IMX93_CLK_LPI2C2_GATE,	"lpi2c2",	"lpi2c2_root",		0x8f40, },
	{ IMX93_CLK_LPI2C3_GATE,	"lpi2c3",	"lpi2c3_root",		0x8f80, },
	{ IMX93_CLK_LPI2C4_GATE,	"lpi2c4",	"lpi2c4_root",		0x8fc0, },
	{ IMX93_CLK_LPI2C5_GATE,	"lpi2c5",	"lpi2c5_root",		0x9000, },
	{ IMX93_CLK_LPI2C6_GATE,	"lpi2c6",	"lpi2c6_root",		0x9040, },
	{ IMX93_CLK_LPI2C7_GATE,	"lpi2c7",	"lpi2c7_root",		0x9080, },
	{ IMX93_CLK_LPI2C8_GATE,	"lpi2c8",	"lpi2c8_root",		0x90c0, },
	{ IMX93_CLK_LPSPI1_GATE,	"lpspi1",	"lpspi1_root",		0x9100, },
	{ IMX93_CLK_LPSPI2_GATE,	"lpspi2",	"lpspi2_root",		0x9140, },
	{ IMX93_CLK_LPSPI3_GATE,	"lpspi3",	"lpspi3_root",		0x9180, },
	{ IMX93_CLK_LPSPI4_GATE,	"lpspi4",	"lpspi4_root",		0x91c0, },
	{ IMX93_CLK_LPSPI5_GATE,	"lpspi5",	"lpspi5_root",		0x9200, },
	{ IMX93_CLK_LPSPI6_GATE,	"lpspi6",	"lpspi6_root",		0x9240, },
	{ IMX93_CLK_LPSPI7_GATE,	"lpspi7",	"lpspi7_root",		0x9280, },
	{ IMX93_CLK_LPSPI8_GATE,	"lpspi8",	"lpspi8_root",		0x92c0, },
	{ IMX93_CLK_I3C1_GATE,		"i3c1",		"i3c1_root",		0x9300, },
	{ IMX93_CLK_I3C2_GATE,		"i3c2",		"i3c2_root",		0x9340, },
	{ IMX93_CLK_USDHC1_GATE,	"usdhc1",	"usdhc1_root",		0x9380, },
	{ IMX93_CLK_USDHC2_GATE,	"usdhc2",	"usdhc2_root",		0x93c0, },
	{ IMX93_CLK_USDHC3_GATE,	"usdhc3",	"usdhc3_root",		0x9400, },
	{ IMX93_CLK_SAI1_GATE,          "sai1",         "sai1_root",            0x9440, 0, &share_count_sai1},
	{ IMX93_CLK_SAI1_IPG,		"sai1_ipg_clk", "bus_aon_root",		0x9440, 0, &share_count_sai1},
	{ IMX93_CLK_SAI2_GATE,          "sai2",         "sai2_root",            0x9480, 0, &share_count_sai2},
	{ IMX93_CLK_SAI2_IPG,		"sai2_ipg_clk", "bus_wakeup_root",	0x9480, 0, &share_count_sai2},
	{ IMX93_CLK_SAI3_GATE,          "sai3",         "sai3_root",            0x94c0, 0, &share_count_sai3},
	{ IMX93_CLK_SAI3_IPG,		"sai3_ipg_clk", "bus_wakeup_root",	0x94c0, 0, &share_count_sai3},
	{ IMX93_CLK_MIPI_CSI_GATE,	"mipi_csi",	"media_apb_root",	0x9580, },
	{ IMX93_CLK_MIPI_DSI_GATE,	"mipi_dsi",	"media_apb_root",	0x95c0, },
	{ IMX93_CLK_LVDS_GATE,		"lvds",		"media_ldb_root",	0x9600, 0, NULL, PLAT_IMX93, },
	{ IMX93_CLK_LCDIF_GATE,		"lcdif",	"media_apb_root",	0x9640, },
	{ IMX93_CLK_PXP_GATE,		"pxp",		"media_apb_root",	0x9680, },
	{ IMX93_CLK_ISI_GATE,		"isi",		"media_apb_root",	0x96c0, },
	{ IMX93_CLK_NIC_MEDIA_GATE,	"nic_media",	"media_axi_root",	0x9700, },
	{ IMX93_CLK_USB_CONTROLLER_GATE, "usb_controller", "hsio_root",		0x9a00, },
	{ IMX93_CLK_USB_TEST_60M_GATE,	"usb_test_60m",	"hsio_usb_test_60m_root", 0x9a40, },
	{ IMX93_CLK_HSIO_TROUT_24M_GATE, "hsio_trout_24m", "clock-osc-24m",	0x9a80, },
	{ IMX93_CLK_PDM_GATE,		"pdm",		"pdm_root",		0x9ac0, },
	{ IMX93_CLK_MQS1_GATE,		"mqs1",		"sai1_root",		0x9b00, },
	{ IMX93_CLK_MQS2_GATE,		"mqs2",		"sai3_root",		0x9b40, },
	{ IMX93_CLK_AUD_XCVR_GATE,	"aud_xcvr",	"audio_xcvr_root",	0x9b80, },
	{ IMX93_CLK_SPDIF_GATE,		"spdif",	"spdif_root",		0x9c00, },
	{ IMX93_CLK_HSIO_32K_GATE,	"hsio_32k",	"clock-osc-24m",	0x9dc0, },
	{ IMX93_CLK_ENET1_GATE,		"enet1",	"wakeup_axi_root",	0x9e00, 0, NULL, PLAT_IMX93, },
	{ IMX93_CLK_ENET_QOS_GATE,	"enet_qos",	"wakeup_axi_root",	0x9e40, 0, NULL, PLAT_IMX93, },
	{ IMX91_CLK_ENET2_REGULAR_GATE, "enet2_regular",        "wakeup_axi_root",      0x9e00, 0, NULL, PLAT_IMX91, },
	{ IMX91_CLK_ENET1_QOS_TSN_GATE,     "enet1_qos_tsn",        "wakeup_axi_root",      0x9e40, 0, NULL, PLAT_IMX91, },
	/* Critical because clk accessed during CPU idle */
	{ IMX93_CLK_SYS_CNT_GATE,	"sys_cnt",	"clock-osc-24m",	0x9e80, CLK_IS_CRITICAL},
	{ IMX93_CLK_TSTMR1_GATE,	"tstmr1",	"bus_aon_root",		0x9ec0, },
	{ IMX93_CLK_TSTMR2_GATE,	"tstmr2",	"bus_wakeup_root",	0x9f00, },
	{ IMX93_CLK_TMC_GATE,		"tmc",		"clock-osc-24m",	0x9f40, },
	{ IMX93_CLK_PMRO_GATE,		"pmro",		"clock-osc-24m",	0x9f80, }
};

static int imx93_clk_probe(struct udevice *dev)
{
	const struct imx93_clk_root *root;
	const struct imx93_clk_ccgr *ccgr;
	struct clk osc_24m_clk, osc_32k_clk, ext1_clk;
	void __iomem *base, *anatop_base;
	int i, ret;
	const unsigned long plat = (unsigned long)dev_get_driver_data(dev);

	clk_dm(IMX93_CLK_DUMMY, clk_register_fixed_rate(NULL, "dummy", 0UL));

	ret = clk_get_by_name(dev, "osc_24m", &osc_24m_clk);
	if (ret)
		return ret;
	clk_dm(IMX93_CLK_24M, dev_get_clk_ptr(osc_24m_clk.dev));

	ret = clk_get_by_name(dev, "osc_32k", &osc_32k_clk);
	if (ret)
		return ret;
	clk_dm(IMX93_CLK_32K, dev_get_clk_ptr(osc_32k_clk.dev));

	ret = clk_get_by_name(dev, "clk_ext1", &ext1_clk);
	if (ret)
		return ret;
	clk_dm(IMX93_CLK_EXT1, dev_get_clk_ptr(ext1_clk.dev));

	clk_dm(IMX93_CLK_SYS_PLL_PFD0,
	       clk_register_fixed_rate(NULL, "sys_pll_pfd0", 1000000000));
	clk_dm(IMX93_CLK_SYS_PLL_PFD0_DIV2,
	       imx_clk_fixed_factor(dev, "sys_pll_pfd0_div2", "sys_pll_pfd0", 1, 2));
	clk_dm(IMX93_CLK_SYS_PLL_PFD1,
	       clk_register_fixed_rate(NULL, "sys_pll_pfd1", 800000000));
	clk_dm(IMX93_CLK_SYS_PLL_PFD1_DIV2,
	       imx_clk_fixed_factor(dev, "sys_pll_pfd1_div2", "sys_pll_pfd1", 1, 2));
	clk_dm(IMX93_CLK_SYS_PLL_PFD2,
	       clk_register_fixed_rate(NULL, "sys_pll_pfd2", 625000000));
	clk_dm(IMX93_CLK_SYS_PLL_PFD2_DIV2,
	       imx_clk_fixed_factor(dev, "sys_pll_pfd2_div2", "sys_pll_pfd2", 1, 2));

	anatop_base = (void *)ANATOP_BASE_ADDR;

	clk_dm(IMX93_CLK_ARM_PLL,
	       imx_clk_fracn_gppll_integer("arm_pll", "clock-osc-24m",
					   anatop_base + 0x1000,
					   &imx_fracn_gppll_integer));
	clk_dm(IMX93_CLK_AUDIO_PLL,
	       imx_clk_fracn_gppll("audio_pll", "clock-osc-24m",
				   anatop_base + 0x1200, &imx_fracn_gppll));
	clk_dm(IMX93_CLK_VIDEO_PLL,
	       imx_clk_fracn_gppll("video_pll", "clock-osc-24m",
				   anatop_base + 0x1400, &imx_fracn_gppll));

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(root_array); i++) {
		root = &root_array[i];
		if (root->plat && !(root->plat & plat))
			continue;
		clk_dm(root->clk, imx93_clk_composite_flags(root->name,
							    parent_names[root->sel],
							    4, base + root->off, 3,
							    root->flags));
	}

	for (i = 0; i < ARRAY_SIZE(ccgr_array); i++) {
		ccgr = &ccgr_array[i];
		if (ccgr->plat && !(ccgr->plat & plat))
			continue;
		clk_dm(ccgr->clk, imx93_clk_gate(NULL, ccgr->name, ccgr->parent_name,
						 ccgr->flags, base + ccgr->off, 0, 1, 1, 3,
						 ccgr->shared_count));
	}

	clk_dm(IMX93_CLK_A55_SEL,
	       imx_clk_mux2(dev, "a55_sel", base + 0x4820, 0, 1,
			    a55_core_sels, ARRAY_SIZE(a55_core_sels)));

	return 0;
}

static const struct udevice_id imx93_clk_ids[] = {
	{ .compatible = "fsl,imx93-ccm", .data = (unsigned long)PLAT_IMX93 },
	{ .compatible = "fsl,imx91-ccm", .data = (unsigned long)PLAT_IMX91 },
	{ /* Sentinel */ },
};

U_BOOT_DRIVER(imx93_clk) = {
	.name = "clk_imx93",
	.id = UCLASS_CLK,
	.of_match = imx93_clk_ids,
	.ops = &ccf_clk_ops,
	.probe = imx93_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
