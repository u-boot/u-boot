// SPDX-License-Identifier: GPL-2.0+
/*
 * Samsung Exynos850 clock driver.
 * Copyright (c) 2023 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <dm.h>
#include <asm/io.h>
#include <dt-bindings/clock/exynos850.h>
#include "clk.h"

enum exynos850_cmu_id {
	CMU_TOP,
	CMU_PERI,
	CMU_CORE,
	CMU_HSI,
};

/* ---- CMU_TOP ------------------------------------------------------------- */

/* Register Offset definitions for CMU_TOP (0x120e0000) */
#define PLL_CON0_PLL_MMC			0x0100
#define PLL_CON3_PLL_MMC			0x010c
#define PLL_CON0_PLL_SHARED0			0x0140
#define PLL_CON3_PLL_SHARED0			0x014c
#define PLL_CON0_PLL_SHARED1			0x0180
#define PLL_CON3_PLL_SHARED1			0x018c
#define CLK_CON_MUX_MUX_CLKCMU_CORE_BUS		0x1014
#define CLK_CON_MUX_MUX_CLKCMU_CORE_CCI		0x1018
#define CLK_CON_MUX_MUX_CLKCMU_CORE_MMC_EMBD	0x101c
#define CLK_CON_MUX_MUX_CLKCMU_CORE_SSS		0x1020
#define CLK_CON_MUX_MUX_CLKCMU_HSI_BUS		0x103c
#define CLK_CON_MUX_MUX_CLKCMU_HSI_MMC_CARD	0x1040
#define CLK_CON_MUX_MUX_CLKCMU_HSI_USB20DRD	0x1044
#define CLK_CON_MUX_MUX_CLKCMU_PERI_BUS		0x1070
#define CLK_CON_MUX_MUX_CLKCMU_PERI_IP		0x1074
#define CLK_CON_MUX_MUX_CLKCMU_PERI_UART	0x1078
#define CLK_CON_DIV_CLKCMU_CORE_BUS		0x1820
#define CLK_CON_DIV_CLKCMU_CORE_CCI		0x1824
#define CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD	0x1828
#define CLK_CON_DIV_CLKCMU_CORE_SSS		0x182c
#define CLK_CON_DIV_CLKCMU_HSI_BUS		0x1848
#define CLK_CON_DIV_CLKCMU_HSI_MMC_CARD		0x184c
#define CLK_CON_DIV_CLKCMU_HSI_USB20DRD		0x1850
#define CLK_CON_DIV_CLKCMU_PERI_BUS		0x187c
#define CLK_CON_DIV_CLKCMU_PERI_IP		0x1880
#define CLK_CON_DIV_CLKCMU_PERI_UART		0x1884
#define CLK_CON_DIV_PLL_SHARED0_DIV2		0x188c
#define CLK_CON_DIV_PLL_SHARED0_DIV3		0x1890
#define CLK_CON_DIV_PLL_SHARED0_DIV4		0x1894
#define CLK_CON_DIV_PLL_SHARED1_DIV2		0x1898
#define CLK_CON_DIV_PLL_SHARED1_DIV3		0x189c
#define CLK_CON_DIV_PLL_SHARED1_DIV4		0x18a0
#define CLK_CON_GAT_GATE_CLKCMU_CORE_BUS	0x201c
#define CLK_CON_GAT_GATE_CLKCMU_CORE_CCI	0x2020
#define CLK_CON_GAT_GATE_CLKCMU_CORE_MMC_EMBD	0x2024
#define CLK_CON_GAT_GATE_CLKCMU_CORE_SSS	0x2028
#define CLK_CON_GAT_GATE_CLKCMU_HSI_BUS		0x2044
#define CLK_CON_GAT_GATE_CLKCMU_HSI_MMC_CARD	0x2048
#define CLK_CON_GAT_GATE_CLKCMU_HSI_USB20DRD	0x204c
#define CLK_CON_GAT_GATE_CLKCMU_PERI_BUS	0x2080
#define CLK_CON_GAT_GATE_CLKCMU_PERI_IP		0x2084
#define CLK_CON_GAT_GATE_CLKCMU_PERI_UART	0x2088

