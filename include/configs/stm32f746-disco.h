/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* For booting Linux, use the first 6MB of memory */
#define CONFIG_SYS_BOOTMAPSZ		SZ_4M + SZ_2M

#define CONFIG_SYS_FLASH_BASE		0x08000000
#define CONFIG_SYS_INIT_SP_ADDR		0x20050000

#ifdef CONFIG_SUPPORT_SPL
#define CONFIG_SYS_LOAD_ADDR		0x08008000
#else
#define CONFIG_SYS_LOAD_ADDR		0xC0400000
#define CONFIG_LOADADDR			0xC0400000
#endif

/*
 * Configuration of the external SDRAM memory
 */

#define CONFIG_SYS_MAX_FLASH_SECT	8
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_STM32_FLASH

#define CONFIG_DW_GMAC_DEFAULT_DMA_PBL	(8)
#define CONFIG_DW_ALTDESCRIPTOR

#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0xC0008000\0"		\
			"fdtfile=stm32f746-disco.dtb\0"	\
			"fdt_addr_r=0xC0408000\0"		\
			"scriptaddr=0xC0418000\0"		\
			"pxefile_addr_r=0xC0428000\0" \
			"ramdisk_addr_r=0xC0438000\0"		\
			BOOTENV

#define CONFIG_DISPLAY_BOARDINFO

/* For SPL */
#ifdef CONFIG_SUPPORT_SPL
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_SPL_LEN		0x00008000
#define CONFIG_SYS_UBOOT_START		0x080083FD
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SYS_FLASH_BASE + \
					 CONFIG_SYS_SPL_LEN)

/* DT blob (fdt) address */
#define CONFIG_SYS_FDT_BASE		(CONFIG_SYS_FLASH_BASE + \
					0x1C0000)
#endif
/* For SPL ends */

/* For splashcreen */

#endif /* __CONFIG_H */
