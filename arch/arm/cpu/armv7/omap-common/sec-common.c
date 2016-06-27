/*
 *
 * Common security related functions for OMAP devices
 *
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Daniel Allred <d-allred@ti.com>
 * Andreas Dannenberg <dannenberg@ti.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <stdarg.h>

#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>

static uint32_t secure_rom_call_args[5] __aligned(ARCH_DMA_MINALIGN);

u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...)
{
	int i;
	u32 num_args;
	va_list ap;

	va_start(ap, flag);

	num_args = va_arg(ap, u32);

	if (num_args > 4)
		return 1;

	/* Copy args to aligned args structure */
	for (i = 0; i < num_args; i++)
		secure_rom_call_args[i + 1] = va_arg(ap, u32);

	secure_rom_call_args[0] = num_args;

	va_end(ap);

	/* if data cache is enabled, flush the aligned args structure */
	flush_dcache_range(
		(unsigned int)&secure_rom_call_args[0],
		(unsigned int)&secure_rom_call_args[0] +
		roundup(sizeof(secure_rom_call_args), ARCH_DMA_MINALIGN));

	return omap_smc_sec(service, proc_id, flag, secure_rom_call_args);
}
