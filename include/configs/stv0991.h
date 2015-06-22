/*
 * (C) Copyright 2014
 * Vikas Manocha, STMicroelectronics, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_STV0991_H
#define __CONFIG_STV0991_H
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_EXCEPTION_VECTORS_HIGH
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_CORTEX_R4

#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_SYS_NO_FLASH

/* ram memory-related information */
#define CONFIG_NR_DRAM_BANKS			1
#define PHYS_SDRAM_1				0x00000000
#define CONFIG_SYS_SDRAM_BASE			PHYS_SDRAM_1
#define PHYS_SDRAM_1_SIZE			0x00198000

#define CONFIG_ENV_SIZE				0x10000
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR				\
	(PHYS_SDRAM_1_SIZE - CONFIG_ENV_SIZE)
#define CONFIG_SYS_MAXARGS			16
#define CONFIG_SYS_MALLOC_LEN			(CONFIG_ENV_SIZE + 16 * 1024)

/* serial port (PL011) configuration */
#define CONFIG_BAUDRATE				115200
#define CONFIG_PL01X_SERIAL

/* user interface */
#define CONFIG_SYS_PROMPT			"STV0991> "
#define CONFIG_SYS_CBSIZE			1024
#define CONFIG_SYS_PBSIZE			(CONFIG_SYS_CBSIZE \
						+sizeof(CONFIG_SYS_PROMPT) + 16)

/* MISC */
#define CONFIG_SYS_LOAD_ADDR			0x00000000
#define CONFIG_SYS_INIT_RAM_SIZE		0x8000
#define CONFIG_SYS_INIT_RAM_ADDR		0x00190000
#define CONFIG_SYS_INIT_SP_OFFSET		\
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
/* U-boot Load Address */
#define CONFIG_SYS_TEXT_BASE			0x00010000
#define CONFIG_SYS_INIT_SP_ADDR			\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* GMAC related configs */

#define CONFIG_MII
#define CONFIG_PHYLIB
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_PHY_MICREL

/* Command support defines */
#define CONFIG_CMD_PING
#define CONFIG_PHY_RESET_DELAY			10000		/* in usec */

#define CONFIG_SYS_MEMTEST_START               0x0000
#define CONFIG_SYS_MEMTEST_END                 1024*1024
#define CONFIG_CMD_MEMTEST

/* Misc configuration */
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

#define CONFIG_BOOTDELAY                       3
#define CONFIG_BOOTCOMMAND                     "go 0x40040000"

#define CONFIG_OF_SEPARATE
#define CONFIG_OF_CONTROL
#define CONFIG_OF_LIBFDT
#endif /* __CONFIG_H */
