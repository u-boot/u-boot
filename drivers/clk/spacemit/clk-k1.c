// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 SpacemiT Technology Co. Ltd
 * Copyright (c) 2024-2025 Haylen Chu <heylenay@4d2.org>
 * Copyright (c) 2025 Junhui Liu <junhui.liu@pigmoral.tech>
 * Copyright (c) 2025-2026 RISCstar Ltd.
 *
 *  Authors: Haylen Chu <heylenay@4d2.org>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <regmap.h>
#include <linux/clk-provider.h>
#include <soc/spacemit/k1-syscon.h>

#include "clk_common.h"
#include "clk_ddn.h"
#include "clk_mix.h"
#include "clk_pll.h"

#include <dt-bindings/clock/spacemit,k1-syscon.h>

#define K1_PLL_ID		100
#define K1_MPMU_ID		200
#define K1_APBC_ID		300
#define K1_APMU_ID		400

struct spacemit_ccu_data {
	struct clk **clks;
	size_t num;
	unsigned long offset;
};

/* APBS clocks start, APBS region contains and only contains all PLL clocks */

/*
 * PLL{1,2} must run at fixed frequencies to provide clocks in correct rates for
 * peripherals.
 */
static const struct ccu_pll_rate_tbl pll1_rate_tbl[] = {
	CCU_PLL_RATE(2457600000UL, 0x0050dd64, 0x330ccccd),
};

static const struct ccu_pll_rate_tbl pll2_rate_tbl[] = {
	CCU_PLL_RATE(3000000000UL, 0x0050dd66, 0x3fe00000),
};

static const struct ccu_pll_rate_tbl pll3_rate_tbl[] = {
	CCU_PLL_RATE(1600000000UL, 0x0050cd61, 0x43eaaaab),
	CCU_PLL_RATE(1800000000UL, 0x0050cd61, 0x4b000000),
	CCU_PLL_RATE(2000000000UL, 0x0050dd62, 0x2aeaaaab),
	CCU_PLL_RATE(2457600000UL, 0x0050dd64, 0x330ccccd),
	CCU_PLL_RATE(3000000000UL, 0x0050dd66, 0x3fe00000),
	CCU_PLL_RATE(3200000000UL, 0x0050dd67, 0x43eaaaab),
};

CCU_PLL_DEFINE(CLK_PLL1, pll1, pll1, "clock-24m", pll1_rate_tbl,
	       APBS_PLL1_SWCR1, APBS_PLL1_SWCR3, MPMU_POSR, POSR_PLL1_LOCK,
	       CLK_SET_RATE_GATE);
CCU_PLL_DEFINE(CLK_PLL2, pll2, pll2, "clock-24m", pll2_rate_tbl,
	       APBS_PLL2_SWCR1, APBS_PLL2_SWCR3, MPMU_POSR, POSR_PLL2_LOCK,
	       CLK_SET_RATE_GATE);
CCU_PLL_DEFINE(CLK_PLL3, pll3, pll3, "clock-24m", pll3_rate_tbl,
	       APBS_PLL3_SWCR1, APBS_PLL3_SWCR3, MPMU_POSR, POSR_PLL3_LOCK,
	       CLK_SET_RATE_GATE);

CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D2, pll1_d2, pll1_d2, "pll1", APBS_PLL1_SWCR2,
		       BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D3, pll1_d3, pll1_d3, "pll1", APBS_PLL1_SWCR2,
		       BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D4, pll1_d4, pll1_d4, "pll1", APBS_PLL1_SWCR2,
		       BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D5, pll1_d5, pll1_d5, "pll1", APBS_PLL1_SWCR2,
		       BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D6, pll1_d6, pll1_d6, "pll1", APBS_PLL1_SWCR2,
		       BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D7, pll1_d7, pll1_d7, "pll1", APBS_PLL1_SWCR2,
		       BIT(6), 7, 1);
CCU_FACTOR_GATE_FLAGS_DEFINE(CLK_PLL1_D8, pll1_d8, pll1_d8, "pll1",
			     APBS_PLL1_SWCR2, BIT(7), 8, 1, CLK_IS_CRITICAL);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D11, pll1_d11_223p4, pll1_d11_223p4, "pll1",
		       APBS_PLL1_SWCR2, BIT(15), 11, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D13, pll1_d13_189, pll1_d13_189, "pll1",
		       APBS_PLL1_SWCR2, BIT(16), 13, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D23, pll1_d23_106p8, pll1_d23_106p8, "pll1",
		       APBS_PLL1_SWCR2, BIT(20), 23, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D64, pll1_d64_38p4, pll1_d64_38p4, "pll1",
		       APBS_PLL1_SWCR2, BIT(0), 64, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D10_AUD, pll1_aud_245p7, pll1_aud_245p7, "pll1",
		       APBS_PLL1_SWCR2, BIT(10), 10, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_D100_AUD, pll1_aud_24p5, pll1_aud_24p5, "pll1",
		       APBS_PLL1_SWCR2, BIT(11), 100, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D1, pll2_d1, pll2_d1, "pll2", APBS_PLL2_SWCR2,
		       BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D2, pll2_d2, pll2_d2, "pll2", APBS_PLL2_SWCR2,
		       BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D3, pll2_d3, pll2_d3, "pll2", APBS_PLL2_SWCR2,
		       BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D4, pll2_d4, pll2_d4, "pll2", APBS_PLL2_SWCR2,
		       BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D5, pll2_d5, pll2_d5, "pll2", APBS_PLL2_SWCR2,
		       BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D6, pll2_d6, pll2_d6, "pll2", APBS_PLL2_SWCR2,
		       BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D7, pll2_d7, pll2_d7, "pll2", APBS_PLL2_SWCR2,
		       BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL2_D8, pll2_d8, pll2_d8, "pll2", APBS_PLL2_SWCR2,
		       BIT(7), 8, 1);

CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D1, pll3_d1, pll3_d1, "pll3", APBS_PLL3_SWCR2,
		       BIT(0), 1, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D2, pll3_d2, pll3_d2, "pll3", APBS_PLL3_SWCR2,
		       BIT(1), 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D3, pll3_d3, pll3_d3, "pll3", APBS_PLL3_SWCR2,
		       BIT(2), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D4, pll3_d4, pll3_d4, "pll3", APBS_PLL3_SWCR2,
		       BIT(3), 4, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D5, pll3_d5, pll3_d5, "pll3", APBS_PLL3_SWCR2,
		       BIT(4), 5, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D6, pll3_d6, pll3_d6, "pll3", APBS_PLL3_SWCR2,
		       BIT(5), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D7, pll3_d7, pll3_d7, "pll3", APBS_PLL3_SWCR2,
		       BIT(6), 7, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL3_D8, pll3_d8, pll3_d8, "pll3", APBS_PLL3_SWCR2,
		       BIT(7), 8, 1);

CCU_FACTOR_DEFINE(CLK_PLL3_20, pll3_20, pll3_20, "pll3_d8", 20, 1);
CCU_FACTOR_DEFINE(CLK_PLL3_40, pll3_40, pll3_40, "pll3_d8", 10, 1);
CCU_FACTOR_DEFINE(CLK_PLL3_80, pll3_80, pll3_80, "pll3_d8", 5, 1);
/* APBS clocks end */

/* MPMU clocks start */
CCU_GATE_DEFINE(CLK_PLL1_307P2, pll1_d8_307p2, pll1_d8_307p2, "pll1_d8",
		MPMU_ACGR, BIT(13), 0);

CCU_FACTOR_DEFINE(CLK_PLL1_76P8, pll1_d32_76p8, pll1_d32_76p8, "pll1_d8_307p2",
		  4, 1);

CCU_FACTOR_DEFINE(CLK_PLL1_61P44, pll1_d40_61p44, pll1_d40_61p44,
		  "pll1_d8_307p2", 5, 1);

CCU_FACTOR_DEFINE(CLK_PLL1_153P6, pll1_d16_153p6, pll1_d16_153p6,
		  "pll1_d8", 2, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_102P4, pll1_d24_102p4, pll1_d24_102p4,
		       "pll1_d8", MPMU_ACGR, BIT(12), 3, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_51P2, pll1_d48_51p2, pll1_d48_51p2,
		       "pll1_d8", MPMU_ACGR, BIT(7), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_51P2_AP, pll1_d48_51p2_ap, pll1_d48_51p2_ap,
		       "pll1_d8", MPMU_ACGR, BIT(11), 6, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_57P6, pll1_m3d128_57p6, pll1_m3d128_57p6,
		       "pll1_d8", MPMU_ACGR, BIT(8), 16, 3);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_25P6, pll1_d96_25p6, pll1_d96_25p6,
		       "pll1_d8", MPMU_ACGR, BIT(4), 12, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_12P8, pll1_d192_12p8, pll1_d192_12p8,
		       "pll1_d8", MPMU_ACGR, BIT(3), 24, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_12P8_WDT, pll1_d192_12p8_wdt, pll1_d192_12p8_wdt,
		       "pll1_d8", MPMU_ACGR, BIT(19), 24, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_6P4, pll1_d384_6p4, pll1_d384_6p4,
		       "pll1_d8", MPMU_ACGR, BIT(2), 48, 1);

CCU_FACTOR_DEFINE(CLK_PLL1_3P2, pll1_d768_3p2, pll1_d768_3p2,
		  "pll1_d384_6p4", 2, 1);
