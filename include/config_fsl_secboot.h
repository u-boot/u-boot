/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_FSL_SECBOOT_H
#define __CONFIG_FSL_SECBOOT_H

#ifdef CONFIG_SECURE_BOOT

#ifndef CONFIG_CMD_ESBC_VALIDATE
#define CONFIG_CMD_ESBC_VALIDATE
#endif

#ifndef CONFIG_EXTRA_ENV
#define CONFIG_EXTRA_ENV	""
#endif

/*
 * Control should not reach back to uboot after validation of images
 * for secure boot flow and therefore bootscript should have
 * the bootm command. If control reaches back to uboot anyhow
 * after validating images, core should just spin.
 */

/*
 * Define the key hash for boot script here if public/private key pair used to
 * sign bootscript are different from the SRK hash put in the fuse
 * Example of defining KEY_HASH is
 * #define CONFIG_BOOTSCRIPT_KEY_HASH \
 *	 "41066b564c6ffcef40ccbc1e0a5d0d519604000c785d97bbefd25e4d288d1c8b"
 */

#ifdef CONFIG_BOOTSCRIPT_KEY_HASH
#define CONFIG_SECBOOT \
	"setenv bs_hdraddr " __stringify(CONFIG_BOOTSCRIPT_HDR_ADDR)";" \
	"setenv bootargs \'root=/dev/ram rw console=ttyS0,115200 "	\
	"ramdisk_size=600000\';"	\
	CONFIG_EXTRA_ENV	\
	"esbc_validate $bs_hdraddr " \
	  __stringify(CONFIG_BOOTSCRIPT_KEY_HASH)";" \
	"source $img_addr;"	\
	"esbc_halt\0"
#else
#define CONFIG_SECBOOT \
	"setenv bs_hdraddr " __stringify(CONFIG_BOOTSCRIPT_HDR_ADDR)";" \
	"setenv bootargs \'root=/dev/ram rw console=ttyS0,115200 "	\
	"ramdisk_size=600000\';"	\
	CONFIG_EXTRA_ENV	\
	"esbc_validate $bs_hdraddr;" \
	"source $img_addr;"	\
	"esbc_halt\0"
#endif

/* For secure boot flow, default environment used will be used */
#if defined(CONFIG_SYS_RAMBOOT)
#if defined(CONFIG_RAMBOOT_SPIFLASH)
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#elif defined(CONFIG_RAMBOOT_NAND)
#undef CONFIG_ENV_IS_IN_NAND
#elif defined(CONFIG_RAMBOOT_SDCARD)
#undef CONFIG_ENV_IS_IN_MMC
#endif
#else /*CONFIG_SYS_RAMBOOT*/
#undef CONFIG_ENV_IS_IN_FLASH
#endif

#define CONFIG_ENV_IS_NOWHERE

/*
 * We don't want boot delay for secure boot flow
 * before autoboot starts
 */
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	0
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND		CONFIG_SECBOOT

/*
 * CONFIG_ZERO_BOOTDELAY_CHECK should not be defined for
 * secure boot flow as defining this would enable a user to
 * reach uboot prompt by pressing some key before start of
 * autoboot
 */
#undef CONFIG_ZERO_BOOTDELAY_CHECK

#endif
#endif
