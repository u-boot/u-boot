/*
 * Copyright (C) 2013, 2014 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6<Q,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"
#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>
#include <linux/sizes.h>

#define CONFIG_MX6

#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define PHYS_SDRAM_SIZE			(512u * SZ_1M)
#elif defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
#define PHYS_SDRAM_SIZE			(1024u * SZ_1M)
#endif

#if defined(CONFIG_MBA6)

#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define CONFIG_DEFAULT_FDT_FILE		"imx6dl-mba6x.dtb"
#elif defined(CONFIG_MX6Q) || defined(CONFIG_MX6Q)
#define CONFIG_DEFAULT_FDT_FILE		"imx6q-mba6x.dtb"
#endif

#endif

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_GENERIC_BOARD

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_MXC_GPIO
#define CONFIG_MXC_UART

/* SPI */
#define CONFIG_CMD_SPI
#define CONFIG_MXC_SPI

/* SPI Flash */
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO

#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	(0 | (IMX_GPIO_NR(3, 19) << 8))
#define CONFIG_SF_DEFAULT_SPEED	50000000
#define CONFIG_SF_DEFAULT_MODE	(SPI_MODE_0)

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75
#if defined(CONFIG_MBA6)
#define CONFIG_DTT_SENSORS		{ 0, 1 }
#else
#define CONFIG_DTT_SENSORS		{ 0 }
#endif
#define CONFIG_DTT_MAX_TEMP		70
#define CONFIG_DTT_MIN_TEMP		-30
#define CONFIG_DTT_HYSTERESIS	3
#define CONFIG_CMD_DTT

/* I2C EEPROM (M24C64) */
#define CONFIG_SYS_I2C_EEPROM_ADDR			0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN			2
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_BITS		5 /* 32 Bytes */
#define CONFIG_SYS_I2C_EEPROM_PAGE_WRITE_DELAY_MS	20
#define CONFIG_CMD_EEPROM

#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08
#define TQMA6_PFUZE100_I2C_BUS		2

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* Fuses */
#define CONFIG_MXC_OCOTP
#define CONFIG_CMD_FUSE

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_PHYLIB
#define CONFIG_MII

#if defined(CONFIG_MBA6)

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_MXC_PHYADDR		0x03
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_KSZ9031

#else

#error "define PHY to use for your baseboard"

#endif

#define CONFIG_ARP_TIMEOUT		200UL
/* Network config - Allow larger/faster download for TFTP/NFS */
#define CONFIG_IP_DEFRAG
#define CONFIG_TFTP_BLOCKSIZE	4096
#define CONFIG_NFS_READ_SIZE	4096

#if defined(CONFIG_MBA6)

#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc1"

#else

#error "define baseboard specific things (uart, number of SD-card slots)"

#endif

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Command definition */
#include <config_cmd_default.h>

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_ITEST
#define CONFIG_CMD_SETEXPR
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY		3

#define CONFIG_LOADADDR			0x12000000

/* place code in last 4 MiB of RAM */
#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define CONFIG_SYS_TEXT_BASE		0x2fc00000
#elif defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
#define CONFIG_SYS_TEXT_BASE		0x4fc00000
#endif

#define CONFIG_ENV_SIZE			(SZ_8K)
/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * SZ_1M)

#if defined(CONFIG_TQMA6X_MMC_BOOT)

#define CONFIG_ENV_IS_IN_MMC
#define TQMA6_UBOOT_OFFSET		SZ_1K
#define TQMA6_UBOOT_SECTOR_START	0x2
#define TQMA6_UBOOT_SECTOR_COUNT	0x7fe

#define CONFIG_ENV_OFFSET		SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

#define TQMA6_FDT_OFFSET		(2 * SZ_1M)
#define TQMA6_FDT_SECTOR_START		0x1000
#define TQMA6_FDT_SECTOR_COUNT		0x800