CCU_FACTOR_DEFINE(CLK_PLL1_1P6, pll1_d1536_1p6, pll1_d1536_1p6,
		  "pll1_d384_6p4", 4, 1);
CCU_FACTOR_DEFINE(CLK_PLL1_0P8, pll1_d3072_0p8, pll1_d3072_0p8,
		  "pll1_d384_6p4", 8, 1);

CCU_GATE_DEFINE(CLK_PLL1_409P6, pll1_d6_409p6, pll1_d6_409p6, "pll1_d6",
		MPMU_ACGR, BIT(0), 0);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_204P8, pll1_d12_204p8, pll1_d12_204p8,
		       "pll1_d6", MPMU_ACGR, BIT(5), 2, 1);

CCU_GATE_DEFINE(CLK_PLL1_491, pll1_d5_491p52, pll1_d5_491p52, "pll1_d5",
		MPMU_ACGR, BIT(21), 0);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_245P76, pll1_d10_245p76, pll1_d10_245p76,
		       "pll1_d5", MPMU_ACGR, BIT(18), 2, 1);

CCU_GATE_DEFINE(CLK_PLL1_614, pll1_d4_614p4, pll1_d4_614p4, "pll1_d4",
		MPMU_ACGR, BIT(15), 0);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_47P26, pll1_d52_47p26, pll1_d52_47p26,
		       "pll1_d4", MPMU_ACGR, BIT(10), 13, 1);
CCU_FACTOR_GATE_DEFINE(CLK_PLL1_31P5, pll1_d78_31p5, pll1_d78_31p5,
		       "pll1_d4", MPMU_ACGR, BIT(6), 39, 2);

CCU_GATE_DEFINE(CLK_PLL1_819, pll1_d3_819p2, pll1_d3_819p2, "pll1_d3",
		MPMU_ACGR, BIT(14), 0);

CCU_GATE_DEFINE(CLK_PLL1_1228, pll1_d2_1228p8, pll1_d2_1228p8, "pll1_d2",
		MPMU_ACGR, BIT(16), 0);

CCU_GATE_DEFINE(CLK_SLOW_UART, slow_uart, slow_uart, "clock-32k", MPMU_ACGR,
		BIT(1), CLK_IGNORE_UNUSED);
CCU_DDN_DEFINE(CLK_SLOW_UART1, slow_uart1_14p74, slow_uart1_14p74,
	       "pll1_d16_153p6", MPMU_SUCCR,
	       CCU_DDN_MASK(16, 13), 16, CCU_DDN_MASK(0, 13), 0, 2, 0);
CCU_DDN_DEFINE(CLK_SLOW_UART2, slow_uart2_48, slow_uart2_48,
	       "pll1_d4_614p4", MPMU_SUCCR_1,
	       CCU_DDN_MASK(16, 13), 16, CCU_DDN_MASK(0, 13), 0, 2, 0);

#if !IS_ENABLED(CONFIG_SPL_BUILD)
CCU_GATE_DEFINE(CLK_WDT, wdt_clk, wdt_clk, "pll1_d96_25p6", MPMU_WDTPCR,
		BIT(1), 0);

CCU_FACTOR_DEFINE(CLK_I2S_153P6, i2s_153p6, i2s_153p6, "pll1_d8_307p2", 2, 1);

static const char * const i2s_153p6_base_parents[] = {
	"i2s_153p6",
	"pll1_d8_307p2",
};

CCU_MUX_DEFINE(CLK_I2S_153P6_BASE, i2s_153p6_base, i2s_153p6_base,
	       i2s_153p6_base_parents, ARRAY_SIZE(i2s_153p6_base_parents),
	       MPMU_FCCR, 29, 1, 0);

static const char * const i2s_sysclk_src_parents[] = {
	"pll1_d96_25p6",
	"i2s_153p6_base"
};

CCU_MUX_GATE_DEFINE(CLK_I2S_SYSCLK_SRC, i2s_sysclk_src, i2s_sysclk_src,
		    i2s_sysclk_src_parents, ARRAY_SIZE(i2s_sysclk_src_parents),
		    MPMU_ISCCR, 30, 1, BIT(31), 0);

CCU_DDN_DEFINE(CLK_I2S_SYSCLK, i2s_sysclk, i2s_sysclk, "i2s_sysclk_src",
	       MPMU_ISCCR, CCU_DDN_MASK(0, 15), 0, CCU_DDN_MASK(15, 12),
	       15, 1, 0);

CCU_FACTOR_DEFINE(CLK_I2S_BCLK_FACTOR, i2s_bclk_factor, i2s_bclk_factor,
		  "i2s_sysclk", 2, 1);
/*
 * Divider of i2s_bclk always implies a 1/2 factor, which is
 * described by i2s_bclk_factor.
 */
CCU_DIV_GATE_DEFINE(CLK_I2S_BCLK, i2s_bclk, i2s_bclk, "i2s_bclk_factor",
		    MPMU_ISCCR, 27, 2, BIT(29), 0);

static const char * const apb_parents[] = {
	"pll1_d96_25p6",
	"pll1_d48_51p2",
	"pll1_d96_25p6",
	"pll1_d24_102p4",
};

CCU_MUX_DEFINE(CLK_APB, apb_clk, apb_clk, apb_parents, ARRAY_SIZE(apb_parents),
	       MPMU_APBCSCR, 0, 2, 0);

CCU_GATE_DEFINE(CLK_WDT_BUS, wdt_bus_clk, wdt_bus_clk, "apb_clk", MPMU_WDTPCR,
		BIT(0), 0);

CCU_GATE_DEFINE(CLK_RIPC, ripc_clk, ripc_clk, "apb_clk", MPMU_RIPCCR, 0x1, 0);
#endif
/* MPMU clocks end */

/* APBC clocks start */
static const char * const uart_clk_parents[] = {
	"pll1_m3d128_57p6",
	"slow_uart1_14p74",
	"slow_uart2_48",
};

CCU_MUX_GATE_DEFINE(CLK_UART0, uart0_clk, uart0_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART1_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);

static const char * const twsi_parents[] = {
	"pll1_d78_31p5",
	"pll1_d48_51p2",
	"pll1_d40_61p44",
};

