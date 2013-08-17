/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */
#ifndef __CONFIGS_M28EVK_H__
#define __CONFIGS_M28EVK_H__


/* System configurations */
#define CONFIG_MX28				/* i.MX28 SoC */
#define MACH_TYPE_M28EVK	3613
#define CONFIG_MACH_TYPE	MACH_TYPE_M28EVK

/* U-Boot Commands */
#define CONFIG_SYS_NO_FLASH
#include <config_cmd_default.h>
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_CMD_USB
#define	CONFIG_VIDEO

#define CONFIG_REGEX			/* Enable regular expression support */

/* Memory configuration */
#define CONFIG_NR_DRAM_BANKS		1		/* 1 bank of DRAM */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x20000000	/* Max 512 MB RAM */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Environment */
#define CONFIG_ENV_SIZE			(16 * 1024)
#define CONFIG_ENV_IS_IN_NAND

/* Environment is in NAND */
#if defined(CONFIG_CMD_NAND) && defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_RANGE		(512 * 1024)
#define CONFIG_ENV_OFFSET		0x300000
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)

#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nand0=gpmi-nand"
#define MTDPARTS_DEFAULT			\
	"mtdparts=gpmi-nand:"			\
		"3m(bootloader)ro,"		\
		"512k(environment),"		\
		"512k(redundant-environment),"	\
		"4m(kernel),"			\
		"128k(fdt),"			\
		"8m(ramdisk),"			\
		"-(filesystem)"
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

/* FEC Ethernet on SoC */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#endif

/* EEPROM */
#ifdef CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#endif

/* RTC */
#ifdef CONFIG_CMD_DATE
/* Use the internal RTC in the MXS chip */
#define CONFIG_RTC_INTERNAL
#ifdef CONFIG_RTC_INTERNAL
#define CONFIG_RTC_MXS
#else
#define CONFIG_RTC_M41T62
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	2000
#endif
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_MXS_PORT0
#define CONFIG_EHCI_MXS_PORT1
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_USB_STORAGE
#endif

/* SPI */
#ifdef CONFIG_CMD_SPI
#define CONFIG_DEFAULT_SPI_BUS		2
#define CONFIG_DEFAULT_SPI_CS		0
#define CONFIG_DEFAULT_SPI_MODE		SPI_MODE_0

/* SPI FLASH */
#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SF_DEFAULT_BUS		2
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		40000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0

#define CONFIG_ENV_SPI_BUS		2
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		40000000
#define CONFIG_ENV_SPI_MODE		SPI_MODE_0
#endif

#endif

/* LCD */
#ifdef CONFIG_VIDEO
#define	CONFIG_VIDEO_LOGO
#define	CONFIG_SPLASH_SCREEN
#define	CONFIG_CMD_BMP
#define	CONFIG_BMP_16BPP
#define	CONFIG_VIDEO_BMP_RLE8
#define	CONFIG_VIDEO_BMP_GZIP
#define	CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(512 << 10)
#endif

/* Booting Linux */
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_BOOTARGS		"console=ttyAMA0,115200n8 "
#define CONFIG_BOOTCOMMAND	"run bootcmd_net"
#define CONFIG_LOADADDR		0x42000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Extra Environment */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"update_nand_full_filename=u-boot.nand\0"			\
	"update_nand_firmware_filename=u-boot.sb\0"			\
	"update_sd_firmware_filename=u-boot.sd\0"			\
	"update_nand_firmware_maxsz=0x100000\0"				\
	"update_nand_stride=0x40\0"	/* MX28 datasheet ch. 12.12 */	\
	"update_nand_count=0x4\0"	/* MX28 datasheet ch. 12.12 */	\
	"update_nand_get_fcb_size="	/* Get size of FCB blocks */	\
		"nand device 0 ; "					\
		"nand info ; "						\
		"setexpr fcb_sz ${update_nand_stride} * ${update_nand_count};" \
		"setexpr update_nand_fcb ${fcb_sz} * ${nand_writesize}\0" \
	"update_nand_full="		/* Update FCB, DBBT and FW */	\
		"if tftp ${update_nand_full_filename} ; then "		\
		"run update_nand_get_fcb_size ; "			\
		"nand scrub -y 0x0 ${filesize} ; "			\
		"nand write.raw ${loadaddr} 0x0 ${fcb_sz} ; "	\
		"setexpr update_off ${loadaddr} + ${update_nand_fcb} ; " \
		"setexpr update_sz ${filesize} - ${update_nand_fcb} ; " \
		"nand write ${update_off} ${update_nand_fcb} ${update_sz} ; " \
		"fi\0"							\
	"update_nand_firmware="		/* Update only firmware */	\
		"if tftp ${update_nand_firmware_filename} ; then "	\
		"run update_nand_get_fcb_size ; "			\
		"setexpr fcb_sz ${update_nand_fcb} * 2 ; " /* FCB + DBBT */ \
		"setexpr fw_sz ${update_nand_firmware_maxsz} * 2 ; "	\
		"setexpr fw_off ${fcb_sz} + ${update_nand_firmware_maxsz};" \
		"nand erase ${fcb_sz} ${fw_sz} ; "			\
		"nand write ${loadaddr} ${fcb_sz} ${filesize} ; "	\
		"nand write ${loadaddr} ${fw_off} ${filesize} ; "	\
		"fi\0"							\
	"update_sd_firmware="		/* Update the SD firmware partition */ \
		"if mmc rescan ; then "					\
		"if tftp ${update_sd_firmware_filename} ; then "	\
		"setexpr fw_sz ${filesize} / 0x200 ; "	/* SD block size */ \
		"setexpr fw_sz ${fw_sz} + 1 ; "				\
		"mmc write ${loadaddr} 0x800 ${fw_sz} ; "		\
		"fi ; "							\
		"fi\0"

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_M28EVK_H__ */
