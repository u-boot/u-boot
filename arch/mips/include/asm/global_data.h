/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/regdef.h>

/* Architecture-specific global data */
struct arch_global_data {
#ifdef CONFIG_DYNAMIC_IO_PORT_BASE
	unsigned long io_port_base;
#endif
#ifdef CONFIG_ARCH_ATH79
	unsigned long id;
	unsigned long soc;
	unsigned long rev;
	unsigned long ver;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("k0")

#endif /* __ASM_GBL_DATA_H */
