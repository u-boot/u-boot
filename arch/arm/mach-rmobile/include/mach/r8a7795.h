/*
 * arch/arm/mach-rmobile/include/mach/r8a7795.h
 *	This file defines registers and value for r8a7795.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_R8A7795_H
#define __ASM_ARCH_R8A7795_H

#include "rcar-gen3-base.h"

/* Module stop control/status register bits */
#define MSTP0_BITS	0x00640800
#define MSTP1_BITS	0xF3EE9390
#define MSTP2_BITS	0x340FAFDC
#define MSTP3_BITS	0xD80C7CDF
#define MSTP4_BITS	0x80000184
#define MSTP5_BITS	0x40BFFF46
#define MSTP6_BITS	0xE5FBEECF
#define MSTP7_BITS	0x39FFFF0E
#define MSTP8_BITS	0x01F19FF4
#define MSTP9_BITS	0xFFDFFFFF
#define MSTP10_BITS	0xFFFEFFE0
#define MSTP11_BITS	0x00000000

/* SDHI */
#define CONFIG_SYS_SH_SDHI0_BASE 0xEE100000
#define CONFIG_SYS_SH_SDHI1_BASE 0xEE120000
#define CONFIG_SYS_SH_SDHI2_BASE 0xEE140000	/* either MMC0 */
#define CONFIG_SYS_SH_SDHI3_BASE 0xEE160000	/* either MMC1 */
#define CONFIG_SYS_SH_SDHI_NR_CHANNEL 4

#endif /* __ASM_ARCH_R8A7795_H */
