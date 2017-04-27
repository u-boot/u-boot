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
	defined(CONFIG_SCSI) || \
	defined(CONFIG_CMD_USB) || \
	defined(CONFIG_CMD_PART) || \
	defined(CONFIG_CMD_GPT) || \
	defined(CONFIG_MMC) || \
	defined(CONFIG_SYSTEMACE) || \
	defined(CONFIG_SANDBOX)
#define HAVE_BLOCK_DEVICE
#endif

#if (CONFIG_IS_ENABLED(PARTITION_UUIDS) || \
	CONFIG_IS_ENABLED(EFI_PARTITION) || \
	defined(CONFIG_RANDOM_UUID) || \
	defined(CONFIG_CMD_UUID) || \
	defined(CONFIG_BOOTP_PXE)) && \
	!defined(CONFIG_LIB_UUID)
#define CONFIG_LIB_UUID
#endif

#if (defined(CONFIG_RANDOM_UUID) || \
	defined(CONFIG_CMD_UUID)) && \
	(!defined(CONFIG_LIB_RAND) && \
	!defined(CONFIG_LIB_HW_RAND))
#define CONFIG_LIB_RAND
#endif

#ifndef CONFIG_SYS_PBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + 128)
#endif

#ifndef CONFIG_FIT_SIGNATURE
#define CONFIG_IMAGE_FORMAT_LEGACY
#endif

#ifdef CONFIG_DISABLE_IMAGE_LEGACY
#undef CONFIG_IMAGE_FORMAT_LEGACY
#endif

#ifdef CONFIG_DM_I2C
# ifdef CONFIG_SYS_I2C
#  error "Cannot define CONFIG_SYS_I2C when CONFIG_DM_I2C is used"
# endif
#endif

#ifndef CONFIG_CMDLINE
#undef CONFIG_CMDLINE_EDITING
#undef CONFIG_SYS_LONGHELP
#endif

#endif	/* __CONFIG_FALLBACKS_H */
