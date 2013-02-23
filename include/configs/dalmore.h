/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/sizes.h>

#include "tegra114-common.h"

/* Must be off for Dalmore to boot !?!? FIXME */
#define CONFIG_SYS_DCACHE_OFF

/* Enable fdt support for Dalmore. Flash the image in u-boot-dtb.bin */
#define CONFIG_DEFAULT_DEVICE_TREE	tegra114-dalmore
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

/* High-level configuration options */
#define V_PROMPT		"Tegra114 (Dalmore) # "
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Dalmore"

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_DALMORE

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ENV_IS_NOWHERE

#define MACH_TYPE_DALMORE	4304	/* not yet in mach-types.h */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
