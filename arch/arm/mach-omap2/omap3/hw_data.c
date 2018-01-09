/*
 * HW data initialization for OMAP3.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/omap.h>
#include <asm/omap_common.h>

struct omap_sys_ctrl_regs const **ctrl =
	(struct omap_sys_ctrl_regs const **)OMAP_SRAM_SCRATCH_SYS_CTRL;

void hw_data_init(void)
{
	*ctrl = &omap3_ctrl;
}
