/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IPROC_COMMON_CONFIGS_H
#define __IPROC_COMMON_CONFIGS_H

#include <linux/stringify.h>

/* Architecture, CPU, chip, etc */
#define CONFIG_IPROC
#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH

/* Memory Info */
#define CONFIG_SYS_TEXT_BASE		0x61000000
#define CONFIG_SYS_SDRAM_BASE		0x61000000

#endif /* __IPROC_COMMON_CONFIGS_H */
