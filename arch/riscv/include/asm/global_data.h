/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

/* Architecture-specific global data */
struct arch_global_data {
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm ("gp")

#endif /* __ASM_GBL_DATA_H */
