/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011 NVIDIA Corporation <www.nvidia.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <tps6586x.h>
#include <asm/io.h>
#include <asm/arch/ap20.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/tegra_i2c.h>
#include <asm/arch/sys_proto.h>

#define VDD_CORE_NOMINAL_T25	0x17	/* 1.3v */
#define VDD_CPU_NOMINAL_T25	0x10	/* 1.125v */

#define VDD_CORE_NOMINAL_T20	0x16	/* 1.275v */
#define VDD_CPU_NOMINAL_T20	0x0f	/* 1.1v */

#define VDD_RELATION		0x02	/*  50mv */
#define VDD_TRANSITION_STEP	0x06	/* 150mv */
#define VDD_TRANSITION_RATE	0x06	/* 3.52mv/us */

int pmu_set_nominal(void)
{
	int core, cpu, bus;

	/* by default, the table has been filled with T25 settings */
	switch (tegra_get_chip_type()) {
	case TEGRA_SOC_T20:
		core = VDD_CORE_NOMINAL_T20;
		cpu = VDD_CPU_NOMINAL_T20;
		break;
	case TEGRA_SOC_T25:
		core = VDD_CORE_NOMINAL_T25;
		cpu = VDD_CPU_NOMINAL_T25;
		break;
	default:
		debug("%s: Unknown chip type\n", __func__);
		return -1;
	}

	bus = tegra_i2c_get_dvc_bus_num();
	if (bus == -1) {
		debug("%s: Cannot find DVC I2C bus\n", __func__);
		return -1;
	}
	tps6586x_init(bus);
	tps6586x_set_pwm_mode(TPS6586X_PWM_SM1);
	return tps6586x_adjust_sm0_sm1(core, cpu, VDD_TRANSITION_STEP,
				VDD_TRANSITION_RATE, VDD_RELATION);
}
