/*
 * (C) Copyright 2017
 * Patrice Chotard, <patrice.chotard@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <config.h>

/* ram memory-related information */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x40000000
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define PHYS_SDRAM_1_SIZE		0x3FE00000
#define CONFIG_SYS_TEXT_BASE		0x7D600000
#define CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1	/* default load addr */

#define CONFIG_SYS_HZ_CLOCK		1000000000	/* 1 GHz */

#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk consoleblank=0 ignore_loglevel"

/* Environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"board= B2260" \
	"load_addr= #CONFIG_SYS_LOAD_ADDR \0"

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE 0x4000

/* Extra Commands */
#define CONFIG_CMD_ASKENV
#define CONFIG_SYS_LONGHELP

#define CONFIG_SETUP_MEMORY_TAGS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		0x1800000
#define CONFIG_SYS_GBL_DATA_SIZE	1024	/* Global data structures */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - \
					 CONFIG_SYS_MALLOC_LEN - \
					 CONFIG_SYS_GBL_DATA_SIZE)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SKIP_LOWLEVEL_INIT

#endif /* __CONFIG_H */
