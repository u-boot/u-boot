/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009, DAVE Srl <www.dave.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * modifications for the MECP5123 by reinhard.arlt@esd-electronics.com
 *
 */

/*
 * MECP5123 board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MECP5123 1
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_SYS_GENERIC_BOARD

/*
 * Memory map for the MECP5123 board:
 *
 * 0x0000_0000 - 0x1FFF_FFFF	DDR RAM (512 MB)
 * 0x3000_0000 - 0x3001_FFFF	SRAM (128 KB)
 * 0x8000_0000 - 0x803F_FFFF	IMMR (4 MB)
 * 0x8200_0000 - 0x8200_FFFF	VPC-3 (64 KB)
 * 0xFFC0_0000 - 0xFFFF_FFFF	NOR Boot FLASH (64 MB)
 */

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1	/* E300 Family */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#define CONFIG_SYS_MPC512X_CLKIN	33333333	/* in Hz */

#define CONFIG_BOARD_EARLY_INIT_F		/* call board_early_init_f() */
#define CONFIG_MISC_INIT_R

#define CONFIG_SYS_IMMR		        0x80000000
#define CONFIG_SYS_DIU_ADDR		(CONFIG_SYS_IMMR+0x2100)

#define CONFIG_SYS_MEMTEST_START	0x00200000      /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * DDR Setup - manually set all parameters as there's no SPD etc.
 */
#define CONFIG_SYS_DDR_SIZE		512		/* MB */

#define CONFIG_SYS_DDR_BASE		0x00000000	/* DDR is sys memory*/
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
#define CONFIG_SYS_MDDRC_SYS_CFG	 0xEA804A00
#define CONFIG_SYS_MDDRC_TIME_CFG0	 0x06183D2E
#define CONFIG_SYS_MDDRC_TIME_CFG1	 0x68EC1168
#define CONFIG_SYS_MDDRC_TIME_CFG2	 0x34310864

#define CONFIG_SYS_DDRCMD_NOP		0x01380000
#define CONFIG_SYS_DDRCMD_PCHG_ALL	0x01100400
#define CONFIG_SYS_DDRCMD_EM2		0x01020000
#define CONFIG_SYS_DDRCMD_EM3		0x01030000
#define CONFIG_SYS_DDRCMD_EN_DLL	0x01010000
#define CONFIG_SYS_DDRCMD_RFSH		0x01080000
#define CONFIG_SYS_MICRON_INIT_DEV_OP	0x01000432
#define CONFIG_SYS_DDRCMD_OCD_DEFAULT	0x01010780

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
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */

#define CONFIG_SYS_FLASH_BASE		0xFFC00000	/* start of FLASH */
#define CONFIG_SYS_FLASH_SIZE		0x00400000	/* max flash size */

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max sectors per device */

#undef CONFIG_SYS_FLASH_CHECKSUM

/*
 * NAND FLASH
 * drivers/mtd/nand/mpc5121_nfc.c (rev 2 silicon only)
 */
#define CONFIG_CMD_NAND
#define CONFIG_NAND_MPC5121_NFC
#define CONFIG_SYS_NAND_BASE            0x40000000
#define CONFIG_SYS_MAX_NAND_DEVICE      1

/*
 * Configuration parameters for MPC5121 NAND driver
 */
#define CONFIG_FSL_NFC_WIDTH		1
#define CONFIG_FSL_NFC_WRITE_SIZE	2048
#define CONFIG_FSL_NFC_SPARE_SIZE	64
#define CONFIG_FSL_NFC_CHIPS		1

#define CONFIG_SYS_SRAM_BASE		0x30000000
#define CONFIG_SYS_SRAM_SIZE		0x00020000	/* 128 KB */

/* Initialize Local Window for NOR FLASH access */
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

/* ALE active low, data size 4bytes */
#define CONFIG_SYS_CS0_CFG		0x05051150

/* Use not alternative CS timing */
#define CONFIG_SYS_CS_ALETIMING		0x00000000

/* ALE active low, data size 4bytes */
#define CONFIG_SYS_CS1_CFG		0x1f1f3090
#define CONFIG_SYS_VPC3_BASE		0x82000000	/* start of VPC3 space */
#define CONFIG_SYS_VPC3_SIZE		0x00010000	/* max VPC3 size */
/* Initialize Local Window for VPC3 access */
#define CONFIG_SYS_CS1_START		CONFIG_SYS_VPC3_BASE
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_VPC3_SIZE

