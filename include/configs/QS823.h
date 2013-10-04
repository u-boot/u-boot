/*
 * (C) Copyright 2003
 * MuLogic B.V.
 *
 * (C) Copyright 2002
 * Simple Network Magic Corporation
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* various debug settings */
#undef CONFIG_SYS_DEVICE_NULLDEV		/* null device */
#undef CONFIG_SILENT_CONSOLE		/* silent console */
#undef CONFIG_SYS_CONSOLE_INFO_QUIET		/* silent console ? */
#undef DEBUG_FLASH			/* debug flash code */
#undef FLASH_DEBUG			/* debug fash code */
#undef DEBUG_ENV			/* debug environment code */

#define CONFIG_SYS_DIRECT_FLASH_TFTP	1	/* allow direct tftp to flash */
#define CONFIG_ENV_OVERWRITE	1	/* allow overwrite MAC address */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC823		1	/* This is a MPC823 CPU */
#define CONFIG_QS823		1	/* ...on a QS823 module */
#define CONFIG_SCC2_ENET	1	/* SCC2 10BaseT ethernet */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

/* Select the target clock speed */
#undef CONFIG_CLOCK_16MHZ		/* cpu=16,777,216 Hz, mem=16Mhz */
#undef CONFIG_CLOCK_33MHZ		/* cpu=33,554,432 Hz, mem=33Mhz */
#undef CONFIG_CLOCK_50MHZ		/* cpu=49,971,200 Hz, mem=33Mhz */
#define CONFIG_CLOCK_66MHZ	1	/* cpu=67,108,864 Hz, mem=66Mhz */
#undef CONFIG_CLOCK_80MHZ		/* cpu=79,986,688 Hz, mem=33Mhz */

#ifdef CONFIG_CLOCK_16MHZ
#define CONFIG_CLOCK_MULT	512
#endif

#ifdef CONFIG_CLOCK_33MHZ
#define CONFIG_CLOCK_MULT	1024
#endif

#ifdef CONFIG_CLOCK_50MHZ
#define CONFIG_CLOCK_MULT	1525
#endif

#ifdef CONFIG_CLOCK_66MHZ
#define CONFIG_CLOCK_MULT	2048
#endif

#ifdef CONFIG_CLOCK_80MHZ
#define CONFIG_CLOCK_MULT	2441
#endif

/* choose flash size, 4Mb or 8Mb */
#define CONFIG_FLASH_4MB	1	/* board has 4Mb flash */
#undef CONFIG_FLASH_8MB			/* board has 8Mb flash */

#define CONFIG_CLOCK_BASE	32768	/* Base clock input freq */

#undef CONFIG_8xx_CONS_SMC1
#define CONFIG_8xx_CONS_SMC2	1	/* Console is on SMC2 */
#undef CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		38400	/* console baudrate = 38.4kbps */

#undef CONFIG_CLOCKS_IN_MHZ		/* clocks passsed to Linux in MHz */

/* Define default IP addresses */
#define CONFIG_IPADDR		192.168.1.99	/* own ip address */
#define CONFIG_SERVERIP		192.168.1.19	/* used for tftp (not nfs?) */

/* message to say directly after booting */
#define CONFIG_PREBOOT		"echo '';" \
	"echo 'type:';" \
	"echo 'run boot_nfs       to boot to NFS';" \
	"echo 'run boot_flash     to boot to flash';" \
	"echo '';" \
	"echo 'run flash_rootfs   to install a new rootfs';" \
	"echo 'run flash_env      to clear the env sector';" \
	"echo 'run flash_rw       to clear the rw fs';" \
	"echo 'run flash_uboot    to install a new u-boot';" \
	"echo 'run flash_kernel   to install a new kernel';"

/* wait 5 seconds before executing CONFIG_BOOTCOMMAND */
#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOTCOMMAND	"run boot_nfs"

#undef CONFIG_BOOTARGS		/* made by set_nfs of set_flash */

