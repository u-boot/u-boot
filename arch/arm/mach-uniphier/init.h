/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MACH_INIT_H
#define __MACH_INIT_H

#include <linux/types.h>

#define UNIPHIER_MAX_NR_DRAM_CH		3

struct uniphier_dram_ch {
	unsigned long base;
	unsigned long size;
	unsigned int width;
};

struct uniphier_board_data {
	unsigned int dram_freq;
	unsigned int dram_nr_ch;
	struct uniphier_dram_ch dram_ch[UNIPHIER_MAX_NR_DRAM_CH];
	unsigned int flags;

#define UNIPHIER_BD_DDR3PLUS			BIT(2)

#define UNIPHIER_BD_BOARD_GET_TYPE(f)		((f) & 0x7)
#define UNIPHIER_BD_BOARD_LD20_REF		0	/* LD20 reference */
#define UNIPHIER_BD_BOARD_LD20_GLOBAL		1	/* LD20 TV Set */
#define UNIPHIER_BD_BOARD_LD20_C1		2	/* LD20 TV Set C1 */
#define UNIPHIER_BD_BOARD_LD21_REF		3	/* LD21 reference */
#define UNIPHIER_BD_BOARD_LD21_GLOBAL		4	/* LD21 TV Set */
};

const struct uniphier_board_data *uniphier_get_board_param(void);

int uniphier_sld3_init(const struct uniphier_board_data *bd);
int uniphier_ld4_init(const struct uniphier_board_data *bd);
int uniphier_pro4_init(const struct uniphier_board_data *bd);
int uniphier_sld8_init(const struct uniphier_board_data *bd);
int uniphier_pro5_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_init(const struct uniphier_board_data *bd);
int uniphier_ld11_init(const struct uniphier_board_data *bd);
int uniphier_ld20_init(const struct uniphier_board_data *bd);

#if defined(CONFIG_MICRO_SUPPORT_CARD)
int uniphier_sbc_init_admulti(const struct uniphier_board_data *bd);
int uniphier_sbc_init_savepin(const struct uniphier_board_data *bd);
int uniphier_ld4_sbc_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_sbc_init(const struct uniphier_board_data *bd);
#else
static inline int uniphier_sbc_init_admulti(
					const struct uniphier_board_data *bd)
{
	return 0;
}

static inline int uniphier_sbc_init_savepin(
					const struct uniphier_board_data *bd)
{
	return 0;
}

static inline int uniphier_ld4_sbc_init(const struct uniphier_board_data *bd)
{
	return 0;
}

static inline int uniphier_pxs2_sbc_init(const struct uniphier_board_data *bd)
{
	return 0;
}
#endif

int uniphier_sld3_bcu_init(const struct uniphier_board_data *bd);
int uniphier_ld4_bcu_init(const struct uniphier_board_data *bd);

int memconf_init(const struct uniphier_board_data *bd);
int uniphier_sld3_memconf_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_memconf_init(const struct uniphier_board_data *bd);

int uniphier_sld3_dpll_init(const struct uniphier_board_data *bd);
int uniphier_ld4_dpll_init(const struct uniphier_board_data *bd);
int uniphier_pro4_dpll_init(const struct uniphier_board_data *bd);
int uniphier_sld8_dpll_init(const struct uniphier_board_data *bd);
int uniphier_ld11_dpll_init(const struct uniphier_board_data *bd);
int uniphier_ld20_dpll_init(const struct uniphier_board_data *bd);

int uniphier_ld4_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_pro5_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_ld11_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_ld20_early_clk_init(const struct uniphier_board_data *bd);

int uniphier_ld4_umc_init(const struct uniphier_board_data *bd);
int uniphier_pro4_umc_init(const struct uniphier_board_data *bd);
int uniphier_sld8_umc_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_umc_init(const struct uniphier_board_data *bd);
int uniphier_ld20_umc_init(const struct uniphier_board_data *bd);
int uniphier_ld11_umc_init(const struct uniphier_board_data *bd);

void uniphier_sld3_pll_init(void);
void uniphier_ld4_pll_init(void);
void uniphier_pro4_pll_init(void);
void uniphier_ld11_pll_init(void);
int uniphier_ld20_pll_init(const struct uniphier_board_data *bd);

void uniphier_ld4_clk_init(void);
void uniphier_pro4_clk_init(void);
void uniphier_pro5_clk_init(void);
void uniphier_pxs2_clk_init(void);
void uniphier_ld11_clk_init(void);
void uniphier_ld20_clk_init(void);

int uniphier_pin_init(const char *pinconfig_name);
void uniphier_smp_kick_all_cpus(void);
void cci500_init(int nr_slaves);

#define pr_err(fmt, args...)	printf(fmt, ##args)

#endif /* __MACH_INIT_H */
