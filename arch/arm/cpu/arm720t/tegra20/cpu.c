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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/pmc.h>
#include "../tegra-common/cpu.h"

static void enable_cpu_power_rail(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	reg = readl(&pmc->pmc_cntrl);
	reg |= CPUPWRREQ_OE;
	writel(reg, &pmc->pmc_cntrl);

	/*
	 * The TI PMU65861C needs a 3.75ms delay between enabling
	 * the power rail and enabling the CPU clock.  This delay
	 * between SM1EN and SM1 is for switching time + the ramp
	 * up of the voltage to the CPU (VDD_CPU from PMU).
	 */
	udelay(3750);
}

void start_cpu(u32 reset_vector)
{
	/* Enable VDD_CPU */
	enable_cpu_power_rail();

	/* Hold the CPUs in reset */
	reset_A9_cpu(1);

	/* Disable the CPU clock */
	enable_cpu_clock(0);

	/* Enable CoreSight */
	clock_enable_coresight(1);

	/*
	 * Set the entry point for CPU execution from reset,
	 *  if it's a non-zero value.
	 */
	if (reset_vector)
		writel(reset_vector, EXCEP_VECTOR_CPU_RESET_VECTOR);

	/* Enable the CPU clock */
	enable_cpu_clock(1);

	/* If the CPU doesn't already have power, power it up */
	powerup_cpu();

	/* Take the CPU out of reset */
	reset_A9_cpu(0);
}
