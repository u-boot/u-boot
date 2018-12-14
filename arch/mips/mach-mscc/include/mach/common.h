/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __ASM_MACH_COMMON_H
#define __ASM_MACH_COMMON_H

#if defined(CONFIG_SOC_OCELOT)
#include <mach/ocelot/ocelot.h>
#include <mach/ocelot/ocelot_devcpu_gcb.h>
#include <mach/ocelot/ocelot_icpu_cfg.h>
#elif defined(CONFIG_SOC_LUTON)
#include <mach/luton/luton.h>
#include <mach/luton/luton_devcpu_gcb.h>
#include <mach/luton/luton_icpu_cfg.h>
#else
#error Unsupported platform
#endif

#define MSCC_DDR_TO	0x20000000	/* DDR RAM base offset */
#define MSCC_MEMCTL1_TO	0x40000000	/* SPI/PI base offset */
#define MSCC_MEMCTL2_TO	0x50000000	/* SPI/PI base offset */
#define MSCC_FLASH_TO	MSCC_MEMCTL1_TO	/* Flash base offset */

#define VCOREIII_TIMER_DIVIDER 25	/* Clock tick ~ 0.1 us */

#endif				/* __ASM_MACH_COMMON_H */
