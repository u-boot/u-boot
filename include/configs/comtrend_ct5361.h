/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <configs/bmips_common.h>
#include <configs/bmips_bcm6348.h>

#define CONFIG_REMAKE_ELF

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(8 * 1024)

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
