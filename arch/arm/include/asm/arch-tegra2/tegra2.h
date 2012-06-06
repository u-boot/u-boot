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

#ifndef _TEGRA2_H_
#define _TEGRA2_H_

#define NV_PA_SDRAM_BASE	0x00000000
#define NV_PA_TMRUS_BASE	0x60005010
#define NV_PA_CLK_RST_BASE	0x60006000
#define NV_PA_APB_MISC_BASE	0x70000000
#define NV_PA_APB_UARTA_BASE	(NV_PA_APB_MISC_BASE + 0x6000)
#define NV_PA_APB_UARTB_BASE	(NV_PA_APB_MISC_BASE + 0x6040)
#define NV_PA_APB_UARTC_BASE	(NV_PA_APB_MISC_BASE + 0x6200)
#define NV_PA_APB_UARTD_BASE	(NV_PA_APB_MISC_BASE + 0x6300)
#define NV_PA_APB_UARTE_BASE	(NV_PA_APB_MISC_BASE + 0x6400)
#define NV_PA_PMC_BASE		0x7000E400

#define TEGRA2_SDRC_CS0		NV_PA_SDRAM_BASE
#define LOW_LEVEL_SRAM_STACK	0x4000FFFC

#ifndef __ASSEMBLY__
struct timerus {
	unsigned int cntr_1us;
};
#else  /* __ASSEMBLY__ */
#define PRM_RSTCTRL		NV_PA_PMC_BASE
#endif

#endif	/* TEGRA2_H */
