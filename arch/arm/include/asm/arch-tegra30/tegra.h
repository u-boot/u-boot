/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TEGRA30_H_
#define _TEGRA30_H_

#define NV_PA_SDRAM_BASE	0x80000000	/* 0x80000000 for real T30 */

#include <asm/arch-tegra/tegra.h>

#define TEGRA_USB1_BASE		0x7D000000

#define BCT_ODMDATA_OFFSET	6116	/* 12 bytes from end of BCT */

#define MAX_NUM_CPU		4

#endif	/* TEGRA30_H */
