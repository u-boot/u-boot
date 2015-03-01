/*
 * (C) Copyright 2010,2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>

/*
 * This is called right before passing control to
 * the Linux kernel point.
 */
int cleanup_before_linux(void)
{
	return 0;
}

/*
 * Perform the low-level reset.
 */
void reset_cpu(ulong addr)
{
	/*
	 * Perform reset but keep priority group unchanged.
	 */
	writel((V7M_AIRCR_VECTKEY << V7M_AIRCR_VECTKEY_SHIFT)
		| (V7M_SCB->aircr & V7M_AIRCR_PRIGROUP_MSK)
		| V7M_AIRCR_SYSRESET, &V7M_SCB->aircr);
}
