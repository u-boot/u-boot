/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <configs/bmips_common.h>
#include <configs/bmips_bcm6838.h>

#ifdef CONFIG_MTD_RAW_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif /* CONFIG_MTD_RAW_NAND */
