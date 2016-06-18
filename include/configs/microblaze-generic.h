/*
 * (C) Copyright 2007-2010 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/xilinx/microblaze-generic/xparameters.h"

/* MicroBlaze CPU */
#define	MICROBLAZE_V5		1

/* linear and spi flash memory */
#ifdef XILINX_FLASH_START
#define	FLASH
#undef	SPIFLASH
#undef	RAMENV	/* hold environment in flash */
#else
#ifdef XILINX_SPI_FLASH_BASEADDR
#undef	FLASH
#define	SPIFLASH
#undef	RAMENV	/* hold environment in flash */
#else
#undef	FLASH
#undef	SPIFLASH
#define	RAMENV	/* hold environment in RAM */
#endif
#endif

/* uart */
# define CONFIG_BAUDRATE	115200
/* The following table includes the supported baudrates */
# define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* setting reset address */
/*#define	CONFIG_SYS_RESET_ADDRESS	CONFIG_SYS_TEXT_BASE*/

/* gpio */
#ifdef XILINX_GPIO_BASEADDR
# define CONFIG_XILINX_GPIO
# define CONFIG_SYS_GPIO_0_ADDR		XILINX_GPIO_BASEADDR
#endif
#define CONFIG_BOARD_LATE_INIT

/* watchdog */
#if defined(XILINX_WATCHDOG_BASEADDR) && defined(XILINX_WATCHDOG_IRQ)
# define CONFIG_WATCHDOG_BASEADDR	XILINX_WATCHDOG_BASEADDR
# define CONFIG_WATCHDOG_IRQ		XILINX_WATCHDOG_IRQ
# ifndef CONFIG_SPL_BUILD
#  define CONFIG_HW_WATCHDOG
#  define CONFIG_XILINX_TB_WATCHDOG
# endif
#endif

#define CONFIG_SYS_MALLOC_LEN	0xC0000

/* Stack location before relocation */
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_TEXT_BASE - \
					 CONFIG_SYS_MALLOC_F_LEN)

/*
 * CFI flash memory layout - Example
 * CONFIG_SYS_FLASH_BASE = 0x2200_0000;
 * CONFIG_SYS_FLASH_SIZE = 0x0080_0000;	  8MB
 *
 * SECT_SIZE = 0x20000;			128kB is one sector
 * CONFIG_ENV_SIZE = SECT_SIZE;		128kB environment store
 *
 * 0x2200_0000	CONFIG_SYS_FLASH_BASE
 *					FREE		256kB
 * 0x2204_0000	CONFIG_ENV_ADDR
 *					ENV_AREA	128kB
 * 0x2206_0000
 *					FREE
 * 0x2280_0000	CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE
 *
 */

#ifdef FLASH
# define CONFIG_SYS_FLASH_BASE		XILINX_FLASH_START
# define CONFIG_SYS_FLASH_SIZE		XILINX_FLASH_SIZE
# define CONFIG_SYS_FLASH_CFI		1
# define CONFIG_FLASH_CFI_DRIVER	1
/* ?empty sector */
# define CONFIG_SYS_FLASH_EMPTY_INFO	1
/* max number of memory banks */
# define CONFIG_SYS_MAX_FLASH_BANKS	1
/* max number of sectors on one chip */
# define CONFIG_SYS_MAX_FLASH_SECT	512
/* hardware flash protection */
# define CONFIG_SYS_FLASH_PROTECTION
/* use buffered writes (20x faster) */
# define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
# ifdef	RAMENV
#  define CONFIG_ENV_IS_NOWHERE	1
#  define CONFIG_ENV_SIZE	0x1000
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

# else	/* FLASH && !RAMENV */
#  define CONFIG_ENV_IS_IN_FLASH	1
/* 128K(one sector) for env */
#  define CONFIG_ENV_SECT_SIZE	0x20000
#  define CONFIG_ENV_ADDR \
			(CONFIG_SYS_FLASH_BASE + (2 * CONFIG_ENV_SECT_SIZE))
#  define CONFIG_ENV_SIZE	0x20000
# endif /* FLASH && !RAMBOOT */
#else /* !FLASH */

#ifdef SPIFLASH
# define CONFIG_SYS_NO_FLASH		1
# define CONFIG_SYS_SPI_BASE		XILINX_SPI_FLASH_BASEADDR
# define CONFIG_SPI			1
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
# define CONFIG_SF_DEFAULT_SPEED	XILINX_SPI_FLASH_MAX_FREQ
# define CONFIG_SF_DEFAULT_CS		XILINX_SPI_FLASH_CS

# ifdef	RAMENV
#  define CONFIG_ENV_IS_NOWHERE	1
#  define CONFIG_ENV_SIZE	0x1000
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)

# else	/* SPIFLASH && !RAMENV */
#  define CONFIG_ENV_IS_IN_SPI_FLASH	1
#  define CONFIG_ENV_SPI_MODE		SPI_MODE_3
#  define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#  define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
/* 128K(two sectors) for env */
#  define CONFIG_ENV_SECT_SIZE	0x10000
#  define CONFIG_ENV_SIZE	(2 * CONFIG_ENV_SECT_SIZE)
/* Warning: adjust the offset in respect of other flash content and size */
#  define CONFIG_ENV_OFFSET	(128 * CONFIG_ENV_SECT_SIZE) /* at 8MB */
# endif /* SPIFLASH && !RAMBOOT */
#else /* !SPIFLASH */

/* ENV in RAM */
# define CONFIG_SYS_NO_FLASH	1
# define CONFIG_ENV_IS_NOWHERE	1
# define CONFIG_ENV_SIZE	0x1000
# define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SIZE)
#endif /* !SPIFLASH */
#endif /* !FLASH */

