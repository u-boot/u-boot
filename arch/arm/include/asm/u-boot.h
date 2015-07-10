/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

#ifdef CONFIG_SYS_GENERIC_BOARD
/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>
#else

#ifndef __ASSEMBLY__
typedef struct bd_info {
    ulong	        bi_arch_number;	/* unique id for this board */
    ulong	        bi_boot_params;	/* where this board expects params */
	unsigned long	bi_arm_freq; /* arm frequency */
	unsigned long	bi_dsp_freq; /* dsp core frequency */
	unsigned long	bi_ddr_freq; /* ddr frequency */
    struct				/* RAM configuration */
    {
	ulong start;
	ulong size;
    }			bi_dram[CONFIG_NR_DRAM_BANKS];
} bd_t;
#endif

#endif /* !CONFIG_SYS_GENERIC_BOARD */

/* For image.h:image_check_target_arch() */
#ifndef CONFIG_ARM64
#define IH_ARCH_DEFAULT IH_ARCH_ARM
#else
#define IH_ARCH_DEFAULT IH_ARCH_ARM64
#endif

#endif	/* _U_BOOT_H_ */