CCU_MUX_GATE_DEFINE(CLK_TWSI2, twsi2_clk, twsi2_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI2_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
/*
 * APBC_TWSI8_CLK_RST has a quirk that reading always results in zero.
 * Combine functional and bus bits together as a gate to avoid sharing the
 * write-only register between different clock hardwares.
 */
CCU_GATE_DEFINE(CLK_TWSI8, twsi8_clk, twsi8_clk, "pll1_d78_31p5",
		APBC_TWSI8_CLK_RST, BIT(1) | BIT(0), 0);

#if !IS_ENABLED(CONFIG_SPL_BUILD)
CCU_MUX_GATE_DEFINE(CLK_UART2, uart2_clk, uart2_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART2_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART3, uart3_clk, uart3_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART3_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART4, uart4_clk, uart4_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART4_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART5, uart5_clk, uart5_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART5_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART6, uart6_clk, uart6_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART6_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART7, uart7_clk, uart7_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART7_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART8, uart8_clk, uart8_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART8_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_UART9, uart9_clk, uart9_clk, uart_clk_parents,
		    ARRAY_SIZE(uart_clk_parents), APBC_UART9_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);

CCU_GATE_DEFINE(CLK_GPIO, gpio_clk, gpio_clk, "clock-24m", APBC_GPIO_CLK_RST,
		BIT(1) | BIT(0), 0);

static const char * const pwm_parents[] = {
	"pll1_d192_12p8",
	"clock-32k",
};

CCU_MUX_GATE_DEFINE(CLK_PWM0, pwm0_clk, pwm0_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM0_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM1, pwm1_clk, pwm1_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM1_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM2, pwm2_clk, pwm2_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM2_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM3, pwm3_clk, pwm3_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM3_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM4, pwm4_clk, pwm4_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM4_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM5, pwm5_clk, pwm5_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM5_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM6, pwm6_clk, pwm6_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM6_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM7, pwm7_clk, pwm7_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM7_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM8, pwm8_clk, pwm8_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM8_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM9, pwm9_clk, pwm9_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM9_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM10, pwm10_clk, pwm10_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM10_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM11, pwm11_clk, pwm11_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM11_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM12, pwm12_clk, pwm12_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM12_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM13, pwm13_clk, pwm13_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM13_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM14, pwm14_clk, pwm14_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM14_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM15, pwm15_clk, pwm15_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM15_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM16, pwm16_clk, pwm16_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM16_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM17, pwm17_clk, pwm17_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM17_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM18, pwm18_clk, pwm18_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM18_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_PWM19, pwm19_clk, pwm19_clk, pwm_parents,
		    ARRAY_SIZE(pwm_parents), APBC_PWM19_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);

static const char * const ssp_parents[] = {
	"pll1_d384_6p4",
	"pll1_d192_12p8",
	"pll1_d96_25p6",
	"pll1_d48_51p2",
	"pll1_d768_3p2",
	"pll1_d1536_1p6",
	"pll1_d3072_0p8",
};

CCU_MUX_GATE_DEFINE(CLK_SSP3, ssp3_clk, ssp3_clk, ssp_parents,
		    ARRAY_SIZE(ssp_parents), APBC_SSP3_CLK_RST, 4, 3,
		    BIT(1), 0);

CCU_GATE_DEFINE(CLK_RTC, rtc_clk, rtc_clk, "clock-32k", APBC_RTC_CLK_RST,
		BIT(7) | BIT(1) | BIT(0), 0);

CCU_MUX_GATE_DEFINE(CLK_TWSI0, twsi0_clk, twsi0_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI0_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TWSI1, twsi1_clk, twsi1_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI1_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TWSI4, twsi4_clk, twsi4_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI4_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TWSI5, twsi5_clk, twsi5_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI5_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TWSI6, twsi6_clk, twsi6_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI6_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TWSI7, twsi7_clk, twsi7_clk, twsi_parents,
		    ARRAY_SIZE(twsi_parents), APBC_TWSI7_CLK_RST,
		    4, 3, BIT(1) | BIT(0), 0);

static const char * const timer_parents[] = {
	"pll1_d192_12p8",
	"clock-32k",
	"pll1_d384_6p4",
	"clock-3m",
	"clock-1m",
};

CCU_MUX_GATE_DEFINE(CLK_TIMERS1, timers1_clk, timers1_clk, timer_parents,
		    ARRAY_SIZE(timer_parents), APBC_TIMERS1_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);
CCU_MUX_GATE_DEFINE(CLK_TIMERS2, timers2_clk, timers2_clk, timer_parents,
		    ARRAY_SIZE(timer_parents), APBC_TIMERS2_CLK_RST, 4, 3,
		    BIT(1) | BIT(0), 0);

CCU_GATE_DEFINE(CLK_AIB, aib_clk, aib_clk, "clock-24m", APBC_AIB_CLK_RST,
		BIT(1) | BIT(0), 0);

CCU_GATE_DEFINE(CLK_ONEWIRE, onewire_clk, onewire_clk, "clock-24m",
		APBC_ONEWIRE_CLK_RST, BIT(1) | BIT(0), 0);

/*
 * When i2s_bclk is selected as the parent clock of sspa,
 * the hardware requires bit3 to be set
 */
CCU_GATE_DEFINE(CLK_SSPA0_I2S_BCLK, sspa0_i2s_bclk, sspa0_i2s_bclk, "i2s_bclk",
		APBC_SSPA0_CLK_RST, BIT(3), 0);
CCU_GATE_DEFINE(CLK_SSPA1_I2S_BCLK, sspa1_i2s_bclk, sspa1_i2s_bclk, "i2s_bclk",
		APBC_SSPA1_CLK_RST, BIT(3), 0);

static const char * const sspa0_parents[] = {
	"pll1_d384_6p4",
	"pll1_d192_12p8",
	"pll1_d96_25p6",
	"pll1_d48_51p2",
	"pll1_d768_3p2",
	"pll1_d1536_1p6",
	"pll1_d3072_0p8",
	"sspa0_i2s_bclk",
};

CCU_MUX_GATE_DEFINE(CLK_SSPA0, sspa0_clk, sspa0_clk, sspa0_parents,
		    ARRAY_SIZE(sspa0_parents), APBC_SSPA0_CLK_RST, 4, 3,
		    BIT(1), 0);

static const char * const sspa1_parents[] = {
	"pll1_d384_6p4",
	"pll1_d192_12p8",
	"pll1_d96_25p6",
	"pll1_d48_51p2",
	"pll1_d768_3p2",
	"pll1_d1536_1p6",
	"pll1_d3072_0p8",
	"sspa1_i2s_bclk",
};

CCU_MUX_GATE_DEFINE(CLK_SSPA1, sspa1_clk, sspa1_clk, sspa1_parents,
		    ARRAY_SIZE(sspa1_parents), APBC_SSPA1_CLK_RST, 4, 3,
		    BIT(1), 0);

CCU_GATE_DEFINE(CLK_DRO, dro_clk, dro_clk, "apb_clk", APBC_DRO_CLK_RST,
		BIT(1) | BIT(0), 0);
CCU_GATE_DEFINE(CLK_IR, ir_clk, ir_clk, "apb_clk", APBC_IR_CLK_RST,
		BIT(1) | BIT(0), 0);
CCU_GATE_DEFINE(CLK_TSEN, tsen_clk, tsen_clk, "apb_clk", APBC_TSEN_CLK_RST,
		BIT(1) | BIT(0), 0);
CCU_GATE_DEFINE(CLK_IPC_AP2AUD, ipc_ap2aud_clk, ipc_ap2aud_clk, "apb_clk",
		APBC_IPC_AP2AUD_CLK_RST, BIT(1) | BIT(0), 0);

static const char * const can_parents[] = {
	"pll3_20",
	"pll3_40",
	"pll3_80",
};

CCU_MUX_GATE_DEFINE(CLK_CAN0, can0_clk, can0_clk, can_parents,
		    ARRAY_SIZE(can_parents), APBC_CAN0_CLK_RST, 4, 3,
		    BIT(1), 0);
CCU_GATE_DEFINE(CLK_CAN0_BUS, can0_bus_clk, can0_bus_clk, "clock-24m",
		APBC_CAN0_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_UART0_BUS, uart0_bus_clk, uart0_bus_clk, "apb_clk",
		APBC_UART1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART2_BUS, uart2_bus_clk, uart2_bus_clk, "apb_clk",
		APBC_UART2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART3_BUS, uart3_bus_clk, uart3_bus_clk, "apb_clk",
		APBC_UART3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART4_BUS, uart4_bus_clk, uart4_bus_clk, "apb_clk",
		APBC_UART4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART5_BUS, uart5_bus_clk, uart5_bus_clk, "apb_clk",
		APBC_UART5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART6_BUS, uart6_bus_clk, uart6_bus_clk, "apb_clk",
		APBC_UART6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART7_BUS, uart7_bus_clk, uart7_bus_clk, "apb_clk",
		APBC_UART7_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART8_BUS, uart8_bus_clk, uart8_bus_clk, "apb_clk",
		APBC_UART8_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_UART9_BUS, uart9_bus_clk, uart9_bus_clk, "apb_clk",
		APBC_UART9_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_GPIO_BUS, gpio_bus_clk, gpio_bus_clk, "apb_clk",
		APBC_GPIO_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_PWM0_BUS, pwm0_bus_clk, pwm0_bus_clk, "apb_clk",
		APBC_PWM0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM1_BUS, pwm1_bus_clk, pwm1_bus_clk, "apb_clk",
		APBC_PWM1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM2_BUS, pwm2_bus_clk, pwm2_bus_clk, "apb_clk",
		APBC_PWM2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM3_BUS, pwm3_bus_clk, pwm3_bus_clk, "apb_clk",
		APBC_PWM3_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM4_BUS, pwm4_bus_clk, pwm4_bus_clk, "apb_clk",
		APBC_PWM4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM5_BUS, pwm5_bus_clk, pwm5_bus_clk, "apb_clk",
		APBC_PWM5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM6_BUS, pwm6_bus_clk, pwm6_bus_clk, "apb_clk",
		APBC_PWM6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM7_BUS, pwm7_bus_clk, pwm7_bus_clk, "apb_clk",
		APBC_PWM7_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM8_BUS, pwm8_bus_clk, pwm8_bus_clk, "apb_clk",
		APBC_PWM8_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM9_BUS, pwm9_bus_clk, pwm9_bus_clk, "apb_clk",
		APBC_PWM9_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM10_BUS, pwm10_bus_clk, pwm10_bus_clk, "apb_clk",
		APBC_PWM10_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM11_BUS, pwm11_bus_clk, pwm11_bus_clk, "apb_clk",
		APBC_PWM11_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM12_BUS, pwm12_bus_clk, pwm12_bus_clk, "apb_clk",
		APBC_PWM12_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM13_BUS, pwm13_bus_clk, pwm13_bus_clk, "apb_clk",
		APBC_PWM13_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM14_BUS, pwm14_bus_clk, pwm14_bus_clk, "apb_clk",
		APBC_PWM14_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM15_BUS, pwm15_bus_clk, pwm15_bus_clk, "apb_clk",
		APBC_PWM15_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM16_BUS, pwm16_bus_clk, pwm16_bus_clk, "apb_clk",
		APBC_PWM16_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM17_BUS, pwm17_bus_clk, pwm17_bus_clk, "apb_clk",
		APBC_PWM17_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM18_BUS, pwm18_bus_clk, pwm18_bus_clk, "apb_clk",
		APBC_PWM18_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_PWM19_BUS, pwm19_bus_clk, pwm19_bus_clk, "apb_clk",
		APBC_PWM19_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_SSP3_BUS, ssp3_bus_clk, ssp3_bus_clk, "apb_clk",
		APBC_SSP3_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_RTC_BUS, rtc_bus_clk, rtc_bus_clk, "apb_clk",
		APBC_RTC_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_TWSI0_BUS, twsi0_bus_clk, twsi0_bus_clk, "apb_clk",
		APBC_TWSI0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI1_BUS, twsi1_bus_clk, twsi1_bus_clk, "apb_clk",
		APBC_TWSI1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI2_BUS, twsi2_bus_clk, twsi2_bus_clk, "apb_clk",
		APBC_TWSI2_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI4_BUS, twsi4_bus_clk, twsi4_bus_clk, "apb_clk",
		APBC_TWSI4_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI5_BUS, twsi5_bus_clk, twsi5_bus_clk, "apb_clk",
		APBC_TWSI5_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI6_BUS, twsi6_bus_clk, twsi6_bus_clk, "apb_clk",
		APBC_TWSI6_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TWSI7_BUS, twsi7_bus_clk, twsi7_bus_clk, "apb_clk",
		APBC_TWSI7_CLK_RST, BIT(0), 0);
CCU_FACTOR_DEFINE(CLK_TWSI8_BUS, twsi8_bus_clk, twsi8_bus_clk, "apb_clk", 1, 1);

CCU_GATE_DEFINE(CLK_TIMERS1_BUS, timers1_bus_clk, timers1_bus_clk, "apb_clk",
		APBC_TIMERS1_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_TIMERS2_BUS, timers2_bus_clk, timers2_bus_clk, "apb_clk",
		APBC_TIMERS2_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_AIB_BUS, aib_bus_clk, aib_bus_clk, "apb_clk",
		APBC_AIB_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_ONEWIRE_BUS, onewire_bus_clk, onewire_bus_clk, "apb_clk",
		APBC_ONEWIRE_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_SSPA0_BUS, sspa0_bus_clk, sspa0_bus_clk, "apb_clk",
		APBC_SSPA0_CLK_RST, BIT(0), 0);
CCU_GATE_DEFINE(CLK_SSPA1_BUS, sspa1_bus_clk, sspa1_bus_clk, "apb_clk",
		APBC_SSPA1_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_TSEN_BUS, tsen_bus_clk, tsen_bus_clk, "apb_clk",
		APBC_TSEN_CLK_RST, BIT(0), 0);

CCU_GATE_DEFINE(CLK_IPC_AP2AUD_BUS, ipc_ap2aud_bus_clk, ipc_ap2aud_bus_clk,
		"apb_clk", APBC_IPC_AP2AUD_CLK_RST, BIT(0), 0);
#endif
/* APBC clocks end */

/* APMU clocks start */
static const char * const pmua_aclk_parents[] = {
	"pll1_d10_245p76",
	"pll1_d8_307p2",
};

CCU_MUX_DIV_FC_DEFINE(CLK_PMUA_ACLK, pmua_aclk, pmua_aclk, pmua_aclk_parents,
		      ARRAY_SIZE(pmua_aclk_parents),
		      APMU_ACLK_CLK_CTRL, APMU_ACLK_CLK_CTRL, 1, 2, BIT(4),
		      0, 1, 0);

static const char * const emmc_parents[] = {
	"pll1_d6_409p6",
	"pll1_d4_614p4",
	"pll1_d52_47p26",
	"pll1_d3_819p2",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_EMMC, emmc_clk, emmc_clk, emmc_parents,
				 ARRAY_SIZE(emmc_parents),
				 APMU_PMUA_EM_CLK_RES_CTRL,
				 APMU_PMUA_EM_CLK_RES_CTRL, 8, 3, BIT(11),
				 6, 2, BIT(4), 0);
CCU_DIV_GATE_DEFINE(CLK_EMMC_X, emmc_x_clk, emmc_x_clk, "pll1_d2_1228p8",
		    APMU_PMUA_EM_CLK_RES_CTRL, 12,
		    3, BIT(15), 0);

CCU_GATE_DEFINE(CLK_EMMC_BUS, emmc_bus_clk, emmc_bus_clk, "pmua_aclk",
		APMU_PMUA_EM_CLK_RES_CTRL, BIT(3), 0);

#if !IS_ENABLED(CONFIG_SPL_BUILD)
static const char * const cci550_clk_parents[] = {
	"pll1_d5_491p52",
	"pll1_d4_614p4",
	"pll1_d3_819p2",
	"pll2_d3",
};

CCU_MUX_DIV_FC_DEFINE(CLK_CCI550, cci550_clk, cci550_clk, cci550_clk_parents,
		      ARRAY_SIZE(cci550_clk_parents),
		      APMU_CCI550_CLK_CTRL, APMU_CCI550_CLK_CTRL, 8, 3,
		      BIT(12), 0, 2, CLK_IS_CRITICAL);

static const char * const cpu_c0_hi_clk_parents[] = {
	"pll3_d2",
	"pll3_d1",
};

CCU_MUX_DEFINE(CLK_CPU_C0_HI, cpu_c0_hi_clk, cpu_c0_hi_clk,
	       cpu_c0_hi_clk_parents, ARRAY_SIZE(cpu_c0_hi_clk_parents),
	       APMU_CPU_C0_CLK_CTRL, 13, 1, 0);
static const char * const cpu_c0_clk_parents[] = {
	"pll1_d4_614p4",
	"pll1_d3_819p2",
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d2_1228p8",
	"pll3_d3",
	"pll2_d3",
	"cpu_c0_hi_clk",
};

CCU_MUX_FC_DEFINE(CLK_CPU_C0_CORE, cpu_c0_core_clk, cpu_c0_core_clk,
		  cpu_c0_clk_parents, ARRAY_SIZE(cpu_c0_clk_parents),
		  APMU_CPU_C0_CLK_CTRL, APMU_CPU_C0_CLK_CTRL,
		  BIT(12), 0, 3, CLK_IS_CRITICAL);
CCU_DIV_DEFINE(CLK_CPU_C0_ACE, cpu_c0_ace_clk, cpu_c0_ace_clk,
	       "cpu_c0_core_clk", APMU_CPU_C0_CLK_CTRL, 6, 3, CLK_IS_CRITICAL);
CCU_DIV_DEFINE(CLK_CPU_C0_TCM, cpu_c0_tcm_clk, cpu_c0_tcm_clk,
	       "cpu_c0_core_clk", APMU_CPU_C0_CLK_CTRL, 9, 3, CLK_IS_CRITICAL);

static const char * const cpu_c1_hi_clk_parents[] = {
	"pll3_d2",
	"pll3_d1",
};

CCU_MUX_DEFINE(CLK_CPU_C1_HI, cpu_c1_hi_clk, cpu_c1_hi_clk,
	       cpu_c1_hi_clk_parents, ARRAY_SIZE(cpu_c1_hi_clk_parents),
	       APMU_CPU_C1_CLK_CTRL, 13, 1, 0);
static const char * const cpu_c1_clk_parents[] = {
	"pll1_d4_614p4",
	"pll1_d3_819p2",
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d2_1228p8",
	"pll3_d3",
	"pll2_d3",
	"cpu_c1_hi_clk",
};

CCU_MUX_FC_DEFINE(CLK_CPU_C1_CORE, cpu_c1_core_clk, cpu_c1_core_clk,
		  cpu_c1_clk_parents, ARRAY_SIZE(cpu_c1_clk_parents),
		  APMU_CPU_C1_CLK_CTRL, APMU_CPU_C1_CLK_CTRL,
		  BIT(12), 0, 3, CLK_IS_CRITICAL);
CCU_DIV_DEFINE(CLK_CPU_C1_ACE, cpu_c1_ace_clk, cpu_c1_ace_clk,
	       "cpu_c1_core_clk", APMU_CPU_C1_CLK_CTRL, 6, 3, CLK_IS_CRITICAL);

static const char * const jpg_parents[] = {
	"pll1_d4_614p4",
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d3_819p2",
	"pll1_d2_1228p8",
	"pll2_d4",
	"pll2_d3",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_JPG, jpg_clk, jpg_clk, jpg_parents,
				 ARRAY_SIZE(jpg_parents),
				 APMU_JPG_CLK_RES_CTRL,
				 APMU_JPG_CLK_RES_CTRL,
				 5, 3, BIT(15), 2, 3, BIT(1), 0);

static const char * const ccic2phy_parents[] = {
	"pll1_d24_102p4",
	"pll1_d48_51p2_ap",
};

CCU_MUX_GATE_DEFINE(CLK_CCIC2PHY, ccic2phy_clk, ccic2phy_clk, ccic2phy_parents,
		    ARRAY_SIZE(ccic2phy_parents), APMU_CSI_CCIC2_CLK_RES_CTRL,
		    7, 1, BIT(5), 0);

static const char * const ccic3phy_parents[] = {
	"pll1_d24_102p4",
	"pll1_d48_51p2_ap",
};

CCU_MUX_GATE_DEFINE(CLK_CCIC3PHY, ccic3phy_clk, ccic3phy_clk, ccic3phy_parents,
		    ARRAY_SIZE(ccic3phy_parents), APMU_CSI_CCIC2_CLK_RES_CTRL,
		    31, 1, BIT(30), 0);

static const char * const csi_parents[] = {
	"pll1_d5_491p52",
	"pll1_d6_409p6",
	"pll1_d4_614p4",
	"pll1_d3_819p2",
	"pll2_d2",
	"pll2_d3",
	"pll2_d4",
	"pll1_d2_1228p8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_CSI, csi_clk, csi_clk, csi_parents,
				 ARRAY_SIZE(csi_parents),
				 APMU_CSI_CCIC2_CLK_RES_CTRL,
				 APMU_CSI_CCIC2_CLK_RES_CTRL, 20, 3, BIT(15),
				 16, 3, BIT(4), 0);

static const char * const camm_parents[] = {
	"pll1_d8_307p2",
	"pll2_d5",
	"pll1_d6_409p6",
	"clock-24m",
};

CCU_MUX_DIV_GATE_DEFINE(CLK_CAMM0, camm0_clk, camm0_clk, camm_parents,
			ARRAY_SIZE(camm_parents),
			APMU_CSI_CCIC2_CLK_RES_CTRL, 23, 4, 8, 2,
			BIT(28), 0);
CCU_MUX_DIV_GATE_DEFINE(CLK_CAMM1, camm1_clk, camm1_clk, camm_parents,
			ARRAY_SIZE(camm_parents),
			APMU_CSI_CCIC2_CLK_RES_CTRL, 23, 4, 8, 2,
			BIT(6), 0);
CCU_MUX_DIV_GATE_DEFINE(CLK_CAMM2, camm2_clk, camm2_clk, camm_parents,
			ARRAY_SIZE(camm_parents),
			APMU_CSI_CCIC2_CLK_RES_CTRL, 23, 4, 8, 2,
			BIT(3), 0);

static const char * const isp_cpp_parents[] = {
	"pll1_d8_307p2",
	"pll1_d6_409p6",
};

CCU_MUX_DIV_GATE_DEFINE(CLK_ISP_CPP, isp_cpp_clk, isp_cpp_clk, isp_cpp_parents,
			ARRAY_SIZE(isp_cpp_parents),
			APMU_ISP_CLK_RES_CTRL, 24, 2, 26, 1,
			BIT(28), 0);
static const char * const isp_bus_parents[] = {
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d8_307p2",
	"pll1_d10_245p76",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_ISP_BUS, isp_bus_clk, isp_bus_clk,
				 isp_bus_parents, ARRAY_SIZE(isp_cpp_parents),
				 APMU_ISP_CLK_RES_CTRL,
				 APMU_ISP_CLK_RES_CTRL, 18, 3, BIT(23),
				 21, 2, BIT(17), 0);
static const char * const isp_parents[] = {
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d4_614p4",
	"pll1_d8_307p2",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_ISP, isp_clk, isp_clk, isp_parents,
				 ARRAY_SIZE(isp_parents),
				 APMU_ISP_CLK_RES_CTRL,
				 APMU_ISP_CLK_RES_CTRL,
				 4, 3, BIT(7), 8, 2, BIT(1), 0);

static const char * const dpumclk_parents[] = {
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d4_614p4",
	"pll1_d8_307p2",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_DPU_MCLK, dpu_mclk, dpu_mclk,
				 dpumclk_parents, ARRAY_SIZE(dpumclk_parents),
				 APMU_LCD_CLK_RES_CTRL2, APMU_LCD_CLK_RES_CTRL1,
				 1, 4, BIT(29), 5, 3, BIT(0), 0);

static const char * const dpuesc_parents[] = {
	"pll1_d48_51p2_ap",
	"pll1_d52_47p26",
	"pll1_d96_25p6",
	"pll1_d32_76p8",
};

CCU_MUX_GATE_DEFINE(CLK_DPU_ESC, dpu_esc_clk, dpu_esc_clk, dpuesc_parents,
		    ARRAY_SIZE(dpuesc_parents), APMU_LCD_CLK_RES_CTRL1, 0, 2,
		    BIT(2), 0);

static const char * const dpubit_parents[] = {
	"pll1_d3_819p2",
	"pll2_d2",
	"pll2_d3",
	"pll1_d2_1228p8",
	"pll2_d4",
	"pll2_d5",
	"pll2_d7",
	"pll2_d8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_DPU_BIT, dpu_bit_clk, dpu_bit_clk,
				 dpubit_parents, ARRAY_SIZE(dpubit_parents),
				 APMU_LCD_CLK_RES_CTRL1,
				 APMU_LCD_CLK_RES_CTRL1, 17, 3, BIT(31),
				 20, 3, BIT(16), 0);

static const char * const dpupx_parents[] = {
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d4_614p4",
	"pll1_d8_307p2",
	"pll2_d7",
	"pll2_d8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_DPU_PXCLK, dpu_pxclk, dpu_pxclk,
				 dpupx_parents, ARRAY_SIZE(dpupx_parents),
				 APMU_LCD_CLK_RES_CTRL2, APMU_LCD_CLK_RES_CTRL1,
				 17, 4, BIT(30), 21, 3, BIT(16), 0);

CCU_GATE_DEFINE(CLK_DPU_HCLK, dpu_hclk, dpu_hclk, "pmua_aclk",
		APMU_LCD_CLK_RES_CTRL1, BIT(5), 0);

static const char * const dpu_spi_parents[] = {
	"pll1_d8_307p2",
	"pll1_d6_409p6",
	"pll1_d10_245p76",
	"pll1_d11_223p4",
	"pll1_d13_189",
	"pll1_d23_106p8",
	"pll2_d3",
	"pll2_d5",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_DPU_SPI, dpu_spi_clk, dpu_spi_clk,
				 dpu_spi_parents, ARRAY_SIZE(dpu_spi_parents),
				 APMU_LCD_SPI_CLK_RES_CTRL,
				 APMU_LCD_SPI_CLK_RES_CTRL, 8, 3,
				 BIT(7), 12, 3, BIT(1), 0);
CCU_GATE_DEFINE(CLK_DPU_SPI_HBUS, dpu_spi_hbus_clk, dpu_spi_hbus_clk,
		"pmua_aclk", APMU_LCD_SPI_CLK_RES_CTRL, BIT(3), 0);
CCU_GATE_DEFINE(CLK_DPU_SPIBUS, dpu_spi_bus_clk, dpu_spi_bus_clk,
		"pmua_aclk", APMU_LCD_SPI_CLK_RES_CTRL, BIT(5), 0);
CCU_GATE_DEFINE(CLK_DPU_SPI_ACLK, dpu_spi_aclk, dpu_spi_aclk,
		"pmua_aclk", APMU_LCD_SPI_CLK_RES_CTRL, BIT(6), 0);

static const char * const v2d_parents[] = {
	"pll1_d5_491p52",
	"pll1_d6_409p6",
	"pll1_d8_307p2",
	"pll1_d4_614p4",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_V2D, v2d_clk, v2d_clk, v2d_parents,
				 ARRAY_SIZE(v2d_parents),
				 APMU_LCD_CLK_RES_CTRL1,
				 APMU_LCD_CLK_RES_CTRL1, 9, 3, BIT(28), 12, 2,
				 BIT(8), 0);

static const char * const ccic_4x_parents[] = {
	"pll1_d5_491p52",
	"pll1_d6_409p6",
	"pll1_d4_614p4",
	"pll1_d3_819p2",
	"pll2_d2",
	"pll2_d3",
	"pll2_d4",
	"pll1_d2_1228p8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_CCIC_4X, ccic_4x_clk, ccic_4x_clk,
				 ccic_4x_parents, ARRAY_SIZE(ccic_4x_parents),
				 APMU_CCIC_CLK_RES_CTRL,
				 APMU_CCIC_CLK_RES_CTRL, 18, 3,
				 BIT(15), 23, 2, BIT(4), 0);

static const char * const ccic1phy_parents[] = {
	"pll1_d24_102p4",
	"pll1_d48_51p2_ap",
};

CCU_MUX_GATE_DEFINE(CLK_CCIC1PHY, ccic1phy_clk, ccic1phy_clk, ccic1phy_parents,
		    ARRAY_SIZE(ccic1phy_parents), APMU_CCIC_CLK_RES_CTRL, 7, 1,
		    BIT(5), 0);

CCU_GATE_DEFINE(CLK_SDH_AXI, sdh_axi_aclk, sdh_axi_aclk, "pmua_aclk",
		APMU_SDH0_CLK_RES_CTRL, BIT(3), 0);
static const char * const sdh01_parents[] = {
	"pll1_d6_409p6",
	"pll1_d4_614p4",
	"pll2_d8",
	"pll2_d5",
	"pll1_d11_223p4",
	"pll1_d13_189",
	"pll1_d23_106p8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_SDH0, sdh0_clk, sdh0_clk, sdh01_parents,
				 ARRAY_SIZE(sdh01_parents),
				 APMU_SDH0_CLK_RES_CTRL,
				 APMU_SDH0_CLK_RES_CTRL, 8, 3, BIT(11), 5, 3,
				 BIT(4), 0);
CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_SDH1, sdh1_clk, sdh1_clk, sdh01_parents,
				 ARRAY_SIZE(sdh01_parents),
				 APMU_SDH1_CLK_RES_CTRL,
				 APMU_SDH1_CLK_RES_CTRL, 8, 3, BIT(11), 5, 3,
				 BIT(4), 0);
