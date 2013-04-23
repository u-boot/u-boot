/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2010 DAVE Srl <www.dave.eu>
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

/*
 * ifm AC14xx (MPC5121e based) board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_AC14XX 1
/*
 * Memory map for the ifm AC14xx board:
 *
 * 0x0000_0000-0x0FFF_FFFF	DDR RAM (256 MB)
 * 0x3000_0000-0x3001_FFFF	On Chip SRAM (128 KB)
 * 0x8000_0000-0x803F_FFFF	IMMR (4 MB)
 * 0xE000_0000-0xEFFF_FFFF	several LPB attached hardware (CSx)
 * 0xFC00_0000-0xFFFF_FFFF	NOR Boot FLASH (64 MB)
 */

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_MPC512X		1	/* MPC512X family */

#define CONFIG_SYS_TEXT_BASE	0xFFF00000

#if defined(CONFIG_VIDEO)
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#endif

#define CONFIG_SYS_MPC512X_CLKIN	25000000	/* in Hz */
#define SCFR1_IPS_DIV			2
#define SCFR1_LPC_DIV			2
#define SCFR1_NFC_DIV			2
#define SCFR1_DIU_DIV			240

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_IMMR			0x80000000
#define CONFIG_SYS_DIU_ADDR		(CONFIG_SYS_IMMR + 0x2100)

/* more aggressive 'mtest' over a wider address range */
#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x00100000      /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x0FE00000

/*
 * DDR Setup - manually set all parameters as there's no SPD etc.
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_MAX_RAM_SIZE		0x20000000

/*
 * DDR Controller Configuration XXX TODO
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

/*
 * NOTE: although this board uses DDR1 only, the common source brings defaults
 * for DDR2 init sequences, that's why we have to keep those here as well
 */

/* DDR1 -- 32bit, drive strength (pad configuration) 3 for control and data */
#define CONFIG_SYS_IOCTRL_MUX_DDR	((0 << 6) | (3 << 3) | (3 << 0))

#define CONFIG_SYS_MDDRC_SYS_CFG (/* 0xEAA09100 */ 0 \
			| (1 << 31)	/* RST_B */ \
			| (1 << 30)	/* CKE */ \
			| (1 << 29)	/* CLK_ON */ \
			| (0 << 28)	/* CMD_MODE */ \
			| (5 << 25)	/* DRAM_ROW_SELECT */ \
			| (5 << 21)	/* DRAM_BANK_SELECT */ \
			| (0 << 18)	/* SELF_REF_EN */ \
			| (0 << 17)	/* 16BIT_MODE */ \
			| (4 << 13)	/* RDLY */ \
			| (1 << 12)	/* HALF_DQS_DLY */ \
			| (0 << 11)	/* QUART_DQS_DLY */ \
			| (1 <<  8)	/* WDLY */ \
			| (0 <<  7)	/* EARLY_ODT */ \
			| (0 <<  6)	/* ON_DIE_TERMINATE */ \
			| (0 <<  5)	/* FIFO_OV_CLEAR */ \
			| (0 <<  4)	/* FIFO_UV_CLEAR */ \
			| (0 <<  1)	/* FIFO_OV_EN */ \
			| (0 <<  0)	/* FIFO_UV_EN */ \
			)

#define CONFIG_SYS_MDDRC_TIME_CFG0	0x04E03124
#define CONFIG_SYS_MDDRC_TIME_CFG1	0x30CA1147
#define CONFIG_SYS_MDDRC_TIME_CFG2	0x32B10864

/* register address only, i.e. template without values */
#define CONFIG_SYS_MICRON_BMODE		0x01000000
#define CONFIG_SYS_MICRON_EMODE		0x01010000
#define CONFIG_SYS_MICRON_EMODE2	0x01020000
#define CONFIG_SYS_MICRON_EMODE3	0x01030000
/*
 * values for mode registers (without mode register address)
 */
/* CAS 2.5 (6), burst seq (0) and length 4 (2) */
#define CONFIG_SYS_MICRON_BMODE_PARAM	0x00000062
#define CONFIG_SYS_MICRON_BMODE_RSTDLL	0x00000100
/* DLL enable, reduced drive strength */
#define CONFIG_SYS_MICRON_EMODE_PARAM	0x00000002

