/*
  * A collection of structures, addresses, and values associated with
  * the Motorola 860 ADS board.	 Copied from the MBX stuff.
  * Magnus Damm added defines for 8xxrom and extended bd_info.
  * Helmut Buchsbaum added bitvalues for BCSRx
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  */

/* ------------------------------------------------------------------------- */

#ifndef _CONFIG_ADS860_H
#define _CONFIG_ADS860_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <mpc8xx_irq.h>

#define CONFIG_MPC860		1
#define CONFIG_ADS		1

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1	    */
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		19200	/* console baudrate */
#define CONFIG_PCMCIA		1	/* To enable PCMCIA support */

#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address defaults */
#define CFG_I2C_SLAVE		0x7F

#define MPC8XX_XIN		32768	/* 32.768 kHz input frequency	*/
#define MPC8XX_FACT		0x5F6	/* Multiply by 1526 */
					/* MPC8XX_FACT * MPC8XX_XIN = 50 MHz */

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled	    */
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#endif

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND			    \
    "bootp; "				    \
    "setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):$(rootpath) "	    \
    "ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname)::off; "   \
    "bootm"

#define CONFIG_LOADS_ECHO	1   /* echo on for serial download  */
#undef	CFG_LOADS_BAUD_CHANGE	    /* don't allow baudrate change  */

#undef	CONFIG_WATCHDOG		    /* watchdog disabled	*/

#define CONFIG_BOOTP_MASK   (CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)


#if 0					/* private command defs */
#define CONFIG_COMMANDS	    (CONFIG_CMD_DFL | CFG_CMD_I2C | \
			     CFG_CMD_IDE | CFG_CMD_PCMCIA)
#endif
					/* default command defs */
#define CONFIG_COMMANDS (CONFIG_CMD_DFL & ~CFG_CMD_CACHE)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


/*
 * Miscellaneous configurable options
 */
#undef	CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT	    "=>"	/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	    1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE	    256		/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE	    (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	    16		/* max number of command args	*/
#define CFG_BARGSIZE	    CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START   0x00100000	/* memtest works on */
#define CFG_MEMTEST_END	    0x00F00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR	    0x00100000

#define CFG_HZ		    1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE  { 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xfff00000
#define CFG_IMMR_SIZE		((uint)(64 * 1024))

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE	    0x00000000
#define CFG_SRAM_BASE	    0x00000000
#define CFG_FLASH_BASE	    0xfe000000
#define CFG_FLASH_SIZE	    ((uint)(8 * 1024 * 1024))	/* max 8Mbyte */

#define CFG_MONITOR_LEN	    (384 << 10) /* Reserve 384 kB for Monitor	*/
#define CFG_MONITOR_BASE    CFG_FLASH_BASE
#define CFG_MALLOC_LEN	    (384 << 10) /* Reserve 384 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	    (8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks	    */
#define CFG_MAX_FLASH_SECT	8	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)  */

#undef	CFG_ENV_IS_IN_NVRAM
#undef	CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_IS_IN_FLASH	1

#define CFG_ENV_OFFSET		0x00040000
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	0x40000 /* see README - env sector total size	*/

/* the other CS:s are determined by looking at parameters in BCSRx */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE  16		/* For all MPC8xx CPUs		*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT 4		/* log base 2 of the above value    */
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control			11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR   (SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
	     SYPCR_SWE | SYPCR_SWRI | SYPCR_SWP)
#else
#define CFG_SYPCR   (SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SUMCR - SIU Module Configuration		    11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR  (SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control			11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR   (TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control	11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR   (PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CFG_PLPRCR  (((MPC8XX_FACT-1) << 20) |	\
		PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register	15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK   SCCR_EBDF11
#define CFG_SCCR    (SCCR_TBS | SCCR_COM00 | SCCR_DFSYNC00 | \
		   SCCR_DFBRG00 | SCCR_DFNL000 | SCCR_DFNH000 | \
		   SCCR_DFLCD000 | SCCR_DFALCD00)


 /*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER	    0

/* Because of the way the 860 starts up and assigns CS0 the
* entire address space, we have to set the memory controller
* differently.	Normally, you write the option register
* first, and then enable the chip select by writing the
* base register.  For CS0, you must write the base register
* first, followed by the option register.
*/

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */
/* the other CS:s are determined by looking at parameters in BCSRx */

#define BCSR_ADDR	   ((uint) 0xff010000)
#define BCSR_SIZE	   ((uint)(64 * 1024))