static const char * const sdh2_parents[] = {
	"pll1_d6_409p6",
	"pll1_d4_614p4",
	"pll2_d8",
	"pll1_d3_819p2",
	"pll1_d11_223p4",
	"pll1_d13_189",
	"pll1_d23_106p8",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_SDH2, sdh2_clk, sdh2_clk, sdh2_parents,
				 ARRAY_SIZE(sdh2_parents),
				 APMU_SDH2_CLK_RES_CTRL,
				 APMU_SDH2_CLK_RES_CTRL, 8, 3, BIT(11), 5, 3,
				 BIT(4), 0);

CCU_GATE_DEFINE(CLK_USB_AXI, usb_axi_clk, usb_axi_clk, "pmua_aclk",
		APMU_USB_CLK_RES_CTRL, BIT(1), 0);
CCU_GATE_DEFINE(CLK_USB_P1, usb_p1_aclk, usb_p1_aclk, "pmua_aclk",
		APMU_USB_CLK_RES_CTRL, BIT(5), 0);
CCU_GATE_DEFINE(CLK_USB30, usb30_clk, usb30_clk, "pmua_aclk",
		APMU_USB_CLK_RES_CTRL, BIT(8), 0);

static const char * const qspi_parents[] = {
	"pll1_d6_409p6",
	"pll2_d8",
	"pll1_d8_307p2",
	"pll1_d10_245p76",
	"pll1_d11_223p4",
	"pll1_d23_106p8",
	"pll1_d5_491p52",
	"pll1_d13_189",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_QSPI, qspi_clk, qspi_clk, qspi_parents,
				 ARRAY_SIZE(qspi_parents),
				 APMU_QSPI_CLK_RES_CTRL,
				 APMU_QSPI_CLK_RES_CTRL, 9, 3, BIT(12), 6, 3,
				 BIT(4), 0);
