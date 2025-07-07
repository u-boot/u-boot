/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2017 Jorge Ramirez-Ortiz <jorge.ramirez-ortiz@linaro.org>
 */
#ifndef _CLOCK_QCOM_H
#define _CLOCK_QCOM_H

#include <asm/io.h>
#include <linux/bitfield.h>
#include <errno.h>

#define CFG_CLK_SRC_CXO   (0 << 8)
#define CFG_CLK_SRC_GPLL0 (1 << 8)
#define CFG_CLK_SRC_GPLL0_AUX2 (2 << 8)
#define CFG_CLK_SRC_GPLL2 (2 << 8)
#define CFG_CLK_SRC_GPLL2_MAIN (2 << 8)
#define CFG_CLK_SRC_GPLL9 (2 << 8)
#define CFG_CLK_SRC_GPLL0_ODD (3 << 8)
#define CFG_CLK_SRC_GPLL6 (4 << 8)
#define CFG_CLK_SRC_GPLL7 (3 << 8)
#define CFG_CLK_SRC_GPLL4 (5 << 8)
#define CFG_CLK_SRC_GPLL0_EVEN (6 << 8)
#define CFG_CLK_SRC_MASK  (7 << 8)

#define RCG_CFG_REG		0x4
#define RCG_M_REG		0x8
#define RCG_N_REG		0xc
#define RCG_D_REG		0x10

struct pll_vote_clk {
	uintptr_t status;
	int status_bit;
	uintptr_t ena_vote;
	int vote_bit;
};

struct vote_clk {
	uintptr_t cbcr_reg;
	uintptr_t ena_vote;
	int vote_bit;
};

struct freq_tbl {
	uint freq;
	uint src;
	u8 pre_div;
	u16 m;
	u16 n;
};

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

struct gate_clk {
	uintptr_t reg;
	u32 en_val;
	uintptr_t cbcr_reg;
	const char *name;
};

/*
 * GATE_CLK() is deprecated: Use GATE_CLK_POLLED() instead to ensure the clock
 * is running before we start making use of devices or registers.
 */
#ifdef DEBUG
#define GATE_CLK(clk, reg, val) [clk] = { reg, val, 0, #clk }
#define GATE_CLK_POLLED(clk, en_reg, val, cbcr_reg) [clk] = { en_reg, val, cbcr_reg, #clk }
#else
#define GATE_CLK(clk, reg, val) [clk] = { reg, val, 0, NULL }
#define GATE_CLK_POLLED(clk, en_reg, val, cbcr_reg) [clk] = { en_reg, val, cbcr_reg, NULL }
#endif

struct qcom_reset_map {
	unsigned int reg;
	u8 bit;
};

struct qcom_power_map {
	unsigned int reg;
};

struct clk;

struct msm_clk_data {
	const struct qcom_power_map	*power_domains;
	unsigned long			num_power_domains;
	const struct qcom_reset_map	*resets;
	unsigned long			num_resets;
	const struct gate_clk		*clks;
	unsigned long			num_clks;

	const phys_addr_t		*dbg_pll_addrs;
	unsigned long			num_plls;
	const phys_addr_t		*dbg_rcg_addrs;
	unsigned long			num_rcgs;
	const char * const		*dbg_rcg_names;

	int (*enable)(struct clk *clk);
	unsigned long (*set_rate)(struct clk *clk, unsigned long rate);
};

struct msm_clk_priv {
	phys_addr_t		base;
	struct msm_clk_data	*data;
};

int qcom_cc_bind(struct udevice *parent);
void clk_enable_gpll0(phys_addr_t base, const struct pll_vote_clk *gpll0);
void clk_bcr_update(phys_addr_t apps_cmd_rgcr);
void clk_enable_cbc(phys_addr_t cbcr);
void clk_enable_vote_clk(phys_addr_t base, const struct vote_clk *vclk);
const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f, uint rate);
void clk_rcg_set_rate_mnd(phys_addr_t base, uint32_t cmd_rcgr,
			  int div, int m, int n, int source, u8 mnd_width);
void clk_rcg_set_rate(phys_addr_t base, uint32_t cmd_rcgr, int div,
		      int source);
void clk_phy_mux_enable(phys_addr_t base, uint32_t cmd_rcgr, bool enabled);

int qcom_gate_clk_en(const struct msm_clk_priv *priv, unsigned long id);

#endif
