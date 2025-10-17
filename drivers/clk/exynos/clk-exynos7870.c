// SPDX-License-Identifier: GPL-2.0-only
/*
 * Samsung Exynos7870 clock driver.
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
 * Author: Kaustabh Chakraborty <kauschluss@disroot.org>
 */

#include "linux/clk-provider.h"
#include <dm.h>
#include <asm/io.h>
#include <dt-bindings/clock/samsung,exynos7870-cmu.h>
#include "clk.h"

enum exynos7870_cmu_id {
	CMU_MIF,
	CMU_FSYS,
	CMU_PERI,
};

/*
 * Register offsets for CMU_MIF (0x10460000)
 */
#define PLL_LOCKTIME_MIF_MEM_PLL					0x0000
#define PLL_LOCKTIME_MIF_MEDIA_PLL					0x0020
#define PLL_LOCKTIME_MIF_BUS_PLL					0x0040
#define PLL_CON0_MIF_MEM_PLL						0x0100
#define PLL_CON0_MIF_MEDIA_PLL						0x0120
#define PLL_CON0_MIF_BUS_PLL						0x0140
#define CLK_CON_GAT_MIF_MUX_MEM_PLL					0x0200
#define CLK_CON_GAT_MIF_MUX_MEM_PLL_CON					0x0200
#define CLK_CON_GAT_MIF_MUX_MEDIA_PLL					0x0204
#define CLK_CON_GAT_MIF_MUX_MEDIA_PLL_CON				0x0204
#define CLK_CON_GAT_MIF_MUX_BUS_PLL					0x0208
#define CLK_CON_GAT_MIF_MUX_BUS_PLL_CON					0x0208
#define CLK_CON_GAT_MIF_MUX_BUSD					0x0220
#define CLK_CON_MUX_MIF_BUSD						0x0220
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_VRA					0x0264
#define CLK_CON_MUX_MIF_CMU_ISP_VRA					0x0264
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_CAM					0x0268
#define CLK_CON_MUX_MIF_CMU_ISP_CAM					0x0268
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_ISP					0x026c
#define CLK_CON_MUX_MIF_CMU_ISP_ISP					0x026c
#define CLK_CON_GAT_MIF_MUX_CMU_DISPAUD_BUS				0x0270
#define CLK_CON_MUX_MIF_CMU_DISPAUD_BUS					0x0270
#define CLK_CON_GAT_MIF_MUX_CMU_DISPAUD_DECON_VCLK			0x0274
#define CLK_CON_MUX_MIF_CMU_DISPAUD_DECON_VCLK				0x0274
#define CLK_CON_GAT_MIF_MUX_CMU_DISPAUD_DECON_ECLK			0x0278
#define CLK_CON_MUX_MIF_CMU_DISPAUD_DECON_ECLK				0x0278
#define CLK_CON_GAT_MIF_MUX_CMU_MFCMSCL_MSCL				0x027c
#define CLK_CON_MUX_MIF_CMU_MFCMSCL_MSCL				0x027c
#define CLK_CON_GAT_MIF_MUX_CMU_MFCMSCL_MFC				0x0280
#define CLK_CON_MUX_MIF_CMU_MFCMSCL_MFC					0x0280
#define CLK_CON_GAT_MIF_MUX_CMU_FSYS_BUS				0x0284
#define CLK_CON_MUX_MIF_CMU_FSYS_BUS					0x0284
#define CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC0				0x0288
#define CLK_CON_MUX_MIF_CMU_FSYS_MMC0					0x0288
#define CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC1				0x028c
#define CLK_CON_MUX_MIF_CMU_FSYS_MMC1					0x028c
#define CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC2				0x0290
#define CLK_CON_MUX_MIF_CMU_FSYS_MMC2					0x0290
#define CLK_CON_GAT_MIF_MUX_CMU_FSYS_USB20DRD_REFCLK			0x029c
#define CLK_CON_MUX_MIF_CMU_FSYS_USB20DRD_REFCLK			0x029c
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_BUS				0x02a0
#define CLK_CON_MUX_MIF_CMU_PERI_BUS					0x02a0
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_UART1				0x02a4
#define CLK_CON_MUX_MIF_CMU_PERI_UART1					0x02a4
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_UART2				0x02a8
#define CLK_CON_MUX_MIF_CMU_PERI_UART2					0x02a8
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_UART0				0x02ac
#define CLK_CON_MUX_MIF_CMU_PERI_UART0					0x02ac
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI2				0x02b0
#define CLK_CON_MUX_MIF_CMU_PERI_SPI2					0x02b0
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI1				0x02b4
#define CLK_CON_MUX_MIF_CMU_PERI_SPI1					0x02b4
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI0				0x02b8
#define CLK_CON_MUX_MIF_CMU_PERI_SPI0					0x02b8
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI3				0x02bc
#define CLK_CON_MUX_MIF_CMU_PERI_SPI3					0x02bc
#define CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI4				0x02c0
#define CLK_CON_MUX_MIF_CMU_PERI_SPI4					0x02c0
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_SENSOR0				0x02c4
#define CLK_CON_MUX_MIF_CMU_ISP_SENSOR0					0x02c4
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_SENSOR1				0x02c8
#define CLK_CON_MUX_MIF_CMU_ISP_SENSOR1					0x02c8
#define CLK_CON_GAT_MIF_MUX_CMU_ISP_SENSOR2				0x02cc
#define CLK_CON_MUX_MIF_CMU_ISP_SENSOR2					0x02cc
#define CLK_CON_DIV_MIF_BUSD						0x0420
#define CLK_CON_DIV_MIF_APB						0x0424
#define CLK_CON_DIV_MIF_HSI2C						0x0430
#define CLK_CON_DIV_MIF_CMU_G3D_SWITCH					0x0460
#define CLK_CON_DIV_MIF_CMU_ISP_VRA					0x0464
#define CLK_CON_DIV_MIF_CMU_ISP_CAM					0x0468
#define CLK_CON_DIV_MIF_CMU_ISP_ISP					0x046c
#define CLK_CON_DIV_MIF_CMU_DISPAUD_BUS					0x0470
#define CLK_CON_DIV_MIF_CMU_DISPAUD_DECON_VCLK				0x0474
#define CLK_CON_DIV_MIF_CMU_DISPAUD_DECON_ECLK				0x0478
#define CLK_CON_DIV_MIF_CMU_MFCMSCL_MSCL				0x047c
#define CLK_CON_DIV_MIF_CMU_MFCMSCL_MFC					0x0480
#define CLK_CON_DIV_MIF_CMU_FSYS_BUS					0x0484
#define CLK_CON_DIV_MIF_CMU_FSYS_MMC0					0x0488
#define CLK_CON_DIV_MIF_CMU_FSYS_MMC1					0x048c
#define CLK_CON_DIV_MIF_CMU_FSYS_MMC2					0x0490
#define CLK_CON_DIV_MIF_CMU_FSYS_USB20DRD_REFCLK			0x049c
#define CLK_CON_DIV_MIF_CMU_PERI_BUS					0x04a0
#define CLK_CON_DIV_MIF_CMU_PERI_UART1					0x04a4
#define CLK_CON_DIV_MIF_CMU_PERI_UART2					0x04a8
#define CLK_CON_DIV_MIF_CMU_PERI_UART0					0x04ac
#define CLK_CON_DIV_MIF_CMU_PERI_SPI2					0x04b0
#define CLK_CON_DIV_MIF_CMU_PERI_SPI1					0x04b4
#define CLK_CON_DIV_MIF_CMU_PERI_SPI0					0x04b8
#define CLK_CON_DIV_MIF_CMU_PERI_SPI3					0x04bc
#define CLK_CON_DIV_MIF_CMU_PERI_SPI4					0x04c0
#define CLK_CON_DIV_MIF_CMU_ISP_SENSOR0					0x04c4
#define CLK_CON_DIV_MIF_CMU_ISP_SENSOR1					0x04c8
#define CLK_CON_DIV_MIF_CMU_ISP_SENSOR2					0x04cc
#define CLK_CON_GAT_MIF_WRAP_ADC_IF_OSC_SYS				0x080c
#define CLK_CON_GAT_MIF_HSI2C_AP_PCLKS					0x0828
#define CLK_CON_GAT_MIF_HSI2C_CP_PCLKS					0x0828
#define CLK_CON_GAT_MIF_WRAP_ADC_IF_PCLK_S0				0x0828
#define CLK_CON_GAT_MIF_WRAP_ADC_IF_PCLK_S1				0x0828
#define CLK_CON_GAT_MIF_CP_PCLK_HSI2C					0x0840
#define CLK_CON_GAT_MIF_CP_PCLK_HSI2C_BAT_0				0x0840
#define CLK_CON_GAT_MIF_CP_PCLK_HSI2C_BAT_1				0x0840
#define CLK_CON_GAT_MIF_HSI2C_AP_PCLKM					0x0840
#define CLK_CON_GAT_MIF_HSI2C_CP_PCLKM					0x0840
#define CLK_CON_GAT_MIF_HSI2C_IPCLK					0x0840
#define CLK_CON_GAT_MIF_HSI2C_ITCLK					0x0840
#define CLK_CON_GAT_MIF_CMU_G3D_SWITCH					0x0860
#define CLK_CON_GAT_MIF_CMU_ISP_VRA					0x0864
#define CLK_CON_GAT_MIF_CMU_ISP_CAM					0x0868
#define CLK_CON_GAT_MIF_CMU_ISP_ISP					0x086c
#define CLK_CON_GAT_MIF_CMU_DISPAUD_BUS					0x0870
#define CLK_CON_GAT_MIF_CMU_DISPAUD_DECON_VCLK				0x0874
#define CLK_CON_GAT_MIF_CMU_DISPAUD_DECON_ECLK				0x0878
#define CLK_CON_GAT_MIF_CMU_MFCMSCL_MSCL				0x087c
#define CLK_CON_GAT_MIF_CMU_MFCMSCL_MFC					0x0880
#define CLK_CON_GAT_MIF_CMU_FSYS_BUS					0x0884
#define CLK_CON_GAT_MIF_CMU_FSYS_MMC0					0x0888
#define CLK_CON_GAT_MIF_CMU_FSYS_MMC1					0x088c
#define CLK_CON_GAT_MIF_CMU_FSYS_MMC2					0x0890
#define CLK_CON_GAT_MIF_CMU_FSYS_USB20DRD_REFCLK			0x089c
#define CLK_CON_GAT_MIF_CMU_PERI_BUS					0x08a0
#define CLK_CON_GAT_MIF_CMU_PERI_UART1					0x08a4
#define CLK_CON_GAT_MIF_CMU_PERI_UART2					0x08a8
#define CLK_CON_GAT_MIF_CMU_PERI_UART0					0x08ac
#define CLK_CON_GAT_MIF_CMU_PERI_SPI2					0x08b0
#define CLK_CON_GAT_MIF_CMU_PERI_SPI1					0x08b4
#define CLK_CON_GAT_MIF_CMU_PERI_SPI0					0x08b8
#define CLK_CON_GAT_MIF_CMU_PERI_SPI3					0x08bc
#define CLK_CON_GAT_MIF_CMU_PERI_SPI4					0x08c0
#define CLK_CON_GAT_MIF_CMU_ISP_SENSOR0					0x08c4
#define CLK_CON_GAT_MIF_CMU_ISP_SENSOR1					0x08c8
#define CLK_CON_GAT_MIF_CMU_ISP_SENSOR2					0x08cc