/* List of parent clocks for Muxes in CMU_TOP: for PURECLKCOMP */
PNAME(mout_shared0_pll_p)	= { "clock-oscclk", "fout_shared0_pll" };
PNAME(mout_shared1_pll_p)	= { "clock-oscclk", "fout_shared1_pll" };
PNAME(mout_mmc_pll_p)		= { "clock-oscclk", "fout_mmc_pll" };
/* List of parent clocks for Muxes in CMU_TOP: for CMU_CORE */
PNAME(mout_core_bus_p)		= { "dout_shared1_div2", "dout_shared0_div3",
				    "dout_shared1_div3", "dout_shared0_div4" };
PNAME(mout_core_cci_p)		= { "dout_shared0_div2", "dout_shared1_div2",
				    "dout_shared0_div3", "dout_shared1_div3" };
PNAME(mout_core_mmc_embd_p)	= { "clock-oscclk", "dout_shared0_div2",
				    "dout_shared1_div2", "dout_shared0_div3",
				    "dout_shared1_div3", "mout_mmc_pll",
				    "clock-oscclk", "clock-oscclk" };
PNAME(mout_core_sss_p)		= { "dout_shared0_div3", "dout_shared1_div3",
				    "dout_shared0_div4", "dout_shared1_div4" };
/* List of parent clocks for Muxes in CMU_TOP: for CMU_HSI */
PNAME(mout_hsi_bus_p)		= { "dout_shared0_div2", "dout_shared1_div2" };
PNAME(mout_hsi_mmc_card_p)	= { "clock-oscclk", "dout_shared0_div2",
				    "dout_shared1_div2", "dout_shared0_div3",
				    "dout_shared1_div3", "mout_mmc_pll",
				    "clock-oscclk", "clock-oscclk" };
PNAME(mout_hsi_usb20drd_p)	= { "clock-oscclk", "dout_shared0_div4",
				    "dout_shared1_div4", "clock-oscclk" };
/* List of parent clocks for Muxes in CMU_TOP: for CMU_PERI */
PNAME(mout_peri_bus_p)		= { "dout_shared0_div4", "dout_shared1_div4" };
PNAME(mout_peri_uart_p)		= { "clock-oscclk", "dout_shared0_div4",
				    "dout_shared1_div4", "clock-oscclk" };
PNAME(mout_peri_ip_p)		= { "clock-oscclk", "dout_shared0_div4",
				    "dout_shared1_div4", "clock-oscclk" };

/* PURECLKCOMP */

static const struct samsung_pll_clock top_pure_pll_clks[] = {
	PLL(pll_0822x, CLK_FOUT_SHARED0_PLL, "fout_shared0_pll", "clock-oscclk",
	    PLL_CON3_PLL_SHARED0),
	PLL(pll_0822x, CLK_FOUT_SHARED1_PLL, "fout_shared1_pll", "clock-oscclk",
	    PLL_CON3_PLL_SHARED1),
	PLL(pll_0831x, CLK_FOUT_MMC_PLL, "fout_mmc_pll", "clock-oscclk",
	    PLL_CON3_PLL_MMC),
};

static const struct samsung_mux_clock top_pure_mux_clks[] = {
	MUX(CLK_MOUT_SHARED0_PLL, "mout_shared0_pll", mout_shared0_pll_p,
	    PLL_CON0_PLL_SHARED0, 4, 1),
	MUX(CLK_MOUT_SHARED1_PLL, "mout_shared1_pll", mout_shared1_pll_p,
	    PLL_CON0_PLL_SHARED1, 4, 1),
	MUX(CLK_MOUT_MMC_PLL, "mout_mmc_pll", mout_mmc_pll_p,
	    PLL_CON0_PLL_MMC, 4, 1),
};

static const struct samsung_div_clock top_pure_div_clks[] = {
	DIV(CLK_DOUT_SHARED0_DIV3, "dout_shared0_div3", "mout_shared0_pll",
	    CLK_CON_DIV_PLL_SHARED0_DIV3, 0, 2),
	DIV(CLK_DOUT_SHARED0_DIV2, "dout_shared0_div2", "mout_shared0_pll",
	    CLK_CON_DIV_PLL_SHARED0_DIV2, 0, 1),
	DIV(CLK_DOUT_SHARED1_DIV3, "dout_shared1_div3", "mout_shared1_pll",
	    CLK_CON_DIV_PLL_SHARED1_DIV3, 0, 2),
	DIV(CLK_DOUT_SHARED1_DIV2, "dout_shared1_div2", "mout_shared1_pll",
	    CLK_CON_DIV_PLL_SHARED1_DIV2, 0, 1),
	DIV(CLK_DOUT_SHARED0_DIV4, "dout_shared0_div4", "dout_shared0_div2",
	    CLK_CON_DIV_PLL_SHARED0_DIV4, 0, 1),
	DIV(CLK_DOUT_SHARED1_DIV4, "dout_shared1_div4", "dout_shared1_div2",
	    CLK_CON_DIV_PLL_SHARED1_DIV4, 0, 1),
};

