/*
 * (C) Copyright 2007-2009 DENX Software Engineering
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
 * MPC5121ADS board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC5121ADS 1
/*
 * Memory map for the MPC5121ADS board:
 *
 * 0x0000_0000 - 0x0FFF_FFFF	DDR RAM (256 MB)
 * 0x3000_0000 - 0x3001_FFFF	SRAM (128 KB)
 * 0x8000_0000 - 0x803F_FFFF	IMMR (4 MB)
 * 0x8200_0000 - 0x8200_001F	CPLD (32 B)
 * 0x8400_0000 - 0x82FF_FFFF	PCI I/O space (16 MB)
 * 0xA000_0000 - 0xAFFF_FFFF	PCI memory space (256 MB)
 * 0xB000_0000 - 0xBFFF_FFFF	PCI memory mapped I/O space (256 MB)
 * 0xFC00_0000 - 0xFFFF_FFFF	NOR Boot FLASH (64 MB)
 */

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC512X		1	/* MPC512X family */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

/* video */
#ifdef CONFIG_FSL_DIU_FB
#define CONFIG_SYS_DIU_ADDR	(CONFIG_SYS_IMMR + 0x2100)
#define CONFIG_VIDEO
#define CONFIG_CMD_BMP
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#endif

/* CONFIG_PCI is defined at config time */

#ifdef CONFIG_MPC5121ADS_REV2
#define CONFIG_SYS_MPC512X_CLKIN	66000000	/* in Hz */
#else
#define CONFIG_SYS_MPC512X_CLKIN	33333333	/* in Hz */
#define CONFIG_PCI
#endif

#define CONFIG_BOARD_EARLY_INIT_F		/* call board_early_init_f() */
#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_IMMR		0x80000000

#define CONFIG_SYS_MEMTEST_START	0x00200000      /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * DDR Setup - manually set all parameters as there's no SPD etc.
 */
#ifdef CONFIG_MPC5121ADS_REV2
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#else
#define CONFIG_SYS_DDR_SIZE		512		/* MB */
#endif
#define CONFIG_SYS_DDR_BASE		0x00000000	/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_MAX_RAM_SIZE		0x20000000

#define CONFIG_SYS_IOCTRL_MUX_DDR	0x00000036

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
#ifdef CONFIG_MPC5121ADS_REV2
#define CONFIG_SYS_MDDRC_SYS_CFG	0xE8604A00
#define CONFIG_SYS_MDDRC_TIME_CFG1	0x54EC1168
#define CONFIG_SYS_MDDRC_TIME_CFG2	0x35210864
#else
#define CONFIG_SYS_MDDRC_SYS_CFG	0xEA804A00
#define CONFIG_SYS_MDDRC_TIME_CFG1	0x68EC1168
#define CONFIG_SYS_MDDRC_TIME_CFG2	0x34310864
#endif
#define CONFIG_SYS_MDDRC_TIME_CFG0	0x06183D2E

#define CONFIG_SYS_MDDRC_SYS_CFG_ELPIDA	 	0xEA802B00
#define CONFIG_SYS_MDDRC_TIME_CFG1_ELPIDA	0x690e1189
#define CONFIG_SYS_MDDRC_TIME_CFG2_ELPIDA	0x35310864

#define CONFIG_SYS_DDRCMD_NOP		0x01380000
#define CONFIG_SYS_DDRCMD_PCHG_ALL	0x01100400
#define CONFIG_SYS_DDRCMD_EM2		0x01020000
#define CONFIG_SYS_DDRCMD_EM3		0x01030000
#define CONFIG_SYS_DDRCMD_EN_DLL	0x01010000
#define CONFIG_SYS_DDRCMD_RFSH		0x01080000

#define DDRCMD_EMR_OCD(pr, ohm) ( \
	(1 << 24)	   | /* MDDRC Command Request	*/ \
	(1 << 16)	   | /* MODE Reg BA[2:0] 	*/ \
	(0 << 12)	   | /* Outputs 0=Enabled	*/ \
	(0 << 11)	   | /* RDQS 			*/ \
	(1 << 10)	   | /* DQS# 			*/ \
	(pr <<  7)	   | /* OCD prog 7=deflt,0=exit	*/ \
		    /* ODT Rtt[1:0] 0=0,1=75,2=150,3=50 */ \
	((ohm & 0x2) <<  5)| /* Rtt1			*/ \
	(0 <<  3)	   | /* additive posted CAS#	*/ \
	((ohm & 0x1) <<  2)| /* Rtt0			*/ \
	(0 <<  0)	   | /* Output Drive Strength	*/ \
	(0 <<  0))	     /* DLL Enable 0=Normal	*/

