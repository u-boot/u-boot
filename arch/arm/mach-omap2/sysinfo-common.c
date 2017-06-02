/*
 * System information routines for all OMAP based boards.
 *
 * (C) Copyright 2017 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/omap.h>
#include <asm/io.h>
#include <asm/omap_common.h>

/**
 * Tell if device is GP/HS/EMU/TST.
 */
u32 get_device_type(void)
{
	return (readl((*ctrl)->control_status) & DEVICE_TYPE_MASK) >>
		DEVICE_TYPE_SHIFT;
}