/* CORE */

static const struct samsung_mux_clock top_core_mux_clks[] = {
	MUX(CLK_MOUT_CORE_BUS, "mout_core_bus", mout_core_bus_p,
	    CLK_CON_MUX_MUX_CLKCMU_CORE_BUS, 0, 2),
	MUX(CLK_MOUT_CORE_CCI, "mout_core_cci", mout_core_cci_p,
	    CLK_CON_MUX_MUX_CLKCMU_CORE_CCI, 0, 2),
	MUX(CLK_MOUT_CORE_MMC_EMBD, "mout_core_mmc_embd", mout_core_mmc_embd_p,
	    CLK_CON_MUX_MUX_CLKCMU_CORE_MMC_EMBD, 0, 3),
	MUX(CLK_MOUT_CORE_SSS, "mout_core_sss", mout_core_sss_p,
	    CLK_CON_MUX_MUX_CLKCMU_CORE_SSS, 0, 2),
};

static const struct samsung_gate_clock top_core_gate_clks[] = {
	GATE(CLK_GOUT_CORE_BUS, "gout_core_bus", "mout_core_bus",
	     CLK_CON_GAT_GATE_CLKCMU_CORE_BUS, 21, 0, 0),
	GATE(CLK_GOUT_CORE_CCI, "gout_core_cci", "mout_core_cci",
	     CLK_CON_GAT_GATE_CLKCMU_CORE_CCI, 21, 0, 0),
	GATE(CLK_GOUT_CORE_MMC_EMBD, "gout_core_mmc_embd", "mout_core_mmc_embd",
	     CLK_CON_GAT_GATE_CLKCMU_CORE_MMC_EMBD, 21, 0, 0),
	GATE(CLK_GOUT_CORE_SSS, "gout_core_sss", "mout_core_sss",
	     CLK_CON_GAT_GATE_CLKCMU_CORE_SSS, 21, 0, 0),
};

static const struct samsung_div_clock top_core_div_clks[] = {
	DIV(CLK_DOUT_CORE_BUS, "dout_core_bus", "gout_core_bus",
	    CLK_CON_DIV_CLKCMU_CORE_BUS, 0, 4),
	DIV(CLK_DOUT_CORE_CCI, "dout_core_cci", "gout_core_cci",
	    CLK_CON_DIV_CLKCMU_CORE_CCI, 0, 4),
	DIV(CLK_DOUT_CORE_MMC_EMBD, "dout_core_mmc_embd", "gout_core_mmc_embd",
	    CLK_CON_DIV_CLKCMU_CORE_MMC_EMBD, 0, 9),
	DIV(CLK_DOUT_CORE_SSS, "dout_core_sss", "gout_core_sss",
	    CLK_CON_DIV_CLKCMU_CORE_SSS, 0, 4),
};

/* HSI */

static const struct samsung_mux_clock top_hsi_mux_clks[] = {
	MUX(CLK_MOUT_HSI_BUS, "mout_hsi_bus", mout_hsi_bus_p,
	    CLK_CON_MUX_MUX_CLKCMU_HSI_BUS, 0, 1),
	MUX(CLK_MOUT_HSI_MMC_CARD, "mout_hsi_mmc_card", mout_hsi_mmc_card_p,
	    CLK_CON_MUX_MUX_CLKCMU_HSI_MMC_CARD, 0, 3),
	MUX(CLK_MOUT_HSI_USB20DRD, "mout_hsi_usb20drd", mout_hsi_usb20drd_p,
	    CLK_CON_MUX_MUX_CLKCMU_HSI_USB20DRD, 0, 2),
};

static const struct samsung_gate_clock top_hsi_gate_clks[] = {
	GATE(CLK_GOUT_HSI_BUS, "gout_hsi_bus", "mout_hsi_bus",
	     CLK_CON_GAT_GATE_CLKCMU_HSI_BUS, 21, 0, 0),
	GATE(CLK_GOUT_HSI_MMC_CARD, "gout_hsi_mmc_card", "mout_hsi_mmc_card",
	     CLK_CON_GAT_GATE_CLKCMU_HSI_MMC_CARD, 21, 0, 0),
	GATE(CLK_GOUT_HSI_USB20DRD, "gout_hsi_usb20drd", "mout_hsi_usb20drd",
	     CLK_CON_GAT_GATE_CLKCMU_HSI_USB20DRD, 21, 0, 0),
};

