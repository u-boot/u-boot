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

/* APB MISC Pin Mux and Tristate (APB_MISC_PP_) registers */
struct pmux_tri_ctlr {
	uint pmt_reserved0;		/* ABP_MISC_PP_ reserved offset 00 */
	uint pmt_reserved1;		/* ABP_MISC_PP_ reserved offset 04 */
	uint pmt_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 08 */
	uint pmt_reserved2;		/* ABP_MISC_PP_ reserved offset 0C */
	uint pmt_reserved3;		/* ABP_MISC_PP_ reserved offset 10 */
	uint pmt_tri_a;			/* _TRI_STATE_REG_A_0, offset 14 */
	uint pmt_tri_b;			/* _TRI_STATE_REG_B_0, offset 18 */
	uint pmt_tri_c;			/* _TRI_STATE_REG_C_0, offset 1C */
	uint pmt_tri_d;			/* _TRI_STATE_REG_D_0, offset 20 */
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

#define Z_GMC			(1 << 29)
#define Z_IRRX			(1 << 20)
#define Z_IRTX			(1 << 19)
#define Z_GMA			(1 << 28)
#define Z_GME			(1 << 0)
#define Z_ATB			(1 << 1)
#define Z_SDB			(1 << 15)
#define Z_SDC			(1 << 1)
#define Z_SDD			(1 << 2)

#endif	/* PINMUX_H */