/* Our flash filesystem looks like this
 *
 * 4Mb board:
 * ffc0 0000 - ffeb ffff	root filesystem (jffs2) (~3Mb)
 * ffec 0000 - ffed ffff	read-write filesystem (ext2)
 * ffee 0000 - ffef ffff	environment
 * fff0 0000 - fff1 ffff	u-boot
 * fff2 0000 - ffff ffff	linux kernel
 *
 * 8Mb board:
 * ff80 0000 - ffeb ffff	root filesystem (jffs2) (~7Mb)
 * ffec 0000 - ffed ffff	read-write filesystem (ext2)
 * ffee 0000 - ffef ffff	environment
 * fff0 0000 - fff1 ffff	u-boot
 * fff2 0000 - ffff ffff	linux kernel
 *
 */

/* environment for 4Mb board */
#ifdef CONFIG_FLASH_4MB
#define CONFIG_EXTRA_ENV_SETTINGS \
	"serial#=QS823\0" \
	"hostname=qs823\0" \
	"netdev=eth0\0" \
	"ethaddr=00:01:02:B4:36:56\0" \
	"rootpath=/exports/rootfs\0" \
	"mtdparts=mtdparts=phys:2816k(root),128k(rw),128k(env),128k(u-boot),-(kernel)\0" \
	/* fill in variables */ \
	"set_ip=setenv ip ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off\0" \
	"set_nfs=setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath $ip init=/sbin/init $mtdparts\0" \
	"set_flash=setenv bootargs root=/dev/mtdblock1 ro $ip init=/sbin/init $mtdparts\0" \
	/* commands */ \
	"boot_nfs=run set_ip; run set_nfs; tftp 0x400000 /tftpboot/vmlinux.UBoot; bootm 0x400000\0" \
	"boot_flash=run set_ip; run set_flash; bootm fff20000\0" \
	/* reinstall flash parts */ \
	"flash_rootfs=protect off ffc00000 ffebffff; era ffc00000 ffebffff; tftp ffc00000 /tftpboot/rootfs.jffs2\0" \
	"flash_rw=protect off ffec0000 ffedffff; era ffec0000 ffedffff\0" \
	"flash_env=protect off ffee0000 ffefffff; era ffee0000 ffefffff\0" \
	"flash_uboot=protect off fff00000 fff1ffff; era fff00000 fff1ffff; tftp fff00000 /tftpboot/u-boot.4mb.bin\0" \
	"flash_kernel=protect off fff20000 ffffffff; era fff20000 ffffffff; tftp fff20000 /tftpboot/vmlinux.UBoot\0"
#endif /* CONFIG_FLASH_4MB */

/* environment for 8Mb board */
#ifdef CONFIG_FLASH_8MB
#define CONFIG_EXTRA_ENV_SETTINGS \
	"serial#=QS823\0" \
	"hostname=qs823\0" \
	"netdev=eth0\0" \
	"ethaddr=00:01:02:B4:36:56\0" \
	"rootpath=/exports/rootfs\0" \
	"mtdparts=mtdparts=phys:6912k(root),128k(rw),128k(env),128k(u-boot),-(kernel)\0" \
	/* fill in variables */ \
	"set_ip=setenv ip ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off\0" \
	"set_nfs=setenv bootargs root=/dev/nfs rw nfsroot=$serverip:$rootpath $ip init=/sbin/init $mtdparts\0" \
	"set_flash=setenv bootargs root=/dev/mtdblock1 ro $ip init=/sbin/init $mtdparts\0" \
	/* commands */ \
	"boot_nfs=run set_ip; run set_nfs; tftp 0x400000 /tftpboot/vmlinux.UBoot; bootm 0x400000\0" \
	"boot_flash=run set_ip; run set_flash; bootm fff20000\0" \
	/* reinstall flash parts */ \
	"flash_rootfs=protect off ff800000 ffebffff; era ff800000 ffebffff; tftp ff800000 /tftpboot/rootfs.jffs2\0" \
	"flash_rw=protect off ffec0000 ffedffff; era ffec0000 ffedffff\0" \
	"flash_env=protect off ffee0000 ffefffff; era ffee0000 ffefffff\0" \
	"flash_uboot=protect off fff00000 fff1ffff; era fff00000 fff1ffff; tftp fff00000 /tftpboot/u-boot.8mb.bin\0" \
	"flash_kernel=protect off fff20000 ffffffff; era fff20000 ffffffff; tftp fff20000 /tftpboot/vmlinux.UBoot\0"
