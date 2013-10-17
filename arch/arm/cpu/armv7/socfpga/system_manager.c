/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	unsigned long offset = CONFIG_SYSMGR_PINMUXGRP_OFFSET;

	const unsigned long *pval = sys_mgr_init_table;
	unsigned long i;

	for (i = 0; i < ARRAY_SIZE(sys_mgr_init_table);
		i++, offset += sizeof(unsigned long)) {
		writel(*pval++, (SOCFPGA_SYSMGR_ADDRESS + offset));
	}
}
