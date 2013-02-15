/*
 *  (C) Copyright 2010-2012
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

#ifndef _TEGRA_GP_PADCTRL_H_
#define _TEGRA_GP_PADCTRL_H_

#define GP_HIDREV			0x804

/* bit fields definitions for APB_MISC_GP_HIDREV register */
#define HIDREV_CHIPID_SHIFT		8
#define HIDREV_CHIPID_MASK		(0xff << HIDREV_CHIPID_SHIFT)
#define HIDREV_MAJORPREV_SHIFT		4
#define HIDREV_MAJORPREV_MASK		(0xf << HIDREV_MAJORPREV_SHIFT)

/* CHIPID field returned from APB_MISC_GP_HIDREV register */
#define CHIPID_TEGRA20			0x20
#define CHIPID_TEGRA30			0x30
#define CHIPID_TEGRA114			0x35

#endif	/* _TEGRA_GP_PADCTRL_H_ */
