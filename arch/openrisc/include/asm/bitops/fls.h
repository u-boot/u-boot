/*
 * OpenRISC Linux
 *
 * Copyright (C) 2010-2011 Jonas Bonn <jonas@southpole.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_OPENRISC_FLS_H
#define __ASM_OPENRISC_FLS_H

static inline int fls(int x)
{
	int ret;

	__asm__ ("l.fl1 %0,%1"
		 : "=r" (ret)
		 : "r" (x));

	return ret;
}

#endif /* __ASM_OPENRISC_FLS_H */
