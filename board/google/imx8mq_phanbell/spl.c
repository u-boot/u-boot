// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 *
 */

#include <config.h>
#include <hang.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/sections.h>
#include <spl.h>

static void spl_dram_init(void)
{
	/* ddr init */
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	init_uart_clk(0);

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* DDR initialization */
	spl_dram_init();

	init_clk_usdhc(0);
	init_clk_usdhc(1);

	board_init_r(NULL, 0);
}
