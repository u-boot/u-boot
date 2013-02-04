/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
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

#ifndef _TEGRA20_H_
#define _TEGRA20_H_

#define NV_PA_SDRAM_BASE	0x00000000

#include <asm/arch-tegra/tegra.h>

#define TEGRA_USB1_BASE		0xC5000000
#define TEGRA_USB3_BASE		0xC5008000

#define BCT_ODMDATA_OFFSET	4068	/* 12 bytes from end of BCT */

#endif	/* TEGRA20_H */
