/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
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
	bool dram_ddr3plus;
	struct uniphier_dram_ch dram_ch[UNIPHIER_MAX_NR_DRAM_CH];
};

const struct uniphier_board_data *uniphier_get_board_param(void);

int uniphier_sld3_init(const struct uniphier_board_data *bd);
int uniphier_ld4_init(const struct uniphier_board_data *bd);
int uniphier_pro4_init(const struct uniphier_board_data *bd);
int uniphier_sld8_init(const struct uniphier_board_data *bd);
int uniphier_pro5_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_init(const struct uniphier_board_data *bd);
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

int uniphier_sld3_pll_init(const struct uniphier_board_data *bd);
int uniphier_ld4_pll_init(const struct uniphier_board_data *bd);
int uniphier_pro4_pll_init(const struct uniphier_board_data *bd);
int uniphier_sld8_pll_init(const struct uniphier_board_data *bd);

int uniphier_sld3_enable_dpll_ssc(const struct uniphier_board_data *bd);
int uniphier_ld4_enable_dpll_ssc(const struct uniphier_board_data *bd);

int uniphier_ld4_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_pro5_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_early_clk_init(const struct uniphier_board_data *bd);
int uniphier_ld20_early_clk_init(const struct uniphier_board_data *bd);

int uniphier_sld3_early_pin_init(const struct uniphier_board_data *bd);
int uniphier_ld20_early_pin_init(const struct uniphier_board_data *bd);

int uniphier_ld4_umc_init(const struct uniphier_board_data *bd);
int uniphier_pro4_umc_init(const struct uniphier_board_data *bd);
int uniphier_sld8_umc_init(const struct uniphier_board_data *bd);
int uniphier_pxs2_umc_init(const struct uniphier_board_data *bd);
int uniphier_ld20_umc_init(const struct uniphier_board_data *bd);

void uniphier_sld3_pin_init(void);
void uniphier_ld4_pin_init(void);
void uniphier_pro4_pin_init(void);
void uniphier_sld8_pin_init(void);
void uniphier_pro5_pin_init(void);
void uniphier_pxs2_pin_init(void);
void uniphier_ld6b_pin_init(void);
void uniphier_ld20_pin_init(void);

void uniphier_ld4_clk_init(void);
void uniphier_pro4_clk_init(void);
void uniphier_pro5_clk_init(void);
void uniphier_pxs2_clk_init(void);
void uniphier_ld20_clk_init(void);

void cci500_init(int nr_slaves);

#define pr_err(fmt, args...)	printf(fmt, ##args)

#endif /* __MACH_INIT_H */