static const struct samsung_pll_clock mif_pll_clks[] = {
	PLL(pll_1417x, CLK_FOUT_MIF_BUS_PLL, "fout_mif_bus_pll", "oscclk",
	    PLL_CON0_MIF_BUS_PLL),
	PLL(pll_1417x, CLK_FOUT_MIF_MEDIA_PLL, "fout_mif_media_pll", "oscclk",
	    PLL_CON0_MIF_MEDIA_PLL),
	PLL(pll_1417x, CLK_FOUT_MIF_MEM_PLL, "fout_mif_mem_pll", "oscclk",
	    PLL_CON0_MIF_MEM_PLL),
};

static const struct samsung_gate_clock mif_pll_gate_clks[] = {
	GATE(CLK_GOUT_MIF_MUX_BUS_PLL_CON,
	     "gout_mif_mux_bus_pll_con", "fout_mif_bus_pll",
	     CLK_CON_GAT_MIF_MUX_BUS_PLL_CON, 12,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_MEDIA_PLL_CON,
	     "gout_mif_mux_media_pll_con", "fout_mif_media_pll",
	     CLK_CON_GAT_MIF_MUX_MEDIA_PLL_CON, 12,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_MEM_PLL_CON,
	     "gout_mif_mux_mem_pll_con", "fout_mif_mem_pll",
	     CLK_CON_GAT_MIF_MUX_MEM_PLL_CON, 12,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_BUS_PLL,
	     "gout_mif_mux_bus_pll", "gout_mif_mux_bus_pll_con",
	     CLK_CON_GAT_MIF_MUX_BUS_PLL, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_MEM_PLL,
	     "gout_mif_mux_mem_pll", "gout_mif_mux_mem_pll_con",
	     CLK_CON_GAT_MIF_MUX_MEM_PLL, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_MEDIA_PLL,
	     "gout_mif_mux_media_pll", "gout_mif_mux_media_pll_con",
	     CLK_CON_GAT_MIF_MUX_MEDIA_PLL, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
};

static const struct samsung_fixed_factor_clock mif_fixed_factor_clks[] = {
	FFACTOR(0, "ffac_mif_mux_bus_pll_div2", "gout_mif_mux_bus_pll_con",
		1, 2, 0),
	FFACTOR(0, "ffac_mif_mux_media_pll_div2", "gout_mif_mux_media_pll_con",
		1, 2, 0),
	FFACTOR(0, "ffac_mif_mux_mem_pll_div2", "gout_mif_mux_mem_pll_con",
		1, 2, 0),
};

/* List of parent clocks for muxes in CMU_MIF */
PNAME(mout_mif_busd_p)				= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2",
						    "ffac_mif_mux_mem_pll_div2" };
