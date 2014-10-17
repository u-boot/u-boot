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

#define MAIN_PLL CORE_PLL

#include <asm/types.h>

#define GENERATE_ENUM(NUM, ENUM) ENUM = NUM,
#define GENERATE_INDX_STR(NUM, STRING) #NUM"\t- "#STRING"\n"
#define CLOCK_INDEXES_LIST	CLK_LIST(GENERATE_INDX_STR)

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

extern const struct keystone_pll_regs keystone_pll_regs[];
extern int dev_speeds[];
extern int arm_speeds[];

void init_plls(int num_pll, struct pll_init_data *config);
void init_pll(const struct pll_init_data *data);
unsigned long clk_get_rate(unsigned int clk);
unsigned long clk_round_rate(unsigned int clk, unsigned long hz);
int clk_set_rate(unsigned int clk, unsigned long hz);
void pass_pll_pa_clk_enable(void);
int get_max_dev_speed(void);
int get_max_arm_speed(void);

#endif
#endif
