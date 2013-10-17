/*
 * (C) Copyright 2011 HALE electronic <helmut.raiger@hale.at>
 * (C) Copyright 2008 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * Configuration settings for the HALE TT-01 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>

/* High Level Configuration Options */
#define CONFIG_ARM1136
#define CONFIG_MX31

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_MACH_TYPE	3726		/* not yet in mach-types.h */
#define CONFIG_SYS_TEXT_BASE	0xA0000000


/*
 * Physical Memory Map:
 *   CS settings are defined by i.MX31:
 *     - CSD0 and CDS1 are 256MB each, starting at 0x80000000 and 0x9000000
 *     - CS0 and CS1 are 128MB each, at A0000000 and A8000000
 *     - CS2 to CS5 are 32MB each, at B0.., B2.., B4.., B6..
 *
 * HALE set-up of the bluetechnix board for now is:
 *   - 128MB DDR (2x64MB, 2x16bit), connected to 32bit DDR ram interface
 *   - NOR-Flash (Spansion 32MB MCP, Flash+16MB PSRAM), 16bit interface at CS0
 *		- S71WS256ND0BFWYM (and CS1 for 64MB S71WS512ND0 without PSRAM)
 *        the flash chip is a mirrorbit S29WS256N !
 *   - the PSRAM is hooked to CS5 (0xB6000000)
 *   - Intel Strata Flash PF48F2000P0ZB00, 16bit interface at (CS0 or) CS1
 *     - 64Mbit = 8MByte (will go away in the production set-up)
 *   - NAND-Flash NAND01GR3B2BZA6 at NAND-FC:
 *		1Gbit=128MB, 2048+64 bytes/page, 64pages x 1024 blocks
 *   - Ethernet controller SMC9118 at CS4 via FPGA, 16bit interface
 *
 * u-boot will support the 32MB nor flash and the 128MB NAND flash, the PSRAM
 * is not used right now. We should be able to reduce the SOM to NAND flash
 * only and boot from there.
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE
#define PHYS_SDRAM_1_SIZE	(128 * 1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR	\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)

/* default load address, 1MB up the road */
#define CONFIG_SYS_LOAD_ADDR		(PHYS_SDRAM_1+0x100000)

/* Size of malloc() pool, make sure possible frame buffer fits */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 10*1024*1024)

/* memtest works on all but the last 1MB (u-boot) and malloc area  */
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END \
	(PHYS_SDRAM_1+(PHYS_SDRAM_1_SIZE-CONFIG_SYS_MALLOC_LEN-0x100000))

/* CFI FLASH driver setup */
#define CONFIG_SYS_FLASH_CFI		/* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER		/* Use drivers/cfi_flash.c */
#define CONFIG_FLASH_SPANSION_S29WS_N
/*
 * TODO: Bluetechnix (the supplier of the SOM) did define these values
 * in their original version of u-boot (1.2 or so). This should be
 * reviewed.
 *
 * #define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
 * #define CONFIG_SYS_FLASH_PROTECTION
 */
#define CONFIG_SYS_FLASH_BASE		CS0_BASE
#define CONFIG_SYS_MAX_FLASH_BANKS 1 /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT (254+8) /* max number of sectors per chip */

/*
 * FLASH and environment organization, only the Spansion chip is supported:
 * - it has 254 * 128kB + 8 * 32kB blocks
 * - this setup uses 4*32k+3*128k as monitor space = 0xA000 0000 to 0xA00F FFFF
 *		and 2 sectors with 128k as environment =
 *		A010 0000 to 0xA011 FFFF and 0xA012 0000 to 0xA013 FFFF
 * - this could be less, but this is only for developer versions of the board
 *   and no-one is going to use the NOR flash anyway.
 *
 * Monitor is at the beginning of the NOR-Flash, 1MB reserved. Again this is
 * way to large, but it avoids ENV overwrite (when updating u-boot) in case
 * size breaks the next boundary (as it has with 128k).
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)

#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(128 * 1024)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE

#define CONFIG_ENV_ADDR (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

/* Hardware drivers */

/*
 * on TT-01 UART1 pins are used by Audio, so we use UART2
 * TT-01 implements a hardware that turns off components depending on
 * the power level. In PL=1 the RS232 transceiver is usually off,
 * make sure that the transceiver is enabled during PL=1 for testing!
 */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART2_BASE

#define CONFIG_MXC_SPI
#define CONFIG_MXC_GPIO