/* Use SRAM for initial stack */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SRAM_BASE /* Init RAM addr */
#define CONFIG_SYS_INIT_RAM_SIZE		CONFIG_SYS_SRAM_SIZE

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* Start of monitor */
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Monitor length */
#define CONFIG_SYS_MALLOC_LEN		(6 * 1024 * 1024) /* Malloc size */

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
#define CONFIG_BAUDRATE		9600	/* ... at 9600 bps */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONSOLE_FIFO_TX_SIZE	FIFOC_PSC3_TX_SIZE
#define CONSOLE_FIFO_TX_ADDR	FIFOC_PSC3_TX_ADDR
#define CONSOLE_FIFO_RX_SIZE	FIFOC_PSC3_RX_SIZE
#define CONSOLE_FIFO_RX_ADDR	FIFOC_PSC3_RX_ADDR

/*
 * Clocks in use
 */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_NFC_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |	\
			 CLOCK_SCCR2_I2C_EN)


#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

/* I2C */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed */
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* slave address */

/*
 * IIM - IC Identification Module
 */
#undef CONFIG_FSL_IIM

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2	/* 16-bit EEPROM address */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Atmel: AT24C32A-10TQ-2.7 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10	/* 10ms of delay */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 5	/* 32-Byte Page Write Mode */
#define CONFIG_SYS_EEPROM_WREN			/* Use EEPROM write protect */

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
#define CONFIG_SYS_RTC_BUS_NUM  0x01
#define CONFIG_SYS_I2C_RTC_ADDR 0x32
#define CONFIG_RTC_RX8025

/*
 * Environment
 */
#define CONFIG_ENV_IS_IN_EEPROM		/* Store env in I2C EEPROM	*/
#define CONFIG_ENV_SIZE		0x1000
#define CONFIG_ENV_OFFSET       0x0000	/* environment starts here	*/

#define CONFIG_LOADS_ECHO		/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change	*/

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_DATE
#undef CONFIG_CMD_FUSE
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_ELF
#define CONFIG_DOS_PARTITION

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
#define CONFIG_SYS_BOOTMAPSZ	(256 << 20)	/* Linux initial memory map */

/* Cache Configuration */
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5
#endif

#define CONFIG_SYS_HID0_INIT	0x000000000
#define CONFIG_SYS_HID0_FINAL	HID0_ENABLE_MACHINE_CHECK
#define CONFIG_SYS_HID2		HID2_HBE

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_TIMESTAMP

#define CONFIG_HOSTNAME		mecp512x
#define CONFIG_BOOTFILE		"/tftpboot/mecp512x/uImage"
#define CONFIG_ROOTPATH		"/tftpboot/mecp512x/target_root"

#define CONFIG_LOADADDR		400000	/* def. location for tftp and bootm */

#define CONFIG_BOOTDELAY	5	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS			/* the boot command will set bootargs*/

#define CONFIG_PREBOOT	"echo;"	\
	"echo Welcome to MECP5123" \
	"echo"

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"u-boot_addr_r=200000\0"					\
	"kernel_addr_r=600000\0"					\
	"fdt_addr_r=880000\0"						\
	"ramdisk_addr_r=900000\0"					\
	"u-boot_addr=FFF00000\0"					\
	"kernel_addr=FFC40000\0"					\
	"fdt_addr=FFEC0000\0"						\
	"ramdisk_addr=FC040000\0"					\
	"ramdiskfile=/tftpboot/mecp512x/uRamdisk\0"			\
	"u-boot=/tftpboot/mecp512x/u-boot.bin\0"			\
	"bootfile=/tftpboot/mecp512x/uImage\0"				\
	"fdtfile=/tftpboot/mecp512x/mecp512x.dtb\0"			\
	"rootpath=/tftpboot/mecp512x/target_root\n"	      		\
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

#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP

#define OF_CPU			"PowerPC,5121@0"
#define OF_SOC_COMPAT		"fsl,mpc5121-immr"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc@80000000/serial@11300"

#endif	/* __CONFIG_H */