CCU_GATE_DEFINE(CLK_QSPI_BUS, qspi_bus_clk, qspi_bus_clk, "pmua_aclk",
		APMU_QSPI_CLK_RES_CTRL, BIT(3), 0);
CCU_GATE_DEFINE(CLK_DMA, dma_clk, dma_clk, "pmua_aclk", APMU_DMA_CLK_RES_CTRL,
		BIT(3), 0);

static const char * const aes_parents[] = {
	"pll1_d12_204p8",
	"pll1_d24_102p4",
};

CCU_MUX_GATE_DEFINE(CLK_AES, aes_clk, aes_clk, aes_parents,
		    ARRAY_SIZE(aes_parents), APMU_AES_CLK_RES_CTRL, 6, 1,
		    BIT(5), 0);

static const char * const vpu_parents[] = {
	"pll1_d4_614p4",
	"pll1_d5_491p52",
	"pll1_d3_819p2",
	"pll1_d6_409p6",
	"pll3_d6",
	"pll2_d3",
	"pll2_d4",
	"pll2_d5",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_VPU, vpu_clk, vpu_clk, vpu_parents,
				 ARRAY_SIZE(vpu_parents), APMU_VPU_CLK_RES_CTRL,
				 APMU_VPU_CLK_RES_CTRL, 13, 3, BIT(21), 10, 3,
				 BIT(3), 0);

