/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _PINMUX_H_
#define _PINMUX_H_

/* Pin groups which we can set to tristate or normal */
enum pmux_pingrp {
	/* APB_MISC_PP_TRISTATE_REG_A_0 */
	PINGRP_ATA,
	PINGRP_ATB,
	PINGRP_ATC,
	PINGRP_ATD,
	PINGRP_CDEV1,
	PINGRP_CDEV2,
	PINGRP_CSUS,
	PINGRP_DAP1,

	PINGRP_DAP2,
	PINGRP_DAP3,
	PINGRP_DAP4,
	PINGRP_DTA,
	PINGRP_DTB,
	PINGRP_DTC,
	PINGRP_DTD,
	PINGRP_DTE,

	PINGRP_GPU,
	PINGRP_GPV,
	PINGRP_I2CP,
	PINGRP_IRTX,
	PINGRP_IRRX,
	PINGRP_KBCB,
	PINGRP_KBCA,
	PINGRP_PMC,

	PINGRP_PTA,
	PINGRP_RM,
	PINGRP_KBCE,
	PINGRP_KBCF,
	PINGRP_GMA,
	PINGRP_GMC,
	PINGRP_SDMMC1,
	PINGRP_OWC,

	/* 32: APB_MISC_PP_TRISTATE_REG_B_0 */
	PINGRP_GME,
	PINGRP_SDC,
	PINGRP_SDD,
	PINGRP_RESERVED0,
	PINGRP_SLXA,
	PINGRP_SLXC,
	PINGRP_SLXD,
	PINGRP_SLXK,

	PINGRP_SPDI,
	PINGRP_SPDO,
	PINGRP_SPIA,
	PINGRP_SPIB,
	PINGRP_SPIC,
	PINGRP_SPID,
	PINGRP_SPIE,
	PINGRP_SPIF,

	PINGRP_SPIG,
	PINGRP_SPIH,
	PINGRP_UAA,
	PINGRP_UAB,
	PINGRP_UAC,
	PINGRP_UAD,
	PINGRP_UCA,
	PINGRP_UCB,

	PINGRP_RESERVED1,
	PINGRP_ATE,
	PINGRP_KBCC,
	PINGRP_RESERVED2,
	PINGRP_RESERVED3,
	PINGRP_GMB,
	PINGRP_GMD,
	PINGRP_DDC,

	/* 64: APB_MISC_PP_TRISTATE_REG_C_0 */
	PINGRP_LD0,
	PINGRP_LD1,
	PINGRP_LD2,
	PINGRP_LD3,
	PINGRP_LD4,
	PINGRP_LD5,
	PINGRP_LD6,
	PINGRP_LD7,

	PINGRP_LD8,
	PINGRP_LD9,
	PINGRP_LD10,
	PINGRP_LD11,
	PINGRP_LD12,
	PINGRP_LD13,
	PINGRP_LD14,
	PINGRP_LD15,

	PINGRP_LD16,
	PINGRP_LD17,
	PINGRP_LHP0,
	PINGRP_LHP1,
	PINGRP_LHP2,
	PINGRP_LVP0,
	PINGRP_LVP1,
	PINGRP_HDINT,

	PINGRP_LM0,
	PINGRP_LM1,
	PINGRP_LVS,
	PINGRP_LSC0,
	PINGRP_LSC1,
	PINGRP_LSCK,
	PINGRP_LDC,
	PINGRP_LCSN,

	/* 96: APB_MISC_PP_TRISTATE_REG_D_0 */
	PINGRP_LSPI,
	PINGRP_LSDA,
	PINGRP_LSDI,
	PINGRP_LPW0,
	PINGRP_LPW1,
	PINGRP_LPW2,
	PINGRP_LDI,
	PINGRP_LHS,

	PINGRP_LPP,
	PINGRP_RESERVED4,
	PINGRP_KBCD,
	PINGRP_GPU7,
	PINGRP_DTF,
	PINGRP_UDA,
	PINGRP_CRTP,
	PINGRP_SDB,
};


#define TEGRA_TRISTATE_REGS 4

/* APB MISC Pin Mux and Tristate (APB_MISC_PP_) registers */
struct pmux_tri_ctlr {
	uint pmt_reserved0;		/* ABP_MISC_PP_ reserved offset 00 */
	uint pmt_reserved1;		/* ABP_MISC_PP_ reserved offset 04 */
	uint pmt_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 08 */
	uint pmt_reserved2;		/* ABP_MISC_PP_ reserved offset 0C */
	uint pmt_reserved3;		/* ABP_MISC_PP_ reserved offset 10 */
	uint pmt_tri[TEGRA_TRISTATE_REGS]; /* _TRI_STATE_REG_A/B/C/D_0 14-20 */
	uint pmt_cfg_ctl;		/* _CONFIG_CTL_0, offset 24 */

	uint pmt_reserved[22];		/* ABP_MISC_PP_ reserved offs 28-7C */

	uint pmt_ctl_a;			/* _PINGRP_MUX_CTL_A_0, offset 80 */
	uint pmt_ctl_b;			/* _PINGRP_MUX_CTL_B_0, offset 84 */
	uint pmt_ctl_c;			/* _PINGRP_MUX_CTL_C_0, offset 88 */
	uint pmt_ctl_d;			/* _PINGRP_MUX_CTL_D_0, offset 8C */
	uint pmt_ctl_e;			/* _PINGRP_MUX_CTL_E_0, offset 90 */
	uint pmt_ctl_f;			/* _PINGRP_MUX_CTL_F_0, offset 94 */
	uint pmt_ctl_g;			/* _PINGRP_MUX_CTL_G_0, offset 98 */
};

/* Converts a pin group to a tristate register: 0=A, 1=B, 2=C, 3=D */
#define TRISTATE_REG(id) ((id) >> 5)

/* Mask value for a tristate (within TRISTATE_REG(id)) */
#define TRISTATE_MASK(id) (1 << ((id) & 0x1f))

/* Set a pin group to tristate */
void pinmux_tristate_enable(enum pmux_pingrp pin);

/* Set a pin group to normal (non tristate) */
void pinmux_tristate_disable(enum pmux_pingrp pin);

#endif	/* PINMUX_H */
