/*
 * Copyright 2012 Stefan Roese <sr@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5200
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_A3M071			/* ... on A3M071 board */
#define CONFIG_MPC5200_DDR		/* ... use DDR RAM	*/

#define	CONFIG_SYS_TEXT_BASE	0x01000000	/* boot low for 32 MiB boards */

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33MHz */

#define CONFIG_MISC_INIT_R
#define CONFIG_SYS_LOWBOOT		/* Enable lowboot	*/

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	    /* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE		\
	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BSP
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_REGINFO

/*
 * IPB Bus clocking configuration.
 */
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */
/* define for 66MHz speed - undef for 33MHz PCI clock speed */
#undef CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C				/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x53
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

/*
 * NOR flash configuration
 */
#define CONFIG_SYS_FLASH_BASE		0xfc000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x40000)

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_FLASH_ERASE_TOUT	240000
#define CONFIG_SYS_FLASH_WRITE_TOUT	500
#define CONFIG_SYS_FLASH_LOCK_TOUT	5
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR			0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR		0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_END		MPC5XXX_SRAM_SIZE


#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)
#define CONFIG_SYS_MALLOC_LEN		(1 << 20)
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC
#define CONFIG_MPC5xxx_FEC_MII100
#define CONFIG_PHY_ADDR			0x00

/*
 * GPIO configuration
 */

/*
 * GPIO-config depends on failsave-level
 * failsave 0 means just MPX-config, no digiboard, no fpga
 *          1 means digiboard ok
 *          2 means fpga ok
 */

/* for failsave-level 0 - full failsave */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x1005C005
/* for failsave-level 1 - only digiboard ok */
#define CONFIG_SYS_GPS_PORT_CONFIG_1	0x1005C005
/* for failsave-level 2 - all ok */
#define CONFIG_SYS_GPS_PORT_CONFIG_2	0x1005C005

/*
 * Configuration matrix
 *                        MSB                          LSB
 * failsave 0  0x1005C005  00010000000001011100000001100101  ( full failsave )
 * failsave 1  0x1005C005  00010000000001011100000001100101  ( digib.-ver ok )
 * failsave 2  0x1005C005  00010000000001011100000001100101  ( all ok )
 *                         || ||| ||  |   ||| |   |   |   |
 *                         || ||| ||  |   ||| |   |   |   |  bit rev name
 *                         ++-+++-++--+---+++-+---+---+---+-  0   31 CS1
 *                          +-+++-++--+---+++-+---+---+---+-  1   30 LPTZ
 *                            ||| ||  |   ||| |   |   |   |   2   29 ALTs
 *                            +++-++--+---+++-+---+---+---+-  3   28 ALTs
 *                             ++-++--+---+++-+---+---+---+-  4   27 CS7
 *                              +-++--+---+++-+---+---+---+-  5   26 CS6
 *                                ||  |   ||| |   |   |   |   6   25 ATA
 *                                ++--+---+++-+---+---+---+-  7   24 ATA
 *                                 +--+---+++-+---+---+---+-  8   23 IR_USB_CLK
 *                                    |   ||| |   |   |   |   9   22 IRDA
 *                                    |   ||| |   |   |   |  10   21 IRDA
 *                                    +---+++-+---+---+---+- 11   20 IRDA
 *                                        ||| |   |   |   |  12   19 Ether
 *                                        ||| |   |   |   |  13   18 Ether
 *                                        ||| |   |   |   |  14   17 Ether
 *                                        +++-+---+---+---+- 15   16 Ether
 *                                         ++-+---+---+---+- 16   15 PCI_DIS
 *                                          +-+---+---+---+- 17   14 USB_SE
 *                                            |   |   |   |  18   13 USB
 *                                            +---+---+---+- 19   12 USB
 *                                                |   |   |  20   11 PSC3
 *                                                |   |   |  21   10 PSC3
 *                                                |   |   |  22    9 PSC3
 *                                                +---+---+- 23    8 PSC3
 *                                                    |   |  24    7 -
 *                                                    |   |  25    6 PSC2
 *                                                    |   |  26    5 PSC2
 *                                                    +---+- 27    4 PSC2
 *                                                        |  28    3 -
 *                                                        |  29    2 PSC1
 *                                                        |  30    1 PSC1
 *                                                        +- 31    0 PSC1
 */


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"=> "

#define CONFIG_CMDLINE_EDITING
#define	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024
#else
#define CONFIG_SYS_CBSIZE		256
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x00100000
#define CONFIG_SYS_MEMTEST_END		0x00f00000

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define CONFIG_SYS_HZ			1000
#define CONFIG_LOOPW
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* don't print console @ startup*/

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		(HID0_ICE | HID0_ICFI)
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#define CONFIG_SYS_BOOTCS_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS2_START		0xe0000000
#define CONFIG_SYS_CS2_SIZE		0x00100000

