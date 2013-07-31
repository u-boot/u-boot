/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#define CONFIG_R360MPI		1

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#define CONFIG_LCD
#define CONFIG_MPC8XX_LCD
#undef  CONFIG_EDT32F10
#define CONFIG_SHARP_LQ057Q3DC02

#define	CONFIG_SPLASH_SCREEN

#define MPC8XX_FACT             1		/* Multiply by 1	*/
#define MPC8XX_XIN              50000000	/* 50 MHz in		*/
#define CONFIG_8xx_GCLK_FREQ    50000000 /* define if can't use get_gclk_freq */

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200	/* console baudrate in bps	*/
#if 0
#define CONFIG_BOOTDELAY	0	/* immediate boot		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

#undef	CONFIG_SCC1_ENET
#define	CONFIG_SCC2_ENET

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#define	CONFIG_MISC_INIT_R		/* have misc_init_r() function	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_CAN_DRIVER		/* CAN Driver support enabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define CONFIG_HARD_I2C		1	/* To I2C with hardware support */
#undef CONFIG_SYS_I2C_SOFT		/* To I2C with software support */
#define CONFIG_SYS_I2C_SPEED		4700	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F

#if defined(CONFIG_SYS_I2C_SOFT)
#define CONFIG_SYS_SYS_I2C_SOFT_SPEED	4700 /* I2C speed and slave address */
#define CONFIG_SYS_SYS_I2C_SOFT_SLAVE	0x7F
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL			0x00000020	/* PB 26 */
#define PB_SDA			0x00000010	/* PB 27 */

#define I2C_INIT		(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE		(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE		(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ		((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)		if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
				else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)		if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
				else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY		udelay(50)
#endif /* #define(CONFIG_SYS_I2C_SOFT) */

#define CONFIG_SYS_I2C_LCD_ADDR	0x8	/* LCD Control */
#define CONFIG_SYS_I2C_KEY_ADDR	0x9	/* Keyboard coprocessor */
#define CONFIG_SYS_I2C_TEM_ADDR	0x49	/* Temperature Sensors */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BMP
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCMCIA
#define CONFIG_CMD_SNTP


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_DEVICE_NULLDEV	1	/* we need the null device	*/
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1	/* must set console from env	*/

#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
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

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * JFFS2 partitions
 */
/* No command line, one static partition
 * use all the space starting at offset 3MB*/
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00300000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=r360-0"
#define MTDPARTS_DEFAULT	"mtdparts=r360-0:-@3m(user)"
*/

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
#define CONFIG_SYS_FLASH_BASE		0x40000000
#if defined(DEBUG)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_OFFSET		0x40000	/* Offset of Environment		*/
#define	CONFIG_ENV_SECT_SIZE	0x20000	/* Total Size of Environment sector	*/
#define	CONFIG_ENV_SIZE		0x4000	/* Used Size of Environment sector	*/
#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

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
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

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
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
#ifdef	CONFIG_80MHz	/* for 80 MHz, we use a 16 MHz clock * 5 */
#define CONFIG_SYS_PLPRCR							\
		( (5-1)<<PLPRCR_MF_SHIFT | PLPRCR_TEXPS | PLPRCR_TMIST )
#else			/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#endif	/* CONFIG_80MHz */

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

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

#if 1
#define CONFIG_IDE_PREINIT	1	/* Use preinit IDE hook */
#define	CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
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
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFF000000	/* OR addr mask */

/*
 * FLASH timing:
 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_ACS_DIV1 | OR_SCY_7_CLK | OR_BI)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )


/*
 * BR2 and OR2 (SDRAM)
 *
 */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

#define CONFIG_SYS_PRELIM_OR2_AM	0xF8000000	/* OR addr mask */

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	(OR_ACS_DIV1  | OR_CSNT_SAM | \
				 OR_SCY_0_CLK | OR_G5LS)

#define CONFIG_SYS_OR2_PRELIM	(CONFIG_SYS_PRELIM_OR2_AM | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR2_PRELIM	((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * BR3 and OR3 (CAN Controller)
 */
#ifdef CONFIG_CAN_DRIVER
#define CONFIG_SYS_CAN_BASE		0xC0000000	/* CAN base address   */
#define CONFIG_SYS_CAN_OR_AM		0xFFFF8000	/* 32 kB address mask */
#define CONFIG_SYS_OR3_CAN		(CONFIG_SYS_CAN_OR_AM | OR_G5LA |OR_BI)
#define CONFIG_SYS_BR3_CAN		((CONFIG_SYS_CAN_BASE & BR_BA_MSK) | \
				 BR_PS_8 | BR_MS_UPMB | BR_V)
#endif	/* CONFIG_CAN_DRIVER */


/*
 * Memory Periodic Timer Prescaler
 *
 * The Divider for PTA (refresh timer) configuration is based on an
 * example SDRAM configuration (64 MBit, one bank). The adjustment to
 * the number of chip selects (NCS) and the actually needed refresh
 * rate is done by setting MPTPR.
 *
 * PTA is calculated from
 *	PTA = (gclk * Trefresh) / ((2 ^ (2 * DFBRG)) * PTP * NCS)
 *
 *	gclk	  CPU clock (not bus clock!)
 *	Trefresh  Refresh cycle * 4 (four word bursts used)
 *
 * 4096  Rows from SDRAM example configuration
 * 1000  factor s -> ms
 *   32  PTP (pre-divider from MPTPR) from SDRAM example configuration
 *    4  Number of refresh cycles per period
 *   64  Refresh cycle in ms per number of rows
 * --------------------------------------------
 * Divider = 4096 * 32 * 1000 / (4 * 64) = 512000
 *
 * 50 MHz => 50.000.000 / Divider =  98
 * 66 Mhz => 66.000.000 / Divider = 129
 * 80 Mhz => 80.000.000 / Divider = 156
 */
#if   defined(CONFIG_80MHz)
#define CONFIG_SYS_MAMR_PTA		156
#elif defined(CONFIG_66MHz)
#define CONFIG_SYS_MAMR_PTA		129
#else		/*   50 MHz */
#define CONFIG_SYS_MAMR_PTA		 98
#endif	/*CONFIG_??MHz */

/*
 * For 16 MBit, refresh rates could be 31.3 us
 * (= 64 ms / 2K = 125 / quad bursts).
 * For a simpler initialization, 15.6 us is used instead.
 *
 * #define CONFIG_SYS_MPTPR_2BK_2K	MPTPR_PTP_DIV32		for 2 banks
 * #define CONFIG_SYS_MPTPR_1BK_2K	MPTPR_PTP_DIV64		for 1 bank
 */
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

#endif	/* __CONFIG_H */
