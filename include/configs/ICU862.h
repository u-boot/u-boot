/*
 * (C) Copyright 2001-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <mpc8xx_irq.h>

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC860		1
#define CONFIG_MPC860T		1
#define CONFIG_ICU862		1
#define CONFIG_MPC862		1

#define	CONFIG_SYS_TEXT_BASE	0x40F00000

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#ifdef CONFIG_100MHz
#define MPC8XX_FACT		24		/* Multiply by 24	*/
#define MPC8XX_XIN		4165000		/* 4.165 MHz in		*/
#define CONFIG_8xx_GCLK_FREQ	(MPC8XX_FACT * MPC8XX_XIN)
				    /* define if cant' use get_gclk_freq */
#else
#if 1				/* for 50MHz version of processor	*/
#define MPC8XX_FACT		12		/* Multiply by 12	*/
#define MPC8XX_XIN		4000000		/* 4 MHz in		*/
#define CONFIG_8xx_GCLK_FREQ	48000000 /* define if cant use get_gclk_freq */
#else				/* for 80MHz version of processor	*/
#define MPC8XX_FACT		20		/* Multiply by 20	*/
#define MPC8XX_XIN		4000000		/* 4 MHz in		*/
#define CONFIG_8xx_GCLK_FREQ    80000000 /* define if cant use get_gclk_freq */
#endif
#endif

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp;"								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm"

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#undef	CONFIG_SCC1_ENET		/* disable SCC1 ethernet */
#define	CONFIG_FEC_ENET		1	/* use FEC ethernet  */
#define	CONFIG_MII		1
#if 1
#define CONFIG_SYS_DISCOVER_PHY	1
#else
#undef	CONFIG_SYS_DISCOVER_PHY
#endif

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* enable I2C and select the hardware/software driver */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
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
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_EEPROM_X40430		/* Use a Xicor X40430 EEPROM	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS  4	/* 16 bytes page write mode	*/

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP


/*
 * Miscellaneous configurable options
 */
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

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0400000	/* 1 ... 4 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xF0000000
#define CONFIG_SYS_IMMR_SIZE		((uint)(64 * 1024))

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
#define CONFIG_SYS_FLASH_SIZE		((uint)(16 * 1024 * 1024))	/* max 16Mbyte */

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100

#if 0
#if defined(DEBUG)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#else
#define CONFIG_SYS_MONITOR_LEN (256 << 10) /* Reserve 256 kB for Monitor */
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc()	*/

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
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms)	*/


#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x00F40000

#define CONFIG_ENV_SECT_SIZE	0x40000	/* Total Size of Environment sector	*/
#define	CONFIG_ENV_SIZE		0x4000	/* Used Size of Environment Sector	*/
#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control					11-9
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
 * SIUMCR - SIU Module Configuration					11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CONFIG_SYS_PLPRCR	(((MPC8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#ifdef CONFIG_100MHz	/* for 100 MHz, external bus is half CPU clock */
#define SCCR_MASK	0
#define CONFIG_SYS_SCCR	(SCCR_TBS	| SCCR_COM00	| SCCR_DFSYNC00	| \
			 SCCR_DFBRG00	| SCCR_DFNL000	| SCCR_DFNH000	| \
			 SCCR_DFLCD000	|SCCR_DFALCD00	| SCCR_EBDF01)
#else			/* up to 50 MHz we use a 1:1 clock */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_TBS	| SCCR_COM00	| SCCR_DFSYNC00	| \
			 SCCR_DFBRG00	| SCCR_DFNL000	| SCCR_DFNH000	| \
			 SCCR_DFLCD000	|SCCR_DFALCD00	)
#endif	/* CONFIG_100MHz */

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register		19-4
 *-----------------------------------------------------------------------
 */
/* +0x09C4 => DRQP = 10 (IDMA requests have lowest priority) */
#define CONFIG_SYS_RCCR 0x0020

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
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
 * PCMCIA Power Switch
 *
 * The ICU862 uses a TPS2205 PC-Card Power-Interface Switch to
 * control the voltages on the PCMCIA slot which is connected to Port B
 *-----------------------------------------------------------------------
 */
			/* Output pins */
#define TPS2205_VCC5	0x00008000	/* PB.16:  5V Voltage Control	*/
#define TPS2205_VCC3	0x00004000	/* PB.17:  3V Voltage Control	*/
#define TPS2205_VPP_PGM	0x00002000	/* PB.18: PGM Voltage Control	*/
#define TPS2205_VPP_VCC	0x00001000	/* PB.19: VPP Voltage Control	*/
#define TPS2205_SHDN	0x00000200	/* PB.22: Shutdown		*/
#define TPS2205_OUTPUTS ( TPS2205_VCC5    | TPS2205_VCC3    | \
			  TPS2205_VPP_PGM | TPS2205_VPP_VCC | \
			  TPS2205_SHDN)

			/* Input pins */
#define TPS2205_OC	0x00000100	/* PB.23: Over-Current		*/
#define TPS2205_INPUTS	( TPS2205_OC )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

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


 /*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_DER		0

/* Because of the way the 860 starts up and assigns CS0 the
* entire address space, we have to set the memory controller
* differently.  Normally, you write the option register
* first, and then enable the chip select by writing the
* base register.  For CS0, you must write the base register
* first, followed by the option register.
*/

/*
 * Init Memory Controller:
 *
 * BR0 and OR0 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x0		/* FLASH bank #1	*/

#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)

#define CONFIG_SYS_OR0_PRELIM	0xFF000954		/* Real values for the board */
#define CONFIG_SYS_BR0_PRELIM	0x40000001		/* Real values for the board */

/*
 * BR1 and OR1 (SDRAM)
 */
#define SDRAM_BASE1_PRELIM	0x00000000	/* SDRAM bank		*/
#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000800	/* BIH is not set	*/

#define CONFIG_SYS_OR1_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_SDRAM)
#define CONFIG_SYS_BR1_PRELIM	((SDRAM_BASE1_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V)

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA		97	/* start with divider for 100 MHz	*/

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

#define CONFIG_SYS_MAMR		0x13a01114

#ifdef CONFIG_MPC860T

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT	SIU_LEVEL1	/* FEC interrupt */

#endif /* CONFIG_MPC860T */


#endif	/* __CONFIG_H */
