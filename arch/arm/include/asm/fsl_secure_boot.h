/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_FIT_SIGNATURE

#define CONFIG_EXTRA_ENV \
	"setenv fdt_high 0xcfffffff;"	\
	"setenv initrd_high 0xcfffffff;"	\
	"setenv hwconfig \'fsl_ddr:ctlr_intlv=null,bank_intlv=null\';"

/* The address needs to be modified according to NOR memory map */
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0x600a0000

#include <config_fsl_secboot.h>
#endif
#endif

#endif