/* FPGA slave io (512kiB) - see ticket #66 */
#define CONFIG_SYS_CS3_START		0xE9000000
#define CONFIG_SYS_CS3_SIZE		0x00080000
/* 00000000 00110010 1 0 1 1 10 01 00 00 0 0 0 0  = 0x0032B900 */
#define CONFIG_SYS_CS3_CFG		0x0032B900

/* Diagnosis Interface - see ticket #63 */
#define CONFIG_SYS_CS4_START		0xEA000000
#define CONFIG_SYS_CS4_SIZE		0x00000001
/* 00000000 00000010 1 0 1 1 10 01 00 00 0 0 0 0  = 0x0002B900 */
#define CONFIG_SYS_CS4_CFG		0x0002B900

/* FPGA master io (64kiB) - see ticket #66 */
#define CONFIG_SYS_CS5_START		0xE8000000
#define CONFIG_SYS_CS5_SIZE		0x00010000
/* 00000000 00110010 1 0 1 1 10 01 00 00 0 0 0 0  = 0x0032B900 */
#define CONFIG_SYS_CS5_CFG		0x0032B900

#ifdef CONFIG_SYS_PCICLK_EQUALS_IPBCLK_DIV2	/* for pci_clk  = 66 MHz */
#define CONFIG_SYS_BOOTCS_CFG		0x0006F900
#define CONFIG_SYS_CS1_CFG		0x0004FB00
#define CONFIG_SYS_CS2_CFG		0x0006F90C
#else	/* for pci_clk = 33 MHz */
#define CONFIG_SYS_BOOTCS_CFG		0x0002F900
#define CONFIG_SYS_CS1_CFG		0x0001FB00
#define CONFIG_SYS_CS2_CFG		0x0002F90C
#endif

#define CONFIG_SYS_CS_BURST		0x00000000
/* set DC for FPGA CS5 and CS3 to 0 - see ticket #66 */
/* R  7  R  6  R  5  R  4  R  3  R  2  R  1  R  0  */
/* 00 11 00 11 00 00 00 11 00 00 00 00 00 00 00 00 */
#define CONFIG_SYS_CS_DEADCYCLE		0x33030000

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*
 * Environment Configuration
 */

#define CONFIG_BOOTDELAY	0	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_mtd\" to boot from flash with mtd filesystem;" \
	"echo Type \"run net_nfs\" to boot from tftp with nfs filesystem;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_SYS_OS_BASE	0xfc080000
#define CONFIG_SYS_FDT_BASE	0xfc060000

#define xstr(s)	str(s)
#define str(s)	#s

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"verify=no\0"							\
	"consoledev=ttyPSC0\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"mtdargs=setenv bootargs root=/dev/mtdblock4 rw rootfstype=jffs2\0"\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} "				\
		"console=${consoledev},${baudrate}\0"			\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr} - ${fdtaddr}\0"			\
	"flash_mtd=run mtdargs addip addtty;"				\
		"bootm ${kernel_addr} - ${fdtaddr}\0"			\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdtaddr}\0"	\
	"net_nfs=sleep 2; tftp ${loadaddr} ${bootfile};"		\
		"tftp c00000 ${fdtfile};"				\
		"run nfsargs addip addtty;"				\
		"bootm ${loadaddr} - c00000\0"				\
	"load=tftp ${loadaddr} u-boot.bin\0"				\
	"update=protect off fc000000 fc03ffff; "			\
		"era fc000000 fc03ffff; cp.b ${loadaddr} fc000000 40000\0"\
	"upd=run load;run update\0"					\
	"fdtaddr=" xstr(CONFIG_SYS_FDT_BASE) "\0"			\
	"fdtfile=dtbFile\0"						\
	"kernel_addr=" xstr(CONFIG_SYS_OS_BASE) "\0"			\
	""

#define CONFIG_BOOTCOMMAND	"run flash_mtd"

/*
 * SPL related defines
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_TEXT_BASE	0xfc000000
#define	CONFIG_SPL_START_S_PATH	"arch/powerpc/cpu/mpc5xxx"
#define CONFIG_SPL_LDSCRIPT	"arch/powerpc/cpu/mpc5xxx/u-boot-spl.lds"
#define CONFIG_SPL_LIBCOMMON_SUPPORT	/* image.c */
#define CONFIG_SPL_LIBGENERIC_SUPPORT	/* string.c */
#define CONFIG_SPL_SERIAL_SUPPORT

/* Place BSS for SPL near end of SDRAM */
#define CONFIG_SPL_BSS_START_ADDR	((128 - 1) << 20)
#define CONFIG_SPL_BSS_MAX_SIZE		(64 << 10)

#define CONFIG_SPL_OS_BOOT
/* Place patched DT blob (fdt) at this address */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x01800000

/* Settings for real U-Boot to be loaded from NOR flash */
#ifndef __ASSEMBLY__
extern char __spl_flash_end[];
#endif
#define CONFIG_SYS_UBOOT_BASE		__spl_flash_end
#define CONFIG_SYS_SPL_MAX_LEN		(32 << 10)
#define CONFIG_SYS_UBOOT_START		0x1000100

#endif /* __CONFIG_H */
