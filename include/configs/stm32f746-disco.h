/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_FLASH_BASE		0x08000000
#define CONFIG_SYS_INIT_SP_ADDR		0x20050000
#define CONFIG_SYS_TEXT_BASE		0x08000000

/*
 * Configuration of the external SDRAM memory
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_RAM_SIZE		(8 * 1024 * 1024)
#define CONFIG_SYS_RAM_CS		1
#define CONFIG_SYS_RAM_FREQ_DIV		2
#define CONFIG_SYS_RAM_BASE		0xC0000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_RAM_BASE
#define CONFIG_SYS_LOAD_ADDR		0xC0400000
#define CONFIG_LOADADDR			0xC0400000

#define CONFIG_SYS_MAX_FLASH_SECT	8
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(8 << 10)

#define CONFIG_STM32_GPIO
#define CONFIG_STM32_FLASH
#define CONFIG_STM32X7_SERIAL

#define CONFIG_DESIGNWARE_ETH
#define CONFIG_DW_GMAC_DEFAULT_DMA_PBL	(8)
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_MII
#define CONFIG_PHY_SMSC

#define CONFIG_STM32_HSE_HZ		25000000
#define CONFIG_SYS_CLK_FREQ		200000000 /* 200 MHz */
#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer is clocked at 1MHz */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)

#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk consoleblank=0 ignore_loglevel"
#define CONFIG_BOOTCOMMAND						\
	"run bootcmd_romfs"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs_romfs=uclinux.physaddr=0x08180000 root=/dev/mtdblock0\0" \
	"bootcmd_romfs=setenv bootargs ${bootargs} ${bootargs_romfs};" \
	"bootm 0x08044000 - 0x08042000\0"


/*
 * Command line configuration.
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#define CONFIG_CMD_MEM
#define CONFIG_CMD_CACHE
#endif /* __CONFIG_H */