static const struct samsung_div_clock top_hsi_div_clks[] = {
	DIV(CLK_DOUT_HSI_BUS, "dout_hsi_bus", "gout_hsi_bus",
	    CLK_CON_DIV_CLKCMU_HSI_BUS, 0, 4),
	DIV(CLK_DOUT_HSI_MMC_CARD, "dout_hsi_mmc_card", "gout_hsi_mmc_card",
	    CLK_CON_DIV_CLKCMU_HSI_MMC_CARD, 0, 9),
	DIV(CLK_DOUT_HSI_USB20DRD, "dout_hsi_usb20drd", "gout_hsi_usb20drd",
	    CLK_CON_DIV_CLKCMU_HSI_USB20DRD, 0, 4),
};

/* PERI */

static const struct samsung_mux_clock top_peri_mux_clks[] = {
	MUX(CLK_MOUT_PERI_BUS, "mout_peri_bus", mout_peri_bus_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_BUS, 0, 1),
	MUX(CLK_MOUT_PERI_UART, "mout_peri_uart", mout_peri_uart_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_UART, 0, 2),
	MUX(CLK_MOUT_PERI_IP, "mout_peri_ip", mout_peri_ip_p,
	    CLK_CON_MUX_MUX_CLKCMU_PERI_IP, 0, 2),
};

static const struct samsung_gate_clock top_peri_gate_clks[] = {
	GATE(CLK_GOUT_PERI_BUS, "gout_peri_bus", "mout_peri_bus",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_BUS, 21, 0, 0),
	GATE(CLK_GOUT_PERI_UART, "gout_peri_uart", "mout_peri_uart",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_UART, 21, 0, 0),
	GATE(CLK_GOUT_PERI_IP, "gout_peri_ip", "mout_peri_ip",
	     CLK_CON_GAT_GATE_CLKCMU_PERI_IP, 21, 0, 0),
};

static const struct samsung_div_clock top_peri_div_clks[] = {
	DIV(CLK_DOUT_PERI_BUS, "dout_peri_bus", "gout_peri_bus",
	    CLK_CON_DIV_CLKCMU_PERI_BUS, 0, 4),
	DIV(CLK_DOUT_PERI_UART, "dout_peri_uart", "gout_peri_uart",
	    CLK_CON_DIV_CLKCMU_PERI_UART, 0, 4),
	DIV(CLK_DOUT_PERI_IP, "dout_peri_ip", "gout_peri_ip",
	    CLK_CON_DIV_CLKCMU_PERI_IP, 0, 4),
};

static const struct samsung_clk_group top_cmu_clks[] = {
	/* CMU_TOP_PURECLKCOMP */
	{ S_CLK_PLL, top_pure_pll_clks, ARRAY_SIZE(top_pure_pll_clks) },
	{ S_CLK_MUX, top_pure_mux_clks, ARRAY_SIZE(top_pure_mux_clks) },
	{ S_CLK_DIV, top_pure_div_clks, ARRAY_SIZE(top_pure_div_clks) },

	/* CMU_TOP clocks for CMU_CORE */
	{ S_CLK_MUX, top_core_mux_clks, ARRAY_SIZE(top_core_mux_clks) },
	{ S_CLK_GATE, top_core_gate_clks, ARRAY_SIZE(top_core_gate_clks) },
	{ S_CLK_DIV, top_core_div_clks, ARRAY_SIZE(top_core_div_clks) },

	/* CMU_TOP clocks for CMU_HSI */
	{ S_CLK_MUX, top_hsi_mux_clks, ARRAY_SIZE(top_hsi_mux_clks) },
	{ S_CLK_GATE, top_hsi_gate_clks, ARRAY_SIZE(top_hsi_gate_clks) },
	{ S_CLK_DIV, top_hsi_div_clks, ARRAY_SIZE(top_hsi_div_clks) },

	/* CMU_TOP clocks for CMU_PERI */
	{ S_CLK_MUX, top_peri_mux_clks, ARRAY_SIZE(top_peri_mux_clks) },
	{ S_CLK_GATE, top_peri_gate_clks, ARRAY_SIZE(top_peri_gate_clks) },
	{ S_CLK_DIV, top_peri_div_clks, ARRAY_SIZE(top_peri_div_clks) },
};

