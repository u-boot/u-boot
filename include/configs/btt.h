/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */
#ifndef __CONFIGS_BTT_H__
#define __CONFIGS_BTT_H__

#include <linux/sizes.h>
/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		SZ_256M	/* Max 256 MB RAM */
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_BTT_H__ */