#define CONFIG_SYS_DDRCMD_NOP		0x01380000
#define CONFIG_SYS_DDRCMD_PCHG_ALL	0x01100400
#define CONFIG_SYS_MICRON_EMR	       ((1 << 24) |	/* CMD_REQ */ \
					(0 << 22) |	/* DRAM_CS */ \
					(0 << 21) |	/* DRAM_RAS */ \
					(0 << 20) |	/* DRAM_CAS */ \
					(0 << 19) |	/* DRAM_WEB */ \
					(1 << 16) |	/* DRAM_BS[2:0] */ \
					(0 << 15) |	/* */ \
					(0 << 12) |	/* A12->out */ \
					(0 << 11) |	/* A11->RDQS */ \
					(0 << 10) |	/* A10->DQS# */ \
					(0 <<  7) |	/* OCD program */ \
					(0 <<  6) |	/* Rtt1 */ \
					(0 <<  3) |	/* posted CAS# */ \
					(0 <<  2) |	/* Rtt0 */ \
					(1 <<  1) |	/* ODS */ \
					(0 <<  0)	/* DLL */ \
				     )
#define CONFIG_SYS_MICRON_EMR2		0x01020000
#define CONFIG_SYS_MICRON_EMR3		0x01030000
#define CONFIG_SYS_DDRCMD_RFSH		0x01080000
#define CONFIG_SYS_MICRON_INIT_DEV_OP	0x01000432
#define CONFIG_SYS_MICRON_EMR_OCD      ((1 << 24) |	/* CMD_REQ */ \
					(0 << 22) |	/* DRAM_CS */ \
					(0 << 21) |	/* DRAM_RAS */ \
					(0 << 20) |	/* DRAM_CAS */ \
					(0 << 19) |	/* DRAM_WEB */ \
					(1 << 16) |	/* DRAM_BS[2:0] */ \
					(0 << 15) |	/* */ \
					(0 << 12) |	/* A12->out */ \
					(0 << 11) |	/* A11->RDQS */ \
					(1 << 10) |	/* A10->DQS# */ \
					(7 <<  7) |	/* OCD program */ \
					(0 <<  6) |	/* Rtt1 */ \
					(0 <<  3) |	/* posted CAS# */ \
					(1 <<  2) |	/* Rtt0 */ \
					(0 <<  1) |	/* ODS */ \
					(0 <<  0)	/* DLL */ \
				     )

/*
 * Backward compatible definitions,
 * so we do not have to change arch/powerpc/cpu/mpc512x/fixed_sdram.c
 */
#define	CONFIG_SYS_DDRCMD_EM2		(CONFIG_SYS_MICRON_EMR2)
#define CONFIG_SYS_DDRCMD_EM3		(CONFIG_SYS_MICRON_EMR3)
#define CONFIG_SYS_DDRCMD_EN_DLL	(CONFIG_SYS_MICRON_EMR)
#define CONFIG_SYS_DDRCMD_OCD_DEFAULT	(CONFIG_SYS_MICRON_EMR_OCD)

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
#define CONFIG_SYS_FLASH_CFI				/* use the CFI code */
#define CONFIG_FLASH_CFI_DRIVER				/* use the CFI driver */
#define CONFIG_SYS_FLASH_BASE		0xFC000000	/* start of FLASH */
#define CONFIG_SYS_FLASH_SIZE		0x04000000	/* max flash size */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ \
	CONFIG_SYS_FLASH_BASE + 0 * CONFIG_SYS_FLASH_SIZE, \
	}
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_PROTECTION

/*
 * SRAM support
 */
#define CONFIG_SYS_SRAM_BASE		0x30000000
#define CONFIG_SYS_SRAM_SIZE		0x00020000	/* 128 KB */

/*
 * CS related parameters
 * TODO document these
 */
