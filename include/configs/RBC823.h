/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Modified by Udi Finkelstein udif@udif.com
 * For the RBC823 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC823		1	/* This is a MPC823 CPU		*/
#define CONFIG_RBC823		1	/* ...on a RBC823 module	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

#if 0
#define DEBUG			1
#define CONFIG_LAST_STAGE_INIT
#endif
#define CONFIG_KEYBOARD		1	/* This board has a custom keybpard */
#define CONFIG_LCD		1	/* use LCD controller ...	*/
#define CONFIG_MPC8XX_LCD
#define CONFIG_HITACHI_SP19X001_Z1A	/* The LCD type we use */

#define	CONFIG_8xx_CONS_SMC2	1	/* Console is on SMC2		*/
#undef	CONFIG_8xx_CONS_SMC1
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/
#if 1
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */
#define CONFIG_8xx_GCLK_FREQ    48000000L

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#undef CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#undef	CONFIG_RTC_MPC8xx		/* don't use internal RTC of MPC8xx (no battery)	*/

#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED 40000
#define CONFIG_SYS_I2C_SLAVE 0xfe
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_WRITE_BITS		4
#define CONFIG_SYS_EEPROM_WRITE_DELAY_MS	10

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_BMP
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CDP
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_KGDB
#define CONFIG_CMD_PING
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_SDRAM

#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_XIMG

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x0100000	/* default load address	*/

/*
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
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFF00000
#if defined(DEBUG)
#define	CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	67	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_OFFSET		0x10000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
/*
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
*/
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWRI | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC00 | SIUMCR_FRC)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 *
 */

/*
 * for 48 MHz, we use a 4 MHz clock * 12
 */
#define CONFIG_SYS_PLPRCR							\
		( (12-1)<<PLPRCR_MF_SHIFT | PLPRCR_TEXPS | PLPRCR_LOLRE )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_RTDIV   | SCCR_RTSEL    | SCCR_CRQEN    | \
			 SCCR_PRQEN   | SCCR_EBDF00   | \
			 SCCR_COM01   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD001 | \
			 SCCR_DFALCD00)

#ifdef NOT_USED
/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define	CONFIG_IDE_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_PCMCIA		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0100

#endif

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CONFIG_SYS_DER	0x2002000F*/
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFFF00000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x04000000	/* D.O.C Millenium	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_PRELIM_OR_AM	0xFFF80000	/* OR addr mask */

/* FLASH timing: ACS = 00, TRLX = 0, CSNT = 1, SCY = 7, EHTR = 1	*/
#define CONFIG_SYS_OR_TIMING_FLASH  (OR_ACS_DIV1 | OR_BI | OR_SCY_7_CLK | OR_EHTR)

#define CONFIG_SYS_OR_TIMING_MSYS   (OR_ACS_DIV1 | OR_BI)

#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V)

#define CONFIG_SYS_OR1_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_MSYS)
#define CONFIG_SYS_BR1_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_MS_UPMB | \
			  BR_PS_8 | BR_V)

/*
 * BR4 and OR4 (SDRAM)
 *
 */
#define SDRAM_BASE4_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/*
 * SDRAM timing:
 */
#define CONFIG_SYS_OR_TIMING_SDRAM	(OR_CSNT_SAM)

#define CONFIG_SYS_OR4_PRELIM	(~(SDRAM_MAX_SIZE-1) | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR4_PRELIM	((SDRAM_BASE4_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	187		/* start with divider for 48 MHz	*/

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CONFIG_SYS_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CONFIG_SYS_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CONFIG_SYS_MAMR_8COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)
/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */
