/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009, DAVE Srl <www.dave.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Aria board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARIA 1
#define CONFIG_DISPLAY_BOARDINFO

/*
 * Memory map for the ARIA board:
 *
 * 0x0000_0000-0x0FFF_FFFF	DDR RAM (256 MB)
 * 0x3000_0000-0x3001_FFFF	On Chip SRAM (128 KB)
 * 0x3010_0000-0x3011_FFFF	On Board SRAM (128 KB) - CS6
 * 0x3020_0000-0x3021_FFFF	FPGA (128 KB) - CS2
 * 0x8000_0000-0x803F_FFFF	IMMR (4 MB)
 * 0x8400_0000-0x82FF_FFFF	PCI I/O space (16 MB)
 * 0xA000_0000-0xAFFF_FFFF	PCI memory space (256 MB)
 * 0xB000_0000-0xBFFF_FFFF	PCI memory mapped I/O space (256 MB)
 * 0xFC00_0000-0xFFFF_FFFF	NOR Boot FLASH (64 MB)
 */

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */
#define CONFIG_FSL_DIU_FB	1	/* FSL DIU */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

/* video */
#undef CONFIG_VIDEO

#if defined(CONFIG_VIDEO)
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#endif

/* CONFIG_PCI is defined at config time */

#define CONFIG_SYS_MPC512X_CLKIN	33000000	/* in Hz */

#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_IMMR			0x80000000
#define CONFIG_SYS_DIU_ADDR		(CONFIG_SYS_IMMR+0x2100)

#define CONFIG_SYS_MEMTEST_START	0x00200000      /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * DDR Setup - manually set all parameters as there's no SPD etc.
 */
#define CONFIG_SYS_DDR_SIZE		256		/* MB */
#define CONFIG_SYS_DDR_BASE		0x00000000
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
#define CONFIG_SYS_MDDRC_SYS_CFG     (	(1 << 31) |	/* RST_B */ \
					(1 << 30) |	/* CKE */ \
					(1 << 29) |	/* CLK_ON */ \
					(0 << 28) |	/* CMD_MODE */ \
					(4 << 25) |	/* DRAM_ROW_SELECT */ \
					(3 << 21) |	/* DRAM_BANK_SELECT */ \
					(0 << 18) |	/* SELF_REF_EN */ \
					(0 << 17) |	/* 16BIT_MODE */ \
					(2 << 13) |	/* RDLY */ \
					(0 << 12) |	/* HALF_DQS_DLY */ \
					(1 << 11) |	/* QUART_DQS_DLY */ \
					(2 <<  8) |	/* WDLY */ \
					(0 <<  7) |	/* EARLY_ODT */ \
					(1 <<  6) |	/* ON_DIE_TERMINATE */ \
					(0 <<  5) |	/* FIFO_OV_CLEAR */ \
					(0 <<  4) |	/* FIFO_UV_CLEAR */ \
					(0 <<  1) |	/* FIFO_OV_EN */ \
					(0 <<  0) 	/* FIFO_UV_EN */ \
				     )

#define CONFIG_SYS_MDDRC_TIME_CFG0	0x030C3D2E
#define CONFIG_SYS_MDDRC_TIME_CFG1	0x55D81189
#define CONFIG_SYS_MDDRC_TIME_CFG2	0x34790863

#define CONFIG_SYS_DDRCMD_NOP		0x01380000
#define CONFIG_SYS_DDRCMD_PCHG_ALL	0x01100400
#define CONFIG_SYS_MICRON_EMR	     (	(1 << 24) |	/* CMD_REQ */ \
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
#define CONFIG_SYS_MICRON_EMR_OCD    (	(1 << 24) |	/* CMD_REQ */ \
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
					(0 <<  1) |	/* ODS (Output Drive Strength) */ \
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
#define CONFIG_SYS_FLASH_BASE		0xF8000000	/* start of FLASH */
#define CONFIG_SYS_FLASH_SIZE		0x08000000	/* max flash size */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* max sectors */