#define FLASH_BASE0_PRELIM  0xfe000000	/* FLASH bank #0    */
#define FLASH_BASE1_PRELIM  0x00000000	/* FLASH bank #1    */

#define CFG_REMAP_OR_AM	    0xff000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM    0xffe00000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0    */
#define CFG_OR_TIMING_FLASH (OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)

#ifdef USE_REAL_FLASH_VALUES
/*
 * These values fit our FADS860T ...
 * The "default" behaviour with 1Mbyte initial doesn't work for us!
 */
#define CFG_BR0_PRELIM	0x0fe000001	/* Real values for the board */
#define CFG_OR0_PRELIM	0x0ffe00d34
#define CFG_BR2_PRELIM	0x000000081
#define CFG_OR2_PRELIM	0x0ff000800
#else
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)   /* 1 Mbyte until detected and only 1 Mbyte is needed*/
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )
#endif

/* BCSRx - Board Control and Status Registers */
/* #define CFG_OR1_REMAP    CFG_OR0_REMAP */
#define CFG_OR1_PRELIM	0xffff8110	/* 64Kbyte address space */
#define CFG_BR1_PRELIM	((BCSR_ADDR) | BR_V )

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	    97	/* start with divider for 100 MHz   */

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit */
#define CFG_MPTPR_2BK_4K    MPTPR_PTP_DIV16	/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_4K    MPTPR_PTP_DIV32	/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit	    */
#define CFG_MPTPR_2BK_8K    MPTPR_PTP_DIV8	/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_8K    MPTPR_PTP_DIV16	/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CFG_MAMR_8COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
	     MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |   \
	     MAMR_RLFA_1X    | MAMR_WLFA_1X    | MAMR_TLFA_4X)
/* 9 column SDRAM */
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
	     MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |   \
	     MAMR_RLFA_1X    | MAMR_WLFA_1X    | MAMR_TLFA_4X)

#define CFG_MAMR	0x13a01114
/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	    /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02	    /* Software reboot		*/


/* values according to the manual */
#define BCSR0	((uint) (BCSR_ADDR + 00))
#define BCSR1	((uint) (BCSR_ADDR + 0x04))
#define BCSR2	((uint) (BCSR_ADDR + 0x08))
#define BCSR3	((uint) (BCSR_ADDR + 0x0c))
#define BCSR4	((uint) (BCSR_ADDR + 0x10))


/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CFG_PCMCIA_MEM_ADDR	(0xE0000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0xE4000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0xEC000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )


/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_IDE_8xx_DIRECT	1   /* PCMCIA interface required    */
#undef	CONFIG_IDE_LED		    /* LED   for ide supported	*/
#define CONFIG_IDE_RESET	1   /* reset for ide supported	*/

#define CFG_IDE_MAXBUS		1   /* max. 2 IDE busses	*/
#define CFG_IDE_MAXDEVICE	(CFG_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CFG_PIO_MODE		0   /* IDE interface in PIO Mode 0  */
#define CFG_PC_IDE_RESET	((ushort)0x0008)    /* PC 12	*/

/* #define CFG_ATA_BASE_ADDR	0xFE100000 */
#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_MEM_ADDR
#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O		*/
#define CFG_ATA_REG_OFFSET	0x0080	/* Offset for normal register accesses	*/
#define CFG_ATA_ALT_OFFSET	0x0100	/* Offset for alternate registers   */


/* (F)ADS bitvalues by Helmut Buchsbaum
 * see MPC8xxADS User's Manual for a proper description
 * of the following structures
 */

#define BCSR0_ERB	((uint)0x80000000)
#define BCSR0_IP	((uint)0x40000000)
#define BCSR0_BDIS	((uint)0x10000000)
#define BCSR0_BPS_MASK	((uint)0x0C000000)
#define BCSR0_ISB_MASK	((uint)0x01800000)
#define BCSR0_DBGC_MASK ((uint)0x00600000)
#define BCSR0_DBPC_MASK ((uint)0x00180000)
#define BCSR0_EBDF_MASK ((uint)0x00060000)

