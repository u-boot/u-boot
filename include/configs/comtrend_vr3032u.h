/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <configs/bmips_common.h>
#include <configs/bmips_bcm63268.h>

#define CONFIG_REMAKE_ELF

#define CONFIG_ENV_SIZE			(8 * 1024)

#ifdef CONFIG_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif /* CONFIG_NAND */
