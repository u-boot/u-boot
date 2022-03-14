/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_EXPORT_H
#define _LINUX_EXPORT_H

/*
 * Export symbols from the kernel to modules.  Forked from module.h
 * to reduce the amount of pointless cruft we feed to gcc when only
 * exporting a simple symbol or two.
 *
 * Try not to add #includes here.  It slows compilation and makes kernel
 * hackers place grumpy comments in header files.
 */

#ifndef __ASSEMBLY__

#define THIS_MODULE	0

#define EXPORT_SYMBOL(...)
#define EXPORT_SYMBOL_GPL(...)
#define EXPORT_SYMBOL_NS(...)
#define EXPORT_SYMBOL_NS_GPL(...)

#endif /* !__ASSEMBLY__ */

#endif /* _LINUX_EXPORT_H */
