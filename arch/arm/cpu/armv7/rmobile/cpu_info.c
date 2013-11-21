/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

	case 0x45:
		printf("CPU: Renesas Electronics R8A7790 rev %d\n",
		       rmobile_get_cpu_rev_integer());
		break;

	case 0x47:
		printf("CPU: Renesas Electronics R8A7791 rev %d\n",
			rmobile_get_cpu_rev_integer());
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