#undef CONFIG_SYS_FLASH_CHECKSUM

/*
 * NAND FLASH support
 * drivers/mtd/nand/mpc5121_nfc.c (rev 2 silicon only)
 */
#define CONFIG_CMD_NAND					/* enable NAND support */
#define CONFIG_JFFS2_NAND				/* with JFFS2 on it */
#define CONFIG_NAND_MPC5121_NFC
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/*
 * Configuration parameters for MPC5121 NAND driver
 */
#define CONFIG_FSL_NFC_WIDTH		1
#define CONFIG_FSL_NFC_WRITE_SIZE	2048
#define CONFIG_FSL_NFC_SPARE_SIZE	64
#define CONFIG_FSL_NFC_CHIPS		CONFIG_SYS_MAX_NAND_DEVICE

#define CONFIG_SYS_SRAM_BASE		0x30000000
#define CONFIG_SYS_SRAM_SIZE		0x00020000	/* 128 KB */

/* Make two SRAM regions contiguous */
#define CONFIG_SYS_ARIA_SRAM_BASE	(CONFIG_SYS_SRAM_BASE + \
					 CONFIG_SYS_SRAM_SIZE)
#define CONFIG_SYS_ARIA_SRAM_SIZE	0x00100000	/* reserve 1MB-window */
#define CONFIG_SYS_CS6_START		CONFIG_SYS_ARIA_SRAM_BASE
#define CONFIG_SYS_CS6_SIZE		CONFIG_SYS_ARIA_SRAM_SIZE

#define CONFIG_SYS_ARIA_FPGA_BASE	(CONFIG_SYS_ARIA_SRAM_BASE + \
					 CONFIG_SYS_ARIA_SRAM_SIZE)
#define CONFIG_SYS_ARIA_FPGA_SIZE	0x20000		/* 128 KB */

#define CONFIG_SYS_CS2_START		CONFIG_SYS_ARIA_FPGA_BASE
#define CONFIG_SYS_CS2_SIZE		CONFIG_SYS_ARIA_FPGA_SIZE

#define CONFIG_SYS_CS0_CFG		0x05059150
#define CONFIG_SYS_CS2_CFG		(	(5 << 24) | \
						(5 << 16) | \
						(1 << 15) | \
						(0 << 14) | \
						(0 << 13) | \
						(1 << 12) | \
						(0 << 10) | \
						(3 <<  8) | /* 32 bit */ \
						(0 <<  7) | \
						(1 <<  6) | \
						(1 <<  4) | \
						(0 <<  3) | \
						(0 <<  2) | \
						(0 <<  1) | \
						(0 <<  0)   \
					)
#define CONFIG_SYS_CS6_CFG		0x05059150

/* Use alternative CS timing for CS0 and CS2 */
#define CONFIG_SYS_CS_ALETIMING	0x00000005

/* Use SRAM for initial stack */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE		CONFIG_SYS_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)

#ifdef	CONFIG_FSL_DIU_FB
#define CONFIG_SYS_MALLOC_LEN		(6 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024)
#endif

/* FPGA */
#define CONFIG_ARIA_FPGA		1

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
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONSOLE_FIFO_TX_SIZE		FIFOC_PSC3_TX_SIZE
#define CONSOLE_FIFO_TX_ADDR		FIFOC_PSC3_TX_ADDR
#define CONSOLE_FIFO_RX_SIZE		FIFOC_PSC3_RX_SIZE
#define CONSOLE_FIFO_RX_ADDR		FIFOC_PSC3_RX_ADDR

#define CONFIG_CMDLINE_EDITING		1	/* command line history */

/*
 * PCI
 */
#ifdef CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE

#define CONFIG_SYS_PCI_MEM_BASE		0xA0000000
#define CONFIG_SYS_PCI_MEM_PHYS		CONFIG_SYS_PCI_MEM_BASE
#define CONFIG_SYS_PCI_MEM_SIZE		0x10000000	/* 256M */
#define CONFIG_SYS_PCI_MMIO_BASE	(CONFIG_SYS_PCI_MEM_BASE + \
					 CONFIG_SYS_PCI_MEM_SIZE)
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
#define CONFIG_I2C_MULTI_BUS