static int exynos850_cmu_top_probe(struct udevice *dev)
{
	return samsung_cmu_register_one(dev, CMU_TOP, top_cmu_clks,
					ARRAY_SIZE(top_cmu_clks));
}

static const struct udevice_id exynos850_cmu_top_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-top" },
	{ }
};

SAMSUNG_CLK_OPS(exynos850_cmu_top, CMU_TOP);

U_BOOT_DRIVER(exynos850_cmu_top) = {
	.name		= "exynos850-cmu-top",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_top_ids,
	.ops		= &exynos850_cmu_top_clk_ops,
	.probe		= exynos850_cmu_top_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/* ---- CMU_PERI ------------------------------------------------------------ */

/* Register Offset definitions for CMU_PERI (0x10030000) */
#define PLL_CON0_MUX_CLKCMU_PERI_BUS_USER	0x0600
#define PLL_CON0_MUX_CLKCMU_PERI_UART_USER	0x0630
#define CLK_CON_GAT_GOUT_PERI_UART_IPCLK	0x20a8
#define CLK_CON_GAT_GOUT_PERI_UART_PCLK		0x20ac

/* List of parent clocks for Muxes in CMU_PERI */
PNAME(mout_peri_bus_user_p)	= { "clock-oscclk", "dout_peri_bus" };
PNAME(mout_peri_uart_user_p)	= { "clock-oscclk", "dout_peri_uart" };

static const struct samsung_mux_clock peri_mux_clks[] = {
	MUX(CLK_MOUT_PERI_BUS_USER, "mout_peri_bus_user", mout_peri_bus_user_p,
	    PLL_CON0_MUX_CLKCMU_PERI_BUS_USER, 4, 1),
	MUX(CLK_MOUT_PERI_UART_USER, "mout_peri_uart_user",
	    mout_peri_uart_user_p, PLL_CON0_MUX_CLKCMU_PERI_UART_USER, 4, 1),
};

static const struct samsung_gate_clock peri_gate_clks[] = {
	GATE(CLK_GOUT_UART_IPCLK, "gout_uart_ipclk", "mout_peri_uart_user",
	     CLK_CON_GAT_GOUT_PERI_UART_IPCLK, 21, 0, 0),
	GATE(CLK_GOUT_UART_PCLK, "gout_uart_pclk", "mout_peri_bus_user",
	     CLK_CON_GAT_GOUT_PERI_UART_PCLK, 21, 0, 0),
};

static const struct samsung_clk_group peri_cmu_clks[] = {
	{ S_CLK_MUX, peri_mux_clks, ARRAY_SIZE(peri_mux_clks) },
	{ S_CLK_GATE, peri_gate_clks, ARRAY_SIZE(peri_gate_clks) },
};

static int exynos850_cmu_peri_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_PERI, peri_cmu_clks,
				    exynos850_cmu_top);
}

static const struct udevice_id exynos850_cmu_peri_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-peri" },
	{ }
};

SAMSUNG_CLK_OPS(exynos850_cmu_peri, CMU_PERI);

