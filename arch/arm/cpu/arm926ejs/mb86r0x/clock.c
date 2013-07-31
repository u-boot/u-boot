/*
 * (C) Copyright 2010
 * Matthias Weisser <weisserm@arcor.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

/*
 * Get the peripheral bus frequency depending on pll pin settings
 */
ulong get_bus_freq(ulong dummy)
{
	struct mb86r0x_crg * crg = (struct mb86r0x_crg *)
					MB86R0x_CRG_BASE;
	uint32_t pllmode;

	pllmode = readl(&crg->crpr) & MB86R0x_CRG_CRPR_PLLMODE;

	if (pllmode == MB86R0x_CRG_CRPR_PLLMODE_X20)
		return 40000000;

	return 41164767;
}