#define CONFIG_SYS_DDRCMD_OCD_DEFAULT	DDRCMD_EMR_OCD(7, 0)
#define CONFIG_SYS_ELPIDA_OCD_EXIT	DDRCMD_EMR_OCD(0, 0)

#define DDRCMD_MODE_REG(cas, wr) ( \
	(1 << 24)    | /* MDDRC Command Request			*/ \
	(0 << 16)    | /* MODE Reg BA[2:0] 			*/ \
	((wr-1) << 9)| /* Write Recovery 			*/ \
	(cas << 4)   | /* CAS 					*/ \
	(0 << 3)     | /* Burst Type:0=Sequential,1=Interleaved	*/ \
	(2 << 0))      /* 4 or 8 Burst Length:0x2=4 0x3=8	*/

#define CONFIG_SYS_MICRON_INIT_DEV_OP	DDRCMD_MODE_REG(3, 3)
#define CONFIG_SYS_ELPIDA_INIT_DEV_OP	DDRCMD_MODE_REG(4, 4)
#define CONFIG_SYS_ELPIDA_RES_DLL	(DDRCMD_MODE_REG(4, 4) | (1 << 8))

/* DDR Priority Manager Configuration */
#define CONFIG_SYS_MDDRCGRP_PM_CFG1	0x00077777
#define CONFIG_SYS_MDDRCGRP_PM_CFG2	0x00000000
#define CONFIG_SYS_MDDRCGRP_HIPRIO_CFG	0x00000001
#define CONFIG_SYS_MDDRCGRP_LUT0_MU	0xFFEEDDCC
#define CONFIG_SYS_MDDRCGRP_LUT0_ML	0xBBAAAAAA
#define CONFIG_SYS_MDDRCGRP_LUT1_MU	0x66666666
#define CONFIG_SYS_MDDRCGRP_LUT1_ML	0x55555555
#define CONFIG_SYS_MDDRCGRP_LUT2_MU	0x44444444
#define CONFIG_SYS_MDDRCGRP_LUT2_ML	0x44444444
#define CONFIG_SYS_MDDRCGRP_LUT3_MU	0x55555555
#define CONFIG_SYS_MDDRCGRP_LUT3_ML	0x55555558
#define CONFIG_SYS_MDDRCGRP_LUT4_MU	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT4_ML	0x11111122
#define CONFIG_SYS_MDDRCGRP_LUT0_AU	0xaaaaaaaa
#define CONFIG_SYS_MDDRCGRP_LUT0_AL	0xaaaaaaaa
#define CONFIG_SYS_MDDRCGRP_LUT1_AU	0x66666666
#define CONFIG_SYS_MDDRCGRP_LUT1_AL	0x66666666
#define CONFIG_SYS_MDDRCGRP_LUT2_AU	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT2_AL	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT3_AU	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT3_AL	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT4_AU	0x11111111
#define CONFIG_SYS_MDDRCGRP_LUT4_AL	0x11111111

/*
 * NOR FLASH on the Local Bus
 */
#undef CONFIG_BKUP_FLASH
#define CONFIG_SYS_FLASH_CFI				/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER			/* use the CFI driver */
#ifdef CONFIG_BKUP_FLASH
#define CONFIG_SYS_FLASH_BASE		0xFF800000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		0x00800000	/* max flash size in bytes */
#else
#define CONFIG_SYS_FLASH_BASE		0xFC000000	/* start of FLASH   */
#define CONFIG_SYS_FLASH_SIZE		0x04000000	/* max flash size in bytes */
#endif
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_SECT	256		/* max sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM

/*
 * NAND FLASH
 * drivers/mtd/nand/mpc5121_nfc.c (rev 2 silicon only)
 */
#define CONFIG_CMD_NAND					/* enable NAND support */
#define CONFIG_JFFS2_NAND				/* with JFFS2 on it */
#define CONFIG_NAND_MPC5121_NFC
#define CONFIG_SYS_NAND_BASE            0x40000000

#define CONFIG_SYS_MAX_NAND_DEVICE      2
#define CONFIG_SYS_NAND_SELECT_DEVICE	/* driver supports mutipl. chips */

/*
 * Configuration parameters for MPC5121 NAND driver
 */
#define CONFIG_FSL_NFC_WIDTH 1
#define CONFIG_FSL_NFC_WRITE_SIZE 2048
#define CONFIG_FSL_NFC_SPARE_SIZE 64
#define CONFIG_FSL_NFC_CHIPS CONFIG_SYS_MAX_NAND_DEVICE

/*
 * CPLD registers area is really only 32 bytes in size, but the smallest possible LP
 * window is 64KB
 */
#define CONFIG_SYS_CPLD_BASE		0x82000000
#define CONFIG_SYS_CPLD_SIZE		0x00010000	/* 64 KB */