U_BOOT_DRIVER(exynos850_cmu_peri) = {
	.name		= "exynos850-cmu-peri",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_peri_ids,
	.ops		= &exynos850_cmu_peri_clk_ops,
	.probe		= exynos850_cmu_peri_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/* ---- CMU_CORE ------------------------------------------------------------ */

/* Register Offset definitions for CMU_CORE (0x12000000) */
#define PLL_CON0_MUX_CLKCMU_CORE_BUS_USER	0x0600
#define PLL_CON0_MUX_CLKCMU_CORE_MMC_EMBD_USER	0x0620
#define PLL_CON0_MUX_CLKCMU_CORE_SSS_USER	0x0630
#define CLK_CON_DIV_DIV_CLK_CORE_BUSP		0x1800
#define CLK_CON_GAT_GOUT_CORE_MMC_EMBD_I_ACLK	0x20e8
#define CLK_CON_GAT_GOUT_CORE_MMC_EMBD_SDCLKIN	0x20ec
#define CLK_CON_GAT_GOUT_CORE_SSS_I_ACLK	0x2128
#define CLK_CON_GAT_GOUT_CORE_SSS_I_PCLK	0x212c

/* List of parent clocks for Muxes in CMU_CORE */
PNAME(mout_core_bus_user_p)		= { "clock-oscclk", "dout_core_bus" };
PNAME(mout_core_mmc_embd_user_p)	= { "clock-oscclk",
					    "dout_core_mmc_embd" };
PNAME(mout_core_sss_user_p)		= { "clock-oscclk", "dout_core_sss" };

static const struct samsung_mux_clock core_mux_clks[] = {
	MUX(CLK_MOUT_CORE_BUS_USER, "mout_core_bus_user", mout_core_bus_user_p,
	    PLL_CON0_MUX_CLKCMU_CORE_BUS_USER, 4, 1),
	MUX_F(CLK_MOUT_CORE_MMC_EMBD_USER, "mout_core_mmc_embd_user",
	      mout_core_mmc_embd_user_p, PLL_CON0_MUX_CLKCMU_CORE_MMC_EMBD_USER,
	      4, 1, CLK_SET_RATE_PARENT, 0),
	MUX(CLK_MOUT_CORE_SSS_USER, "mout_core_sss_user", mout_core_sss_user_p,
	    PLL_CON0_MUX_CLKCMU_CORE_SSS_USER, 4, 1),
};

static const struct samsung_div_clock core_div_clks[] = {
	DIV(CLK_DOUT_CORE_BUSP, "dout_core_busp", "mout_core_bus_user",
	    CLK_CON_DIV_DIV_CLK_CORE_BUSP, 0, 2),
};

static const struct samsung_gate_clock core_gate_clks[] = {
	GATE(CLK_GOUT_MMC_EMBD_ACLK, "gout_mmc_embd_aclk", "dout_core_busp",
	     CLK_CON_GAT_GOUT_CORE_MMC_EMBD_I_ACLK, 21, 0, 0),
	GATE(CLK_GOUT_MMC_EMBD_SDCLKIN, "gout_mmc_embd_sdclkin",
	     "mout_core_mmc_embd_user", CLK_CON_GAT_GOUT_CORE_MMC_EMBD_SDCLKIN,
	     21, CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_SSS_ACLK, "gout_sss_aclk", "mout_core_sss_user",
	     CLK_CON_GAT_GOUT_CORE_SSS_I_ACLK, 21, 0, 0),
	GATE(CLK_GOUT_SSS_PCLK, "gout_sss_pclk", "dout_core_busp",
	     CLK_CON_GAT_GOUT_CORE_SSS_I_PCLK, 21, 0, 0),
};

static const struct samsung_clk_group core_cmu_clks[] = {
	{ S_CLK_MUX, core_mux_clks, ARRAY_SIZE(core_mux_clks) },
	{ S_CLK_DIV, core_div_clks, ARRAY_SIZE(core_div_clks) },
	{ S_CLK_GATE, core_gate_clks, ARRAY_SIZE(core_gate_clks) },
};

static int exynos850_cmu_core_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_CORE, core_cmu_clks,
				    exynos850_cmu_top);
}

static const struct udevice_id exynos850_cmu_core_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-core" },
	{ }
};

SAMSUNG_CLK_OPS(exynos850_cmu_core, CMU_CORE);

