/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Renesas RCar Gen3 CPG MSSR driver
 *
 * Copyright (C) 2017-2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on the following driver from Linux kernel:
 * r8a7796 Clock Pulse Generator / Module Standby and Software Reset
 *
 * Copyright (C) 2016 Glider bvba
 */

#ifndef __DRIVERS_CLK_RENESAS_CPG_MSSR__
#define __DRIVERS_CLK_RENESAS_CPG_MSSR__

#include <linux/bitops.h>

enum clk_reg_layout {
	CLK_REG_LAYOUT_RCAR_GEN2_AND_GEN3 = 0,
};

struct cpg_mssr_info {
	const struct cpg_core_clk	*core_clk;
	unsigned int			core_clk_size;
	enum clk_reg_layout		reg_layout;
	const struct mssr_mod_clk	*mod_clk;
	unsigned int			mod_clk_size;
	const struct mstp_stop_table	*mstp_table;
	unsigned int			mstp_table_size;
	const char			*reset_node;
	unsigned int			reset_modemr_offset;
	const char			*extalr_node;
	const char			*extal_usb_node;
	unsigned int			mod_clk_base;
	unsigned int			clk_extal_id;
	unsigned int			clk_extalr_id;
	unsigned int			clk_extal_usb_id;
	unsigned int			pll0_div;
	const void			*(*get_pll_config)(const u32 cpg_mode);
	const u16			*status_regs;
	const u16			*control_regs;
	const u16			*reset_regs;
	const u16			*reset_clear_regs;
};

/*
 * Definitions of CPG Core Clocks
 *
 * These include:
 *   - Clock outputs exported to DT
 *   - External input clocks
 *   - Internal CPG clocks
 */
struct cpg_core_clk {
	/* Common */
	const char *name;
	unsigned int id;
	unsigned int type;
	/* Depending on type */
	unsigned int parent;	/* Core Clocks only */
	unsigned int div;
	unsigned int mult;
	unsigned int offset;
};

enum clk_types {
	/* Generic */
	CLK_TYPE_IN,		/* External Clock Input */
	CLK_TYPE_FF,		/* Fixed Factor Clock */
	CLK_TYPE_DIV6P1,	/* DIV6 Clock with 1 parent clock */
	CLK_TYPE_DIV6_RO,	/* DIV6 Clock read only with extra divisor */
	CLK_TYPE_FR,		/* Fixed Rate Clock */

	/* Custom definitions start here */
	CLK_TYPE_CUSTOM,
};

#define DEF_TYPE(_name, _id, _type...)	\
	{ .name = _name, .id = _id, .type = _type }
#define DEF_BASE(_name, _id, _type, _parent...)	\
	DEF_TYPE(_name, _id, _type, .parent = _parent)

#define DEF_INPUT(_name, _id) \
	DEF_TYPE(_name, _id, CLK_TYPE_IN)
#define DEF_FIXED(_name, _id, _parent, _div, _mult)	\
	DEF_BASE(_name, _id, CLK_TYPE_FF, _parent, .div = _div, .mult = _mult)
#define DEF_DIV6P1(_name, _id, _parent, _offset)	\
	DEF_BASE(_name, _id, CLK_TYPE_DIV6P1, _parent, .offset = _offset)
#define DEF_DIV6_RO(_name, _id, _parent, _offset, _div)	\
	DEF_BASE(_name, _id, CLK_TYPE_DIV6_RO, _parent, .offset = _offset, .div = _div, .mult = 1)
#define DEF_RATE(_name, _id, _rate)	\
	DEF_TYPE(_name, _id, CLK_TYPE_FR, .mult = _rate)

/*
 * Definitions of Module Clocks
 */
struct mssr_mod_clk {
	const char *name;
	unsigned int id;
	unsigned int parent;	/* Add MOD_CLK_BASE for Module Clocks */
};

/* Convert from sparse base-100 to packed index space */
#define MOD_CLK_PACK(x)	((x) - ((x) / 100) * (100 - 32))

#define MOD_CLK_ID(x)	(MOD_CLK_BASE + MOD_CLK_PACK(x))

#define DEF_MOD(_name, _mod, _parent...)	\
	{ .name = _name, .id = MOD_CLK_ID(_mod), .parent = _parent }

struct mstp_stop_table {
	u32	sdis;
	u32	sen;
	u32	rdis;
	u32	ren;
};

#define TSTR0		0x04
#define TSTR0_STR0	BIT(0)

bool renesas_clk_is_mod(struct clk *clk);
int renesas_clk_get_mod(struct clk *clk, struct cpg_mssr_info *info,
			const struct mssr_mod_clk **mssr);
int renesas_clk_get_core(struct clk *clk, struct cpg_mssr_info *info,
			 const struct cpg_core_clk **core);
int renesas_clk_get_parent(struct clk *clk, struct cpg_mssr_info *info,
			   struct clk *parent);
int renesas_clk_endisable(struct clk *clk, void __iomem *base,
			  struct cpg_mssr_info *info, bool enable);
int renesas_clk_remove(void __iomem *base, struct cpg_mssr_info *info);

/*
 * Module Standby and Software Reset register offets.
 *
 * If the registers exist, these are valid for SH-Mobile, R-Mobile,
 * R-Car Gen2, R-Car Gen3, and RZ/G1.
 * These are NOT valid for R-Car Gen1 and RZ/A1!
 */

/*
 * Module Stop Status Register offsets
 */

static const u16 mstpsr[] = {
	0x030, 0x038, 0x040, 0x048, 0x04C, 0x03C, 0x1C0, 0x1C4,
	0x9A0, 0x9A4, 0x9A8, 0x9AC,
};

/*
 * System Module Stop Control Register offsets
 */

static const u16 smstpcr[] = {
	0x130, 0x134, 0x138, 0x13C, 0x140, 0x144, 0x148, 0x14C,
	0x990, 0x994, 0x998, 0x99C,
};

/*
 * Software Reset Register offsets
 */

static const u16 srcr[] = {
	0x0A0, 0x0A8, 0x0B0, 0x0B8, 0x0BC, 0x0C4, 0x1C8, 0x1CC,
	0x920, 0x924, 0x928, 0x92C,
};

/* Realtime Module Stop Control Register offsets */
#define RMSTPCR(i)	((i) < 8 ? smstpcr[i] - 0x20 : smstpcr[i] - 0x10)

/* Modem Module Stop Control Register offsets (r8a73a4) */
#define MMSTPCR(i)	(smstpcr[i] + 0x20)

/* Software Reset Clearing Register offsets */

static const u16 srstclr[] = {
	0x940, 0x944, 0x948, 0x94C, 0x950, 0x954, 0x958, 0x95C,
	0x960, 0x964, 0x968, 0x96C,
};

#endif /* __DRIVERS_CLK_RENESAS_CPG_MSSR__ */
