/*
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jrosoft.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_ARCH_AT91SAM9_MATRIX_H
#define __ASM_ARCH_AT91SAM9_MATRIX_H

#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#include <asm/arch/at91sam9260_matrix.h>
#elif defined(CONFIG_AT91SAM9261)
#include <asm/arch/at91sam9261_matrix.h>
#elif defined(CONFIG_AT91SAM9263)
#include <asm/arch/at91sam9263_matrix.h>
#elif defined(CONFIG_AT91SAM9RL)
#include <asm/arch/at91sam9rl_matrix.h>
#elif defined(CONFIG_AT91CAP9)
#include <asm/arch/at91cap9_matrix.h>
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#include <asm/arch/at91sam9g45_matrix.h>
#else
#error "Unsupported AT91SAM9/CAP9 processor"
#endif

#endif /* __ASM_ARCH_AT91SAM9_MATRIX_H */
