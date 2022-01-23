// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <spl.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong dummy)
{
	spl_early_init();
	preloader_console_init();
	timer_init();
	dram_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)(CONFIG_SYS_LOAD_ADDR);
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* boot linux */
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);
	return 0;
}
#endif