/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x7F
#if 0
#define CONFIG_SYS_I2C_NOPROBES	{{0,0x69}}	/* Don't probe these addrs */
#endif

/*
 * IIM - IC Identification Module
 */
#undef CONFIG_FSL_IIM

/*
 * EEPROM configuration for Atmel AT24C32A-10TQ-2.7:
 * 16-bit addresses, 10ms write delay, 32-Byte Page Write Mode
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5

/*
 * Ethernet configuration
 */
#define CONFIG_MPC512x_FEC		1
#define CONFIG_PHY_ADDR			0x17
#define CONFIG_MII			1	/* MII PHY management */
#define CONFIG_FEC_AN_TIMEOUT		1
#define CONFIG_HAS_ETH0

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_FLASH	1
/* This has to be a multiple of the flash sector size */
#define CONFIG_ENV_ADDR			(CONFIG_SYS_MONITOR_BASE + \
					 CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_SECT_SIZE		0x20000	/* one sector (256K) */

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_ADDR + \
					 CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_LOADS_ECHO		1
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1

#define CONFIG_CMD_EEPROM
#undef CONFIG_CMD_FUSE
#undef CONFIG_CMD_IDE
#define CONFIG_CMD_JFFS2
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
 * Dynamic MTD partition support
 */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#define MTDIDS_DEFAULT		"nor0=f8000000.flash,nand0=mpc5121.nand"

/*
 * NOR flash layout:
 *
 * F8000000 - FEAFFFFF	107 MiB		User Data
 * FEB00000 - FFAFFFFF	 16 MiB		Root File System
 * FFB00000 - FFFEFFFF	  4 MiB		Linux Kernel
 * FFF00000 - FFFBFFFF	768 KiB		U-Boot (up to 512 KiB) and 2 x * env
 * FFFC0000 - FFFFFFFF	256 KiB		Device Tree
 *
 * NAND flash layout: one big partition
 */
#define MTDPARTS_DEFAULT	"mtdparts=f8000000.flash:107m(user),"	\
						"16m(rootfs),"		\
						"4m(kernel),"		\
						"768k(u-boot),"		\
						"256k(dtb);"		\
					"mpc5121.nand:-(data)"

/*
 * Watchdog timeout = CONFIG_SYS_WATCHDOG_VALUE * 65536 / IPS clock.
 * For example, when IPS is set to 66MHz and CONFIG_SYS_WATCHDOG_VALUE
 * is set to 0xFFFF, watchdog timeouts after about 64s. For details
 * refer to chapter 36 of the MPC5121e Reference Manual.
 */
/* #define CONFIG_WATCHDOG */		/* enable watchdog */
#define CONFIG_SYS_WATCHDOG_VALUE 0xFFFF

 /*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */

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

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)

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

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE		230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_TIMESTAMP

#define CONFIG_HOSTNAME			aria
#define CONFIG_BOOTFILE			"aria/uImage"
#define CONFIG_ROOTPATH			"/opt/eldk/ppc_6xx"

#define CONFIG_LOADADDR			400000	/* default load addr */

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
	"kernel_addr=FFB00000\0"					\
	"fdt_addr=FFFC0000\0"						\
	"ramdisk_addr=FEB00000\0"					\
	"ramdiskfile=aria/uRamdisk\0"				\
	"u-boot=aria/u-boot.bin\0"					\
	"fdtfile=aria/aria.dtb\0"					\
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

#define CONFIG_SYS_IDE_MAXBUS		1	/* 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* 1 drive per IDE bus	*/

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

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_NFC_EN |				\
			 CLOCK_SCCR1_PATA_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN |		\
			 CLOCK_SCCR2_DIU_EN |		\
			 CLOCK_SCCR2_I2C_EN)

#endif	/* __CONFIG_H */
