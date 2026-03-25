// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025-2026, RISCstar Ltd.
 */

#include <spl.h>

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed:%d\n", ret);

	riscv_cpu_setup();

	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NONE;
}
