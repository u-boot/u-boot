/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/io.h>

#ifndef CONFIG_HAVE_ARCH_IOREMAP
static inline void __iomem *ioremap(resource_size_t offset,
				    resource_size_t size)
{
	return (void __iomem *)(unsigned long)offset;
}

static inline void iounmap(void __iomem *addr)
{
}

#define devm_ioremap(dev, offset, size)		ioremap(offset, size)
#endif

#endif /* _LINUX_IO_H */
