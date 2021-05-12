/* SPDX-License-Identifier:    GPL-2.0
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX2_COMMON_H__
#define __OCTEONTX2_COMMON_H__

#define CONFIG_SUPPORT_RAW_INITRD

/** Maximum size of image supported for bootm (and bootable FIT images) */
#define CONFIG_SYS_BOOTM_LEN		(256 << 20)

/** Memory base address */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_TEXT_BASE

/** Stack starting address */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xffff0)

/** Heap size for U-Boot */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 64 * 1024 * 1024)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

#define CONFIG_LAST_STAGE_INIT

/* Allow environment variable to be overwritten */
#define CONFIG_ENV_OVERWRITE

/* Autoboot options */
#define CONFIG_RESET_TO_RETRY
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_BOOT_RETRY_MIN		30

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/** Extra environment settings */
#define CONFIG_EXTRA_ENV_SETTINGS	\
					"loadaddr=20080000\0"	\
					"ethrotate=yes\0"	\
					"autoload=0\0"

/** Environment defines */
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/** Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MAXARGS		64	/** max command args */

#define CONFIG_SYS_MMC_MAX_BLK_COUNT	8192

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		env_get("prompt")

#if defined(CONFIG_MMC_OCTEONTX)
#define MMC_SUPPORTS_TUNING
/** EMMC specific defines */
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_SUPPORT_EMMC_RPMB
#endif

#endif /* __OCTEONTX2_COMMON_H__ */