#endif /* CONFIG_FLASH_8MB */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#undef CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change */
#undef CONFIG_WATCHDOG			/* watchdog disabled */
#undef CONFIG_STATUS_LED		/* Status LED disabled */
#undef CONFIG_CAN_DRIVER		/* CAN Driver support disabled */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#undef CONFIG_MAC_PARTITION
#undef CONFIG_DOS_PARTITION

#define CONFIG_RTC_MPC8xx	/* use internal RTC of MPC8xx */


/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_RUN


/*-----------------------------------------------------------------------
 * Environment variable storage is in FLASH, one sector before U-boot
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SECT_SIZE	0x20000		/* 128Kb, one whole sector */
#define CONFIG_ENV_SIZE		0x2000		/* 8kb */
#define CONFIG_ENV_ADDR		0xffee0000	/* address of env sector */

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory */

#define CONFIG_SYS_HUSH_PARSER		1		/* use "hush" command parser */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works */
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM */

#define CONFIG_SYS_LOAD_ADDR		0x400000	/* default load address */

/*-----------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00		/* Size of used area in DPRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFF800000	/* Allow an 8Mbyte window */

#define FLASH_BASE0_4M_PRELIM	0xFFC00000	/* Base for 4M Flash */
#define FLASH_BASE0_8M_PRELIM	0xFF800000	/* Base for 8M Flash */

#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	0xFFF00000	/* U-boot location */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * TODO flash parameters
 * FLASH organization for Intel Strataflash
 */
#undef  CONFIG_SYS_FLASH_16BIT				/* 32-bit wide flash memory */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	71		/* max number of sectors on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000		/* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms) */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16		/* For all MPC8xx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4		/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control 11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */

#ifdef CONFIG_WATCHDOG
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWE | SYPCR_SWRI | SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWRI | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration 11-6
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DLK | SIUMCR_DPC | SIUMCR_MPRE | SIUMCR_MLRC01 | SIUMCR_GB5E)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control 11-26
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register 11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control 11-31
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register 15-30
 *-----------------------------------------------------------------------
 */

/* MF (Multiplication Factor of SPLL) */
/* Sets the QS823 to specified clock from 32KHz clock at EXTAL. */
#define vPLPRCR_MF	((CONFIG_CLOCK_MULT+1) << 20)
#define CONFIG_SYS_PLPRCR	(vPLPRCR_MF | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST | PLPRCR_LOLRE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_CLOCK_16MHZ) || defined(CONFIG_CLOCK_33MHZ) || defined(CONFIG_CLOCK_50MHZ)
#define CONFIG_SYS_SCCR		(SCCR_TBS | SCCR_EBDF00 | SCCR_DFBRG00)
#define CONFIG_SYS_BRGCLK_PRESCALE	1
#endif

#if defined(CONFIG_CLOCK_66MHZ)
#define CONFIG_SYS_SCCR		(SCCR_TBS | SCCR_EBDF00 | SCCR_DFBRG01)
#define CONFIG_SYS_BRGCLK_PRESCALE	4
#endif

#if defined(CONFIG_CLOCK_80MHZ)
#define CONFIG_SYS_SCCR		(SCCR_TBS | SCCR_EBDF01 | SCCR_DFBRG01)
#define CONFIG_SYS_BRGCLK_PRESCALE	4
#endif

#define SCCR_MASK		CONFIG_SYS_SCCR

