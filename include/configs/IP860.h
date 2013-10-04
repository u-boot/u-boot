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

#define CONFIG_MPC860		1	/* This is a MPC860 CPU		*/
#define CONFIG_IP860		1	/* ...on a IP860 board		*/

#define	CONFIG_SYS_TEXT_BASE	0x10000000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_RESET_PHY_R	1	/* Call reset_phy()		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo" \
"\0load=tftp \"/tftpboot/u-boot.bin\"\0update=protect off 1:0;era 1:0;cp.b 100000 10000000 ${filesize}\0"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/


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

# define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM X24C16		*/
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* takes up to 10 msec */

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH

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

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00F00000	/* 1 ... 15MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x00100000	/* default load address	*/

#define	CONFIG_SYS_PIO_MODE		0	/* IDE interface in PIO Mode 0	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xF1000000	/* Non-standard value!!	*/

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
#define CONFIG_SYS_FLASH_BASE		0x10000000
#ifdef	DEBUG
#define	CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 512 kB for Monitor	*/
#else
#if 0 /* need more space for I2C tests */
#define	CONFIG_SYS_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)
#endif
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
#define CONFIG_SYS_MAX_FLASH_SECT	124	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#undef	CONFIG_ENV_IS_IN_FLASH
#undef	CONFIG_ENV_IS_IN_NVRAM
#undef  CONFIG_ENV_IS_IN_NVRAM
#undef	DEBUG_I2C
#define	CONFIG_ENV_IS_IN_EEPROM

#ifdef	CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_ADDR		0x20000000	/* use SRAM	*/
#define CONFIG_ENV_SIZE		(16<<10)	/* use 16 kB	*/
#endif	/* CONFIG_ENV_IS_IN_NVRAM */

#ifdef	CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		 512	/* Leave 512 bytes free for other data	*/
#define CONFIG_ENV_SIZE		1536	/* Use remaining space			*/
#endif	/* CONFIG_ENV_IS_IN_EEPROM */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif
#define CONFIG_SYS_DELAYED_ICACHE	1	/* enable ICache not before
						 * running in RAM.
						 */

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 * +0x0004
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
 * +0x0000 => 0x80600800
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_EARB   | SIUMCR_EARP0 | \
			 SIUMCR_DBGC11 | SIUMCR_MLRC10)

/*-----------------------------------------------------------------------
 * Clock Setting - get clock frequency from Board Revision Register
 *-----------------------------------------------------------------------
 */
#ifndef __ASSEMBLY__
extern  unsigned long           ip860_get_clk_freq (void);
#endif
#define	CONFIG_8xx_GCLK_FREQ	ip860_get_clk_freq()

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 * +0x0200 => 0x00C2
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 * +0x0240 => 0x0082
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit, set PLL multiplication factor !
 */
/* +0x0286 => was: 0x0000D000 */
#define CONFIG_SYS_PLPRCR							\
		(	PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST |	\
			/*PLPRCR_CSRC|*/ PLPRCR_LPM_NORMAL |		\
			PLPRCR_CSR   | PLPRCR_LOLRE /*|PLPRCR_FIOPD*/	\
		)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_COM00	|   SCCR_TBS	  |	\
			 SCCR_RTDIV	|   SCCR_RTSEL	  |	\
			 /*SCCR_CRQEN|*/  /*SCCR_PRQEN|*/	\
			 SCCR_EBDF00	|   SCCR_DFSYNC00 |	\
			 SCCR_DFBRG00	|   SCCR_DFNL000  |	\
			 SCCR_DFNH000)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
/* +0x0220 => 0x00C3 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)


/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register		19-4
 *-----------------------------------------------------------------------
 */
/* +0x09C4 => TIMEP=1 */
#define CONFIG_SYS_RCCR 0x0100

/*-----------------------------------------------------------------------
 * RMDS - RISC Microcode Development Support Control Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RMDS 0

/*-----------------------------------------------------------------------
 * DER - Debug Event Register
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_DER	0

/*
 * Init Memory Controller:
 */

/*
 * MAMR settings for SDRAM	- 16-14
 * => 0xC3804114
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	0xC3

#define CONFIG_SYS_MAMR	((CONFIG_SYS_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)
/*
 * BR1 and OR1 (FLASH)
 */
#define FLASH_BASE		0x10000000	/* FLASH bank #0	*/

/* used to re-map FLASH
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
/* allow for max 8 MB of Flash */
#define CONFIG_SYS_REMAP_OR_AM		0xFF800000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFF800000	/* OR addr mask */

#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM | OR_ACS_DIV2 | OR_BI | OR_SCY_6_CLK)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
/* 16 bit, bank valid */
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE & BR_BA_MSK) | BR_PS_32 | BR_V )

#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR0_PRELIM
#define CONFIG_SYS_BR1_PRELIM	CONFIG_SYS_BR0_PRELIM

/*
 * BR2/OR2 - SDRAM
 */