#define TQMA6_KERNEL_SECTOR_START	0x2000
#define TQMA6_KERNEL_SECTOR_COUNT	0x2000

#define TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                       \
	"uboot_start="__stringify(TQMA6_UBOOT_SECTOR_START)"\0"                \
	"uboot_size="__stringify(TQMA6_UBOOT_SECTOR_COUNT)"\0"                 \
	"fdt_start="__stringify(TQMA6_FDT_SECTOR_START)"\0"                    \
	"fdt_size="__stringify(TQMA6_FDT_SECTOR_COUNT)"\0"                     \
	"kernel_start="__stringify(TQMA6_KERNEL_SECTOR_START)"\0"              \
	"kernel_size="__stringify(TQMA6_KERNEL_SECTOR_COUNT)"\0"               \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0"                       \
	"loadimage=mmc dev ${mmcdev}; "                                        \
		"mmc read ${loadaddr} ${kernel_start} ${kernel_size};\0"       \
	"loadfdt=mmc dev ${mmcdev}; "                                          \
		"mmc read ${fdt_addr} ${fdt_start} ${fdt_size};\0"             \
	"update_uboot=if tftp ${uboot}; then "                                 \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} / 0x200; "                   \
			"setexpr blkc ${blkc} + 1; "                           \
			"if itest ${blkc} <= ${uboot_size}; then "             \
				"mmc write ${loadaddr} ${uboot_start} "        \
					"${blkc}; "                            \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \
	"update_kernel=run kernel_name; "                                      \
		"if tftp ${kernel}; then "                                     \
			"if itest ${filesize} > 0; then "                      \
				"mmc dev ${mmcdev}; mmc rescan; "              \
				"setexpr blkc ${filesize} / 0x200; "           \
				"setexpr blkc ${blkc} + 1; "                   \
				"if itest ${blkc} <= ${kernel_size}; then "    \
					"mmc write ${loadaddr} "               \
						"${kernel_start} ${blkc}; "    \
				"fi; "                                         \
			"fi; "                                                 \
		"fi; "                                                         \
		"setenv filesize; setenv blkc \0"                              \
	"update_fdt=if tftp ${fdt_file}; then "                                \
		"if itest ${filesize} > 0; then "                              \
			"mmc dev ${mmcdev}; mmc rescan; "                      \
			"setexpr blkc ${filesize} / 0x200; "                   \
			"setexpr blkc ${blkc} + 1; "                           \
			"if itest ${blkc} <= ${fdt_size}; then "               \
				"mmc write ${loadaddr} ${fdt_start} ${blkc}; " \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize; setenv blkc \0"                              \

#define CONFIG_BOOTCOMMAND \
	"run mmcboot; run netboot; run panicboot"

#elif defined(CONFIG_TQMA6X_SPI_BOOT)

#define CONFIG_FLASH_SECTOR_SIZE	0x10000

#define TQMA6_UBOOT_OFFSET		0x400
#define TQMA6_UBOOT_SECTOR_START	0x0
/* max u-boot size: 512k */
#define TQMA6_UBOOT_SECTOR_SIZE		CONFIG_FLASH_SECTOR_SIZE
#define TQMA6_UBOOT_SECTOR_COUNT	0x8
#define TQMA6_UBOOT_SIZE		(TQMA6_UBOOT_SECTOR_SIZE * \
					 TQMA6_UBOOT_SECTOR_COUNT)

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET		(TQMA6_UBOOT_SIZE)
#define CONFIG_ENV_SECT_SIZE		CONFIG_FLASH_SECTOR_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
					 CONFIG_ENV_SECT_SIZE)

#define CONFIG_ENV_SPI_BUS		(CONFIG_SF_DEFAULT_BUS)
#define CONFIG_ENV_SPI_CS		(CONFIG_SF_DEFAULT_CS)
#define CONFIG_ENV_SPI_MAX_HZ		(CONFIG_SF_DEFAULT_SPEED)
#define CONFIG_ENV_SPI_MODE		(CONFIG_SF_DEFAULT_MODE)