/*-----------------------------------------------------------------------
 * Debug Enable Register
 * 0x73E67C0F - All interrupts handled by BDM
 * 0x00824001 - Only interrupts needed by MWDebug.exe handled by BDM
 *-----------------------------------------------------------------------
#define CONFIG_SYS_DER			0x73E67C0F
#define CONFIG_SYS_DER			0x0082400F

 #-------------------------------------------------------------------------
 # Program the Debug Enable Register (DER). This register provides the user
 # with the reason for entering into the debug mode. We want all conditions
 # to end up as an exception. We don't want to enter into debug mode for
 # any condition. See the back of of the Development Support section of the
 # MPC860 User Manual for a description of this register.
 #-------------------------------------------------------------------------
*/
#define CONFIG_SYS_DER			0

/*-----------------------------------------------------------------------
 * Memory Controller Initialization Constants
 *-----------------------------------------------------------------------
 */

/*
 * BR0 and OR0 (AMD dual FLASH devices)
 * Base address = 0xFFF0_0000 - 0xFFF7_FFFF (After relocation)
 */
#define CONFIG_SYS_PRELIM_OR_AM
#define CONFIG_SYS_OR_TIMING_FLASH

/*
 *-----------------------------------------------------------------------
 * Base Register 0 (BR0): Bank 0 is assigned to the 8Mbyte (2M X 32)
 *                        flash that resides on the QS823.
 *-----------------------------------------------------------------------
 */

/* BA (Base Address) = 0xFF80+0b for a total of 17 bits. 17 bit base addr */
/*                     represents a minumum 32K block size. */
#define vBR0_BA			((0xFF80 << 16) + (0 << 15))
#define CONFIG_SYS_BR0_PRELIM		(vBR0_BA | BR_V)

/* AM (Address Mask) = 0xFF80+0b = We've masked the upper 9 bits        */
/*                                 which defines a 8 Mbyte memory block. */
#define vOR0_AM			((0xFF80 << 16) + (0 << 15))

#if defined(CONFIG_CLOCK_50MHZ) || defined(CONFIG_CLOCK_80MHZ)
/*  0101 = Add a 5 clock cycle wait state */
#define CONFIG_SYS_OR0_PRELIM		(vOR0_AM | OR_CSNT_SAM | 0R_ACS_DIV4 | OR_BI | OR_SCY_5_CLK)
#endif

#if defined(CONFIG_CLOCK_33MHZ) || defined(CONFIG_CLOCK_66MHZ)
/*  0011 = Add a 3 clock cycle wait state */
/*  29.8ns clock * (3 + 2) = 149ns cycle time */
#define CONFIG_SYS_OR0_PRELIM		(vOR0_AM | OR_CSNT_SAM | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK)
#endif

#if defined(CONFIG_CLOCK_16MHZ)
/*  0010 = Add a 2 clock cycle wait state */
#define CONFIG_SYS_OR0_PRELIM		(vOR0_AM | OR_CSNT_SAM | OR_ACS_DIV4 | OR_BI | OR_SCY_2_CLK)
#endif

/*
 * BR1 and OR1 (SDRAM)
 * Base Address = 0x00000000 - 0x00FF_FFFF (16M After relocation)
 * Base Address = 0x00000000 - 0x01FF_FFFF (32M After relocation)
 * Base Address = 0x00000000 - 0x03FF_FFFF (64M After relocation)
 * Base Address = 0x00000000 - 0x07FF_FFFF (128M After relocation)
 */

#define SDRAM_BASE		0x00000000	/* SDRAM bank */
#define SDRAM_PRELIM_OR_AM	0xF8000000	/* map max. 128 MB */

/* AM (Address Mask) = 0xF800+0b = We've masked the upper 5 bits which
 *                                 represents a 128 Mbyte block the DRAM in
 *                                 this address base.
 */
#define vOR1_AM			((0xF800 << 16) + (0 << 15))
#define vBR1_BA			((0x0000 << 16) + (0 << 15))
#define CONFIG_SYS_OR1			(vOR1_AM | OR_CSNT_SAM | OR_BI)
#define CONFIG_SYS_BR1			(vBR1_BA | BR_MS_UPMA | BR_V)

/* Machine A Mode Register */

/* PTA Periodic Timer A */

#if defined(CONFIG_CLOCK_80MHZ)
#define vMAMR_PTA		(19 << 24)
#endif

#if defined(CONFIG_CLOCK_66MHZ)
#define vMAMR_PTA		(16 << 24)
#endif

#if defined(CONFIG_CLOCK_50MHZ)
#define vMAMR_PTA		(195 << 24)
#endif

#if defined(CONFIG_CLOCK_33MHZ)
#define vMAMR_PTA		(131 << 24)
#endif

#if defined(CONFIG_CLOCK_16MHZ)
#define vMAMR_PTA		(65 << 24)
#endif

/* For boards with 16M of SDRAM */
#define SDRAM_16M_MAX_SIZE	0x01000000	/* max 16MB SDRAM */
#define CONFIG_SYS_16M_MAMR		(vMAMR_PTA | MAMR_AMA_TYPE_0 | MAMR_DSA_2_CYCL | MAMR_G0CLA_A11 |\
MAMR_RLFA_1X | MAMR_WLFA_1X | MAMR_TLFA_4X)

/* For boards with 32M of SDRAM */
#define SDRAM_32M_MAX_SIZE	0x02000000	/* max 32MB SDRAM */
#define CONFIG_SYS_32M_MAMR		(vMAMR_PTA | MAMR_AMA_TYPE_1 | MAMR_DSA_2_CYCL | MAMR_G0CLA_A10 |\
MAMR_RLFA_1X | MAMR_WLFA_1X | MAMR_TLFA_4X)


/* Memory Periodic Timer Prescaler Register */

#if defined(CONFIG_CLOCK_66MHZ) || defined(CONFIG_CLOCK_80MHZ)
/* Divide by 32 */
#define CONFIG_SYS_MPTPR		0x02
#endif

#if defined(CONFIG_CLOCK_16MHZ) || defined(CONFIG_CLOCK_33MHZ) || defined(CONFIG_CLOCK_50MHZ)
/* Divide by 16 */
#define CONFIG_SYS_MPTPR		0x04
#endif

/*
 * BR2 and OR2 (Unused)
 * Base address = 0xF020_0000 - 0xF020_0FFF
 *
 */
#define CONFIG_SYS_OR2_PRELIM		0xFFF00000
#define CONFIG_SYS_BR2_PRELIM		0xF0200000

/*
 * BR3 and OR3 (External Bus CS3)
 * Base address = 0xF030_0000 - 0xF030_0FFF
 *
 */
#define CONFIG_SYS_OR3_PRELIM		0xFFF00000
#define CONFIG_SYS_BR3_PRELIM		0xF0300000

/*
 * BR4 and OR4 (External Bus CS3)
 * Base address = 0xF040_0000 - 0xF040_0FFF
 *
 */
#define CONFIG_SYS_OR4_PRELIM		0xFFF00000
#define CONFIG_SYS_BR4_PRELIM		0xF0400000


/*
 * BR4 and OR4 (External Bus CS3)
 * Base address = 0xF050_0000 - 0xF050_0FFF
 *
 */
#define CONFIG_SYS_OR5_PRELIM		0xFFF00000
#define CONFIG_SYS_BR5_PRELIM		0xF0500000

/*
 * BR6 and OR6 (Unused)
 * Base address = 0xF060_0000 - 0xF060_0FFF
 *
 */
#define CONFIG_SYS_OR6_PRELIM		0xFFF00000
#define CONFIG_SYS_BR6_PRELIM		0xF0600000

/*
 * BR7 and OR7 (Unused)
 * Base address = 0xF070_0000 - 0xF070_0FFF
 *
 */
#define CONFIG_SYS_OR7_PRELIM		0xFFF00000
#define CONFIG_SYS_BR7_PRELIM		0xF0700000

/*
 * Sanity checks
 */
#if defined(CONFIG_SCC1_ENET) && defined(CONFIG_FEC_ENET)
#error Both CONFIG_SCC1_ENET and CONFIG_FEC_ENET configured
#endif

#endif /* __CONFIG_H */
