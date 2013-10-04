/*
  * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 * Derived from ../tqm8xx/tqm8xx.c
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC855		1	/* This is a MPC855 CPU		*/
#define CONFIG_KUP4K		1	/* ...on a KUP4K module */

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200	/* console baudrate		*/
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second	*/

#define CONFIG_BOARD_TYPES	1	/* support board types		*/

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS						\
"slot_a_boot=setenv bootargs root=/dev/sda2 ip=off;"				\
 "run addhw; mw.b 400000 00 80; diskboot 400000 0:1; bootm 400000\0"		\
"slot_b_boot=setenv bootargs root=/dev/sda2 ip=off;"				\
 "run addhw; mw.b 400000 00 80; diskboot 400000 2:1; bootm 400000\0"		\
"nfs_boot=mw.b 400000 00 80; dhcp; run nfsargs addip addhw; bootm 400000\0"	\
"fat_boot=mw.b 400000 00 80; fatload ide 2:1 400000 st.bin; run addhw;		\
 bootm 400000 \0"								\
"panic_boot=echo No Bootdevice !!! reset\0"					\
"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${rootpath}\0"		\
"ramargs=setenv bootargs root=/dev/ram rw\0"					\
"addip=setenv bootargs ${bootargs} ip=${ipaddr}::${gatewayip}"			\
 ":${netmask}:${hostname}:${netdev}:off\0"					\
"addhw=setenv bootargs ${bootargs} ${mtdparts} console=${console} ${debug}	\
 hw=${hw} key1=${key1} panic=1 mem=${mem}\0"					\
"console=ttyCPM0,115200\0"							\
"netdev=eth0\0"									\
"contrast=20\0"									\
"silent=1\0"									\
"mtdparts=" MTDPARTS_DEFAULT "\0"						\
"load=tftp 200000 bootloader-4k.bitmap;tftp 100000 bootloader-4k.bin\0"		\
"update=protect off 1:0-9;era 1:0-9;cp.b 100000 40000000 ${filesize};"		\
 "cp.b 200000 40050000 14000\0"

#define CONFIG_BOOTCOMMAND  \
    "run fat_boot;run slot_b_boot;run slot_a_boot;run nfs_boot;run panic_boot"

#define CONFIG_PREBOOT	"setenv preboot; saveenv"

#define CONFIG_MISC_INIT_R	1
#define CONFIG_MISC_INIT_F	1

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE	/* don't allow baudrate change	*/

#define	CONFIG_WATCHDOG	1		/* watchdog enabled		*/

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

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * enable I2C and select the hardware/software driver
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	93000	/* 93 kHz is supposed to work */
#define CONFIG_SYS_I2C_SOFT_SLAVE	0xFE

/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		0x00000020	/* PB 26 */
#define PB_SDA		0x00000010	/* PB 27 */

#define I2C_INIT	(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE	(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE	(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ	((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY	udelay(2)	/* 1/4 I2C clock duration */

/*-----------------------------------------------------------------------
 * I2C Configuration
 */

#define CONFIG_SYS_I2C_PICIO_ADDR	0x21	/* PCF8574 IO Expander */
#define CONFIG_SYS_I2C_RTC_ADDR		0x51	/* PCF8563 RTC */

/* List of I2C addresses to be verified by POST */

#define CONFIG_SYS_POST_I2C_ADDRS	{CONFIG_SYS_I2C_PICIO_ADDR,	\
					 CONFIG_SYS_I2C_RTC_ADDR,	\
					}

#define CONFIG_RTC_PCF8563		/* use Philips PCF8563 RTC	*/

#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_MII

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MII
#define CONFIG_CMD_NFS
#define CONFIG_CMD_FAT
#define CONFIG_CMD_SNTP

#ifdef CONFIG_POST
    #define CONFIG_CMD_DIAG
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size	*/
#endif
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x000400000	/* memtest works on     */
#define CONFIG_SYS_MEMTEST_END		0x005C00000	/* 4 ... 92 MB in DRAM  */
#define CONFIG_SYS_ALT_MEMTEST 1
#define CONFIG_SYS_MEMTEST_SCRATCH	0x90000200	/* using latch as scratch register */

#define CONFIG_SYS_LOAD_ADDR		0x400000	/* default load address */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 115200 }

