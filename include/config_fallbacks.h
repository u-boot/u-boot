/*
 * Copyright 2012 Texas Instruments
 *
 * This file is licensed under the terms of the GNU General Public
 * License Version 2. This file is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __CONFIG_FALLBACKS_H
#define __CONFIG_FALLBACKS_H

#ifdef CONFIG_SPL
#ifdef CONFIG_SPL_PAD_TO
#ifdef CONFIG_SPL_MAX_SIZE
#if CONFIG_SPL_PAD_TO && CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#error CONFIG_SPL_PAD_TO < CONFIG_SPL_MAX_SIZE
#endif
#endif
#else
#ifdef CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_PAD_TO	CONFIG_SPL_MAX_SIZE
#else
#define CONFIG_SPL_PAD_TO	0
#endif
#endif
#endif

#ifndef CONFIG_SYS_BAUDRATE_TABLE
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#endif

#if defined(CONFIG_CMD_FAT) && !defined(CONFIG_FS_FAT)
#define CONFIG_FS_FAT
#endif

#if (defined(CONFIG_CMD_EXT4) || defined(CONFIG_CMD_EXT2)) && \
						!defined(CONFIG_FS_EXT4)
#define CONFIG_FS_EXT4
#endif

#if defined(CONFIG_CMD_EXT4_WRITE) && !defined(CONFIG_EXT4_WRITE)
#define CONFIG_EXT4_WRITE
#endif

/* Rather than repeat this expression each time, add a define for it */
#if defined(CONFIG_CMD_IDE) || \
	defined(CONFIG_CMD_SATA) || \
	defined(CONFIG_CMD_SCSI) || \
	defined(CONFIG_CMD_USB) || \
	defined(CONFIG_CMD_PART) || \
	defined(CONFIG_MMC) || \
	defined(CONFIG_SYSTEMACE)
#define HAVE_BLOCK_DEVICE
#endif

#endif	/* __CONFIG_FALLBACKS_H */
