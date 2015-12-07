/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/irq.h>
#include <asm/post.h>
#include <asm/processor.h>

int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);
#ifdef CONFIG_SYS_X86_TSC_TIMER
	timer_set_base(rdtsc());
#endif

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	return 0;
}

#ifndef CONFIG_EFI_STUB
int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}
#endif

void reset_cpu(ulong addr)
{
	/* cold reset */
	x86_full_reset();
}

int arch_misc_init(void)
{
	return pirq_init();
}
