// SPDX-License-Identifier: GPL-2.0+
/*
 * CPU code for Qualcomm IPQ40xx SoC
 *
 * Copyright (c) 2024 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 */

#include <cpu_func.h>
#include <init.h>

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;
	return fdtdec_setup_mem_size_base();
}

/*
 * Enable/Disable D-cache.
 * I-cache is already enabled in start.S
 */
void enable_caches(void)
{
	dcache_enable();
}

void disable_caches(void)
{
	dcache_disable();
}

/*
 * In case boards need specific init code, they can override this stub.
 */
int __weak board_init(void)
{
	return 0;
}