#define BCSR1_FLASH_EN		 ((uint)0x80000000)
#define BCSR1_DRAM_EN		 ((uint)0x40000000)
#define BCSR1_ETHEN		 ((uint)0x20000000)
#define BCSR1_IRDEN		 ((uint)0x10000000)
#define BCSR1_FLASH_CFG_EN	 ((uint)0x08000000)
#define BCSR1_CNT_REG_EN_PROTECT ((uint)0x04000000)
#define BCSR1_BCSR_EN		 ((uint)0x02000000)
#define BCSR1_RS232EN_1		 ((uint)0x01000000)
#define BCSR1_PCCEN		 ((uint)0x00800000)
#define BCSR1_PCCVCC0		 ((uint)0x00400000)
#define BCSR1_PCCVCCON		    BCSR1_PCCVCC0
#define BCSR1_PCCVPP_MASK	 ((uint)0x00300000)
#define BCSR1_DRAM_HALF_WORD	 ((uint)0x00080000)
#define BCSR1_RS232EN_2		 ((uint)0x00040000)
#define BCSR1_SDRAM_EN		 ((uint)0x00020000)
#define BCSR1_PCCVCC1		 ((uint)0x00010000)

#define BCSR2_FLASH_PD_MASK	 ((uint)0xF0000000)
#define BCSR2_DRAM_PD_MASK	 ((uint)0x07800000)
#define BCSR2_DRAM_PD_SHIFT	 (23)
#define BCSR2_EXTTOLI_MASK	 ((uint)0x00780000)
#define BCSR2_DBREVNR_MASK	 ((uint)0x00030000)

#define BCSR3_DBID_MASK		 ((ushort)0x3800)
#define BCSR3_CNT_REG_EN_PROTECT ((ushort)0x0400)
#define BCSR3_BREVNR0		 ((ushort)0x0080)
#define BCSR3_FLASH_PD_MASK	 ((ushort)0x0070)
#define BCSR3_BREVN1		 ((ushort)0x0008)
#define BCSR3_BREVN2_MASK	 ((ushort)0x0003)

#define BCSR4_ETHLOOP		 ((uint)0x80000000)
#define BCSR4_TFPLDL		 ((uint)0x40000000)
#define BCSR4_TPSQEL		 ((uint)0x20000000)
#define BCSR4_SIGNAL_LAMP	 ((uint)0x10000000)
#ifdef CONFIG_MPC823
#define BCSR4_USB_EN		 ((uint)0x08000000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860SAR
#define BCSR4_UTOPIA_EN		 ((uint)0x08000000)
#endif /* CONFIG_MPC860SAR */
#ifdef CONFIG_MPC860T
#define BCSR4_FETH_EN		 ((uint)0x08000000)
#endif /* CONFIG_MPC860T */
#ifdef CONFIG_MPC823
#define BCSR4_USB_SPEED		 ((uint)0x04000000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860T
#define BCSR4_FETHCFG0		 ((uint)0x04000000)
#endif /* CONFIG_MPC860T */
#ifdef CONFIG_MPC823
#define BCSR4_VCCO		 ((uint)0x02000000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860T
#define BCSR4_FETHFDE		 ((uint)0x02000000)
#endif /* CONFIG_MPC860T */
#ifdef CONFIG_MPC823
#define BCSR4_VIDEO_ON		 ((uint)0x00800000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC823
#define BCSR4_VDO_EKT_CLK_EN	 ((uint)0x00400000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860T
#define BCSR4_FETHCFG1		 ((uint)0x00400000)
#endif /* CONFIG_MPC860T */
#ifdef CONFIG_MPC823
#define BCSR4_VIDEO_RST		 ((uint)0x00200000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860T
#define BCSR4_FETHRST		 ((uint)0x00200000)
#endif /* CONFIG_MPC860T */
#ifdef CONFIG_MPC823
#define BCSR4_MODEM_EN		 ((uint)0x00100000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC823
#define BCSR4_DATA_VOICE	 ((uint)0x00080000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC850
#define BCSR4_DATA_VOICE	 ((uint)0x00080000)
#endif /* CONFIG_MPC850 */

#define CONFIG_DRAM_50MHZ	1
#define CONFIG_SDRAM_50MHZ

#ifdef CONFIG_MPC860T
/* Interrupt level assignments.
 */
#define FEC_INTERRUPT	SIU_LEVEL1  /* FEC interrupt */
#endif /* CONFIG_MPC860T */

/* We don't use the 8259.
 */
#define NR_8259_INTS	0

/* Machine type
 */
#define _MACH_8xx (_MACH_ads)

#if 0
#define CONFIG_DISK_SPINUP_TIME 1000000
#endif
#undef CONFIG_DISK_SPINUP_TIME	/* usin´ Compact Flash */


/* PCMCIA configuration
 */
#define PCMCIA_MAX_SLOTS    2
#ifdef CONFIG_MPC860
#define PCMCIA_SLOT_A 1
#endif

#endif	/* _CONFIG_ADS860_H */