static const char * const gpu_parents[] = {
	"pll1_d4_614p4",
	"pll1_d5_491p52",
	"pll1_d3_819p2",
	"pll1_d6_409p6",
	"pll3_d6",
	"pll2_d3",
	"pll2_d4",
	"pll2_d5",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_GPU, gpu_clk, gpu_clk, gpu_parents,
				 ARRAY_SIZE(gpu_parents), APMU_GPU_CLK_RES_CTRL,
				 APMU_GPU_CLK_RES_CTRL, 12, 3, BIT(15), 18, 3,
				 BIT(4), 0);

static const char * const audio_parents[] = {
	"pll1_aud_245p7",
	"pll1_d8_307p2",
	"pll1_d6_409p6",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_AUDIO, audio_clk, audio_clk, audio_parents,
				 ARRAY_SIZE(audio_parents),
				 APMU_AUDIO_CLK_RES_CTRL,
				 APMU_AUDIO_CLK_RES_CTRL, 4, 3, BIT(15),
				 7, 3, BIT(12), 0);

static const char * const hdmi_parents[] = {
	"pll1_d6_409p6",
	"pll1_d5_491p52",
	"pll1_d4_614p4",
	"pll1_d8_307p2",
};

CCU_MUX_DIV_GATE_SPLIT_FC_DEFINE(CLK_HDMI, hdmi_mclk, hdmi_mclk, hdmi_parents,
				 ARRAY_SIZE(hdmi_parents),
				 APMU_HDMI_CLK_RES_CTRL,
				 APMU_HDMI_CLK_RES_CTRL, 1, 4, BIT(29), 5,
				 3, BIT(0), 0);

CCU_GATE_DEFINE(CLK_PCIE0_MASTER, pcie0_master_clk, pcie0_master_clk,
		"pmua_aclk", APMU_PCIE_CLK_RES_CTRL_0, BIT(2), 0);
CCU_GATE_DEFINE(CLK_PCIE0_SLAVE, pcie0_slave_clk, pcie0_slave_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_0, BIT(1), 0);
CCU_GATE_DEFINE(CLK_PCIE0_DBI, pcie0_dbi_clk, pcie0_dbi_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_0, BIT(0), 0);

CCU_GATE_DEFINE(CLK_PCIE1_MASTER, pcie1_master_clk, pcie1_master_clk,
		"pmua_aclk", APMU_PCIE_CLK_RES_CTRL_1, BIT(2), 0);
CCU_GATE_DEFINE(CLK_PCIE1_SLAVE, pcie1_slave_clk, pcie1_slave_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_1, BIT(1), 0);
CCU_GATE_DEFINE(CLK_PCIE1_DBI, pcie1_dbi_clk, pcie1_dbi_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_1, BIT(0), 0);

CCU_GATE_DEFINE(CLK_PCIE2_MASTER, pcie2_master_clk, pcie2_master_clk,
		"pmua_aclk", APMU_PCIE_CLK_RES_CTRL_2, BIT(2), 0);
CCU_GATE_DEFINE(CLK_PCIE2_SLAVE, pcie2_slave_clk, pcie2_slave_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_2, BIT(1), 0);
CCU_GATE_DEFINE(CLK_PCIE2_DBI, pcie2_dbi_clk, pcie2_dbi_clk, "pmua_aclk",
		APMU_PCIE_CLK_RES_CTRL_2, BIT(0), 0);

CCU_GATE_DEFINE(CLK_EMAC0_BUS, emac0_bus_clk, emac0_bus_clk, "pmua_aclk",
		APMU_EMAC0_CLK_RES_CTRL, BIT(0), 0);
CCU_GATE_DEFINE(CLK_EMAC0_PTP, emac0_ptp_clk, emac0_ptp_clk, "pll2_d6",
		APMU_EMAC0_CLK_RES_CTRL, BIT(15), 0);
CCU_GATE_DEFINE(CLK_EMAC1_BUS, emac1_bus_clk, emac1_bus_clk, "pmua_aclk",
		APMU_EMAC1_CLK_RES_CTRL, BIT(0), 0);
CCU_GATE_DEFINE(CLK_EMAC1_PTP, emac1_ptp_clk, emac1_ptp_clk, "pll2_d6",
		APMU_EMAC1_CLK_RES_CTRL, BIT(15), 0);

#endif
/* APMU clocks end */

static struct clk *k1_ccu_pll_clks[] = {
	&pll1.common.clk,
	&pll2.common.clk,
	&pll3.common.clk,
	&pll1_d2.common.clk,
	&pll1_d3.common.clk,
	&pll1_d4.common.clk,
	&pll1_d5.common.clk,
	&pll1_d6.common.clk,
	&pll1_d7.common.clk,
	&pll1_d8.common.clk,
	&pll1_d11_223p4.common.clk,
	&pll1_d13_189.common.clk,
	&pll1_d23_106p8.common.clk,
	&pll1_d64_38p4.common.clk,
	&pll1_aud_245p7.common.clk,
	&pll1_aud_24p5.common.clk,
	&pll2_d1.common.clk,
	&pll2_d2.common.clk,
	&pll2_d3.common.clk,
	&pll2_d4.common.clk,
	&pll2_d5.common.clk,
	&pll2_d6.common.clk,
	&pll2_d7.common.clk,
	&pll2_d8.common.clk,
	&pll3_d1.common.clk,
	&pll3_d2.common.clk,
	&pll3_d3.common.clk,
	&pll3_d4.common.clk,
	&pll3_d5.common.clk,
	&pll3_d6.common.clk,
	&pll3_d7.common.clk,
	&pll3_d8.common.clk,
	&pll3_80.common.clk,
	&pll3_40.common.clk,
	&pll3_20.common.clk,
};

static const struct spacemit_ccu_data k1_ccu_pll_data = {
	.clks		= k1_ccu_pll_clks,
	.num		= ARRAY_SIZE(k1_ccu_pll_clks),
	.offset		= K1_PLL_ID,
};