U_BOOT_DRIVER(exynos850_cmu_core) = {
	.name		= "exynos850-cmu-core",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_core_ids,
	.ops		= &exynos850_cmu_core_clk_ops,
	.probe		= exynos850_cmu_core_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

/* ---- CMU_HSI ------------------------------------------------------------- */

/* Register Offset definitions for CMU_HSI (0x13400000) */
#define PLL_CON0_MUX_CLKCMU_HSI_BUS_USER			0x0600
#define PLL_CON0_MUX_CLKCMU_HSI_MMC_CARD_USER			0x0610
#define PLL_CON0_MUX_CLKCMU_HSI_USB20DRD_USER			0x0620
#define CLK_CON_GAT_HSI_USB20DRD_TOP_I_REF_CLK_50		0x200c
#define CLK_CON_GAT_HSI_USB20DRD_TOP_I_PHY_REFCLK_26		0x2010
#define CLK_CON_GAT_GOUT_HSI_MMC_CARD_I_ACLK			0x2024
#define CLK_CON_GAT_GOUT_HSI_MMC_CARD_SDCLKIN			0x2028
#define CLK_CON_GAT_GOUT_HSI_USB20DRD_TOP_ACLK_PHYCTRL_20	0x203c
#define CLK_CON_GAT_GOUT_HSI_USB20DRD_TOP_BUS_CLK_EARLY		0x2040

/* List of parent clocks for Muxes in CMU_HSI */
PNAME(mout_hsi_bus_user_p)	= { "clock-oscclk", "dout_hsi_bus" };
PNAME(mout_hsi_mmc_card_user_p)	= { "clock-oscclk", "dout_hsi_mmc_card" };
PNAME(mout_hsi_usb20drd_user_p)	= { "clock-oscclk", "dout_hsi_usb20drd" };

static const struct samsung_mux_clock hsi_mux_clks[] __initconst = {
	MUX(CLK_MOUT_HSI_BUS_USER, "mout_hsi_bus_user", mout_hsi_bus_user_p,
	    PLL_CON0_MUX_CLKCMU_HSI_BUS_USER, 4, 1),
	MUX_F(CLK_MOUT_HSI_MMC_CARD_USER, "mout_hsi_mmc_card_user",
	      mout_hsi_mmc_card_user_p, PLL_CON0_MUX_CLKCMU_HSI_MMC_CARD_USER,
	      4, 1, CLK_SET_RATE_PARENT, 0),
	MUX(CLK_MOUT_HSI_USB20DRD_USER, "mout_hsi_usb20drd_user",
	    mout_hsi_usb20drd_user_p, PLL_CON0_MUX_CLKCMU_HSI_USB20DRD_USER,
	    4, 1),
};

static const struct samsung_gate_clock hsi_gate_clks[] __initconst = {
	GATE(CLK_GOUT_USB_REF_CLK, "gout_usb_ref", "mout_hsi_usb20drd_user",
	     CLK_CON_GAT_HSI_USB20DRD_TOP_I_REF_CLK_50, 21, 0, 0),
	GATE(CLK_GOUT_USB_PHY_REF_CLK, "gout_usb_phy_ref", "clock-oscclk",
	     CLK_CON_GAT_HSI_USB20DRD_TOP_I_PHY_REFCLK_26, 21, 0, 0),
	GATE(CLK_GOUT_MMC_CARD_ACLK, "gout_mmc_card_aclk", "mout_hsi_bus_user",
	     CLK_CON_GAT_GOUT_HSI_MMC_CARD_I_ACLK, 21, 0, 0),
	GATE(CLK_GOUT_MMC_CARD_SDCLKIN, "gout_mmc_card_sdclkin",
	     "mout_hsi_mmc_card_user",
	     CLK_CON_GAT_GOUT_HSI_MMC_CARD_SDCLKIN, 21, CLK_SET_RATE_PARENT, 0),
	GATE(CLK_GOUT_USB_PHY_ACLK, "gout_usb_phy_aclk", "mout_hsi_bus_user",
	     CLK_CON_GAT_GOUT_HSI_USB20DRD_TOP_ACLK_PHYCTRL_20, 21, 0, 0),
	GATE(CLK_GOUT_USB_BUS_EARLY_CLK, "gout_usb_bus_early",
	     "mout_hsi_bus_user",
	     CLK_CON_GAT_GOUT_HSI_USB20DRD_TOP_BUS_CLK_EARLY, 21, 0, 0),
};

static const struct samsung_clk_group hsi_cmu_clks[] = {
	{ S_CLK_MUX, hsi_mux_clks, ARRAY_SIZE(hsi_mux_clks) },
	{ S_CLK_GATE, hsi_gate_clks, ARRAY_SIZE(hsi_gate_clks) },
};

static int exynos850_cmu_hsi_probe(struct udevice *dev)
{
	return samsung_register_cmu(dev, CMU_HSI, hsi_cmu_clks,
				    exynos850_cmu_hsi);
}

static const struct udevice_id exynos850_cmu_hsi_ids[] = {
	{ .compatible = "samsung,exynos850-cmu-hsi" },
	{ }
};

SAMSUNG_CLK_OPS(exynos850_cmu_hsi, CMU_HSI);

U_BOOT_DRIVER(exynos850_cmu_hsi) = {
	.name		= "exynos850-cmu-hsi",
	.id		= UCLASS_CLK,
	.of_match	= exynos850_cmu_hsi_ids,
	.ops		= &exynos850_cmu_hsi_clk_ops,
	.probe		= exynos850_cmu_hsi_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
