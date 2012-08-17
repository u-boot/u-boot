/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q Sabre Lite board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MX6Q
#define CONFIG_SYS_MX6_HCLK	       24000000
#define CONFIG_SYS_MX6_CLK32	       32768
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_MACH_TYPE	3769

#include <asm/arch/imx-regs.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN	       (CONFIG_ENV_SIZE + 2 * 1024 * 1024)

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	       UART2_BASE

#define CONFIG_CMD_SF
#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SST
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS  0
#define CONFIG_SF_DEFAULT_CS   (0|(GPIO_NUMBER(3, 19)<<8))
#define CONFIG_SF_DEFAULT_SPEED 25000000
#define CONFIG_SF_DEFAULT_MODE (SPI_MODE_0)
#endif

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_SYS_FSL_USDHC_NUM       2

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		6
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	       1
#define CONFIG_BAUDRATE			       115200
#define CONFIG_SYS_BAUDRATE_TABLE      {9600, 19200, 38400, 57600, 115200}

/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY	       3

#define CONFIG_LOADADDR			       0x10800000
#define CONFIG_SYS_TEXT_BASE	       0x17800000

#define CONFIG_EXTRA_ENV_SETTINGS \
       "script=boot.scr\0" \
       "uimage=uImage\0" \
	"console=ttymxc1\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
       "mmcdev=0\0" \
       "mmcpart=2\0" \
       "mmcroot=/dev/mmcblk0p3 rootwait rw\0" \
       "mmcargs=setenv bootargs console=${console},${baudrate} " \
	       "root=${mmcroot}\0" \
       "loadbootscript=" \
	       "fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
       "bootscript=echo Running bootscript from mmc ...; " \
	       "source\0" \
       "loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${uimage}\0" \
       "mmcboot=echo Booting from mmc ...; " \
	       "run mmcargs; " \
	       "bootm\0" \
       "netargs=setenv bootargs console=${console},${baudrate} " \
	       "root=/dev/nfs " \
	       "ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
       "netboot=echo Booting from net ...; " \
	       "run netargs; " \
	       "dhcp ${uimage}; bootm\0" \

#define CONFIG_BOOTCOMMAND \
       "mmc dev ${mmcdev};" \
       "if mmc rescan ${mmcdev}; then " \
	       "if run loadbootscript; then " \
		       "run bootscript; " \
	       "else " \
		       "if run loaduimage; then " \
			       "run mmcboot; " \
		       "else run netboot; " \
		       "fi; " \
	       "fi; " \
       "else run netboot; fi"

#define CONFIG_ARP_TIMEOUT     200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_SYS_PROMPT	       "MX6QSABRELITE U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE	       256

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	       16
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END	       0x10010000

#define CONFIG_SYS_LOAD_ADDR	       CONFIG_LOADADDR
#define CONFIG_SYS_HZ		       1000

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE	       (128 * 1024)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS	       1
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			       (1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
       (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
       (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE			(8 * 1024)

#define CONFIG_ENV_IS_IN_MMC
/* #define CONFIG_ENV_IS_IN_SPI_FLASH */

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(6 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(8 * 1024)
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#endif

#define CONFIG_OF_LIBFDT

#define CONFIG_SYS_DCACHE_OFF

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

#endif			       /* __CONFIG_H */