#define SDRAM_BASE		0x00000000	/* SDRAM bank */
#define SDRAM_PRELIM_OR_AM	0xF8000000	/* map max. 128 MB */
#define SDRAM_TIMING		0x00000A00	/* SDRAM-Timing */

#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB SDRAM */

#define CONFIG_SYS_OR2		(SDRAM_PRELIM_OR_AM | SDRAM_TIMING )
#define CONFIG_SYS_BR2		((SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * BR3/OR3 - SRAM (16 bit)
 */
#define	SRAM_BASE	0x20000000
#define CONFIG_SYS_OR3		0xFFF00130		/* BI/SCY = 5/TRLX (internal) */
#define CONFIG_SYS_BR3		((SRAM_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define SRAM_SIZE	(1 + (~(CONFIG_SYS_OR3 & BR_BA_MSK)))
#define CONFIG_SYS_OR3_PRELIM	CONFIG_SYS_OR3			/* Make sure to map early */
#define CONFIG_SYS_BR3_PRELIM	CONFIG_SYS_BR3			/* in case it's used for ENV */
#define	CONFIG_SYS_SRAM_BASE	SRAM_BASE
#define	CONFIG_SYS_SRAM_SIZE	SRAM_SIZE

/*
 * BR4/OR4 - Board Control & Status (8 bit)
 */
#define	BCSR_BASE	0xFC000000
#define CONFIG_SYS_OR4		0xFFFF0120		/* BI (internal) */
#define CONFIG_SYS_BR4		((BCSR_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)

/*
 * BR5/OR5 - IP Slot A/B (16 bit)
 */
#define	IP_SLOT_BASE	0x40000000
#define CONFIG_SYS_OR5		0xFE00010C		/* SETA/TRLX/BI/ SCY=0 (external) */
#define CONFIG_SYS_BR5		((IP_SLOT_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)

/*
 * BR6/OR6 - VME STD  (16 bit)
 */
#define	VME_STD_BASE	0xFE000000
#define CONFIG_SYS_OR6		0xFF00010C		/* SETA/TRLX/BI/SCY=0  (external) */
#define CONFIG_SYS_BR6		((VME_STD_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)

/*
 * BR7/OR7 - SHORT I/O + RTC + IACK  (16 bit)
 */
#define VME_SHORT_BASE	0xFF000000
#define CONFIG_SYS_OR7		0xFF00010C		/* SETA/TRLX/BI/ SCY=0 (external) */
#define CONFIG_SYS_BR7		((VME_SHORT_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)

/*-----------------------------------------------------------------------
 * Board Control and Status Region:
 *-----------------------------------------------------------------------
 */
#ifndef __ASSEMBLY__
typedef	struct ip860_bcsr_s {
	unsigned char	shmem_addr;	/* +00 shared memory address register	*/
	unsigned char	reserved0;
	unsigned char	mbox_addr;	/* +02 mailbox address register		*/
	unsigned char	reserved1;
	unsigned char	vme_int_mask;	/* +04 VME Bus interrupt mask register	*/
	unsigned char	reserved2;
	unsigned char	vme_int_pend;	/* +06 VME interrupt pending register	*/
	unsigned char	reserved3;
	unsigned char	bd_int_mask;	/* +08 board interrupt mask register	*/
	unsigned char	reserved4;
	unsigned char	bd_int_pend;	/* +0A board interrupt pending register	*/
	unsigned char	reserved5;
	unsigned char	bd_ctrl;	/* +0C board control register		*/
	unsigned char	reserved6;
	unsigned char	bd_status;	/* +0E board status  register		*/
	unsigned char	reserved7;
	unsigned char	vme_irq;	/* +10 VME interrupt request register	*/
	unsigned char	reserved8;
	unsigned char	vme_ivec;	/* +12 VME interrupt vector register	*/
	unsigned char	reserved9;
	unsigned char	cli_mbox;	/* +14 clear mailbox irq		*/
	unsigned char	reservedA;
	unsigned char	rtc;		/* +16 RTC control register		*/
	unsigned char	reservedB;
	unsigned char	mbox_data;	/* +18 mailbox read/write register	*/
	unsigned char	reservedC;
	unsigned char	wd_trigger;	/* +1A Watchdog trigger register	*/
	unsigned char	reservedD;
	unsigned char	rmw_req;	/* +1C RMW request register		*/
	unsigned char	reservedE;
	unsigned char	bd_rev;		/* +1E Board Revision register		*/
} ip860_bcsr_t;
#endif	/* __ASSEMBLY__ */

/*-----------------------------------------------------------------------
 * Board Control Register: bd_ctrl (Offset 0x0C)
 *-----------------------------------------------------------------------
 */
#define BD_CTRL_IPLSE	0x80	/* IP Slot Long Select Enable		*/
#define BD_CTRL_WDOGE	0x40	/* Watchdog Enable			*/
#define BD_CTRL_FLWE	0x20	/* Flash Write Enable			*/
#define BD_CTRL_RWDN	0x10	/* VMEBus Requester Release When Done Enable */

#endif	/* __CONFIG_H */
