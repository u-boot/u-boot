/*
 * (C) Copyright 2011
 * Helmut Raiger, HALE electronic GmbH, helmut.raiger@hale.at
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

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

struct mxc_weimcs {
	u32 upper;
	u32 lower;
	u32 additional;
};

void mxc_setup_weimcs(int cs, const struct mxc_weimcs *weimcs);
int mxc_mmc_init(bd_t *bis);
u32 get_cpu_rev(void);
#endif
