/*
 * DENX M53 configuration
 * Copyright (C) 2012-2013 Marek Vasut <marex@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __M53EVK_CONFIG_H__
#define __M53EVK_CONFIG_H__

#define CONFIG_MX53
#define CONFIG_MXC_GPIO
#define CONFIG_SYS_HZ		1000

#include <asm/arch/imx-regs.h>

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_NO_FLASH

/*
 * U-Boot Commands
 */
#include <config_cmd_default.h>
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SATA
#define CONFIG_CMD_USB

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		2
#define PHYS_SDRAM_1			CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE		(512 * 1024 * 1024)
#define PHYS_SDRAM_2			CSD1_BASE_ADDR
#define PHYS_SDRAM_2_SIZE		(512 * 1024 * 1024)
#define PHYS_SDRAM_SIZE			(PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE)
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)
#define CONFIG_SYS_MEMTEST_START	0x70000000
#define CONFIG_SYS_MEMTEST_END		0xaff00000

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_TEXT_BASE		0x71000000

/*
 * U-Boot general configurations
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT	"=> "
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O buffer size */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
						/* Print buffer size */
#define CONFIG_SYS_MAXARGS	32		/* Max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot argument buffer size */
#define CONFIG_VERSION_VARIABLE			/* U-BOOT version */
#define CONFIG_AUTO_COMPLETE			/* Command auto complete */
#define CONFIG_CMDLINE_EDITING			/* Command history etc */
#define CONFIG_SYS_HUSH_PARSER

/*
 * Serial Driver
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/*
 * MMC Driver
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1
#endif

/*
 * NAND
 */
#define CONFIG_ENV_SIZE			(16 * 1024)
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		NFC_BASE_ADDR_AXI
#define CONFIG_NAND_MXC
#define CONFIG_MXC_NAND_REGS_BASE	NFC_BASE_ADDR_AXI
#define CONFIG_MXC_NAND_IP_REGS_BASE	NFC_BASE_ADDR
#define CONFIG_SYS_NAND_LARGEPAGE
#define CONFIG_MXC_NAND_HWECC
#define CONFIG_SYS_NAND_USE_FLASH_BBT

/* Environment is in NAND */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_RANGE		(512 * 1024)
#define CONFIG_ENV_OFFSET		0x100000
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)

#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nand0=mxc-nand"
#define MTDPARTS_DEFAULT			\
	"mtdparts=mxc-nand:"			\
		"1m(bootloader)ro,"		\
		"512k(environment),"		\
		"512k(redundant-environment),"	\
		"4m(kernel),"			\
		"128k(fdt),"			\
		"8m(ramdisk),"			\
		"-(filesystem)"
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

/*
 * Ethernet on SOC (FEC)
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x0
#define CONFIG_MII
#define CONFIG_DISCOVER_PHY
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#endif

/*
 * I2C
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_HARD_I2C
#define CONFIG_I2C_MXC
#define CONFIG_SYS_I2C_BASE		I2C2_BASE_ADDR
#define CONFIG_SYS_I2C_SPEED		100000
#endif

/*
 * RTC
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_M41T62
#define CONFIG_SYS_I2C_RTC_ADDR		0x68
#define CONFIG_SYS_M41T11_BASE_YEAR	2000
#endif

/*
 * USB
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX5
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT		1
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#endif

/*
 * SATA
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTFILE		"m53evk/uImage"
#define CONFIG_BOOTARGS		"console=ttymxc1,115200"
#define CONFIG_LOADADDR		0x70800000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR
#define CONFIG_OF_LIBFDT

/*
 * NAND SPL
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TARGET		"u-boot-with-nand-spl.imx"
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_TEXT_BASE		0x70008000
#define CONFIG_SPL_PAD_TO		0x8000
#define CONFIG_SPL_STACK		0x70004000
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SYS_NAND_U_BOOT_OFFS	CONFIG_SPL_PAD_TO
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_SIZE		(256 * 1024 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0

#endif	/* __M53EVK_CONFIG_H__ */