PNAME(mout_mif_cmu_fsys_bus_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_fsys_mmc0_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_fsys_mmc1_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_fsys_mmc2_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_fsys_usb20drd_refclk_p)	= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_peri_bus_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_peri_spi0_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "oscclk" };
PNAME(mout_mif_cmu_peri_spi1_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "oscclk" };
PNAME(mout_mif_cmu_peri_spi2_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "oscclk" };
PNAME(mout_mif_cmu_peri_spi3_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "oscclk" };
PNAME(mout_mif_cmu_peri_spi4_p)			= { "ffac_mif_mux_bus_pll_div2",
						    "oscclk" };
PNAME(mout_mif_cmu_peri_uart2_p)		= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_peri_uart0_p)		= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };
PNAME(mout_mif_cmu_peri_uart1_p)		= { "ffac_mif_mux_bus_pll_div2",
						    "ffac_mif_mux_media_pll_div2" };

static const struct samsung_mux_clock mif_mux_clks[] = {
	MUX(CLK_MOUT_MIF_BUSD,
	    "mout_mif_busd", mout_mif_busd_p,
	    CLK_CON_MUX_MIF_BUSD, 12, 2),
	MUX(CLK_MOUT_MIF_CMU_FSYS_BUS,
	    "mout_mif_cmu_fsys_bus", mout_mif_cmu_fsys_bus_p,
	    CLK_CON_MUX_MIF_CMU_FSYS_BUS, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_FSYS_MMC0,
	    "mout_mif_cmu_fsys_mmc0", mout_mif_cmu_fsys_mmc0_p,
	    CLK_CON_MUX_MIF_CMU_FSYS_MMC0, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_FSYS_MMC1,
	    "mout_mif_cmu_fsys_mmc1", mout_mif_cmu_fsys_mmc1_p,
	    CLK_CON_MUX_MIF_CMU_FSYS_MMC1, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_FSYS_MMC2,
	    "mout_mif_cmu_fsys_mmc2", mout_mif_cmu_fsys_mmc2_p,
	    CLK_CON_MUX_MIF_CMU_FSYS_MMC2, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_FSYS_USB20DRD_REFCLK,
	    "mout_mif_cmu_fsys_usb20drd_refclk", mout_mif_cmu_fsys_usb20drd_refclk_p,
	    CLK_CON_MUX_MIF_CMU_FSYS_USB20DRD_REFCLK, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_BUS,
	    "mout_mif_cmu_peri_bus", mout_mif_cmu_peri_bus_p,
	    CLK_CON_MUX_MIF_CMU_PERI_BUS, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_SPI0,
	    "mout_mif_cmu_peri_spi0", mout_mif_cmu_peri_spi0_p,
	    CLK_CON_MUX_MIF_CMU_PERI_SPI0, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_SPI1,
	    "mout_mif_cmu_peri_spi1", mout_mif_cmu_peri_spi1_p,
	    CLK_CON_MUX_MIF_CMU_PERI_SPI1, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_SPI2,
	    "mout_mif_cmu_peri_spi2", mout_mif_cmu_peri_spi2_p,
	    CLK_CON_MUX_MIF_CMU_PERI_SPI2, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_SPI3,
	    "mout_mif_cmu_peri_spi3", mout_mif_cmu_peri_spi3_p,
	    CLK_CON_MUX_MIF_CMU_PERI_SPI3, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_SPI4,
	    "mout_mif_cmu_peri_spi4", mout_mif_cmu_peri_spi4_p,
	    CLK_CON_MUX_MIF_CMU_PERI_SPI4, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_UART0,
	    "mout_mif_cmu_peri_uart0", mout_mif_cmu_peri_uart0_p,
	    CLK_CON_MUX_MIF_CMU_PERI_UART0, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_UART1,
	    "mout_mif_cmu_peri_uart1", mout_mif_cmu_peri_uart1_p,
	    CLK_CON_MUX_MIF_CMU_PERI_UART1, 12, 1),
	MUX(CLK_MOUT_MIF_CMU_PERI_UART2,
	    "mout_mif_cmu_peri_uart2", mout_mif_cmu_peri_uart2_p,
	    CLK_CON_MUX_MIF_CMU_PERI_UART2, 12, 1),
};

