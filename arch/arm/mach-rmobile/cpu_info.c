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

/* CPU infomation table */
static const struct {
	u16 cpu_type;
	u8 cpu_name[10];
} rmobile_cpuinfo[] = {
	{ 0x37, "SH73A0" },
	{ 0x40, "R8A7740" },
	{ 0x45, "R8A7790" },
	{ 0x47, "R8A7791" },
	{ 0x4B, "R8A7793" },
	{ 0x4C, "R8A7794" },
	{ 0x0, "CPU" },
};

int print_cpuinfo(void)
{
	int i = 0;
	u32 cpu_type = rmobile_get_cpu_type();
	for (; i < ARRAY_SIZE(rmobile_cpuinfo); i++) {
		if (rmobile_cpuinfo[i].cpu_type == cpu_type) {
			printf("CPU: Renesas Electronics %s rev %d.%d\n",
			       rmobile_cpuinfo[i].cpu_name,
			       rmobile_get_cpu_rev_integer(),
			       rmobile_get_cpu_rev_fraction());
			break;
		}
	}
	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
