/*
 * Authors: Xiangfu Liu <xiangfu.z@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __CONFIG_QI_LB60_H
#define __CONFIG_QI_LB60_H

#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_JZSOC		/* Jz SoC */
#define CONFIG_JZ4740		/* Jz4740 SoC */
#define CONFIG_NAND_JZ4740

#define CONFIG_SYS_CPU_SPEED	336000000	/* CPU clock: 336 MHz */
#define CONFIG_SYS_EXTAL	12000000	/* EXTAL freq: 12 MHz */
#define CONFIG_SYS_HZ		(CONFIG_SYS_EXTAL / 256) /* incrementer freq */
#define CONFIG_SYS_MIPS_TIMER_FREQ	CONFIG_SYS_CPU_SPEED

#define CONFIG_SYS_UART_BASE	JZ4740_UART0_BASE /* Base of the UART channel */
#define CONFIG_BAUDRATE		57600
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)
#define CONFIG_BOOTDELAY	0
#define CONFIG_BOOTARGS		"mem=32M console=tty0 console=ttyS0,57600n8 ubi.mtd=2 rootfstype=ubifs root=ubi0:rootfs rw rootwait"
#define CONFIG_BOOTCOMMAND	"nand read 0x80600000 0x400000 0x200000;bootm"

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/

#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_NAND

/*
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT "NanoNote# "
#define CONFIG_SYS_CBSIZE 256 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000	/* Cached addr */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x80600000
#define CONFIG_SYS_MEMTEST_START	0x80100000
#define CONFIG_SYS_MEMTEST_END		0x80800000

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_NAND		/* use NAND for environment vars */

#define CONFIG_SYS_NAND_5_ADDR_CYCLE
/*
 * if board nand flash is 1GB, set to 1
 * if board nand flash is 2GB, set to 2
 * for change the PAGE_SIZE and BLOCK_SIZE
 * will delete when there is no 1GB flash
 */
#define NANONOTE_NAND_SIZE	2

#define CONFIG_SYS_NAND_PAGE_SIZE	(2048 * NANONOTE_NAND_SIZE)
#define CONFIG_SYS_NAND_BLOCK_SIZE	(256 * NANONOTE_NAND_SIZE << 10)
/* nand bad block was marked at this page in a block, start from 0 */
#define CONFIG_SYS_NAND_BADBLOCK_PAGE	127
#define CONFIG_SYS_NAND_PAGE_COUNT	128
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
/* ECC offset position in oob area, default value is 6 if it isn't defined */
#define CONFIG_SYS_NAND_ECC_POS		(6 * NANONOTE_NAND_SIZE)
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	9
#define CONFIG_SYS_NAND_ECCPOS		\
		{12, 13, 14, 15, 16, 17, 18, 19,\
		20, 21, 22, 23, 24, 25, 26, 27, \
		28, 29, 30, 31, 32, 33, 34, 35, \
		36, 37, 38, 39, 40, 41, 42, 43, \
		44, 45, 46, 47, 48, 49, 50, 51, \
		52, 53, 54, 55, 56, 57, 58, 59, \
		60, 61, 62, 63, 64, 65, 66, 67, \
		68, 69, 70, 71, 72, 73, 74, 75, \
		76, 77, 78, 79, 80, 81, 82, 83}

#define CONFIG_SYS_NAND_OOBSIZE		128
#define CONFIG_SYS_NAND_BASE		0xB8000000
#define CONFIG_SYS_ONENAND_BASE		CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELECT_DEVICE	1 /* nand driver supports mutipl.*/
#define CONFIG_NAND_SPL_TEXT_BASE	0x80000000

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 8k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 8kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 */
#define CONFIG_SYS_NAND_U_BOOT_DST	0x80100000 /* Load NUB to this addr */
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
/* Start NUB from this addr*/

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CONFIG_SYS_NAND_U_BOOT_OFFS (256 << 10) /* Offset to RAM U-Boot image */
#define CONFIG_SYS_NAND_U_BOOT_SIZE (512 << 10) /* Size of RAM U-Boot image */

#define CONFIG_ENV_SIZE		(4 << 10)
#define CONFIG_ENV_OFFSET	\
	(CONFIG_SYS_NAND_BLOCK_SIZE + CONFIG_SYS_NAND_U_BOOT_SIZE)
#define CONFIG_ENV_OFFSET_REDUND \
	(CONFIG_ENV_OFFSET  + CONFIG_SYS_NAND_BLOCK_SIZE)

#define CONFIG_SYS_TEXT_BASE	0x80100000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE

/*
 * SDRAM Info.
 */
#define CONFIG_NR_DRAM_BANKS	1

/*
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE	16384
#define CONFIG_SYS_ICACHE_SIZE	16384
#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * GPIO definition
 */
#define GPIO_LCD_CS	(2 * 32 + 21)
#define GPIO_AMP_EN	(3 * 32 + 4)

#define GPIO_SDPW_EN	(3 * 32 + 2)
#define GPIO_SD_DETECT	(3 * 32 + 0)

#define GPIO_BUZZ_PWM	(3 * 32 + 27)
#define GPIO_USB_DETECT	(3 * 32 + 28)

#define GPIO_AUDIO_POP	(1 * 32 + 29)
#define GPIO_COB_TEST	(1 * 32 + 30)

#define GPIO_KEYOUT_BASE	(2 * 32 + 10)
#define GPIO_KEYIN_BASE	(3 * 32 + 18)
#define GPIO_KEYIN_8	(3 * 32 + 26)

#define GPIO_SD_CD_N	GPIO_SD_DETECT		/* SD Card insert detect */
#define GPIO_SD_VCC_EN_N	GPIO_SDPW_EN	/* SD Card Power Enable */

#define SPEN	GPIO_LCD_CS	/* LCDCS :Serial command enable      */
#define SPDA	(2 * 32 + 22)	/* LCDSCL:Serial command clock input */
#define SPCK	(2 * 32 + 23)	/* LCDSDA:Serial command data input  */

/* SDRAM paramters */
#define SDRAM_BW16		1	/* Data bus width: 0-32bit, 1-16bit */
#define SDRAM_BANK4		1	/* Banks each chip: 0-2bank, 1-4bank */
#define SDRAM_ROW		13	/* Row address: 11 to 13 */
#define SDRAM_COL		9	/* Column address: 8 to 12 */
#define SDRAM_CASL		2	/* CAS latency: 2 or 3 */

/* SDRAM Timings, unit: ns */
#define SDRAM_TRAS		45	/* RAS# Active Time */
#define SDRAM_RCD		20	/* RAS# to CAS# Delay */
#define SDRAM_TPC		20	/* RAS# Precharge Time */
#define SDRAM_TRWL		7	/* Write Latency Time */
#define SDRAM_TREF		15625	/* Refresh period: 8192 cycles/64ms */

#endif
