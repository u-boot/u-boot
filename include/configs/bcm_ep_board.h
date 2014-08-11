/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BCM_EP_BOARD_H
#define __BCM_EP_BOARD_H

#include <asm/arch/configs.h>

/* Architecture, CPU, chip, etc */
#define CONFIG_ARMV7
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_SYS_GENERIC_BOARD

/*
 * Memory configuration
 * (these must be defined elsewhere)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#error	CONFIG_SYS_TEXT_BASE must be defined!
#endif
#ifndef CONFIG_SYS_SDRAM_BASE
#error	CONFIG_SYS_SDRAM_BASE must be defined!
#endif
#ifndef CONFIG_SYS_SDRAM_SIZE
#error	CONFIG_SYS_SDRAM_SIZE must be defined!
#endif

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_STACKSIZE		(256 * 1024)

/* Some commands use this as the default load address */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/* No mtest functions as recommended */
#undef CONFIG_CMD_MEMORY

/*
 * This is the initial SP which is used only briefly for relocating the u-boot
 * image to the top of SDRAM. After relocation u-boot moves the stack to the
 * proper place.
 */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Serial Info */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_BAUDRATE			115200

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_NO_FLASH	/* Not using NAND/NOR unmanaged flash */

/* console configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
			sizeof(CONFIG_SYS_PROMPT) + 16)	/* Printbuffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * One partition type must be defined for part.c
 * This is necessary for the fatls command to work on an SD card
 * for example.
 */
#define CONFIG_DOS_PARTITION

/* version string, parser, etc */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_SYS_LONGHELP

#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/* Commands */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE

/* Enable devicetree support */
#define CONFIG_OF_LIBFDT

/* SHA hashing */
#define CONFIG_CMD_HASH
#define CONFIG_HASH_VERIFY
#define CONFIG_SHA1
#define CONFIG_SHA256

/* Enable Time Command */
#define CONFIG_CMD_TIME

#define CONFIG_CMD_BOOTZ

/* Misc utility code */
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CRC32_VERIFY

#endif /* __BCM_EP_BOARD_H */