static const struct samsung_gate_clock mif_mux_gate_clks[] = {
	GATE(CLK_GOUT_MIF_MUX_BUSD,
	     "gout_mif_mux_busd", "mout_mif_busd",
	     CLK_CON_GAT_MIF_MUX_BUSD, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_FSYS_BUS,
	     "gout_mif_mux_cmu_fsys_bus", "mout_mif_cmu_fsys_bus",
	     CLK_CON_GAT_MIF_MUX_CMU_FSYS_BUS, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_FSYS_MMC0,
	     "gout_mif_mux_cmu_fsys_mmc0", "mout_mif_cmu_fsys_mmc0",
	     CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC0, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_FSYS_MMC1,
	     "gout_mif_mux_cmu_fsys_mmc1", "mout_mif_cmu_fsys_mmc1",
	     CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC1, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_FSYS_MMC2,
	     "gout_mif_mux_cmu_fsys_mmc2", "mout_mif_cmu_fsys_mmc2",
	     CLK_CON_GAT_MIF_MUX_CMU_FSYS_MMC2, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_FSYS_USB20DRD_REFCLK,
	     "gout_mif_mux_cmu_fsys_usb20drd_refclk", "mout_mif_cmu_fsys_usb20drd_refclk",
	     CLK_CON_GAT_MIF_MUX_CMU_FSYS_USB20DRD_REFCLK, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_BUS,
	     "gout_mif_mux_cmu_peri_bus", "mout_mif_cmu_peri_bus",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_BUS, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_SPI0,
	     "gout_mif_mux_cmu_peri_spi0", "mout_mif_cmu_peri_spi0",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI0, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_SPI1,
	     "gout_mif_mux_cmu_peri_spi1", "mout_mif_cmu_peri_spi1",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI1, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_SPI2,
	     "gout_mif_mux_cmu_peri_spi2", "mout_mif_cmu_peri_spi2",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI2, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_SPI3,
	     "gout_mif_mux_cmu_peri_spi3", "mout_mif_cmu_peri_spi3",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI3, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_SPI4,
	     "gout_mif_mux_cmu_peri_spi4", "mout_mif_cmu_peri_spi4",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_SPI4, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_UART0,
	     "gout_mif_mux_cmu_peri_uart0", "mout_mif_cmu_peri_uart0",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_UART0, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_UART1,
	     "gout_mif_mux_cmu_peri_uart1", "mout_mif_cmu_peri_uart1",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_UART1, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_MUX_CMU_PERI_UART2,
	     "gout_mif_mux_cmu_peri_uart2", "mout_mif_cmu_peri_uart2",
	     CLK_CON_GAT_MIF_MUX_CMU_PERI_UART2, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
};

static const struct samsung_div_clock mif_div_clks[] = {
	DIV(CLK_DOUT_MIF_HSI2C,
	    "dout_mif_hsi2c", "ffac_mif_mux_media_pll_div2",
	    CLK_CON_DIV_MIF_HSI2C, 0, 4),
	DIV(CLK_DOUT_MIF_BUSD,
	    "dout_mif_busd", "gout_mif_mux_busd",
	    CLK_CON_DIV_MIF_BUSD, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_FSYS_BUS,
	    "dout_mif_cmu_fsys_bus", "gout_mif_mux_cmu_fsys_bus",
	    CLK_CON_DIV_MIF_CMU_FSYS_BUS, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_FSYS_MMC0,
	    "dout_mif_cmu_fsys_mmc0", "gout_mif_mux_cmu_fsys_mmc0",
	    CLK_CON_DIV_MIF_CMU_FSYS_MMC0, 0, 10),
	DIV(CLK_DOUT_MIF_CMU_FSYS_MMC1,
	    "dout_mif_cmu_fsys_mmc1", "gout_mif_mux_cmu_fsys_mmc1",
	    CLK_CON_DIV_MIF_CMU_FSYS_MMC1, 0, 10),
	DIV(CLK_DOUT_MIF_CMU_FSYS_MMC2,
	    "dout_mif_cmu_fsys_mmc2", "gout_mif_mux_cmu_fsys_mmc2",
	    CLK_CON_DIV_MIF_CMU_FSYS_MMC2, 0, 10),
	DIV(CLK_DOUT_MIF_CMU_FSYS_USB20DRD_REFCLK,
	    "dout_mif_cmu_fsys_usb20drd_refclk", "gout_mif_mux_cmu_fsys_usb20drd_refclk",
	    CLK_CON_DIV_MIF_CMU_FSYS_USB20DRD_REFCLK, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_PERI_BUS,
	    "dout_mif_cmu_peri_bus", "gout_mif_mux_cmu_peri_bus",
	    CLK_CON_DIV_MIF_CMU_PERI_BUS, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_PERI_SPI0,
	    "dout_mif_cmu_peri_spi0", "gout_mif_mux_cmu_peri_spi0",
	    CLK_CON_DIV_MIF_CMU_PERI_SPI0, 0, 6),
	DIV(CLK_DOUT_MIF_CMU_PERI_SPI1,
	    "dout_mif_cmu_peri_spi1", "gout_mif_mux_cmu_peri_spi1",
	    CLK_CON_DIV_MIF_CMU_PERI_SPI1, 0, 6),
	DIV(CLK_DOUT_MIF_CMU_PERI_SPI2,
	    "dout_mif_cmu_peri_spi2", "gout_mif_mux_cmu_peri_spi2",
	    CLK_CON_DIV_MIF_CMU_PERI_SPI2, 0, 6),
	DIV(CLK_DOUT_MIF_CMU_PERI_SPI3,
	    "dout_mif_cmu_peri_spi3", "gout_mif_mux_cmu_peri_spi3",
	    CLK_CON_DIV_MIF_CMU_PERI_SPI3, 0, 6),
	DIV(CLK_DOUT_MIF_CMU_PERI_SPI4,
	    "dout_mif_cmu_peri_spi4", "gout_mif_mux_cmu_peri_spi4",
	    CLK_CON_DIV_MIF_CMU_PERI_SPI4, 0, 6),
	DIV(CLK_DOUT_MIF_CMU_PERI_UART0,
	    "dout_mif_cmu_peri_uart0", "gout_mif_mux_cmu_peri_uart0",
	    CLK_CON_DIV_MIF_CMU_PERI_UART0, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_PERI_UART1,
	    "dout_mif_cmu_peri_uart1", "gout_mif_mux_cmu_peri_uart1",
	    CLK_CON_DIV_MIF_CMU_PERI_UART1, 0, 4),
	DIV(CLK_DOUT_MIF_CMU_PERI_UART2,
	    "dout_mif_cmu_peri_uart2", "gout_mif_mux_cmu_peri_uart2",
	    CLK_CON_DIV_MIF_CMU_PERI_UART2, 0, 4),
	DIV(CLK_DOUT_MIF_APB,
	    "dout_mif_apb", "dout_mif_busd",
	    CLK_CON_DIV_MIF_APB, 0, 2),
};

static const struct samsung_gate_clock mif_gate_clks[] = {
	GATE(CLK_GOUT_MIF_WRAP_ADC_IF_OSC_SYS,
	     "gout_mif_wrap_adc_if_osc_sys", "oscclk",
	     CLK_CON_GAT_MIF_WRAP_ADC_IF_OSC_SYS, 3,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_AP_PCLKS,
	     "gout_mif_hsi2c_ap_pclks", "dout_mif_apb",
	     CLK_CON_GAT_MIF_HSI2C_AP_PCLKS, 14,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_CP_PCLKS,
	     "gout_mif_hsi2c_cp_pclks", "dout_mif_apb",
	     CLK_CON_GAT_MIF_HSI2C_CP_PCLKS, 15,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_WRAP_ADC_IF_PCLK_S0,
	     "gout_mif_wrap_adc_if_pclk_s0", "dout_mif_apb",
	     CLK_CON_GAT_MIF_WRAP_ADC_IF_PCLK_S0, 20,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_WRAP_ADC_IF_PCLK_S1,
	     "gout_mif_wrap_adc_if_pclk_s1", "dout_mif_apb",
	     CLK_CON_GAT_MIF_WRAP_ADC_IF_PCLK_S1, 21,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_FSYS_BUS,
	     "gout_mif_cmu_fsys_bus", "dout_mif_cmu_fsys_bus",
	     CLK_CON_GAT_MIF_CMU_FSYS_BUS, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_FSYS_MMC0,
	     "gout_mif_cmu_fsys_mmc0", "dout_mif_cmu_fsys_mmc0",
	     CLK_CON_GAT_MIF_CMU_FSYS_MMC0, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_FSYS_MMC1,
	     "gout_mif_cmu_fsys_mmc1", "dout_mif_cmu_fsys_mmc1",
	     CLK_CON_GAT_MIF_CMU_FSYS_MMC1, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_FSYS_MMC2,
	     "gout_mif_cmu_fsys_mmc2", "dout_mif_cmu_fsys_mmc2",
	     CLK_CON_GAT_MIF_CMU_FSYS_MMC2, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_FSYS_USB20DRD_REFCLK,
	     "gout_mif_cmu_fsys_usb20drd_refclk", "dout_mif_cmu_fsys_usb20drd_refclk",
	     CLK_CON_GAT_MIF_CMU_FSYS_USB20DRD_REFCLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_BUS,
	     "gout_mif_cmu_peri_bus", "dout_mif_cmu_peri_bus",
	     CLK_CON_GAT_MIF_CMU_PERI_BUS, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_SPI0,
	     "gout_mif_cmu_peri_spi0", "dout_mif_cmu_peri_spi0",
	     CLK_CON_GAT_MIF_CMU_PERI_SPI0, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_SPI1,
	     "gout_mif_cmu_peri_spi1", "dout_mif_cmu_peri_spi1",
	     CLK_CON_GAT_MIF_CMU_PERI_SPI1, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_SPI2,
	     "gout_mif_cmu_peri_spi2", "dout_mif_cmu_peri_spi2",
	     CLK_CON_GAT_MIF_CMU_PERI_SPI2, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_SPI3,
	     "gout_mif_cmu_peri_spi3", "dout_mif_cmu_peri_spi3",
	     CLK_CON_GAT_MIF_CMU_PERI_SPI3, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_SPI4,
	     "gout_mif_cmu_peri_spi4", "dout_mif_cmu_peri_spi4",
	     CLK_CON_GAT_MIF_CMU_PERI_SPI4, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_UART0,
	     "gout_mif_cmu_peri_uart0", "dout_mif_cmu_peri_uart0",
	     CLK_CON_GAT_MIF_CMU_PERI_UART0, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_UART1,
	     "gout_mif_cmu_peri_uart1", "dout_mif_cmu_peri_uart1",
	     CLK_CON_GAT_MIF_CMU_PERI_UART1, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CMU_PERI_UART2,
	     "gout_mif_cmu_peri_uart2", "dout_mif_cmu_peri_uart2",
	     CLK_CON_GAT_MIF_CMU_PERI_UART2, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CP_PCLK_HSI2C,
	     "gout_mif_cp_pclk_hsi2c", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_CP_PCLK_HSI2C, 6,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CP_PCLK_HSI2C_BAT_0,
	     "gout_mif_cp_pclk_hsi2c_bat_0", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_CP_PCLK_HSI2C_BAT_0, 4,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_CP_PCLK_HSI2C_BAT_1,
	     "gout_mif_cp_pclk_hsi2c_bat_1", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_CP_PCLK_HSI2C_BAT_1, 5,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_AP_PCLKM,
	     "gout_mif_hsi2c_ap_pclkm", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_HSI2C_AP_PCLKM, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_CP_PCLKM,
	     "gout_mif_hsi2c_cp_pclkm", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_HSI2C_CP_PCLKM, 1,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_IPCLK,
	     "gout_mif_hsi2c_ipclk", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_HSI2C_IPCLK, 2,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_MIF_HSI2C_ITCLK,
	     "gout_mif_hsi2c_itclk", "dout_mif_hsi2c",
	     CLK_CON_GAT_MIF_HSI2C_ITCLK, 3,
	     CLK_SET_RATE_PARENT, 0),
};

static const struct samsung_clk_group mif_cmu_clks[] = {
	{ S_CLK_PLL, mif_pll_clks, ARRAY_SIZE(mif_pll_clks) },
	{ S_CLK_GATE, mif_pll_gate_clks, ARRAY_SIZE(mif_pll_gate_clks) },
	{ S_CLK_FFACTOR, mif_fixed_factor_clks, ARRAY_SIZE(mif_fixed_factor_clks) },
	{ S_CLK_MUX, mif_mux_clks, ARRAY_SIZE(mif_mux_clks) },
	{ S_CLK_GATE, mif_mux_gate_clks, ARRAY_SIZE(mif_mux_gate_clks) },
	{ S_CLK_DIV, mif_div_clks, ARRAY_SIZE(mif_div_clks) },
	{ S_CLK_GATE, mif_gate_clks, ARRAY_SIZE(mif_gate_clks) },
};

static int exynos7870_cmu_mif_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_MIF, mif_cmu_clks,
				    exynos7870_cmu_mif);
}