#define CONFIG_SYS_CONSOLE_INFO_QUIET 1

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x40000000
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	19	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x40000 /*   Offset   of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/
#define CONFIG_ENV_SECT_SIZE	0x10000

/*-----------------------------------------------------------------------
 * Dynamic MTD partition support
 */
#define MTDPARTS_DEFAULT	"mtdparts=40000000.flash:256k(u-boot),"	\
						"64k(env),"		\
						"128k(splash),"		\
						"512k(etc),"		\
						"64k(hw-info)"

/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#define CONFIG_SYS_HWINFO_OFFSET	0x000F0000	/* offset of HW Info block */
#define CONFIG_SYS_HWINFO_SIZE		0x00000100	/* size   of HW Info block */
#define CONFIG_SYS_HWINFO_MAGIC	0x4B26500D		/* 'K&P<CR>' */

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
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC00)

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
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
#define CONFIG_SYS_PLPRCR ( (5-1)<<PLPRCR_MF_SHIFT | PLPRCR_TEXPS | PLPRCR_TMIST )

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
#define CONFIG_SYS_SCCR	(SCCR_TBS | SCCR_EBDF01 |  \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */

/* KUP4K use both slots, SLOT_A as "primary". */
#define CONFIG_PCMCIA_SLOT_A 1

#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

#define PCMCIA_SOCKETS_NO 2
#define PCMCIA_MEM_WIN_NO 8
/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define CONFIG_IDE_PREINIT	1	/* Use preinit IDE hook */
#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#define CONFIG_IDE_LED		1	/* LED	 for ide supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	4

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_IDE1_OFFSET	(4 * CONFIG_SYS_PCMCIA_MEM_SIZE)

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(2 * CONFIG_SYS_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0100

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_DER 0

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
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_ACS_DIV2  | OR_CSNT_SAM | \
				 OR_SCY_5_CLK | OR_EHTR | OR_BI)

#define CONFIG_SYS_OR0_REMAP \
	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM \
	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM \
	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )


/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000A00

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
 * 4096	 Rows from SDRAM example configuration
 * 1000	 factor s -> ms
 *   32	 PTP (pre-divider from MPTPR) from SDRAM example configuration
 *    4	 Number of refresh cycles per period
 *   64	 Refresh cycle in ms per number of rows
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
#define CONFIG_SYS_MPTPR 0x400

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CONFIG_SYS_MAMR_8COL 0x68802114
/* 9 column SDRAM */
#define CONFIG_SYS_MAMR_9COL 0x68904114

/*
 * Chip Selects
 */
#define CONFIG_SYS_OR0
#define CONFIG_SYS_BR0

#define CONFIG_SYS_OR1_8COL 0xFF000A00
#define CONFIG_SYS_BR1_8COL 0x00000081
#define CONFIG_SYS_OR2_8COL 0xFE000A00
#define CONFIG_SYS_BR2_8COL 0x01000081
#define CONFIG_SYS_OR3_8COL 0xFC000A00
#define CONFIG_SYS_BR3_8COL 0x02000081

#define CONFIG_SYS_OR1_9COL 0xFE000A00
#define CONFIG_SYS_BR1_9COL 0x00000081
#define CONFIG_SYS_OR2_9COL 0xFE000A00
#define CONFIG_SYS_BR2_9COL 0x02000081
#define CONFIG_SYS_OR3_9COL 0xFE000A00
#define CONFIG_SYS_BR3_9COL 0x04000081

#define CONFIG_SYS_OR4 0xFFFF8926
#define CONFIG_SYS_BR4 0x90000401

#define CONFIG_SYS_OR5 0xFFC007F0  /* EPSON: 4 MB  17 WS or externel TA */
#define CONFIG_SYS_BR5 0x80080801  /* Start at 0x80080000 */

#define LATCH_ADDR 0x90000200

#define CONFIG_AUTOBOOT_KEYED		/* use key strings to stop autoboot */
#define CONFIG_AUTOBOOT_STOP_STR	"."
#define CONFIG_SILENT_CONSOLE		1
#define CONFIG_SYS_DEVICE_NULLDEV	1       /* enble null device		*/
#define CONFIG_VERSION_VARIABLE		1

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif	/* __CONFIG_H */