#if IS_ENABLED(CONFIG_SPL_BUILD)
static struct clk *k1_ccu_mpmu_clks[] = {
	&pll1_d8_307p2.common.clk,
	&pll1_d32_76p8.common.clk,
	&pll1_d40_61p44.common.clk,
	&pll1_d16_153p6.common.clk,
	&pll1_d24_102p4.common.clk,
	&pll1_d48_51p2.common.clk,
	&pll1_d48_51p2_ap.common.clk,
	&pll1_m3d128_57p6.common.clk,
	&pll1_d96_25p6.common.clk,
	&pll1_d192_12p8.common.clk,
	&pll1_d192_12p8_wdt.common.clk,
	&pll1_d384_6p4.common.clk,
	&pll1_d768_3p2.common.clk,
	&pll1_d1536_1p6.common.clk,
	&pll1_d3072_0p8.common.clk,
	&pll1_d6_409p6.common.clk,
	&pll1_d12_204p8.common.clk,
	&pll1_d5_491p52.common.clk,
	&pll1_d10_245p76.common.clk,
	&pll1_d4_614p4.common.clk,
	&pll1_d52_47p26.common.clk,
	&pll1_d78_31p5.common.clk,
	&pll1_d3_819p2.common.clk,
	&pll1_d2_1228p8.common.clk,
	&slow_uart.common.clk,
	&slow_uart1_14p74.common.clk,
	&slow_uart2_48.common.clk,
};
#else
static struct clk *k1_ccu_mpmu_clks[] = {
	&pll1_d8_307p2.common.clk,
	&pll1_d32_76p8.common.clk,
	&pll1_d40_61p44.common.clk,
	&pll1_d16_153p6.common.clk,
	&pll1_d24_102p4.common.clk,
	&pll1_d48_51p2.common.clk,
	&pll1_d48_51p2_ap.common.clk,
	&pll1_m3d128_57p6.common.clk,
	&pll1_d96_25p6.common.clk,
	&pll1_d192_12p8.common.clk,
	&pll1_d192_12p8_wdt.common.clk,
	&pll1_d384_6p4.common.clk,
	&pll1_d768_3p2.common.clk,
	&pll1_d1536_1p6.common.clk,
	&pll1_d3072_0p8.common.clk,
	&pll1_d6_409p6.common.clk,
	&pll1_d12_204p8.common.clk,
	&pll1_d5_491p52.common.clk,
	&pll1_d10_245p76.common.clk,
	&pll1_d4_614p4.common.clk,
	&pll1_d52_47p26.common.clk,
	&pll1_d78_31p5.common.clk,
	&pll1_d3_819p2.common.clk,
	&pll1_d2_1228p8.common.clk,
	&slow_uart.common.clk,
	&slow_uart1_14p74.common.clk,
	&slow_uart2_48.common.clk,
	&wdt_clk.common.clk,
	&apb_clk.common.clk,
	&ripc_clk.common.clk,
	&i2s_153p6.common.clk,
	&i2s_153p6_base.common.clk,
	&i2s_sysclk_src.common.clk,
	&i2s_sysclk.common.clk,
	&i2s_bclk_factor.common.clk,
	&i2s_bclk.common.clk,
	&wdt_bus_clk.common.clk,
};
#endif

static const struct spacemit_ccu_data k1_ccu_mpmu_data = {
	.clks		= k1_ccu_mpmu_clks,
	.num		= ARRAY_SIZE(k1_ccu_mpmu_clks),
	.offset		= K1_MPMU_ID,
};

#if IS_ENABLED(CONFIG_SPL_BUILD)
static struct clk *k1_ccu_apbc_clks[] = {
	&uart0_clk.common.clk,
	&twsi2_clk.common.clk,
	&twsi8_clk.common.clk,
};
#else
static struct clk *k1_ccu_apbc_clks[] = {
	&uart0_clk.common.clk,
	&uart2_clk.common.clk,
	&uart3_clk.common.clk,
	&uart4_clk.common.clk,
	&uart5_clk.common.clk,
	&uart6_clk.common.clk,
	&uart7_clk.common.clk,
	&uart8_clk.common.clk,
	&uart9_clk.common.clk,
	&gpio_clk.common.clk,
	&pwm0_clk.common.clk,
	&pwm1_clk.common.clk,
	&pwm2_clk.common.clk,
	&pwm3_clk.common.clk,
	&pwm4_clk.common.clk,
	&pwm5_clk.common.clk,
	&pwm6_clk.common.clk,
	&pwm7_clk.common.clk,
	&pwm8_clk.common.clk,
	&pwm9_clk.common.clk,
	&pwm10_clk.common.clk,
	&pwm11_clk.common.clk,
	&pwm12_clk.common.clk,
	&pwm13_clk.common.clk,
	&pwm14_clk.common.clk,
	&pwm15_clk.common.clk,
	&pwm16_clk.common.clk,
	&pwm17_clk.common.clk,
	&pwm18_clk.common.clk,
	&pwm19_clk.common.clk,
	&ssp3_clk.common.clk,
	&rtc_clk.common.clk,
	&twsi0_clk.common.clk,
	&twsi1_clk.common.clk,
	&twsi2_clk.common.clk,
	&twsi4_clk.common.clk,
	&twsi5_clk.common.clk,
	&twsi6_clk.common.clk,
	&twsi7_clk.common.clk,
	&twsi8_clk.common.clk,
	&timers1_clk.common.clk,
	&timers2_clk.common.clk,
	&aib_clk.common.clk,
	&onewire_clk.common.clk,
	&sspa0_clk.common.clk,
	&sspa1_clk.common.clk,
	&dro_clk.common.clk,
	&ir_clk.common.clk,
	&tsen_clk.common.clk,
	&ipc_ap2aud_clk.common.clk,
	&can0_clk.common.clk,
	&can0_bus_clk.common.clk,
	&uart0_bus_clk.common.clk,
	&uart2_bus_clk.common.clk,
	&uart3_bus_clk.common.clk,
	&uart4_bus_clk.common.clk,
	&uart5_bus_clk.common.clk,
	&uart6_bus_clk.common.clk,
	&uart7_bus_clk.common.clk,
	&uart8_bus_clk.common.clk,
	&uart9_bus_clk.common.clk,
	&gpio_bus_clk.common.clk,
	&pwm0_bus_clk.common.clk,
	&pwm1_bus_clk.common.clk,
	&pwm2_bus_clk.common.clk,
	&pwm3_bus_clk.common.clk,
	&pwm4_bus_clk.common.clk,
	&pwm5_bus_clk.common.clk,
	&pwm6_bus_clk.common.clk,
	&pwm7_bus_clk.common.clk,
	&pwm8_bus_clk.common.clk,
	&pwm9_bus_clk.common.clk,
	&pwm10_bus_clk.common.clk,
	&pwm11_bus_clk.common.clk,
	&pwm12_bus_clk.common.clk,
	&pwm13_bus_clk.common.clk,
	&pwm14_bus_clk.common.clk,
	&pwm15_bus_clk.common.clk,
	&pwm16_bus_clk.common.clk,
	&pwm17_bus_clk.common.clk,
	&pwm18_bus_clk.common.clk,
	&pwm19_bus_clk.common.clk,
	&ssp3_bus_clk.common.clk,
	&rtc_bus_clk.common.clk,
	&twsi0_bus_clk.common.clk,
	&twsi1_bus_clk.common.clk,
	&twsi2_bus_clk.common.clk,
	&twsi4_bus_clk.common.clk,
	&twsi5_bus_clk.common.clk,
	&twsi6_bus_clk.common.clk,
	&twsi7_bus_clk.common.clk,
	&twsi8_bus_clk.common.clk,
	&timers1_bus_clk.common.clk,
	&timers2_bus_clk.common.clk,
	&aib_bus_clk.common.clk,
	&onewire_bus_clk.common.clk,
	&sspa0_bus_clk.common.clk,
	&sspa1_bus_clk.common.clk,
	&tsen_bus_clk.common.clk,
	&ipc_ap2aud_bus_clk.common.clk,
	&sspa0_i2s_bclk.common.clk,
	&sspa1_i2s_bclk.common.clk,
};
#endif

static const struct spacemit_ccu_data k1_ccu_apbc_data = {
	.clks		= k1_ccu_apbc_clks,
	.num		= ARRAY_SIZE(k1_ccu_apbc_clks),
	.offset		= K1_APBC_ID,
};

#if IS_ENABLED(CONFIG_SPL_BUILD)
static struct clk *k1_ccu_apmu_clks[] = {
	&emmc_clk.common.clk,
	&emmc_x_clk.common.clk,
	&pmua_aclk.common.clk,
	&emmc_bus_clk.common.clk,
};
#else
static struct clk *k1_ccu_apmu_clks[] = {
	&cci550_clk.common.clk,
	&cpu_c0_hi_clk.common.clk,
	&cpu_c0_core_clk.common.clk,
	&cpu_c0_ace_clk.common.clk,
	&cpu_c0_tcm_clk.common.clk,
	&cpu_c1_hi_clk.common.clk,
	&cpu_c1_core_clk.common.clk,
	&cpu_c1_ace_clk.common.clk,
	&ccic_4x_clk.common.clk,
	&ccic1phy_clk.common.clk,
	&sdh_axi_aclk.common.clk,
	&sdh0_clk.common.clk,
	&sdh1_clk.common.clk,
	&sdh2_clk.common.clk,
	&usb_p1_aclk.common.clk,
	&usb_axi_clk.common.clk,
	&usb30_clk.common.clk,
	&qspi_clk.common.clk,
	&qspi_bus_clk.common.clk,
	&dma_clk.common.clk,
	&aes_clk.common.clk,
	&vpu_clk.common.clk,
	&gpu_clk.common.clk,
	&emmc_clk.common.clk,
	&emmc_x_clk.common.clk,
	&audio_clk.common.clk,
	&hdmi_mclk.common.clk,
	&pmua_aclk.common.clk,
	&pcie0_master_clk.common.clk,
	&pcie0_slave_clk.common.clk,
	&pcie0_dbi_clk.common.clk,
	&pcie1_master_clk.common.clk,
	&pcie1_slave_clk.common.clk,
	&pcie1_dbi_clk.common.clk,
	&pcie2_master_clk.common.clk,
	&pcie2_slave_clk.common.clk,
	&pcie2_dbi_clk.common.clk,
	&emac0_bus_clk.common.clk,
	&emac0_ptp_clk.common.clk,
	&emac1_bus_clk.common.clk,
	&emac1_ptp_clk.common.clk,
	&jpg_clk.common.clk,
	&ccic2phy_clk.common.clk,
	&ccic3phy_clk.common.clk,
	&csi_clk.common.clk,
	&camm0_clk.common.clk,
	&camm1_clk.common.clk,
	&camm2_clk.common.clk,
	&isp_cpp_clk.common.clk,
	&isp_bus_clk.common.clk,
	&isp_clk.common.clk,
	&dpu_mclk.common.clk,
	&dpu_esc_clk.common.clk,
	&dpu_bit_clk.common.clk,
	&dpu_pxclk.common.clk,
	&dpu_hclk.common.clk,
	&dpu_spi_clk.common.clk,
	&dpu_spi_hbus_clk.common.clk,
	&dpu_spi_bus_clk.common.clk,
	&dpu_spi_aclk.common.clk,
	&v2d_clk.common.clk,
	&emmc_bus_clk.common.clk,
};
#endif

