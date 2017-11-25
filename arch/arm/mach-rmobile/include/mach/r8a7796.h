/*
 * arch/arm/include/asm/arch-rcar_gen3/r8a7796.h
 *	This file defines registers and value for r8a7796.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_R8A7796_H
#define __ASM_ARCH_R8A7796_H

#include "rcar-gen3-base.h"

/* Module stop control/status register bits */
#define MSTP0_BITS	0x00200000
#define MSTP1_BITS	0xFFFFFFFF
#define MSTP2_BITS	0x340E2FDC
#define MSTP3_BITS	0xFFFFFFDF
#define MSTP4_BITS	0x80000184
#define MSTP5_BITS	0xC3FFFFFF
#define MSTP6_BITS	0xFFFFFFFF
#define MSTP7_BITS	0xFFFFFFFF
#define MSTP8_BITS	0x01F1FFF7
#define MSTP9_BITS	0xFFFFFFFE
#define MSTP10_BITS	0xFFFEFFE0
#define MSTP11_BITS	0x000000B7

#endif /* __ASM_ARCH_R8A7796_H */
