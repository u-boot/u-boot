/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author(s): Patrice CHOTARD, <patrice.chotard@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_FLASH_BASE		0x08000000

#define CONFIG_SYS_INIT_SP_ADDR		0x10010000

#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF

/*
 * Configuration of the external SDRAM memory
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_RAM_FREQ_DIV		2
#define CONFIG_SYS_RAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_RAM_BASE
#define CONFIG_SYS_LOAD_ADDR		0x00400000
#define CONFIG_LOADADDR			0x00400000

#define CONFIG_SYS_MAX_FLASH_SECT	12
#define CONFIG_SYS_MAX_FLASH_BANKS	2

#define CONFIG_ENV_OFFSET		(256 << 10)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			(8 << 10)

#define CONFIG_STM32_FLASH

#define CONFIG_SYS_CLK_FREQ		180000000 /* 180 MHz */
#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SYS_CBSIZE		1024

#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

#define CONFIG_BOOTCOMMAND						\
	"run boot_sd"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"boot_sd=mmc dev 0;fatload mmc 0 0x00700000 stm32429i-eval.dtb; fatload mmc 0 0x00008000 zImage; icache off; bootz 0x00008000 - 0x00700000"

/*
 * Command line configuration.
 */

#endif /* __CONFIG_H */
