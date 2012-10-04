/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
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

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	icache_enable();
	return 0;
}
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	dcache_enable();
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
static u32 __rmobile_get_cpu_type(void)
{
	return 0x0;
}
u32 rmobile_get_cpu_type(void)
		__attribute__((weak, alias("__rmobile_get_cpu_type")));

static u32 __rmobile_get_cpu_rev_integer(void)
{
	return 0;
}
u32 rmobile_get_cpu_rev_integer(void)
		__attribute__((weak, alias("__rmobile_get_cpu_rev_integer")));

static u32 __rmobile_get_cpu_rev_fraction(void)
{
	return 0;
}
u32 rmobile_get_cpu_rev_fraction(void)
		__attribute__((weak, alias("__rmobile_get_cpu_rev_fraction")));

int print_cpuinfo(void)
{
	switch (rmobile_get_cpu_type()) {
	case 0x37:
		printf("CPU: Renesas Electronics SH73A0 rev %d.%d\n",
		       rmobile_get_cpu_rev_integer(),
		       rmobile_get_cpu_rev_fraction());
		break;
	case 0x40:
		printf("CPU: Renesas Electronics R8A7740 rev %d.%d\n",
		       rmobile_get_cpu_rev_integer(),
		       rmobile_get_cpu_rev_fraction());
		break;

	default:
		printf("CPU: Renesas Electronics CPU rev %d.%d\n",
		       rmobile_get_cpu_rev_integer(),
		       rmobile_get_cpu_rev_fraction());
		break;
	}
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
