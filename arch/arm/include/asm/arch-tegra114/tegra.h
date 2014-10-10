/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef _TEGRA114_H_
#define _TEGRA114_H_

#define NV_PA_SDRAM_BASE	0x80000000	/* 0x80000000 for real T114 */
#define NV_PA_TSC_BASE		0x700F0000	/* System Counter TSC regs */
#define NV_PA_MC_BASE		0x70019000

#include <asm/arch-tegra/tegra.h>

#define BCT_ODMDATA_OFFSET	1752	/* offset to ODMDATA word */

#undef NVBOOTINFOTABLE_BCTSIZE
#undef NVBOOTINFOTABLE_BCTPTR
#define NVBOOTINFOTABLE_BCTSIZE        0x48    /* BCT size in BIT in IRAM */
#define NVBOOTINFOTABLE_BCTPTR 0x4C    /* BCT pointer in BIT in IRAM */

#define MAX_NUM_CPU            4

#endif /* TEGRA114_H */
