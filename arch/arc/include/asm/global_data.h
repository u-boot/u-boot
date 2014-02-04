/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	__ASM_ARC_GLOBAL_DATA_H
#define __ASM_ARC_GLOBAL_DATA_H

/* Architecture-specific global data */
struct arch_global_data {
	int running_on_hw;
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("r25")

#endif /* __ASM_ARC_GLOBAL_DATA_H */
