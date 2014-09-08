/*
 * Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/fpga_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	uint32_t regs = (uint32_t)&sysmgr_regs->emacio[0];
	int i;

	for (i = 0; i < ARRAY_SIZE(sys_mgr_init_table); i++) {
		writel(sys_mgr_init_table[i], regs);
		regs += sizeof(regs);
	}
}
