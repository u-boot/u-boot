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
 * Reset the cpu by setting software reset request bit
 */
void reset_cpu(ulong ignored)
{
	struct mb86r0x_crg * crg = (struct mb86r0x_crg *)
					MB86R0x_CRG_BASE;

	writel(MB86R0x_CRSR_SWRSTREQ, &crg->crsr);
	while (1)
		/* NOP */;
	/* Never reached */
}
