/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <configs/bmips_common.h>
#include <configs/bmips_bcm63268.h>

#define CONFIG_REMAKE_ELF

#ifdef CONFIG_MTD_RAW_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELF_INIT
#endif /* CONFIG_MTD_RAW_NAND */