/* CS0 Flash */
#define CONFIG_SYS_CS0_CFG		0x00031110
#define CONFIG_SYS_CS0_START		0xFC000000
#define CONFIG_SYS_CS0_SIZE		0x04000000
/* CS1 FRAM */
#define CONFIG_SYS_CS1_CFG		0x00011000
#define CONFIG_SYS_CS1_START		0xE0000000
#define CONFIG_SYS_CS1_SIZE		0x00010000
/* CS2 AS-i 1 */
#define CONFIG_SYS_CS2_CFG		0x00009100
#define CONFIG_SYS_CS2_START		0xE0100000
#define CONFIG_SYS_CS2_SIZE		0x00080000
/* CS3 netX */
#define CONFIG_SYS_CS3_CFG		0x000A1140
#define CONFIG_SYS_CS3_START		0xE0300000
#define CONFIG_SYS_CS3_SIZE		0x00020000
/* CS5 safety */
#define CONFIG_SYS_CS5_CFG		0x0011F000
#define CONFIG_SYS_CS5_START		0xE0400000
#define CONFIG_SYS_CS5_SIZE		0x00010000
/* CS6 AS-i 2 */
#define CONFIG_SYS_CS6_CFG		0x00009100
#define CONFIG_SYS_CS6_START		0xE0200000
#define CONFIG_SYS_CS6_SIZE		0x00080000

/* Don't use alternative CS timing for any CS */
#define CONFIG_SYS_CS_ALETIMING		0x00000000
#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE		0x00000020
#define CONFIG_SYS_CS_HOLDCYCLE		0x00000020

/* Use SRAM for initial stack */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_INIT_RAM_END		CONFIG_SYS_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_SIZE	0x100
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - \
					 CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)

#ifdef	CONFIG_FSL_DIU_FB
#define CONFIG_SYS_MALLOC_LEN		(6 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)
#endif

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX		1

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE		3	/* console on PSC3 */
#define CONFIG_SYS_PSC3
#if CONFIG_PSC_CONSOLE != 3
#error CONFIG_PSC_CONSOLE must be 3
#endif

#define CONFIG_BAUDRATE			115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONSOLE_FIFO_TX_SIZE		FIFOC_PSC3_TX_SIZE
#define CONSOLE_FIFO_TX_ADDR		FIFOC_PSC3_TX_ADDR
#define CONSOLE_FIFO_RX_SIZE		FIFOC_PSC3_RX_SIZE
#define CONSOLE_FIFO_RX_ADDR		FIFOC_PSC3_RX_ADDR

/*
 * Clocks in use
 */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |		\
			 CLOCK_SCCR1_LPC_EN |		\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) | \
			 CLOCK_SCCR1_PSC_EN(7) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |	\
			 CLOCK_SCCR1_DDR_EN |		\
			 CLOCK_SCCR1_FEC_EN |		\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN |		\
			 CLOCK_SCCR2_DIU_EN |		\
			 CLOCK_SCCR2_I2C_EN)


#define CONFIG_CMDLINE_EDITING		1	/* command line history */

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CONFIG_I2C_MULTI_BUS

/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * IIM - IC Identification Module
 */
#undef CONFIG_FSL_IIM

/*
 * EEPROM configuration for Atmel AT24C01:
 * 8-bit addresses, 30ms write delay, 32-Byte Page Write Mode
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x52
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	30
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5

/*
 * Ethernet configuration
 */
#define CONFIG_MPC512x_FEC		1
#define CONFIG_NET_MULTI
#define CONFIG_PHY_ADDR			0x1F
#define CONFIG_MII			1	/* MII PHY management */
#define CONFIG_FEC_AN_TIMEOUT		1
#define CONFIG_HAS_ETH0

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH		1
/* This has to be a multiple of the flash sector size */
#define CONFIG_ENV_ADDR			0xFFF40000
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_SECT_SIZE		0x20000

/* Address and size of Redundant Environment Sector */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + \
					 CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO		1
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1

#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#undef CONFIG_CMD_FUSE
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_EXT2
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_CMD_IDE) || defined(CONFIG_CMD_EXT2)
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION
#endif /* defined(CONFIG_CMD_IDE) */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"ac14xx> "	/* Monitor Command Prompt */

