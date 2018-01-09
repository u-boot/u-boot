/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>

DECLARE_GLOBAL_DATA_PTR;

/* Global declaration of gd */
struct global_data *global_data_ptr;

void arch_setup_gd(gd_t *new_gd)
{
	global_data_ptr = new_gd;

	/*
	 * TODO(sjg@chromium.org): For some reason U-Boot does not boot
	 * without this line. It fails to start up U-Boot proper and instead
	 * restarts SPL. Need to figure out why:
	 *
	 * U-Boot SPL 2017.01
	 *
	 * U-Boot SPL 2017.01
	 * CPU:   Intel(R) Core(TM) i5-3427U CPU @ 1.80GHz
	 * Trying to boot from SPIJumping to 64-bit U-Boot: Note many
	 * features are missing
	 *
	 * U-Boot SPL 2017.01
	 */
#ifdef CONFIG_DEBUG_UART
	printch(' ');
#endif
}

int cpu_has_64bit(void)
{
	return true;
}

void enable_caches(void)
{
	/* Not implemented */
}

void disable_caches(void)
{
	/* Not implemented */
}

int dcache_status(void)
{
	return true;
}

int x86_mp_init(void)
{
	/* Not implemented */
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return 0;
}
