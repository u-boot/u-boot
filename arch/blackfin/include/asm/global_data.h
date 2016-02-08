/*
 * U-Boot - global_data.h Declarations for global data of U-Boot
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/u-boot.h>

/* Architecture-specific global data */
struct arch_global_data {
	unsigned long board_type;
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("P3")

#endif
