/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Versatile PB.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS	1	/* This is an arm926ejs CPU core */
#define CONFIG_VERSATILE	1	/* in Versatile Platform Board	*/
#define CONFIG_ARCH_VERSATILE	1	/* Specifically, a Versatile	*/

#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000

#define CONFIG_SYS_TIMERBASE		0x101E2000	/* Timer 0 and 1 base */
#define CONFIG_SYS_TIMER_RATE		(1000000 / 256)
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMERBASE + 0x4)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/*
 * control registers
 */
#define VERSATILE_SCTL_BASE		0x101E0000	/* System controller */

/*
 * System controller bit assignment
 */
#define VERSATILE_REFCLK	0
#define VERSATILE_TIMCLK	1

#define VERSATILE_TIMER1_EnSel	15
#define VERSATILE_TIMER2_EnSel	17
#define VERSATILE_TIMER3_EnSel	19
#define VERSATILE_TIMER4_EnSel	21

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R		1
/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE			8192
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE	0x10010000
#undef CONFIG_SMC91111_EXT_PHY

/*
 * NS16550 Configuration
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK	24000000
#define CONFIG_PL01x_PORTS				\
			{(void *)CONFIG_SYS_SERIAL0,	\
			 (void *)CONFIG_SYS_SERIAL1 }
#define CONFIG_CONS_INDEX	0

#define CONFIG_BAUDRATE			38400
#define CONFIG_SYS_SERIAL0		0x101F1000
#define CONFIG_SYS_SERIAL1		0x101F2000

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS		"root=/dev/nfs mem=128M ip=dhcp "\
				"netdev=25,0,0xf1010000,0xf1010010,eth0 "\
				"console=ttyAMA0,38400n1"

/*
 * Static configuration when assigning fixed address
 */
#define CONFIG_BOOTFILE		"/tftpboot/uImage" /* file to load */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
/* Monitor Command Prompt	 */
#ifdef CONFIG_ARCH_VERSATILE_AB
# define CONFIG_SYS_PROMPT	"VersatileAB # "
#else
# define CONFIG_SYS_PROMPT	"VersatilePB # "
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB */
#define PHYS_FLASH_SIZE		0x04000000	/* 64MB */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x00800000
#define CONFIG_SYS_INIT_RAM_SIZE	0x000FFFFF
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_GBL_DATA_OFFSET)

#define CONFIG_BOARD_EARLY_INIT_F

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#ifdef CONFIG_ARCH_VERSATILE_QEMU
#define CONFIG_SYS_TEXT_BASE		0x10000
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_MONITOR_LEN		0x80000
#else
#define CONFIG_SYS_TEXT_BASE		0x01000000
/*
 * Use the CFI flash driver for ease of use
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_ENV_IS_IN_FLASH	1
/*
 *	System control register
 */
#define VERSATILE_SYS_BASE		0x10000000
#define VERSATILE_SYS_FLASH_OFFSET	0x4C
#define VERSATILE_FLASHCTRL		\
		(VERSATILE_SYS_BASE + VERSATILE_SYS_FLASH_OFFSET)
/* Enable writing to flash */
#define VERSATILE_FLASHPROG_FLVPPEN	(1 << 0)

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2 * CONFIG_SYS_HZ) /* Erase Timeout */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2 * CONFIG_SYS_HZ) /* Write Timeout */

/*
 * Note that CONFIG_SYS_MAX_FLASH_SECT allows for a parameter block
 * i.e.
 *	the bottom "sector" (bottom boot), or top "sector"
 *	(top boot), is a seperate erase region divided into
 *	4 (equal) smaller sectors. This, notionally, allows
 *	quicker erase/rewrire of the most frequently changed
 *	area......
 *	CONFIG_SYS_MAX_FLASH_SECT is padded up to a multiple of 4
 */

#ifdef CONFIG_ARCH_VERSATILE_AB
#define FLASH_SECTOR_SIZE		0x00020000	/* 128 KB sectors */
#define CONFIG_ENV_SECT_SIZE		(2 * FLASH_SECTOR_SIZE)
#define CONFIG_SYS_MAX_FLASH_SECT	(520)
#endif

#ifdef CONFIG_ARCH_VERSATILE_PB		/* Versatile PB is default	*/
#define FLASH_SECTOR_SIZE		0x00040000	/* 256 KB sectors */
#define CONFIG_ENV_SECT_SIZE		FLASH_SECTOR_SIZE
#define CONFIG_SYS_MAX_FLASH_SECT	(260)
#endif

#define CONFIG_SYS_FLASH_BASE		0x34000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SYS_MONITOR_LEN		(4 * CONFIG_ENV_SECT_SIZE)

/* The ARM Boot Monitor is shipped in the lowest sector of flash */

#define FLASH_TOP			(CONFIG_SYS_FLASH_BASE + PHYS_FLASH_SIZE)
#define CONFIG_ENV_ADDR			(FLASH_TOP - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_MONITOR_BASE		(CONFIG_ENV_ADDR - CONFIG_SYS_MONITOR_LEN)

#define CONFIG_SYS_FLASH_PROTECTION	/* The devices have real protection */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes */
#endif

#endif	/* __CONFIG_H */
