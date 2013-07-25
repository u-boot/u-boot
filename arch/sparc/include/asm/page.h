/* page.h:  Various defines and such for MMU operations on the Sparc for
 *          the Linux kernel.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 * Copyright (C) 2007 Daniel Hellstrom (daniel@gaisler.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPARC_PAGE_H
#define _SPARC_PAGE_H

#include <linux/config.h>
#ifdef CONFIG_SUN4
#define PAGE_SHIFT   13
#else
#define PAGE_SHIFT   12
#endif

#ifndef __ASSEMBLY__
/* I have my suspicions... -DaveM */
#define PAGE_SIZE    (1UL << PAGE_SHIFT)
#else
#define PAGE_SIZE    (1 << PAGE_SHIFT)
#endif

#define PAGE_MASK    (~(PAGE_SIZE-1))

#endif				/* _SPARC_PAGE_H */
