/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

#include <asm/ibmpc.h>

#ifndef __CONFIG_X86_COMMON_H
#define __CONFIG_X86_COMMON_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* Generic TPM interfaced through LPC bus */
#define CONFIG_TPM_TIS_BASE_ADDRESS        0xfed40000

/*-----------------------------------------------------------------------
 * Real Time Clock Configuration
 */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0
#define CONFIG_SYS_ISA_IO      CONFIG_SYS_ISA_IO_BASE_ADDRESS

/*-----------------------------------------------------------------------
 * Serial Configuration
 */
#define CONFIG_SYS_NS16550_PORT_MAPPED

/*
 * Miscellaneous configurable options
 */

/*-----------------------------------------------------------------------
 * CPU Features
 */

#define CONFIG_SYS_STACK_SIZE			(32 * 1024)

/*-----------------------------------------------------------------------
 * Environment configuration
 */

/*-----------------------------------------------------------------------
 * USB configuration
 */

/* Default environment */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_HOSTNAME		"x86"
#define CONFIG_RAMDISK_ADDR	0x4000000
#if defined(CONFIG_GENERATE_ACPI_TABLE) || defined(CONFIG_EFI_STUB)
#define CONFIG_OTHBOOTARGS	"othbootargs=\0"
#else
#define CONFIG_OTHBOOTARGS	"othbootargs=acpi=off\0"
#endif

#if defined(CONFIG_DISTRO_DEFAULTS)
#define DISTRO_BOOTENV		BOOTENV
#else
#define DISTRO_BOOTENV
#endif

#ifndef SPLASH_SETTINGS
#define SPLASH_SETTINGS
#endif

#define CONFIG_EXTRA_ENV_SETTINGS			\
	DISTRO_BOOTENV					\
	CONFIG_STD_DEVICES_SETTINGS			\
	SPLASH_SETTINGS					\
	"pciconfighost=1\0"				\
	"netdev=eth0\0"					\
	"consoledev=ttyS0\0"				\
	CONFIG_OTHBOOTARGS				\
	"scriptaddr=0x7000000\0"			\
	"kernel_addr_r=0x1000000\0"			\
	"ramdisk_addr_r=0x4000000\0"			\
	"ramdiskfile=initramfs.gz\0"


#endif	/* __CONFIG_H */