#ifdef CONFIG_CMD_KGDB
# define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
# define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + \
				 sizeof(CONFIG_SYS_PROMPT) + 16)
/* max number of command args */
#define CONFIG_SYS_MAXARGS	32
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* decrementer freq: 1ms ticks */
#define CONFIG_SYS_HZ		1000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/* Cache Configuration */
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of 32 */
#endif

#define CONFIG_SYS_HID0_INIT		0x000000000
#define CONFIG_SYS_HID0_FINAL		(HID0_ENABLE_MACHINE_CHECK | \
					 HID0_ICE)
#define CONFIG_SYS_HID2	HID2_HBE

#define CONFIG_HIGH_BATS		1	/* High BATs supported */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD			0x01
#define BOOTFLAG_WARM			0x02

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE		230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX		2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_TIMESTAMP

#define CONFIG_HOSTNAME		ac14xx
#define CONFIG_BOOTFILE		"ac14xx/uImage"
#define CONFIG_ROOTPATH		"/opt/eldk/ppc_6xx"

/* default load addr for tftp and bootm */
#define CONFIG_LOADADDR		400000

#define CONFIG_BOOTDELAY	2	/* -1 disables auto-boot */

/* XXX TODO need to specify the builtin environment */
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#define CONFIG_EXTRA_ENV_SETTINGS_DEVEL					\
	"muster_nr=00\0"						\
	"fromram=run ramargs addip addtty; "				\
		"tftp ${fdt_addr_r} k6m2/ac14xx.dtb-${muster_nr}; "	\
		"tftp ${kernel_addr_r} k6m2/uImage-${muster_nr}; "	\
		"tftp ${ramdisk_addr_r} k6m2/uFS-${muster_nr}; "	\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r} ${fdt_addr_r}\0" \
	"fromnfs=run nfsargs addip addtty; "				\
		"tftp ${fdt_addr_r} k6m2/ac14xx.dtb-${muster_nr}; "	\
		"tftp ${kernel_addr_r} k6m2/uImage-${muster_nr}; "	\
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0"		\
	"fromflash=run nfsargs addip addtty; "				\
		"bootm fc020000 - fc000000\0"				\
	"mtdargsrec=setenv bootargs root=/dev/mtdblock1 ro\0"		\
	"recovery=run mtdargsrec addip addtty; "			\
		"bootm ffd20000 - ffee0000\0"				\
	"production=run ramargs addip addtty; "				\
		"bootm fc020000 fc400000 fc000000\0"			\
	"mtdargs=setenv bootargs root=/dev/mtdblock1 ro\0"		\
	"prodmtd=run mtdargs addip addtty; "				\
		"bootm fc020000 - fc000000\0"				\
	""

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr_r=200000\0"					\
	"kernel_addr_r=600000\0"					\
	"fdt_addr_r=a00000\0"						\
	"ramdisk_addr_r=b00000\0"					\
	"u-boot_addr=FFF00000\0"					\
	"kernel_addr=FC020000\0"					\
	"fdt_addr=FC000000\0"						\
	"ramdisk_addr=FC400000\0"					\
	"verify=n\0"							\
	"ramdiskfile=ac14xx/uRamdisk\0"					\
	"u-boot=ac14xx/u-boot.bin\0"					\
	"bootfile=ac14xx/uImage\0"					\
	"fdtfile=ac14xx/ac14xx.dtb\0"					\
	"rootpath=/opt/eldk/ppc_6xx\n"					\
	"netdev=eth0\0"							\
	"consdev=ttyPSC0\0"						\
	"hostname=ac14xx\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}-${muster_nr}\0"	\
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
	CONFIG_EXTRA_ENV_SETTINGS_DEVEL					\
	"upd=run load update\0"						\
	""

#define CONFIG_BOOTCOMMAND	"run production"

#define CONFIG_FIT		1
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_SUPPORT_OLD_DEVICE_TREES	1

#define OF_CPU			"PowerPC,5121@0"
#define OF_SOC_COMPAT		"fsl,mpc5121-immr"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc@80000000/serial@11300"

#endif	/* __CONFIG_H */
