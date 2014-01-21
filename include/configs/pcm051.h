/*
 * pcm051.h
 *
 * Phytec phyCORE-AM335x (pcm051) boards information header
 *
 * Copyright (C) 2013 Lemonage Software GmbH
 * Author Lars Poeschel <poeschel@lemonage.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_PCM051_H
#define __CONFIG_PCM051_H

#define CONFIG_AM33XX
#define CONFIG_OMAP
#define CONFIG_OMAP_COMMON

#include <asm/arch/omap.h>

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"U-Boot# "
#define CONFIG_SYS_NO_FLASH
#define MACH_TYPE_PCM051		4144	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_PCM051

#define CONFIG_OF_LIBFDT
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_VERSION_VARIABLE

/* set to negative value for no autoboot */
#define CONFIG_BOOTDELAY		1
#define CONFIG_ENV_VARS_UBOOT_CONFIG
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80007fc0\0" \
	"fdtaddr=0x80000000\0" \
	"rdaddr=0x81000000\0" \
	"bootfile=uImage\0" \
	"fdtfile=pcm051.dtb\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 ro\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"ramroot=/dev/ram0 rw ramdisk_size=65536 initrd=${rdaddr},64M\0" \
	"ramrootfstype=ext2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=fatload mmc ${mmcdev} ${rdaddr} ramdisk.gz\0" \
	"loaduimagefat=fatload mmc ${mmcdev} ${loadaddr} ${bootfile}\0" \
	"loaduimage=ext2load mmc ${mmcdev}:2 ${loadaddr} ${bootfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"ramboot=echo Booting from ramdisk ...; " \
		"run ramargs; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loaduimage; then " \
			"run mmcboot;" \
		"fi;" \
	"fi;" \

/* Clock Defines */
#define V_OSCK				25000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_CMD_ECHO

/* max number of command args */
#define CONFIG_SYS_MAXARGS		16

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * memtest works on 8 MB in DRAM after skipping 32MB from
 * start addr of ram disk
 */
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + (64 << 20))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

#define CONFIG_SYS_LOAD_ADDR		0x80007fc0 /* Default load address */

#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2

#define CONFIG_SPI
#define CONFIG_OMAP3_SPI
#define CONFIG_MTD_DEVICE
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		24000000

 /* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1		/*  1 bank of DRAM */
#define CONFIG_MAX_RAM_BANK_SIZE	(1024 << 19)	/* 512MiB */

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
						GENERATED_GBL_DATA_SIZE)
 /* Platform/Board specific defs */
#define CONFIG_SYS_TIMERBASE		0x48040000	/* Use Timer2 */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

#define CONFIG_CONS_INDEX		1
/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

/* I2C Configuration */
#define CONFIG_I2C
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP24XX
#define CONFIG_CMD_EEPROM
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_MULTI_EEPROMS

#define CONFIG_OMAP_GPIO

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/* CPU */
#define CONFIG_ARCH_CPU_INIT

#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_CONSOLE_INFO_QUIET

#define CONFIG_ENV_IS_NOWHERE

/* Defines for SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_BOARD_INIT
/*
 * Place the image at the start of the ROM defined image space.
 * We limit our size to the ROM-defined downloaded image area, and use the
 * rest of the space for stack.
 */
#define CONFIG_SPL_TEXT_BASE		0x402F0400
#define CONFIG_SPL_MAX_SIZE		(0x4030C000 - CONFIG_SPL_TEXT_BASE)
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR

#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT
#define CONFIG_SPL_NET_SUPPORT
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_NET_VCI_STRING	"pcm051 U-Boot SPL"
#define CONFIG_SPL_ETH_SUPPORT
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_SPL_SPI_CS		0
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_SPI_U_BOOT_SIZE	0x40000
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80800000
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

/* Since SPL did pll and ddr initialization for us,
 * we don't need to do it twice.
 */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SKIP_LOWLEVEL_INIT
#endif

/*
 * USB configuration
 */
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_MUSB_GADGET
#define CONFIG_MUSB_PIO_ONLY
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_MUSB_HOST
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_PERIPHERAL
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#ifdef CONFIG_MUSB_HOST
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#endif

#ifdef CONFIG_MUSB_GADGET
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_RNDIS
#endif /* CONFIG_MUSB_GADGET */

/* Unsupported features */
#undef CONFIG_USE_IRQ

#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_DRIVER_TI_CPSW
#define CONFIG_MII
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT         10
#define CONFIG_NET_MULTI
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ADDR			0
#define CONFIG_PHY_SMSC

#endif	/* ! __CONFIG_PCM051_H */
