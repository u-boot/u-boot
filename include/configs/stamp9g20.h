/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Achim Ehrlich <aehrlich@taskit.de>
 * taskit GmbH <www.taskit.de>
 *
 * (C) Copyright 2012
 * Markus Hubig <mhubig@imko.de>
 * IMKO GmbH <www.imko.de>
 *
 * Configuation settings for the stamp9g20 CPU module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires adapting the initial boot
 * program. Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */
#define CONFIG_SYS_TEXT_BASE		0x23f00000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432MHz crystal */

/* misc settings */
#define CONFIG_CMDLINE_TAG		/* pass commandline to Kernel */
#define CONFIG_SETUP_MEMORY_TAGS	/* pass memory defs to kernel */
#define CONFIG_INITRD_TAG		/* pass initrd param to kernel */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_BOARD_EARLY_INIT_f	/* call board_early_init_f() */
#define CONFIG_BOARD_POSTCLK_INIT	/* call board_postclk_init() */
#define CONFIG_DISPLAY_CPUINFO		/* display CPU Info at startup */

/* setting board specific options */
#ifdef CONFIG_PORTUXG20
# define CONFIG_MACH_TYPE		MACH_TYPE_PORTUXG20
# define CONFIG_MACB
#else
# define CONFIG_MACH_TYPE		MACH_TYPE_STAMP9G20
#endif

/*
 * SDRAM: 1 bank, 64 MB, base address 0x20000000
 * Already initialized before u-boot gets started.
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		ATMEL_BASE_CS1
#define CONFIG_SYS_SDRAM_SIZE		(64 << 20)

/*
 * Perform a SDRAM Memtest from the start of SDRAM
 * till the beginning of the U-Boot position in RAM.
 */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_TEXT_BASE - 0x100000)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN \
	ROUND(3 * CONFIG_ENV_SIZE + (128 << 10), 0x1000)

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above that
 * address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM1 + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* NAND flash settings */
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PC13

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO		/* enable the GPIO features */
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200

/* LED configuration */
#define CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED

/* The LED PINs */
#define CONFIG_RED_LED			AT91_PIN_PC5
#define CONFIG_GREEN_LED		AT91_PIN_PC4
#define CONFIG_YELLOW_LED		AT91_PIN_PC10

#define STATUS_LED_RED			0
#define STATUS_LED_GREEN		1
#define STATUS_LED_YELLOW		2

/* Red LED */
#define STATUS_LED_BIT			STATUS_LED_RED
#define STATUS_LED_STATE		STATUS_LED_OFF
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)

/* Green LED */
#define STATUS_LED_BIT1			STATUS_LED_GREEN
#define STATUS_LED_STATE1		STATUS_LED_ON
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)

/* Yellow LED */
#define STATUS_LED_BIT2			STATUS_LED_YELLOW
#define STATUS_LED_STATE2		STATUS_LED_OFF
#define STATUS_LED_PERIOD2		(CONFIG_SYS_HZ / 2)

/* Boot status LED */
#define STATUS_LED_BOOT			STATUS_LED_GREEN

/*
 * Ethernet configuration
 *
 * PortuxG20 has always ethernet but for Stamp9G20 you
 * can enable it here if your baseboard features ethernet.
 */

#define CONFIG_MACB
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_MCS7830

#ifdef CONFIG_MACB
# define CONFIG_RMII			/* use reduced MII inteface */
# define CONFIG_NET_RETRY_COUNT	20      /* # of DHCP/BOOTP retries */
#define CONFIG_AT91_WANTS_COMMON_PHY

/* BOOTP and DHCP options */
# define CONFIG_BOOTP_BOOTFILESIZE
# define CONFIG_BOOTP_BOOTPATH
# define CONFIG_BOOTP_GATEWAY
# define CONFIG_BOOTP_HOSTNAME
# define CONFIG_NFSBOOTCOMMAND						\
	"setenv autoload yes; setenv autoboot yes; "			\
	"setenv bootargs ${basicargs} ${mtdparts} "			\
	"root=/dev/nfs ip=dhcp nfsroot=${serverip}:/srv/nfs/rootfs; "	\
	"dhcp"
#endif /* CONFIG_MACB */

/* Enable the watchdog */
#define CONFIG_AT91SAM9_WATCHDOG
#define CONFIG_HW_WATCHDOG

/* USB configuration */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_PLLB
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_UHP_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

/* General Boot Parameter */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTCOMMAND		"run flashboot"
#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE \
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

/*
 * RAM Memory address where to put the
 * Linux Kernel befor starting.
 */
#define CONFIG_SYS_LOAD_ADDR		0x22000000

/*
 * The NAND Flash partitions:
 * ==========================================
 * 0x0000000-0x001ffff -> 128k, bootstrap
 * 0x0020000-0x005ffff -> 256k, u-boot
 * 0x0060000-0x007ffff -> 128k, env1
 * 0x0080000-0x009ffff -> 128k, env2 (backup)
 * 0x0100000-0x06fffff ->   6M, kernel
 * 0x0700000-0x8000000 -> 121M, RootFS
 */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		((128 + 256) << 10)
#define CONFIG_ENV_OFFSET_REDUND	((128 + 256 + 128) << 10)
#define CONFIG_ENV_SIZE			(128 << 10)

/*
 * Predefined environment variables.
 * Usefull to define some easy to use boot commands.
 */
#define	CONFIG_EXTRA_ENV_SETTINGS					\
									\
	"basicargs=console=ttyS0,115200\0"				\
									\
	"mtdparts=mtdparts=atmel_nand:128k(bootstrap)ro,"		\
		"256k(uboot)ro,128k(env1)ro,"				\
		"128k(env2)ro,6M(linux),-(root)rw\0"			\
									\
	"flashboot=setenv bootargs ${basicargs} ${mtdparts} "		\
		"root=/dev/mtdblock5 rootfstype=jffs2; "		\
		"nand read 0x22000000 0x100000 0x600000; "		\
		"bootm 22000000\0"					\
									\
	"sdboot=setenv bootargs ${basicargs} ${mtdparts} "		\
		"root=/dev/mmcblk0p1 rootwait; "			\
		"nand read 0x22000000 0x100000 0x600000; "		\
		"bootm 22000000"

/* Command line & features configuration */
#include <config_cmd_default.h>
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS

#define CONFIG_CMD_NAND
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_CMD_LED

#ifdef CONFIG_MACB
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
#else
# undef CONFIG_CMD_BOOTD
# undef CONFIG_CMD_NET
# undef CONFIG_CMD_NFS
#endif /* CONFIG_MACB */

#endif /* __CONFIG_H */
