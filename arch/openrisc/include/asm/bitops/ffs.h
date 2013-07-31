/*
 * OpenRISC Linux
 *
 * Copyright (C) 2010-2011 Jonas Bonn <jonas@southpole.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_OPENRISC_FFS_H
#define __ASM_OPENRISC_FFS_H

static inline int ffs(int x)
{
	int ret;

	__asm__ ("l.ff1 %0,%1"
		 : "=r" (ret)
		 : "r" (x));

	return ret;
}

#endif /* __ASM_OPENRISC_FFS_H */
