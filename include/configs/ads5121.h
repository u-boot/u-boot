/*
 * (C) Copyright 2007 DENX Software Engineering
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * ADS5121 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG
#undef DEBUG

/*
 * Memory map for the ADS5121 board:
 *
 * 0x0000_0000 - 0x0FFF_FFFF	DDR RAM (256 MB)
 * 0x3000_0000 - 0x3001_FFFF	SRAM (128 KB)
 * 0x8000_0000 - 0x803F_FFFF	IMMR (4 MB)
 * 0x8200_0000 - 0x8200_001F	CPLD (32 B)
 * 0xFC00_0000 - 0xFFFF_FFFF	NOR Boot FLASH (64 MB)
 */

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC512X		1	/* MPC512X family */

#undef CONFIG_PCI

#define CFG_MPC512X_CLKIN	66000000	/* in Hz */

#define CONFIG_BOARD_EARLY_INIT_F		/* call board_early_init_f() */

#define CFG_IMMR		0x80000000

#define CFG_MEMTEST_START	0x00200000      /* memtest region */
#define CFG_MEMTEST_END		0x00400000

/*
 * DDR Setup - manually set all parameters as there's no SPD etc.
 */
#define CFG_DDR_SIZE		256		/* MB */
#define CFG_DDR_BASE		0x00000000	/* DDR is system memory*/
#define CFG_SDRAM_BASE		CFG_DDR_BASE

/* DDR Controller Configuration
 *
 * SYS_CFG:
 *	[31:31]	MDDRC Soft Reset:	Diabled
 *	[30:30]	DRAM CKE pin:		Enabled
 *	[29:29]	DRAM CLK:		Enabled
 *	[28:28]	Command Mode:		Enabled (For initialization only)
 *	[27:25]	DRAM Row Select:	dram_row[15:0] = magenta_address[25:10]
 *	[24:21]	DRAM Bank Select:	dram_bank[1:0] = magenta_address[11:10]
 *	[20:19]	Read Test:		DON'T USE
 *	[18:18]	Self Refresh:		Enabled
 *	[17:17]	16bit Mode:		Disabled
 *	[16:13] Ready Delay:		2
 *	[12:12]	Half DQS Delay:		Disabled
 *	[11:11]	Quarter DQS Delay:	Disabled
 *	[10:08]	Write Delay:		2
 *	[07:07]	Early ODT:		Disabled
 *	[06:06]	On DIE Termination:	Disabled
 *	[05:05]	FIFO Overflow Clear:	DON'T USE here
 *	[04:04]	FIFO Underflow Clear:	DON'T USE here
 *	[03:03]	FIFO Overflow Pending:	DON'T USE here
 *	[02:02]	FIFO Underlfow Pending:	DON'T USE here
 *	[01:01]	FIFO Overlfow Enabled:	Enabled
 *	[00:00]	FIFO Underflow Enabled:	Enabled
 * TIME_CFG0
 *	[31:16]	DRAM Refresh Time:	0 CSB clocks
 *	[15:8]	DRAM Command Time:	0 CSB clocks
 *	[07:00]	DRAM Precharge Time:	0 CSB clocks
 * TIME_CFG1
 *	[31:26]	DRAM tRFC:
 *	[25:21]	DRAM tWR1:
 *	[20:17]	DRAM tWRT1:
 *	[16:11]	DRAM tDRR:
 *	[10:05]	DRAM tRC:
 *	[04:00]	DRAM tRAS:
 * TIME_CFG2
 *	[31:28]	DRAM tRCD:
 *	[27:23]	DRAM tFAW:
 *	[22:19]	DRAM tRTW1:
 *	[18:15]	DRAM tCCD:
 *	[14:10] DRAM tRTP:
 *	[09:05]	DRAM tRP:
 *	[04:00] DRAM tRPA
 */

#define CFG_MDDRC_SYS_CFG	0xF8604200
#define CFG_MDDRC_SYS_CFG_RUN	0xE8604200
#define CFG_MDDRC_SYS_CFG_EN	0x30000000
#define CFG_MDDRC_TIME_CFG0	0x0000281E
#define CFG_MDDRC_TIME_CFG0_RUN	0x01F4281E
#define CFG_MDDRC_TIME_CFG1	0x54EC1168
#define CFG_MDDRC_TIME_CFG2	0x35210864