#define CONFIG_SYS_SRAM_BASE		0x30000000
#define CONFIG_SYS_SRAM_SIZE		0x00020000	/* 128 KB */

#define CONFIG_SYS_CS0_CFG		0x05059310	/* ALE active low, data size 4bytes */
#define CONFIG_SYS_CS2_CFG		0x05059010	/* ALE active low, data size 1byte */
#define CONFIG_SYS_CS_ALETIMING	0x00000005	/* Use alternative CS timing for CS0 and CS2 */

/* Use SRAM for initial stack */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SRAM_BASE		/* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_SRAM_SIZE		/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE		/* Start of monitor */
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)		/* Reserve 512 kB for Mon */
#ifdef	CONFIG_FSL_DIU_FB
#define CONFIG_SYS_MALLOC_LEN		(6 * 1024 * 1024)	/* Reserved for malloc */
#else
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)
#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX     1

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	3	/* console is on PSC3 */
#define CONFIG_SYS_PSC3
#if CONFIG_PSC_CONSOLE != 3
#error CONFIG_PSC_CONSOLE must be 3
#endif
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONSOLE_FIFO_TX_SIZE	FIFOC_PSC3_TX_SIZE
#define CONSOLE_FIFO_TX_ADDR	FIFOC_PSC3_TX_ADDR
#define CONSOLE_FIFO_RX_SIZE	FIFOC_PSC3_RX_SIZE
#define CONSOLE_FIFO_RX_ADDR	FIFOC_PSC3_RX_ADDR

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

/*
 * PCI
 */
#ifdef CONFIG_PCI

/*
 * General PCI
 */
#define CONFIG_SYS_PCI_MEM_BASE	0xA0000000
#define CONFIG_SYS_PCI_MEM_PHYS	CONFIG_SYS_PCI_MEM_BASE
#define CONFIG_SYS_PCI_MEM_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI_MMIO_BASE	(CONFIG_SYS_PCI_MEM_BASE + CONFIG_SYS_PCI_MEM_SIZE)
#define CONFIG_SYS_PCI_MMIO_PHYS	CONFIG_SYS_PCI_MMIO_BASE
#define CONFIG_SYS_PCI_MMIO_SIZE	0x10000000	/* 256M */
#define CONFIG_SYS_PCI_IO_BASE		0x00000000
#define CONFIG_SYS_PCI_IO_PHYS		0x84000000
#define CONFIG_SYS_PCI_IO_SIZE		0x01000000	/* 16M */


#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#undef CONFIG_SOFT_I2C			/* so disable bit-banged I2C */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#if 0
#define CONFIG_SYS_I2C_NOPROBES	{{0,0x69}}	/* Don't probe these addrs */
#endif

/*
 * IIM - IC Identification Module
 */
#undef CONFIG_IIM

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2	/* 16-bit EEPROM address */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* Atmel: AT24C32A-10TQ-2.7 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* 10ms of delay */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5	/* 32-Byte Page Write Mode */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC512x_FEC	1
#define CONFIG_PHY_ADDR		0x1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_FEC_AN_TIMEOUT	1
#define CONFIG_HAS_ETH0

/*
 * Configure on-board RTC
 */
#define CONFIG_RTC_M41T62			/* use M41T62 rtc via i2 */
#define CONFIG_SYS_I2C_RTC_ADDR		0x68	/* at address 0x68		*/

/*
 * USB  Support
 */
#define CONFIG_CMD_USB

#if defined(CONFIG_CMD_USB)
#define CONFIG_USB_EHCI				/* Enable EHCI Support	*/
#define CONFIG_USB_EHCI_FSL			/* On a FSL platform	*/
#define CONFIG_EHCI_MMIO_BIG_ENDIAN		/* With big-endian regs	*/
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#endif

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
/* This has to be a multiple of the Flash sector size */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE		0x2000
#ifdef CONFIG_BKUP_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000	/* one sector (256K) for env */
#else
#define CONFIG_ENV_SECT_SIZE	0x40000	/* one sector (256K) for env */
#endif

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#undef CONFIG_CMD_FUSE

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

/*
 * Dynamic MTD partition support
 */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=fc000000.flash,nand0=mpc5121.nand"

/*
 * NOR flash layout:
 *
 * FC000000 - FEABFFFF 42.75 MiB	User Data
 * FEAC0000 - FFABFFFF  16 MiB		Root File System
 * FFAC0000 - FFEBFFFF   4 MiB		Linux Kernel
 * FFEC0000 - FFEFFFFF 256 KiB		Device Tree
 * FFF00000 - FFFFFFFF   1 MiB		U-Boot (up to 512 KiB) and 2 x * env
 *
 * NAND flash layout: one big partition
 */
