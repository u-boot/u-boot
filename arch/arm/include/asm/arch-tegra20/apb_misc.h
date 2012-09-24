/*
 * Copyright (c) 2012 The Chromium OS Authors.
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

#ifndef _GP_PADCTRL_H_
#define _GP_PADCTRL_H_

/* APB_MISC_PP registers */
struct apb_misc_pp_ctlr {
	u32	reserved0[2];
	u32	strapping_opt_a;/* 0x08: APB_MISC_PP_STRAPPING_OPT_A */
};

/* bit fields definitions for APB_MISC_PP_STRAPPING_OPT_A register */
#define RAM_CODE_SHIFT		4
#define RAM_CODE_MASK		(0xf << RAM_CODE_SHIFT)

#endif
