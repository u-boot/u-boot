// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <asm/arch/misc.h>

int board_early_init_f(void)
{
	socfpga_get_sys_mgr_addr();
	return 0;
}
