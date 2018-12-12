// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <log.h>
#include <asm/csr.h>

/*
 * prior_stage_fdt_address must be stored in the data section since it is used
 * before the bss section is available.
 */
phys_addr_t prior_stage_fdt_address __attribute__((section(".data")));

static inline bool supports_extension(char ext)
{
	return csr_read(misa) & (1 << (ext - 'a'));
}

static int riscv_cpu_probe(void)
{
#ifdef CONFIG_CPU
	int ret;

	/* probe cpus so that RISC-V timer can be bound */
	ret = cpu_probe_all();
	if (ret)
		return log_msg_ret("RISC-V cpus probe failed\n", ret);
#endif

	return 0;
}

int arch_cpu_init_dm(void)
{
	return riscv_cpu_probe();
}

int arch_early_init_r(void)
{
	return riscv_cpu_probe();
}
