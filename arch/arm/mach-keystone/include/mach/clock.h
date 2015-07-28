/*
 * keystone2: common clock header file
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#ifndef __ASSEMBLY__

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/clock-k2hk.h>
#endif

#ifdef CONFIG_SOC_K2E
#include <asm/arch/clock-k2e.h>
#endif

#ifdef CONFIG_SOC_K2L
#include <asm/arch/clock-k2l.h>
#endif

#define CORE_PLL MAIN_PLL
#define DDR3_PLL DDR3A_PLL

#include <asm/types.h>

#define GENERATE_ENUM(NUM, ENUM) ENUM = NUM,
#define GENERATE_INDX_STR(NUM, STRING) #NUM"\t- "#STRING"\n"
#define CLOCK_INDEXES_LIST	CLK_LIST(GENERATE_INDX_STR)

enum {
	SPD800,
	SPD850,
	SPD1000,
	SPD1200,
	SPD1250,
	SPD1350,
	SPD1400,
	SPD1500,
	NUM_SPDS,
};

/* PLL identifiers */
enum {
	MAIN_PLL,
	TETRIS_PLL,
	PASS_PLL,
	DDR3A_PLL,
	DDR3B_PLL,
	MAX_PLL_COUNT,
};

enum ext_clk_e {
	sys_clk,
	alt_core_clk,
	pa_clk,
	tetris_clk,
	ddr3a_clk,
	ddr3b_clk,
	ext_clk_count /* number of external clocks */
};

enum clk_e {
	CLK_LIST(GENERATE_ENUM)
};

struct keystone_pll_regs {
	u32 reg0;
	u32 reg1;
};

/* PLL configuration data */
struct pll_init_data {
	int pll;
	int pll_m;		/* PLL Multiplier */
	int pll_d;		/* PLL divider */
	int pll_od;		/* PLL output divider */
};

extern unsigned int external_clk[ext_clk_count];
extern const struct keystone_pll_regs keystone_pll_regs[];
extern s16 divn_val[];
extern int speeds[];

void init_plls(void);
void init_pll(const struct pll_init_data *data);
struct pll_init_data *get_pll_init_data(int pll);
unsigned long clk_get_rate(unsigned int clk);
unsigned long clk_round_rate(unsigned int clk, unsigned long hz);
int clk_set_rate(unsigned int clk, unsigned long hz);
int get_max_dev_speed(void);
int get_max_arm_speed(void);

#endif
#endif