static int clk_k1_enable(struct clk *clk)
{
	const struct spacemit_ccu_data *data;
	struct clk *c;
	struct clk *pclk;
	int ret, i;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(clk->dev);
	for (i = 0; i < data->num; i++) {
		if (clk->id == data->clks[i]->id) {
			c = data->clks[i];
			break;
		}
	}
	if (i == data->num)
		c = clk;

	pclk = clk_get_parent(c);
	if (!IS_ERR_OR_NULL(pclk)) {
		ret = ccf_clk_enable(pclk);
		if (ret)
			return ret;
	}
	ret = ccu_gate_enable(c);
	return ret;
}

static int clk_k1_disable(struct clk *clk)
{
	const struct spacemit_ccu_data *data;
	struct clk *c;
	struct clk *pclk;
	int ret, i;

	data = (struct spacemit_ccu_data *)dev_get_driver_data(clk->dev);
	for (i = 0; i < data->num; i++) {
		if (clk->id == data->clks[i]->id) {
			c = data->clks[i];
			break;
		}
	}
	if (i == data->num)
		c = clk;

	pclk = clk_get_parent(c);
	if (!IS_ERR_OR_NULL(pclk)) {
		ret = ccf_clk_disable(pclk);
		if (ret)
			return ret;
	}
	ret = ccu_gate_disable(c);
	return ret;
}

#define K1_CLK_OPS(name)				\
static const struct clk_ops k1_##name##_clk_ops = {	\
		.set_rate = ccf_clk_set_rate,		\
		.get_rate = ccf_clk_get_rate,		\
		.enable = clk_k1_enable,		\
		.disable = clk_k1_disable,		\
		.set_parent = ccf_clk_set_parent,	\
		.of_xlate = k1_##name##_clk_of_xlate,	\
}

static const struct spacemit_ccu_data k1_ccu_apmu_data = {
	.clks		= k1_ccu_apmu_clks,
	.num		= ARRAY_SIZE(k1_ccu_apmu_clks),
	.offset		= K1_APMU_ID,
};

struct clk_retry_item {
	struct ccu_common *common;
	struct list_head link;
};

static LIST_HEAD(retry_list);

static int k1_clk_retry_register(void)
{
	struct clk_retry_item *item, *tmp;
	int retries = 5;
	int ret;

	while (!list_empty(&retry_list) && retries) {
		list_for_each_entry_safe(item, tmp, &retry_list, link) {
			struct ccu_common *common = item->common;

			ret = common->init(common);
			if (ret)
				return ret;

			list_del(&item->link);
			kfree(item);
		}
		retries--;
	}

	return 0;
}

static int k1_clk_register(struct udevice *dev, struct regmap *regmap,
			   struct regmap *lock_regmap,
			   const struct spacemit_ccu_data *data)
{
	int i, ret;

	for (i = 0; i < data->num; i++) {
		struct clk *clk = data->clks[i];
		struct ccu_common *common;

		if (!clk)
			continue;

		common = clk_to_ccu_common(clk);
		common->regmap = regmap;
		common->lock_regmap = lock_regmap;

		clk->id = common->clk.id + data->offset;

		ret = common->init(common);
		if (ret)
			return ret;
	}

	return 0;
}

static int k1_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap, *lock_regmap = NULL;
	const struct spacemit_ccu_data *data;
	int ret;

	clk_register_fixed_rate(NULL, "clock-1m", 1000000);
	clk_register_fixed_rate(NULL, "clock-24m", 24000000);
	clk_register_fixed_rate(NULL, "clock-3m", 3000000);
	clk_register_fixed_rate(NULL, "clock-32k", 32000);

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	/*
	 * The lock status of PLLs locate in MPMU region, while PLLs themselves
	 * are in APBS region. Reference to MPMU syscon is required to check PLL
	 * status.
	 */
	if (device_is_compatible(dev, "spacemit,k1-pll")) {
		struct ofnode_phandle_args mpmu_args;

		ret = dev_read_phandle_with_args(dev, "spacemit,mpmu", NULL, 0, 0,
						 &mpmu_args);
		if (ret)
			return ret;

		ret = regmap_init_mem(mpmu_args.node, &lock_regmap);
		if (ret)
			return ret;
	}

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k1_clk_register(dev, base_regmap, lock_regmap, data);
	if (ret)
		return -EPROBE_DEFER;

	return k1_clk_retry_register();
}

static int k1_apbc_clk_probe(struct udevice *dev)
{
	struct regmap *base_regmap, *lock_regmap = NULL;
	const struct spacemit_ccu_data *data;
	int ret;
	struct clk clk;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	clk_register_fixed_rate(NULL, "clock-1m", 1000000);
	clk_register_fixed_rate(NULL, "clock-24m", 24000000);
	clk_register_fixed_rate(NULL, "clock-3m", 3000000);
	clk_register_fixed_rate(NULL, "clock-32k", 32000);

	/* probe PLL controller */
	ret = clk_get_by_index(dev, 5, &clk);
	if (ret)
		return -EPROBE_DEFER;

	/* probe MPMU controller */
	ret = clk_get_by_index(dev, 4, &clk);
	if (ret)
		return -EPROBE_DEFER;

	ret = regmap_init_mem(dev_ofnode(dev), &base_regmap);
	if (ret)
		return ret;

	/*
	 * The lock status of PLLs locate in MPMU region, while PLLs themselves
	 * are in APBS region. Reference to MPMU syscon is required to check PLL
	 * status.
	 */
	if (device_is_compatible(dev, "spacemit,k1-pll")) {
		struct ofnode_phandle_args mpmu_args;

		ret = dev_read_phandle_with_args(dev, "spacemit,mpmu", NULL, 0, 0,
						 &mpmu_args);
		if (ret)
			return ret;

		ret = regmap_init_mem(mpmu_args.node, &lock_regmap);
		if (ret)
			return ret;
	}

	data = (struct spacemit_ccu_data *)dev_get_driver_data(dev);

	ret = k1_clk_register(dev, base_regmap, lock_regmap, data);
	if (ret)
		return -EPROBE_DEFER;

	return k1_clk_retry_register();
}

static int k1_pll_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K1_PLL_ID + args->args[0];
	else
		clk->id = K1_PLL_ID;

	return 0;
}

static int k1_mpmu_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K1_MPMU_ID + args->args[0];
	else
		clk->id = K1_MPMU_ID;

	return 0;
}

static int k1_apbc_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K1_APBC_ID + args->args[0];
	else
		clk->id = K1_APBC_ID;

	return 0;
}

static int k1_apmu_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = K1_APMU_ID + args->args[0];
	else
		clk->id = K1_APMU_ID;

	return 0;
}

static const struct udevice_id k1_pll_clk_match[] = {
	{ .compatible = "spacemit,k1-pll",
	      .data = (ulong)&k1_ccu_pll_data },
	{ /* sentinel */ },
};

K1_CLK_OPS(pll);

U_BOOT_DRIVER(k1_pll_clk) = {
	.name		= "k1_pll_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_pll_clk_match,
	.probe		= k1_clk_probe,
	.ops		= &k1_pll_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_mpmu_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-mpmu",
	  .data = (ulong)&k1_ccu_mpmu_data },
	{ /* sentinel */ },
};

K1_CLK_OPS(mpmu);

U_BOOT_DRIVER(k1_mpmu_clk) = {
	.name		= "k1_mpmu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_mpmu_clk_match,
	.probe		= k1_clk_probe,
	.ops		= &k1_mpmu_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_apbc_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-apbc",
	  .data = (ulong)&k1_ccu_apbc_data },
	{ /* sentinel */ },
};

K1_CLK_OPS(apbc);

U_BOOT_DRIVER(k1_apbc_clk) = {
	.name		= "k1_apbc_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_apbc_clk_match,
	.probe		= k1_apbc_clk_probe,
	.ops		= &k1_apbc_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_apmu_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-apmu",
	  .data = (ulong)&k1_ccu_apmu_data },
	{ /* sentinel */ },
};

K1_CLK_OPS(apmu);

U_BOOT_DRIVER(k1_apmu_clk) = {
	.name		= "k1_apmu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_apmu_clk_match,
	.probe		= k1_clk_probe,
	.ops		= &k1_apmu_clk_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_rcpu_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-rcpu" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(k1_rcpu_clk) = {
	.name		= "k1_rcpu_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_rcpu_clk_match,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_rcpu2_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-rcpu2" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(k1_rcpu2_clk) = {
	.name		= "k1_rcpu2_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_rcpu2_clk_match,
	.flags		= DM_FLAG_PRE_RELOC,
};

static const struct udevice_id k1_apbc2_clk_match[] = {
	{ .compatible = "spacemit,k1-syscon-apbc2" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(k1_apbc2_clk) = {
	.name		= "k1_apbc2_clk",
	.id		= UCLASS_CLK,
	.of_match	= k1_apbc2_clk_match,
	.flags		= DM_FLAG_PRE_RELOC,
};