/* MC13783 connected to CSPI3 and SS0 */
#define CONFIG_POWER
#define CONFIG_POWER_SPI
#define CONFIG_POWER_FSL

#define CONFIG_FSL_PMIC_BUS		2
#define CONFIG_FSL_PMIC_CS		0
#define CONFIG_FSL_PMIC_CLK		1000000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32

#define CONFIG_RTC_MC13XXX

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
/* console is UART2 on TT-01 */
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* ethernet setup for the onboard smc9118 */
#define CONFIG_MII
#define CONFIG_SMC911X
/* 16 bit, onboard ethernet, decoded via MACH-MX0 FPGA at 0x84200000 */
#define CONFIG_SMC911X_BASE		(CS4_BASE+0x200000)
#define CONFIG_SMC911X_16_BIT

/* mmc driver */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MXC_MMC
#define CONFIG_MXC_MCI_REGS_BASE       SDHC1_BASE_ADDR

/* video support */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_MX3
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_LOGO
/* splash image won't work with NAND boot, use preboot script */
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_CONSOLE_EXTRA_INFO /* display additional board info */
#define CONFIG_VGA_AS_SINGLE_DEVICE /* display is an output only device */

/* allow stdin, stdout and stderr variables to redirect output */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SILENT_CONSOLE		/* UARTs used externally (release) */
#define CONFIG_SYS_DEVICE_NULLDEV	/* allow console to be turned off */
#define CONFIG_PREBOOT

/* allow decompressing max. 4MB */
#define CONFIG_VIDEO_BMP_GZIP
/* this is not only used by cfb_console.c for the logo, but also in cmd_bmp.c */
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (4*1024*1024)

/*
 * Command definition
 */

#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_NAND
/*
 * #define CONFIG_CMD_NAND_LOCK_UNLOCK the NAND01... chip does not support
 * the NAND_CMD_LOCK_STATUS command, however the NFC of i.MX31 supports
 * a software locking scheme.
 */
#define CONFIG_CMD_BMP

#define CONFIG_BOOTDELAY	3

/*
 * currently a default setting for booting via script is implemented
 *   set user to login name and serverip to tftp host, define your
 *   boot behaviour in bootscript.loginname
 *
 * TT-01 board specific TFT setup (used by drivers/video/mx3fb.c)
 *
 *  This set-up is for the L5F30947T04 by Epson, which is
 *   800x480, 33MHz pixel clock, 60Hz vsync, 31.6kHz hsync
 *  sync must be set to: DI_D3_DRDY_SHARP_POL | DI_D3_CLK_POL
 */
#define	CONFIG_EXTRA_ENV_SETTINGS \
"videomode=epson\0" \
"epson=video=ctfb:x:800,y:480,depth:16,mode:0,pclk:30076," \
	"le:215,ri:1,up:32,lo:13,hs:7,vs:10,sync:100663296,vmode:0\0" \
"bootcmd=dhcp bootscript.${user}; source\0"

#define CONFIG_BOOTP_SERVERIP /* tftp serverip not overruled by dhcp server */
#define CONFIG_BOOTP_SEND_HOSTNAME /* if env-var 'hostname' is set, send it */

/* Miscellaneous configurable options */
#define CONFIG_SYS_HUSH_PARSER

#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_PROMPT	"TT01> "
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				sizeof(CONFIG_SYS_PROMPT)+16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS	16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

#define CONFIG_SYS_HZ			1000

#define CONFIG_CMDLINE_EDITING

/* MMC boot support */
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

#define CONFIG_NAND_MXC
#define CONFIG_SYS_MAX_NAND_DEVICE		1

/*
 * actually this is nothing someone wants to configure!
 * CONFIG_SYS_NAND_BASE despite being passed to board_nand_init()
 * is not used by the driver.
 */
#define CONFIG_MXC_NAND_REGS_BASE	NFC_BASE_ADDR
#define CONFIG_SYS_NAND_BASE		NFC_BASE_ADDR
#define CONFIG_MXC_NAND_HWECC

/* the current u-boot driver does not use the nand flash setup! */
#define CONFIG_SYS_NAND_LARGEPAGE
/*
 * it's not 16 bit:
 * #define CONFIG_SYS_NAND_BUSWIDTH_16BIT
 *    the current u-boot mxc_nand.c tries to auto-detect, but this only
 *    reads the boot settings during reset (which might be wrong)
 */

#endif /* __CONFIG_H */
