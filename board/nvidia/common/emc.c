/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/emc.h>
#include <asm/arch/pmu.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

/* These rates are hard-coded for now, until fdt provides them */
#define EMC_SDRAM_RATE_T20	(333000 * 2 * 1000)
#define EMC_SDRAM_RATE_T25	(380000 * 2 * 1000)

int board_emc_init(void)
{
	unsigned rate;

	switch (tegra_get_chip_type()) {
	default:
	case TEGRA_SOC_T20:
		rate  = EMC_SDRAM_RATE_T20;
		break;
	case TEGRA_SOC_T25:
		rate  = EMC_SDRAM_RATE_T25;
		break;
	}
	return tegra_set_emc(gd->fdt_blob, rate);
}
