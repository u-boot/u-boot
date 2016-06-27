/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Andreas Dannenberg <dannenberg@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_OMAP_SEC_COMMON_H_
#define	_OMAP_SEC_COMMON_H_

#include <common.h>

/*
 * Invoke secure ROM API on high-security (HS) device variants. It formats
 * the variable argument list into the format expected by the ROM code before
 * triggering the actual low-level smc entry.
 */
u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...);

#endif /* _OMAP_SEC_COMMON_H_ */
