/*
 * arch/arm/include/asm/arch-rmobile/r8a7790.h
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
*/

#ifndef __ASM_ARCH_R8A7790_H
#define __ASM_ARCH_R8A7790_H

#include "rcar-base.h"

#define R8A7790_CUT_ES2X	2
#define IS_R8A7790_ES2()	\
	(rmobile_get_cpu_rev_integer() == R8A7790_CUT_ES2X)

#endif /* __ASM_ARCH_R8A7790_H */