#define CFG_MICRON_NOP		0x01380000
#define CFG_MICRON_PCHG_ALL	0x01100400
#define CFG_MICRON_MR		0x01000022
#define CFG_MICRON_EM2		0x01020000
#define CFG_MICRON_EM3		0x01030000
#define CFG_MICRON_EN_DLL	0x01010000
#define CFG_MICRON_RST_DLL	0x01000932
#define CFG_MICRON_RFSH		0x01080000
#define CFG_MICRON_INIT_DEV_OP	0x01000832
#define CFG_MICRON_OCD_DEFAULT	0x01010780
#define CFG_MICRON_OCD_EXIT	0x01010400

/* DDR Priority Manager Configuration */
#define CFG_MDDRCGRP_PM_CFG1	0x000777AA
#define CFG_MDDRCGRP_PM_CFG2	0x00000055
#define CFG_MDDRCGRP_HIPRIO_CFG	0x00000000
#define CFG_MDDRCGRP_LUT0_MU    0x11111117
#define CFG_MDDRCGRP_LUT0_ML	0x7777777A
#define CFG_MDDRCGRP_LUT1_MU    0x4444EEEE
#define CFG_MDDRCGRP_LUT1_ML	0xEEEEEEEE
#define CFG_MDDRCGRP_LUT2_MU    0x44444444
#define CFG_MDDRCGRP_LUT2_ML	0x44444444
#define CFG_MDDRCGRP_LUT3_MU    0x55555555
#define CFG_MDDRCGRP_LUT3_ML	0x55555558
#define CFG_MDDRCGRP_LUT4_MU    0x11111111
#define CFG_MDDRCGRP_LUT4_ML	0x1111117C
#define CFG_MDDRCGRP_LUT0_AU    0x33333377
#define CFG_MDDRCGRP_LUT0_AL	0x7777EEEE
#define CFG_MDDRCGRP_LUT1_AU    0x11111111
#define CFG_MDDRCGRP_LUT1_AL	0x11111111
#define CFG_MDDRCGRP_LUT2_AU    0x11111111
#define CFG_MDDRCGRP_LUT2_AL	0x11111111
#define CFG_MDDRCGRP_LUT3_AU    0x11111111
#define CFG_MDDRCGRP_LUT3_AL	0x11111111
#define CFG_MDDRCGRP_LUT4_AU    0x11111111
#define CFG_MDDRCGRP_LUT4_AL	0x11111111

/*
 * NOR FLASH on the Local Bus
 */
#define CFG_FLASH_CFI				/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER			/* use the CFI driver */
#define CFG_FLASH_BASE		0xFC000000	/* start of FLASH   */
#define CFG_FLASH_SIZE		0x04000000	/* max flash size in bytes */
#define CFG_FLASH_USE_BUFFER_WRITE

#define CFG_MAX_FLASH_BANKS	1		/* number of banks */
#define CFG_FLASH_BANKS_LIST 	{CFG_FLASH_BASE}
#define CFG_MAX_FLASH_SECT	256		/* max sectors per device */

#undef CFG_FLASH_CHECKSUM

/*
 * CPLD registers area is really only 32 bytes in size, but the smallest possible LP
 * window is 64KB
 */
#define CFG_CPLD_BASE		0x82000000
#define CFG_CPLD_SIZE		0x00010000	/* 64 KB */

#define CFG_SRAM_BASE		0x30000000
#define CFG_SRAM_SIZE		0x00020000	/* 128 KB */

#define CFG_CS0_CFG		0x05059310	/* ALE active low, data size 4bytes */
#define CFG_CS2_CFG		0x05059010	/* ALE active low, data size 1byte */

/* Use SRAM for initial stack */
#define CFG_INIT_RAM_ADDR	CFG_SRAM_BASE		/* Initial RAM address */
#define CFG_INIT_RAM_END	CFG_SRAM_SIZE		/* End of used area in RAM */

