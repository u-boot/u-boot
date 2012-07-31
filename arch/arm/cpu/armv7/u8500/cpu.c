/*
 * Copyright (C) 2012 Linaro Limited
 * Mathieu Poirier <mathieu.poirier@linaro.org>
 *
 * Based on original code from Joakim Axelsson at ST-Ericsson
 * (C) Copyright 2010 ST-Ericsson
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
#include <asm/arch/prcmu.h>
#include <asm/arch/clock.h>

#ifdef CONFIG_ARCH_CPU_INIT
/*
 * SOC specific cpu init
 */
int arch_cpu_init(void)
{
	db8500_prcmu_init();
	db8500_clocks_init();

	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */
