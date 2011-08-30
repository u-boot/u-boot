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

/* Pins which we can set to tristate or normal */
enum pmux_pin {
	/* APB_MISC_PP_TRISTATE_REG_A_0 */
	PIN_ATA,
	PIN_ATB,
	PIN_ATC,
	PIN_ATD,
	PIN_CDEV1,
	PIN_CDEV2,
	PIN_CSUS,
	PIN_DAP1,

	PIN_DAP2,
	PIN_DAP3,
	PIN_DAP4,
	PIN_DTA,
	PIN_DTB,
	PIN_DTC,
	PIN_DTD,
	PIN_DTE,

	PIN_GPU,
	PIN_GPV,
	PIN_I2CP,
	PIN_IRTX,
	PIN_IRRX,
	PIN_KBCB,
	PIN_KBCA,
	PIN_PMC,

	PIN_PTA,
	PIN_RM,
	PIN_KBCE,
	PIN_KBCF,
	PIN_GMA,
	PIN_GMC,
	PIN_SDMMC1,
	PIN_OWC,

	/* 32: APB_MISC_PP_TRISTATE_REG_B_0 */
	PIN_GME,
	PIN_SDC,
	PIN_SDD,
	PIN_RESERVED0,
	PIN_SLXA,
	PIN_SLXC,
	PIN_SLXD,
	PIN_SLXK,

	PIN_SPDI,
	PIN_SPDO,
	PIN_SPIA,
	PIN_SPIB,
	PIN_SPIC,
	PIN_SPID,
	PIN_SPIE,
	PIN_SPIF,

	PIN_SPIG,
	PIN_SPIH,
	PIN_UAA,
	PIN_UAB,
	PIN_UAC,
	PIN_UAD,
	PIN_UCA,
	PIN_UCB,

	PIN_RESERVED1,
	PIN_ATE,
	PIN_KBCC,
	PIN_RESERVED2,
	PIN_RESERVED3,
	PIN_GMB,
	PIN_GMD,
	PIN_DDC,

	/* 64: APB_MISC_PP_TRISTATE_REG_C_0 */
	PIN_LD0,
	PIN_LD1,
	PIN_LD2,
	PIN_LD3,
	PIN_LD4,
	PIN_LD5,
	PIN_LD6,
	PIN_LD7,

	PIN_LD8,
	PIN_LD9,
	PIN_LD10,
	PIN_LD11,
	PIN_LD12,
	PIN_LD13,
	PIN_LD14,
	PIN_LD15,

	PIN_LD16,
	PIN_LD17,
	PIN_LHP0,
	PIN_LHP1,
	PIN_LHP2,
	PIN_LVP0,
	PIN_LVP1,
	PIN_HDINT,

	PIN_LM0,
	PIN_LM1,
	PIN_LVS,
	PIN_LSC0,
	PIN_LSC1,
	PIN_LSCK,
	PIN_LDC,
	PIN_LCSN,

	/* 96: APB_MISC_PP_TRISTATE_REG_D_0 */
	PIN_LSPI,
	PIN_LSDA,
	PIN_LSDI,
	PIN_LPW0,
	PIN_LPW1,
	PIN_LPW2,
	PIN_LDI,
	PIN_LHS,

	PIN_LPP,
	PIN_RESERVED4,
	PIN_KBCD,
	PIN_GPU7,
	PIN_DTF,
	PIN_UDA,
	PIN_CRTP,
	PIN_SDB,
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

	uint pmt_ctl_a;			/* _PIN_MUX_CTL_A_0, offset 80 */
	uint pmt_ctl_b;			/* _PIN_MUX_CTL_B_0, offset 84 */
	uint pmt_ctl_c;			/* _PIN_MUX_CTL_C_0, offset 88 */
	uint pmt_ctl_d;			/* _PIN_MUX_CTL_D_0, offset 8C */
	uint pmt_ctl_e;			/* _PIN_MUX_CTL_E_0, offset 90 */
	uint pmt_ctl_f;			/* _PIN_MUX_CTL_F_0, offset 94 */
	uint pmt_ctl_g;			/* _PIN_MUX_CTL_G_0, offset 98 */
};

/* Converts a pin number to a tristate register: 0=A, 1=B, 2=C, 3=D */
#define TRISTATE_REG(id) ((id) >> 5)

/* Mask value for a tristate (within TRISTATE_REG(id)) */
#define TRISTATE_MASK(id) (1 << ((id) & 0x1f))

/* Set a pin to tristate */
void pinmux_tristate_enable(enum pmux_pin pin);

/* Set a pin to normal (non tristate) */
void pinmux_tristate_disable(enum pmux_pin pin);

#endif	/* PINMUX_H */
