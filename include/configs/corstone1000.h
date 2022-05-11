/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2022 ARM Limited
 * (C) Copyright 2022 Linaro
 * Rui Miguel Silva <rui.silva@linaro.org>
 * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 *
 * Configuration for Corstone1000. Parts were derived from other ARM
 * configurations.
 */

#ifndef __CORSTONE1000_H
#define __CORSTONE1000_H

#include <linux/sizes.h>

#define V2M_BASE		0x80000000

#define CONFIG_PL011_CLOCK	50000000

/* Physical Memory Map */
#define PHYS_SDRAM_1		(V2M_BASE)
#define PHYS_SDRAM_1_SIZE	0x80000000

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1

#define CONFIG_EXTRA_ENV_SETTINGS							\
				"usb_pgood_delay=250\0"					\
				"boot_bank_flag=0x08002000\0"				\
				"kernel_addr_bank_0=0x083EE000\0"			\
				"kernel_addr_bank_1=0x0936E000\0"			\
				"retrieve_kernel_load_addr="				\
					"if itest.l *${boot_bank_flag} == 0; then "	\
					    "setenv kernel_addr $kernel_addr_bank_0;"	\
					"else "						\
					    "setenv kernel_addr $kernel_addr_bank_1;"	\
					"fi;"						\
					"\0"						\
				"kernel_addr_r=0x88200000\0"

#endif