#define TQMA6_FDT_OFFSET		(CONFIG_ENV_OFFSET_REDUND + \
					 CONFIG_ENV_SECT_SIZE)
#define TQMA6_FDT_SECT_SIZE		(CONFIG_FLASH_SECTOR_SIZE)

#define TQMA6_FDT_SECTOR_START		0x0a /* 8 Sector u-boot, 2 Sector env */
#define TQMA6_FDT_SECTOR_COUNT		0x01

#define TQMA6_KERNEL_SECTOR_START	0x10
#define TQMA6_KERNEL_SECTOR_COUNT	0x60

#define TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                       \
	"mmcblkdev=0\0"                                                        \
	"uboot_offset="__stringify(TQMA6_UBOOT_OFFSET)"\0"                     \
	"uboot_sectors="__stringify(TQMA6_UBOOT_SECTOR_COUNT)"\0"              \
	"fdt_start="__stringify(TQMA6_FDT_SECTOR_START)"\0"                    \
	"fdt_sectors="__stringify(TQMA6_FDT_SECTOR_COUNT)"\0"                  \
	"kernel_start="__stringify(TQMA6_KERNEL_SECTOR_START)"\0"              \
	"kernel_sectors="__stringify(TQMA6_KERNEL_SECTOR_COUNT)"\0"            \
	"update_uboot=if tftp ${uboot}; then "                                 \
		"if itest ${filesize} > 0; then "                              \
			"setexpr blkc ${filesize} + "                          \
				__stringify(TQMA6_UBOOT_OFFSET) "; "           \
			"setexpr size ${uboot_sectors} * "                     \
				__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "      \
			"if itest ${blkc} <= ${size}; then "                   \
				"sf probe; "                                   \
				"sf erase 0 ${size}; "                         \
				"sf write ${loadaddr} ${uboot_offset} "        \
					"${filesize}; "                        \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize 0; setenv blkc; setenv size \0"               \
	"update_kernel=run kernel_name; if tftp ${kernel}; then "              \
		"if itest ${filesize} > 0; then "                              \
			"setexpr size ${kernel_sectors} * "                    \
				__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "      \
			"setexpr offset ${kernel_start} * "                    \
				__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "      \
			"if itest ${filesize} <= ${size}; then "               \
				"sf probe; "                                   \
				"sf erase ${offset} ${size}; "                 \
				"sf write ${loadaddr} ${offset} "              \
					"${filesize}; "                        \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize 0; setenv size ; setenv offset\0"             \
	"update_fdt=if tftp ${fdt_file}; then "                                \
		"if itest ${filesize} > 0; then "                              \
			"setexpr size ${fdt_sectors} * "                       \
				__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "      \
			"setexpr offset ${fdt_start} * "                       \
				__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "      \
			"if itest ${filesize} <= ${size}; then "               \
				"sf probe; "                                   \
				"sf erase ${offset} ${size}; "                 \
				"sf write ${loadaddr} ${offset} "              \
					"${filesize}; "                        \
			"fi; "                                                 \
		"fi; fi; "                                                     \
		"setenv filesize 0; setenv size ; setenv offset\0"             \
	"loadimage=sf probe; "                                                 \
		"setexpr size ${kernel_sectors} * "                            \
			__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "              \
		"setexpr offset ${kernel_start} * "                            \
			__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "              \
		"sf read ${loadaddr} ${offset} ${size}; "                      \
		"setenv size ; setenv offset\0"                                \
	"loadfdt=sf probe; "                                                   \
		"setexpr size ${fdt_sectors} * "                               \
			__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "              \
		"setexpr offset ${fdt_start} * "                               \
			__stringify(CONFIG_FLASH_SECTOR_SIZE)"; "              \
		"sf read ${${fdt_addr}} ${offset} ${size}; "                   \
		"setenv size ; setenv offset\0"                                \


