/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham, Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include "asm/types.h"

/* Architecture-specific global data */
struct arch_global_data {
	void *timer;
	void *uart;
	unsigned int uart_freq;
#ifdef CONFIG_LEON3
	unsigned int snooping_available;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("%g7")

#endif				/* __ASM_GBL_DATA_H */