static const struct udevice_id exynos7870_cmu_mif_ids[] = {
	{ .compatible = "samsung,exynos7870-cmu-mif" },
	{ }
};

SAMSUNG_CLK_OPS(exynos7870_cmu_mif, CMU_MIF);

U_BOOT_DRIVER(exynos7870_cmu_mif) = {
	.name		= "exynos7870-cmu-mif",
	.id		= UCLASS_CLK,
	.of_match	= exynos7870_cmu_mif_ids,
	.ops		= &exynos7870_cmu_mif_clk_ops,
	.probe		= exynos7870_cmu_mif_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/*
 * Register offsets for CMU_FSYS (0x13730000)
 */
#define PLL_LOCKTIME_FSYS_USB_PLL					0x0000
#define PLL_CON0_FSYS_USB_PLL						0x0100
#define CLK_CON_GAT_FSYS_MUX_USB_PLL					0x0200
#define CLK_CON_GAT_FSYS_MUX_USB_PLL_CON				0x0200
#define CLK_CON_GAT_FSYS_MUX_USB20DRD_PHYCLOCK_USER			0x0230
#define CLK_CON_GAT_FSYS_MUX_USB20DRD_PHYCLOCK_USER_CON			0x0230
#define CLK_CON_GAT_FSYS_BUSP3_HCLK					0x0804
#define CLK_CON_GAT_FSYS_MMC0_ACLK					0x0804
#define CLK_CON_GAT_FSYS_MMC1_ACLK					0x0804
#define CLK_CON_GAT_FSYS_MMC2_ACLK					0x0804
#define CLK_CON_GAT_FSYS_PDMA0_ACLK_PDMA0				0x0804
#define CLK_CON_GAT_FSYS_PPMU_ACLK					0x0804
#define CLK_CON_GAT_FSYS_PPMU_PCLK					0x0804
#define CLK_CON_GAT_FSYS_SROMC_HCLK					0x0804
#define CLK_CON_GAT_FSYS_UPSIZER_BUS1_ACLK				0x0804
#define CLK_CON_GAT_FSYS_USB20DRD_ACLK_HSDRD				0x0804
#define CLK_CON_GAT_FSYS_USB20DRD_HCLK_USB20_CTRL			0x0804
#define CLK_CON_GAT_FSYS_USB20DRD_HSDRD_REF_CLK				0x0828

static const struct samsung_fixed_rate_clock fsys_fixed_rate_clks[] = {
	FRATE(0, "frat_fsys_usb20drd_phyclock", 60000000),
};

static const struct samsung_pll_clock fsys_pll_clks[] = {
	PLL(pll_1417x, CLK_FOUT_FSYS_USB_PLL, "fout_fsys_usb_pll", "oscclk",
	    PLL_CON0_FSYS_USB_PLL),
};

static const struct samsung_gate_clock fsys_gate_clks[] = {
	GATE(CLK_GOUT_FSYS_BUSP3_HCLK,
	     "gout_fsys_busp3_hclk", "bus",
	     CLK_CON_GAT_FSYS_BUSP3_HCLK, 2,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_UPSIZER_BUS1_ACLK,
	     "gout_fsys_upsizer_bus1_aclk", "bus",
	     CLK_CON_GAT_FSYS_UPSIZER_BUS1_ACLK, 12,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_PPMU_ACLK,
	     "gout_fsys_ppmu_aclk", "bus",
	     CLK_CON_GAT_FSYS_PPMU_ACLK, 17,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_PPMU_PCLK,
	     "gout_fsys_ppmu_pclk", "bus",
	     CLK_CON_GAT_FSYS_PPMU_PCLK, 18,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_USB20DRD_HSDRD_REF_CLK,
	     "gout_fsys_usb20drd_hsdrd_ref_clk", "usb20drd",
	     CLK_CON_GAT_FSYS_USB20DRD_HSDRD_REF_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MUX_USB20DRD_PHYCLOCK_USER_CON,
	     "gout_fsys_mux_usb20drd_phyclock_user_con", "frat_fsys_usb20drd_phyclock",
	     CLK_CON_GAT_FSYS_MUX_USB20DRD_PHYCLOCK_USER_CON, 12,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MUX_USB_PLL_CON,
	     "gout_fsys_mux_usb_pll_con", "fout_fsys_usb_pll",
	     CLK_CON_GAT_FSYS_MUX_USB_PLL_CON, 12,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MMC0_ACLK,
	     "gout_fsys_mmc0_aclk", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_MMC0_ACLK, 8,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MMC1_ACLK,
	     "gout_fsys_mmc1_aclk", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_MMC1_ACLK, 9,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MMC2_ACLK,
	     "gout_fsys_mmc2_aclk", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_MMC2_ACLK, 10,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_USB20DRD_ACLK_HSDRD,
	     "gout_fsys_usb20drd_aclk_hsdrd", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_USB20DRD_ACLK_HSDRD, 20,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_SROMC_HCLK,
	     "gout_fsys_sromc_hclk", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_SROMC_HCLK, 6,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_USB20DRD_HCLK_USB20_CTRL,
	     "gout_fsys_usb20drd_hclk_usb20_ctrl", "gout_fsys_busp3_hclk",
	     CLK_CON_GAT_FSYS_USB20DRD_HCLK_USB20_CTRL, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MUX_USB_PLL,
	     "gout_fsys_mux_usb_pll", "gout_fsys_mux_usb_pll_con",
	     CLK_CON_GAT_FSYS_MUX_USB_PLL, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_MUX_USB20DRD_PHYCLOCK_USER,
	     "gout_fsys_mux_usb20drd_phyclock_user", "gout_fsys_mux_usb20drd_phyclock_user_con",
	     CLK_CON_GAT_FSYS_MUX_USB20DRD_PHYCLOCK_USER, 21,
	     CLK_IS_CRITICAL | CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_FSYS_PDMA0_ACLK_PDMA0,
	     "gout_fsys_pdma0_aclk_pdma0", "gout_fsys_upsizer_bus1_aclk",
	     CLK_CON_GAT_FSYS_PDMA0_ACLK_PDMA0, 7,
	     CLK_SET_RATE_PARENT, 0),
};

static const struct samsung_clk_group fsys_cmu_clks[] = {
	{ S_CLK_FRATE, fsys_fixed_rate_clks, ARRAY_SIZE(fsys_fixed_rate_clks) },
	{ S_CLK_PLL, fsys_pll_clks, ARRAY_SIZE(fsys_pll_clks) },
	{ S_CLK_GATE, fsys_gate_clks, ARRAY_SIZE(fsys_gate_clks) },
};

static int exynos7870_cmu_fsys_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_FSYS, fsys_cmu_clks,
				    exynos7870_cmu_fsys);
}