#define CONFIG_BOOTCOMMAND                                                     \
	"sf probe; run mmcboot; run netboot; run panicboot"                    \

#else

#error "need to define boot source"

#endif

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA6_FDT_ADDRESS		0x18000000

#define CONFIG_EXTRA_ENV_SETTINGS                                              \
	"board=tqma6\0"                                                        \
	"uimage=uImage\0"                                                      \
	"zimage=zImage\0"                                                      \
	"boot_type=bootz\0"                                                    \
	"kernel_name=if test \"${boot_type}\" != bootz; then "                 \
		"setenv kernel ${uimage}; "                                    \
		"else setenv kernel ${zimage}; fi\0"                           \
	"uboot=u-boot.imx\0"                                                   \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0"                               \
	"fdt_addr="__stringify(TQMA6_FDT_ADDRESS)"\0"                          \
	"console=" CONFIG_CONSOLE_DEV "\0"                                     \
	"fdt_high=0xffffffff\0"                                                \
	"initrd_high=0xffffffff\0"                                             \
	"addtty=setenv bootargs ${bootargs} console=${console},${baudrate}\0"  \
	"addfb=setenv bootargs ${bootargs} "                                   \
		"imx-fbdev.legacyfb_depth=32 consoleblank=0\0"                 \
	"mmcpart=2\0"                                                          \
	"mmcblkdev=0\0"                                                        \
	"mmcargs=run addmmc addtty addfb\0"                                    \
	"addmmc=setenv bootargs ${bootargs} "                                  \
		"root=/dev/mmcblk${mmcblkdev}p${mmcpart} rw rootwait\0"        \
	"mmcboot=echo Booting from mmc ...; "                                  \
		"setenv bootargs; "                                            \
		"run mmcargs; "                                                \
		"run loadimage; "                                              \
		"if run loadfdt; then "                                        \
			"echo boot device tree kernel ...; "                   \
			"${boot_type} ${loadaddr} - ${fdt_addr}; "             \
		"else "                                                        \
			"${boot_type}; "                                       \
		"fi;\0"                                                        \
		"setenv bootargs \0"                                           \
	"netdev=eth0\0"                                                        \
	"rootpath=/srv/nfs/tqma6\0"                                            \
	"ipmode=static\0"                                                      \
	"netargs=run addnfs addip addtty addfb\0"                              \
	"addnfs=setenv bootargs ${bootargs} "                                  \
		"root=/dev/nfs rw "                                            \
		"nfsroot=${serverip}:${rootpath},v3,tcp;\0"                    \
	"addip_static=setenv bootargs ${bootargs} "                            \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:"            \
		"${hostname}:${netdev}:off\0"                                  \
	"addip_dynamic=setenv bootargs ${bootargs} ip=dhcp\0"                  \
	"addip=if test \"${ipmode}\" != static; then "                         \
		"run addip_dynamic; else run addip_static; fi\0"               \
	"set_getcmd=if test \"${ipmode}\" != static; then "                    \
		"setenv getcmd dhcp; setenv autoload yes; "                    \
		"else setenv getcmd tftp; setenv autoload no; fi\0"            \
	"netboot=echo Booting from net ...; "                                  \
		"run kernel_name; "                                            \
		"run set_getcmd; "                                             \
		"setenv bootargs; "                                            \
		"run netargs; "                                                \
		"if ${getcmd} ${kernel}; then "                                \
			"if ${getcmd} ${fdt_addr} ${fdt_file}; then "          \
				"${boot_type} ${loadaddr} - ${fdt_addr}; "     \
			"fi; "                                                 \
		"fi; "                                                         \
		"echo ... failed\0"                                            \
	"panicboot=echo No boot device !!! reset\0"                            \
	TQMA6_EXTRA_BOOTDEV_ENV_SETTINGS                                      \

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		512

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE		(128u * SZ_1K)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#endif /* __CONFIG_H */
