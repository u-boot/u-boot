// SPDX-License-Identifier: GPL-2.0
/*
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */
#include <fdtdec.h>
#include <init.h>
#include <sysreset.h>
#include <asm/system.h>
#include <linux/io.h>

int print_cpuinfo(void)
{
	printf("CPU:   Airoha EN7523/EN7529/EN7562\n");
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

void __noreturn reset_cpu(void)
{
	writel(0x80000000, 0x1FB00040);
	while (1) {
		/* loop forever */
	}
}
