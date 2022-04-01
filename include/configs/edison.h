/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Intel Corp.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/ibmpc.h>

/* Miscellaneous configurable options */

#define CONFIG_SYS_CBSIZE	2048
#define CONFIG_SYS_MAXARGS	128
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#define CONFIG_SYS_STACK_SIZE			(32 * 1024)

#define CONFIG_SYS_MONITOR_LEN			(256 * 1024)

/* RTC */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS	0

#endif