#define MTDPARTS_DEFAULT	"mtdparts=fc000000.flash:43776k(user),"	\
						"16m(rootfs),"		\
						"4m(kernel),"		\
						"256k(dtb),"		\
						"1m(u-boot);"		\
					"mpc5121.nand:-(data)"


#if defined(CONFIG_CMD_IDE) || defined(CONFIG_CMD_EXT2) || defined(CONFIG_CMD_USB)

#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_CMD_FAT
#define CONFIG_SUPPORT_VFAT

#endif /* defined(CONFIG_CMD_IDE) */

/*
 * Watchdog timeout = CONFIG_SYS_WATCHDOG_VALUE * 65536 / IPS clock.
 * For example, when IPS is set to 66MHz and CONFIG_SYS_WATCHDOG_VALUE is set
 * to 0xFFFF, watchdog timeouts after about 64s. For details refer
 * to chapter 36 of the MPC5121e Reference Manual.
 */
/* #define CONFIG_WATCHDOG */		/* enable watchdog */
#define CONFIG_SYS_WATCHDOG_VALUE 0xFFFF

 /*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */

#ifdef CONFIG_CMD_KGDB
	#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#endif


#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)	/* Initial Memory map for Linux*/

/* Cache Configuration */
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5	/*log base 2 of the above value*/
#endif

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	(HID0_ENABLE_MACHINE_CHECK | HID0_ICE)
#define CONFIG_SYS_HID2	HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_TIMESTAMP

#define CONFIG_HOSTNAME		mpc5121ads
#define CONFIG_BOOTFILE		"mpc5121ads/uImage"
#define CONFIG_ROOTPATH		"/opt/eldk/ppc_6xx"

#define CONFIG_LOADADDR		400000	/* default location for tftp and bootm */

#define CONFIG_BOOTDELAY	5	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS			/* the boot command will set bootargs */

#define CONFIG_BAUDRATE		115200

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr_r=200000\0"					\
	"kernel_addr_r=600000\0"					\
	"fdt_addr_r=880000\0"						\
	"ramdisk_addr_r=900000\0"					\
	"u-boot_addr=FFF00000\0"					\
	"kernel_addr=FFAC0000\0"					\
	"fdt_addr=FFEC0000\0"						\
	"ramdisk_addr=FEAC0000\0"					\
	"ramdiskfile=mpc5121ads/uRamdisk\0"				\
	"u-boot=mpc5121ads/u-boot.bin\0"				\
	"bootfile=mpc5121ads/uImage\0"					\
	"fdtfile=mpc5121ads/mpc5121ads.dtb\0"				\
	"rootpath=/opt/eldk/ppc_6xx\n"					\
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
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${fdt_addr_r} ${fdtfile};"			\
		"run nfsargs addip addtty;"				\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"net_self=tftp ${kernel_addr_r} ${bootfile};"			\
		"tftp ${ramdisk_addr_r} ${ramdiskfile};"		\
		"tftp ${fdt_addr_r} ${fdtfile};"			\
		"run ramargs addip addtty;"				\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0"\
	"load=tftp ${u-boot_addr_r} ${u-boot}\0"			\
	"update=protect off ${u-boot_addr} +${filesize};"		\
		"era ${u-boot_addr} +${filesize};"			\
		"cp.b ${u-boot_addr_r} ${u-boot_addr} ${filesize}\0"	\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_SUPPORT_OLD_DEVICE_TREES	1

#define OF_CPU			"PowerPC,5121@0"
#define OF_SOC_COMPAT		"fsl,mpc5121-immr"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc@80000000/serial@11300"

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/
#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for IDE not supported	*/

#define CONFIG_IDE_RESET		/* reset for IDE supported	*/
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_BASE_ADDR	get_pata_base()

/* Offset for data I/O			RefMan MPC5121EE Table 28-10	*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x00A0)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers	RefMan MPC5121EE Table 28-23	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x00D8)

/* Interval between registers	*/
#define CONFIG_SYS_ATA_STRIDE		4

#define ATA_BASE_ADDR			get_pata_base()

/*
 * Control register bit definitions
 */
#define FSL_ATA_CTRL_FIFO_RST_B		0x80000000
#define FSL_ATA_CTRL_ATA_RST_B		0x40000000
#define FSL_ATA_CTRL_FIFO_TX_EN		0x20000000
#define FSL_ATA_CTRL_FIFO_RCV_EN	0x10000000
#define FSL_ATA_CTRL_DMA_PENDING	0x08000000
#define FSL_ATA_CTRL_DMA_ULTRA		0x04000000
#define FSL_ATA_CTRL_DMA_WRITE		0x02000000
#define FSL_ATA_CTRL_IORDY_EN		0x01000000

#endif	/* __CONFIG_H */
