/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *  (C) Copyright 2011
 *  Avionic Design GmbH <www.avionic-design.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra2-common.h"

/* High-level configuration options */
#define TEGRA2_SYSMEM			"mem=384M@0M nvmem=128M@384M"
#define V_PROMPT			"Tegra2 (Medcom) # "
#define CONFIG_TEGRA2_BOARD_STRING	"Avionic Design Medcom"
#define CONFIG_SYS_BOARD_ODMDATA	0x2b0d8011

/* Board-specific serial config */
#define CONFIG_SERIAL_MULTI
#define CONFIG_TEGRA2_ENABLE_UARTD	/* UARTD: debug UART */
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_ENV_IS_NOWHERE

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA2_MMC
#define CONFIG_CMD_MMC

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

#define CONFIG_FIT

#define CONFIG_BOOTCOMMAND				\
	"mmc rescan;"					\
	"ext2load mmc 0 0x17000000 /boot/uImage;"	\
	"bootm"

#endif /* __CONFIG_H */