static const struct udevice_id exynos7870_cmu_fsys_ids[] = {
	{ .compatible = "samsung,exynos7870-cmu-fsys" },
	{ }
};

SAMSUNG_CLK_OPS(exynos7870_cmu_fsys, CMU_FSYS);

U_BOOT_DRIVER(exynos7870_cmu_fsys) = {
	.name		= "exynos7870-cmu-fsys",
	.id		= UCLASS_CLK,
	.of_match	= exynos7870_cmu_fsys_ids,
	.ops		= &exynos7870_cmu_fsys_clk_ops,
	.probe		= exynos7870_cmu_fsys_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/*
 * Register offsets for CMU_PERI (0x101f0000)
 */
#define CLK_CON_GAT_PERI_PWM_MOTOR_OSCCLK				0x0800
#define CLK_CON_GAT_PERI_TMU_CLK					0x0800
#define CLK_CON_GAT_PERI_TMU_CPUCL0_CLK					0x0800
#define CLK_CON_GAT_PERI_TMU_CPUCL1_CLK					0x0800
#define CLK_CON_GAT_PERI_BUSP1_PERIC0_HCLK				0x0810
#define CLK_CON_GAT_PERI_GPIO2_PCLK					0x0810
#define CLK_CON_GAT_PERI_GPIO5_PCLK					0x0810
#define CLK_CON_GAT_PERI_GPIO6_PCLK					0x0810
#define CLK_CON_GAT_PERI_GPIO7_PCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C1_IPCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C2_IPCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C3_IPCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C4_IPCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C5_IPCLK					0x0810
#define CLK_CON_GAT_PERI_HSI2C6_IPCLK					0x0810
#define CLK_CON_GAT_PERI_I2C0_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C1_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C2_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C3_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C4_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C5_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C6_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C7_PCLK					0x0810
#define CLK_CON_GAT_PERI_I2C8_PCLK					0x0810
#define CLK_CON_GAT_PERI_MCT_PCLK					0x0810
#define CLK_CON_GAT_PERI_PWM_MOTOR_PCLK_S0				0x0810
#define CLK_CON_GAT_PERI_SFRIF_TMU_CPUCL0_PCLK				0x0814
#define CLK_CON_GAT_PERI_SFRIF_TMU_CPUCL1_PCLK				0x0814
#define CLK_CON_GAT_PERI_SFRIF_TMU_PCLK					0x0814
#define CLK_CON_GAT_PERI_SPI0_PCLK					0x0814
#define CLK_CON_GAT_PERI_SPI1_PCLK					0x0814
#define CLK_CON_GAT_PERI_SPI2_PCLK					0x0814
#define CLK_CON_GAT_PERI_SPI3_PCLK					0x0814
#define CLK_CON_GAT_PERI_SPI4_PCLK					0x0814
#define CLK_CON_GAT_PERI_UART0_PCLK					0x0814
#define CLK_CON_GAT_PERI_UART1_PCLK					0x0814
#define CLK_CON_GAT_PERI_UART2_PCLK					0x0814
#define CLK_CON_GAT_PERI_WDT_CPUCL0_PCLK				0x0814
#define CLK_CON_GAT_PERI_WDT_CPUCL1_PCLK				0x0814
#define CLK_CON_GAT_PERI_UART1_EXT_UCLK					0x0830
#define CLK_CON_GAT_PERI_UART2_EXT_UCLK					0x0834
#define CLK_CON_GAT_PERI_UART0_EXT_UCLK					0x0838
#define CLK_CON_GAT_PERI_SPI2_SPI_EXT_CLK				0x083c
#define CLK_CON_GAT_PERI_SPI1_SPI_EXT_CLK				0x0840
#define CLK_CON_GAT_PERI_SPI0_SPI_EXT_CLK				0x0844
#define CLK_CON_GAT_PERI_SPI3_SPI_EXT_CLK				0x0848
#define CLK_CON_GAT_PERI_SPI4_SPI_EXT_CLK				0x084c

static const struct samsung_gate_clock peri_gate_clks[] = {
	GATE(CLK_GOUT_PERI_PWM_MOTOR_OSCCLK,
	     "gout_peri_pwm_motor_oscclk", "oscclk",
	     CLK_CON_GAT_PERI_PWM_MOTOR_OSCCLK, 2,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_TMU_CLK,
	     "gout_peri_tmu_clk", "oscclk",
	     CLK_CON_GAT_PERI_TMU_CLK, 6,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_TMU_CPUCL0_CLK,
	     "gout_peri_tmu_cpucl0_clk", "oscclk",
	     CLK_CON_GAT_PERI_TMU_CPUCL0_CLK, 4,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_TMU_CPUCL1_CLK,
	     "gout_peri_tmu_cpucl1_clk", "oscclk",
	     CLK_CON_GAT_PERI_TMU_CPUCL1_CLK, 5,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_BUSP1_PERIC0_HCLK,
	     "gout_peri_busp1_peric0_hclk", "bus",
	     CLK_CON_GAT_PERI_BUSP1_PERIC0_HCLK, 3,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_GPIO2_PCLK,
	     "gout_peri_gpio2_pclk", "bus",
	     CLK_CON_GAT_PERI_GPIO2_PCLK, 7,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_GPIO5_PCLK,
	     "gout_peri_gpio5_pclk", "bus",
	     CLK_CON_GAT_PERI_GPIO5_PCLK, 8,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_GPIO6_PCLK,
	     "gout_peri_gpio6_pclk", "bus",
	     CLK_CON_GAT_PERI_GPIO6_PCLK, 9,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_GPIO7_PCLK,
	     "gout_peri_gpio7_pclk", "bus",
	     CLK_CON_GAT_PERI_GPIO7_PCLK, 10,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C5_IPCLK,
	     "gout_peri_hsi2c5_ipclk", "bus",
	     CLK_CON_GAT_PERI_HSI2C5_IPCLK, 15,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C6_IPCLK,
	     "gout_peri_hsi2c6_ipclk", "bus",
	     CLK_CON_GAT_PERI_HSI2C6_IPCLK, 16,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_MCT_PCLK,
	     "gout_peri_mct_pclk", "bus",
	     CLK_CON_GAT_PERI_MCT_PCLK, 26,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_PWM_MOTOR_PCLK_S0,
	     "gout_peri_pwm_motor_pclk_s0", "bus",
	     CLK_CON_GAT_PERI_PWM_MOTOR_PCLK_S0, 29,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SFRIF_TMU_CPUCL0_PCLK,
	     "gout_peri_sfrif_tmu_cpucl0_pclk", "bus",
	     CLK_CON_GAT_PERI_SFRIF_TMU_CPUCL0_PCLK, 1,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SFRIF_TMU_CPUCL1_PCLK,
	     "gout_peri_sfrif_tmu_cpucl1_pclk", "bus",
	     CLK_CON_GAT_PERI_SFRIF_TMU_CPUCL1_PCLK, 2,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SFRIF_TMU_PCLK,
	     "gout_peri_sfrif_tmu_pclk", "bus",
	     CLK_CON_GAT_PERI_SFRIF_TMU_PCLK, 3,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI0_PCLK,
	     "gout_peri_spi0_pclk", "bus",
	     CLK_CON_GAT_PERI_SPI0_PCLK, 6,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI1_PCLK,
	     "gout_peri_spi1_pclk", "bus",
	     CLK_CON_GAT_PERI_SPI1_PCLK, 5,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI2_PCLK,
	     "gout_peri_spi2_pclk", "bus",
	     CLK_CON_GAT_PERI_SPI2_PCLK, 4,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI3_PCLK,
	     "gout_peri_spi3_pclk", "bus",
	     CLK_CON_GAT_PERI_SPI3_PCLK, 7,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI4_PCLK,
	     "gout_peri_spi4_pclk", "bus",
	     CLK_CON_GAT_PERI_SPI4_PCLK, 8,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_WDT_CPUCL0_PCLK,
	     "gout_peri_wdt_cpucl0_pclk", "bus",
	     CLK_CON_GAT_PERI_WDT_CPUCL0_PCLK, 13,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_WDT_CPUCL1_PCLK,
	     "gout_peri_wdt_cpucl1_pclk", "bus",
	     CLK_CON_GAT_PERI_WDT_CPUCL1_PCLK, 14,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI0_SPI_EXT_CLK,
	     "gout_peri_spi0_spi_ext_clk", "spi0",
	     CLK_CON_GAT_PERI_SPI0_SPI_EXT_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI1_SPI_EXT_CLK,
	     "gout_peri_spi1_spi_ext_clk", "spi1",
	     CLK_CON_GAT_PERI_SPI1_SPI_EXT_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI2_SPI_EXT_CLK,
	     "gout_peri_spi2_spi_ext_clk", "spi2",
	     CLK_CON_GAT_PERI_SPI2_SPI_EXT_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI3_SPI_EXT_CLK,
	     "gout_peri_spi3_spi_ext_clk", "spi3",
	     CLK_CON_GAT_PERI_SPI3_SPI_EXT_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_SPI4_SPI_EXT_CLK,
	     "gout_peri_spi4_spi_ext_clk", "spi4",
	     CLK_CON_GAT_PERI_SPI4_SPI_EXT_CLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART0_EXT_UCLK,
	     "gout_peri_uart0_ext_uclk", "uart0",
	     CLK_CON_GAT_PERI_UART0_EXT_UCLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART1_EXT_UCLK,
	     "gout_peri_uart1_ext_uclk", "uart1",
	     CLK_CON_GAT_PERI_UART1_EXT_UCLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART2_EXT_UCLK,
	     "gout_peri_uart2_ext_uclk", "uart2",
	     CLK_CON_GAT_PERI_UART2_EXT_UCLK, 0,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C1_IPCLK,
	     "gout_peri_hsi2c1_ipclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_HSI2C1_IPCLK, 11,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C2_IPCLK,
	     "gout_peri_hsi2c2_ipclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_HSI2C2_IPCLK, 12,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C3_IPCLK,
	     "gout_peri_hsi2c3_ipclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_HSI2C3_IPCLK, 13,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_HSI2C4_IPCLK,
	     "gout_peri_hsi2c4_ipclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_HSI2C4_IPCLK, 14,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C0_PCLK,
	     "gout_peri_i2c0_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C0_PCLK, 21,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C1_PCLK,
	     "gout_peri_i2c1_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C1_PCLK, 23,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C2_PCLK,
	     "gout_peri_i2c2_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C2_PCLK, 22,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C3_PCLK,
	     "gout_peri_i2c3_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C3_PCLK, 20,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C4_PCLK,
	     "gout_peri_i2c4_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C4_PCLK, 17,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C5_PCLK,
	     "gout_peri_i2c5_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C5_PCLK, 18,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C6_PCLK,
	     "gout_peri_i2c6_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C6_PCLK, 19,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C7_PCLK,
	     "gout_peri_i2c7_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C7_PCLK, 24,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_I2C8_PCLK,
	     "gout_peri_i2c8_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_I2C8_PCLK, 25,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART0_PCLK,
	     "gout_peri_uart0_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_UART0_PCLK, 10,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART1_PCLK,
	     "gout_peri_uart1_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_UART1_PCLK, 11,
	     CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_PERI_UART2_PCLK,
	     "gout_peri_uart2_pclk", "gout_peri_busp1_peric0_hclk",
	     CLK_CON_GAT_PERI_UART2_PCLK, 12,
	     CLK_SET_RATE_PARENT, 0),
};

static const struct samsung_clk_group peri_cmu_clks[] = {
	{ S_CLK_GATE, peri_gate_clks, ARRAY_SIZE(peri_gate_clks) },
};

static int exynos7870_cmu_peri_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_PERI, peri_cmu_clks,
				    exynos7870_cmu_peri);
}

static const struct udevice_id exynos7870_cmu_peri_ids[] = {
	{ .compatible = "samsung,exynos7870-cmu-peri" },
	{ }
};

SAMSUNG_CLK_OPS(exynos7870_cmu_peri, CMU_PERI);

U_BOOT_DRIVER(exynos7870_cmu_peri) = {
	.name		= "exynos7870-cmu-peri",
	.id		= UCLASS_CLK,
	.of_match	= exynos7870_cmu_peri_ids,
	.ops		= &exynos7870_cmu_peri_clk_ops,
	.probe		= exynos7870_cmu_peri_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