#if defined(XILINX_USE_ICACHE)
# define CONFIG_ICACHE
#else
# undef CONFIG_ICACHE
#endif

#if defined(XILINX_USE_DCACHE)
# define CONFIG_DCACHE
#else
# undef CONFIG_DCACHE
#endif

#ifndef XILINX_DCACHE_BYTE_SIZE
#define XILINX_DCACHE_BYTE_SIZE	32768
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MFSL

#if defined(CONFIG_DCACHE) || defined(CONFIG_ICACHE)
#else
#endif

#if defined(FLASH)
# define CONFIG_CMD_JFFS2
# define CONFIG_CMD_UBI
# undef CONFIG_CMD_UBIFS

# if !defined(RAMENV)
#  define CONFIG_CMD_SAVES
# endif

#else
#if defined(SPIFLASH)

# if !defined(RAMENV)
#  define CONFIG_CMD_SAVES
# endif
#else
# undef CONFIG_CMD_JFFS2
# undef CONFIG_CMD_UBI
# undef CONFIG_CMD_UBIFS
#endif
#endif

#if defined(CONFIG_CMD_JFFS2)
# define CONFIG_MTD_PARTITIONS
#endif

#if defined(CONFIG_CMD_UBIFS)
# define CONFIG_CMD_UBI
# define CONFIG_LZO
#endif

#if defined(CONFIG_CMD_UBI)
# define CONFIG_MTD_PARTITIONS
# define CONFIG_RBTREE
#endif

#if defined(CONFIG_MTD_PARTITIONS)
/* MTD partitions */
#define CONFIG_CMD_MTDPARTS	/* mtdparts command line support */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=flash-0"

/* default mtd partition table */
#define MTDPARTS_DEFAULT	"mtdparts=flash-0:256k(u-boot),"\
				"256k(env),3m(kernel),1m(romfs),"\
				"1m(cramfs),-(jffs2)"
#endif

/* size of console buffer */
#define	CONFIG_SYS_CBSIZE	512
 /* print buffer size */
#define	CONFIG_SYS_PBSIZE \
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define	CONFIG_SYS_MAXARGS	15
#define	CONFIG_SYS_LONGHELP
/* default load address */
#define	CONFIG_SYS_LOAD_ADDR	0

#define	CONFIG_BOOTARGS		"root=romfs"
#define	CONFIG_HOSTNAME		XILINX_BOARD_NAME
#define	CONFIG_BOOTCOMMAND	"base 0;tftp 11000000 image.img;bootm"
#define	CONFIG_IPADDR		192.168.0.3
#define	CONFIG_SERVERIP		192.168.0.5
#define	CONFIG_GATEWAYIP	192.168.0.1

/* architecture dependent code */
#define	CONFIG_SYS_USR_EXCEP	/* user exception */

#define	CONFIG_PREBOOT	"echo U-BOOT for ${hostname};setenv preboot;echo"

#define	CONFIG_EXTRA_ENV_SETTINGS	"unlock=yes\0" \
					"nor0=flash-0\0"\
					"mtdparts=mtdparts=flash-0:"\
					"256k(u-boot),256k(env),3m(kernel),"\
					"1m(romfs),1m(cramfs),-(jffs2)\0"\
					"nc=setenv stdout nc;"\
					"setenv stdin nc\0" \
					"serial=setenv stdout serial;"\
					"setenv stdin serial\0"

#define CONFIG_CMDLINE_EDITING

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* Enable flat device tree support */
#define CONFIG_LMB		1

#if defined(CONFIG_XILINX_AXIEMAC)
# define CONFIG_MII		1
# define CONFIG_PHY_GIGE	1
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	1
# define CONFIG_PHY_ATHEROS	1
# define CONFIG_PHY_BROADCOM	1
# define CONFIG_PHY_DAVICOM	1
# define CONFIG_PHY_LXT		1
# define CONFIG_PHY_MARVELL	1
# define CONFIG_PHY_MICREL	1
# define CONFIG_PHY_MICREL_KSZ9021
# define CONFIG_PHY_NATSEMI	1
# define CONFIG_PHY_REALTEK	1
# define CONFIG_PHY_VITESSE	1
#else
# undef CONFIG_MII
#endif

/* SPL part */
#define CONFIG_CMD_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SPL_LDSCRIPT	"arch/microblaze/cpu/u-boot-spl.lds"

#define CONFIG_SPL_RAM_DEVICE
#ifdef CONFIG_SYS_FLASH_BASE
# define CONFIG_SPL_NOR_SUPPORT
# define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_FLASH_BASE
#endif

/* for booting directly linux */
#define CONFIG_SPL_OS_BOOT

#define CONFIG_SYS_OS_BASE		(CONFIG_SYS_FLASH_BASE + \
					 0x60000)
#define CONFIG_SYS_FDT_BASE		(CONFIG_SYS_FLASH_BASE + \
					 0x40000)
#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_TEXT_BASE + \
					 0x1000000)

/* SP location before relocation, must use scratch RAM */
/* BRAM start */
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
/* BRAM size - will be generated */
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

# define CONFIG_SPL_STACK_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_MALLOC_F_LEN)

/* Just for sure that there is a space for stack */
#define CONFIG_SPL_STACK_SIZE		0x100

#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE

#define CONFIG_SPL_MAX_FOOTPRINT	(CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_INIT_RAM_ADDR - \
					 CONFIG_SYS_MALLOC_F_LEN - \
					 CONFIG_SPL_STACK_SIZE)

#endif	/* __CONFIG_H */