#define CFG_GBL_DATA_SIZE	0x100			/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE		/* Start of monitor */
#define CFG_MONITOR_LEN		(256 * 1024)		/* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(512 * 1024)		/* Reserved for malloc */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX     1
#undef CONFIG_SERIAL_SOFTWARE_FIFO

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	3	/* console is on PSC3 */
#if CONFIG_PSC_CONSOLE != 3
#error CONFIG_PSC_CONSOLE must be 3
#endif
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONSOLE_FIFO_TX_SIZE	FIFOC_PSC3_TX_SIZE
#define CONSOLE_FIFO_TX_ADDR	FIFOC_PSC3_TX_ADDR
#define CONSOLE_FIFO_RX_SIZE	FIFOC_PSC3_RX_SIZE
#define CONSOLE_FIFO_RX_ADDR	FIFOC_PSC3_RX_ADDR

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#undef CONFIG_SOFT_I2C			/* so disable bit-banged I2C */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F
#if 0
#define CFG_I2C_NOPROBES	{{0,0x69}}	/* Don't probe these addrs */
#endif

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR_LEN		2	/* 16-bit EEPROM address */
#define CFG_I2C_EEPROM_ADDR		0x50	/* Atmel: AT24C32A-10TQ-2.7 */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* 10ms of delay */
#define CFG_EEPROM_PAGE_WRITE_BITS	5	/* 32-Byte Page Write Mode */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC512x_FEC	1
#define CONFIG_NET_MULTI
#define CONFIG_PHY_ADDR		0x1
#define CONFIG_MII		1	/* MII PHY management		*/

#if 0
/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_DS1374			/* use ds1374 rtc via i2c	*/
#define CFG_I2C_RTC_ADDR		0x68	/* at address 0x68		*/
#endif

/*
 * Environment
 */
#define CFG_ENV_IS_IN_FLASH	1
/* This has to be a multiple of the Flash sector size */
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define CFG_ENV_SIZE		0x2000
#define CFG_ENV_SECT_SIZE	0x40000	/* one sector (256K) for env */

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_EEPROM

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

/*
 * Watchdog timeout = CFG_WATCHDOG_VALUE * 65536 / IPS clock.
 * For example, when IPS is set to 66MHz and CFG_WATCHDOG_VALUE is set
 * to 0xFFFF, watchdog timeouts after about 64s. For details refer
 * to chapter 36 of the MPC5121e Reference Manual.
 */
/* #define CONFIG_WATCHDOG */		/* enable watchdog */
#define CFG_WATCHDOG_VALUE 0xFFFF

 /*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory */
#define CFG_LOAD_ADDR	0x2000000	/* default load address */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt */

#ifdef CONFIG_CMD_KGDB
	#define CFG_CBSIZE	1024	/* Console I/O Buffer Size */
#else
	#define CFG_CBSIZE	256	/* Console I/O Buffer Size */
#endif


#define CFG_PBSIZE (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CFG_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

#define CFG_HID0_INIT	0x000000000
#define CFG_HID0_FINAL	HID0_ENABLE_MACHINE_CHECK
#define CFG_HID2	HID2_HBE

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_TIMESTAMP

#define CONFIG_HOSTNAME		ads5121
#define CONFIG_BOOTFILE		ads5121/uImage

#define CONFIG_LOADADDR		400000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY	5	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS			/* the boot command will set bootargs */

#define CONFIG_BAUDRATE		115200

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr_r=200000\0"					\
	"kernel_addr_r=200000\0"					\
	"fdt_addr_r=400000\0"						\
	"ramdisk_addr_r=500000\0"					\
	"u-boot_addr=FFF00000\0"					\
	"kernel_addr=FC000000\0"					\
	"fdt_addr=FC2C0000\0"						\
	"ramdisk_addr=FC300000\0"					\
	"ramdiskfile=ads5121/uRamdisk\0"				\
	"fdtfile=ads5121/ads5121.dtb\0"					\
	"u-boot=ads5121/u-boot.bin\0"					\
	"netdev=eth0\0"							\
	"consdev=ttyPSC0\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr_r} - ${fdt_addr}\0"		\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${fdt_addr_r} ${fdtfile};"			\
		"run nfsargs addip addtty;"				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"net_self=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${ramdisk_addr_r} ${ramdiskfile};"		\
		"tftp ${fdt_addr} ${fdtfile};"				\
		"run ramargs addip addtty;"				\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr}\0"\
	"load=tftp ${u-boot_addr} ${u-boot}\0"				\
	"update=protect off ${u-boot_addr} +${filesize};"		\
		"era ${u-boot_addr} +${filesize};"			\
		"cp.b ${u-boot_addr_r} ${u-boot_addr} ${filesize}\0"	\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5121@0"
#define OF_SOC			"soc@80000000"
#define OF_SOC_OLD		"soc5121@80000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc@80000000/serial@11300"

#endif	/* __CONFIG_H */
