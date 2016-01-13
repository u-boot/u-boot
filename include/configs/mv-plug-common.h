/*
 * (C) Copyright 2009-2015
 * Marvell Semiconductor <www.marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_MARVELL_PLUG_H
#define _CONFIG_MARVELL_PLUG_H


/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/* Add target to build it automatically upon "make" */
#define CONFIG_BUILD_TARGET     "u-boot.kwb"

/*
 * Compression configuration
 */
#ifdef CONFIG_SYS_MVFS
#define CONFIG_BZIP2
#define CONFIG_LZMA
#define CONFIG_CMD_BOOTZ
#endif /* CONFIG_SYS_MVFS */

/*
 * Enable device tree support
 */
#define CONFIG_OF_LIBFDT

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MII

/*
 * Extra file system
 */
#define CONFIG_CMD_EXT4

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * RTC driver configuration
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_MV
#endif /* CONFIG_CMD_DATE */

#define CONFIG_SYS_ALT_MEMTEST

#endif /* _CONFIG_MARVELL_PLUG_H */
