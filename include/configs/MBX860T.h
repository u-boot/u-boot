 /*
  * A collection of structures, addresses, and values associated with
  * the Motorola 860T MBX board.
  * Copied from the FADS stuff, which was originally copied from the MBX stuff!
  * Magnus Damm added defines for 8xxrom and extended bd_info.
  * Helmut Buchsbaum added bitvalues for BCSRx
  * Rob Taylor coverted it back to MBX
  *
  * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
  */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <mpc8xx_irq.h>

#define CONFIG_MPC860		1
#define CONFIG_MPC860T		1
#define CONFIG_MBX		1

#define	CONFIG_SYS_TEXT_BASE	0xfe000000

#define CONFIG_8xx_CPUCLOCK	40
#define CONFIG_8xx_BUSCLOCK	(CONFIG_8xx_CPUCLOCK)
#define TARGET_SYSTEM_FREQUENCY 40

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#define CONFIG_BAUDRATE		9600

#define MPC8XX_FACT	10				/* Multiply by 10		*/
#define MPC8XX_XIN	40000000		/* 50 MHz in	*/
#define MPC8XX_HZ ((MPC8XX_XIN) * (MPC8XX_FACT))

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#if 1
#define CONFIG_8xx_BOOTDELAY	-1	/* autoboot disabled		*/
#define CONFIG_8xx_TFTP_MODE
#else
#define CONFIG_8xx_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#undef	CONFIG_8xx_TFTP_MODE
#endif

#define CONFIG_MISC_INIT_R

#define CONFIG_DRAM_SPEED	(CONFIG_8xx_BUSCLOCK)	/* MHz		*/
#define CONFIG_BOOTCOMMAND	"bootm FE020000"	/* autoboot command */
#define CONFIG_BOOTARGS		" "
/*
 * Miscellaneous configurable options
 */
#undef	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		":>"		/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0800000	/* 4 ... 8 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR			0xFFA00000
#define CONFIG_SYS_IMMR_SIZE		((uint)(64 * 1024))
#define CONFIG_SYS_NVRAM_BASE		0xFA000000 /* NVRAM                          */
#define CONFIG_SYS_NVRAM_OR		0xffe00000 /* w/o speed dependent flags!!    */
#define CONFIG_SYS_CSR_BASE		0xFA100000 /* Control/Status Registers       */
#define CONFIG_SYS_PCIMEM_BASE		0x80000000 /* PCI I/O and Memory Spaces      */
#define CONFIG_SYS_PCIMEM_OR		0xA0000108
#define CONFIG_SYS_PCIBRIDGE_BASE	0xFA210000 /* PCI-Bus Bridge Registers       */
#define CONFIG_SYS_PCIBRIDGE_OR	0xFFFF0108

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2f00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_VPD_SIZE	256 /* size in bytes reserved for vpd buffer */
#define CONFIG_SYS_INIT_VPD_OFFSET	(CONFIG_SYS_GBL_DATA_OFFSET - CONFIG_SYS_INIT_VPD_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_VPD_OFFSET-8)

/*-----------------------------------------------------------------------
 * Offset in DPMEM where we keep the VPD data
 */
#define CONFIG_SYS_DPRAMVPD		(CONFIG_SYS_INIT_VPD_OFFSET - 0x2000)

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x00000000
/*0xFE000000*/
#define CONFIG_SYS_FLASH_SIZE		((uint)(8 * 1024 * 1024))	/* max 8Mbyte */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_HWINFO_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN - CONFIG_SYS_HWINFO_LEN)
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
#define CONFIG_SYS_MAX_FLASH_BANKS	4	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	16	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * NVRAM Configuration
 *
 * Note: the MBX is special because there is already a firmware on this
 * board: EPPC-Bug from Motorola. To avoid collisions in NVRAM Usage, we
 * access the NVRAM at the offset 0x1000.
 */
#define CONFIG_ENV_IS_IN_NVRAM	1	/* turn on NVRAM env feature */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_NVRAM_BASE + 0x1000)
#define CONFIG_ENV_SIZE		0x1000

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
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DPC | SIUMCR_MLRC10 | SIUMCR_SEME)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF | PISCR_PTE)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit - leave PLL multiplication factor unchanged !
 */
#define CONFIG_SYS_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	(SCCR_RTDIV | SCCR_RTSEL)
#define CONFIG_SYS_SCCR	SCCR_TBS

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
 * BR0/1 and OR0/1 (FLASH)
 */
/* the other CS:s are determined by looking at parameters in BCSRx */


#define BCSR_ADDR		((uint) 0xFF010000)
#define BCSR_SIZE		((uint)(64 * 1024))

#define FLASH_BASE0_PRELIM	0xFE000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFF010000	/* FLASH bank #0	*/

#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFFF00000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(0xFF800000 | OR_CSNT_SAM | OR_BI | OR_SCY_3_CLK)   /* 1 Mbyte until detected and only 1 Mbyte is needed*/
#define CONFIG_SYS_BR0_PRELIM	(0xFE000000 | BR_V )

/* BCSRx - Board Control and Status Registers */
#define CONFIG_SYS_OR1_REMAP	CONFIG_SYS_OR0_REMAP
#define CONFIG_SYS_OR1_PRELIM	0xFFC00000 | OR_ACS_DIV4
#define CONFIG_SYS_BR1_PRELIM	(0x00000000 | BR_MS_UPMA | BR_V )


/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA		97		/* start with divider for 100 MHz	*/

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

#define CONFIG_SYS_MAMR		0x13821000

/* values according to the manual */


#define PCMCIA_MEM_ADDR		((uint)0xff020000)
#define PCMCIA_MEM_SIZE		((uint)(64 * 1024))

#define	BCSR0			((uint) (BCSR_ADDR + 00))
#define	BCSR1			((uint) (BCSR_ADDR + 0x04))
#define	BCSR2			((uint) (BCSR_ADDR + 0x08))
#define	BCSR3			((uint) (BCSR_ADDR + 0x0c))
#define	BCSR4			((uint) (BCSR_ADDR + 0x10))

/* FADS bitvalues by Helmut Buchsbaum
 * see MPC8xxADS User's Manual for a proper description
 * of the following structures
 */

#define BCSR0_ERB       ((uint)0x80000000)
#define BCSR0_IP        ((uint)0x40000000)
#define BCSR0_BDIS      ((uint)0x10000000)
#define BCSR0_BPS_MASK  ((uint)0x0C000000)
#define BCSR0_ISB_MASK  ((uint)0x01800000)
#define BCSR0_DBGC_MASK ((uint)0x00600000)
#define BCSR0_DBPC_MASK ((uint)0x00180000)
#define BCSR0_EBDF_MASK ((uint)0x00060000)

#define BCSR1_FLASH_EN           ((uint)0x80000000)
#define BCSR1_DRAM_EN            ((uint)0x40000000)
#define BCSR1_ETHEN              ((uint)0x20000000)
#define BCSR1_IRDEN              ((uint)0x10000000)
#define BCSR1_FLASH_CFG_EN       ((uint)0x08000000)
#define BCSR1_CNT_REG_EN_PROTECT ((uint)0x04000000)
#define BCSR1_BCSR_EN            ((uint)0x02000000)
#define BCSR1_RS232EN_1          ((uint)0x01000000)
#define BCSR1_PCCEN              ((uint)0x00800000)
#define BCSR1_PCCVCC0            ((uint)0x00400000)
#define BCSR1_PCCVPP_MASK        ((uint)0x00300000)
#define BCSR1_DRAM_HALF_WORD     ((uint)0x00080000)
#define BCSR1_RS232EN_2          ((uint)0x00040000)
#define BCSR1_SDRAM_EN           ((uint)0x00020000)
#define BCSR1_PCCVCC1            ((uint)0x00010000)

#define BCSR2_FLASH_PD_MASK      ((uint)0xF0000000)
#define BCSR2_DRAM_PD_MASK       ((uint)0x07800000)
#define BCSR2_DRAM_PD_SHIFT      (23)
#define BCSR2_EXTTOLI_MASK       ((uint)0x00780000)
#define BCSR2_DBREVNR_MASK       ((uint)0x00030000)

#define BCSR3_DBID_MASK          ((ushort)0x3800)
#define BCSR3_CNT_REG_EN_PROTECT ((ushort)0x0400)
#define BCSR3_BREVNR0            ((ushort)0x0080)
#define BCSR3_FLASH_PD_MASK      ((ushort)0x0070)
#define BCSR3_BREVN1             ((ushort)0x0008)
#define BCSR3_BREVN2_MASK        ((ushort)0x0003)

#define BCSR4_ETHLOOP            ((uint)0x80000000)
#define BCSR4_TFPLDL             ((uint)0x40000000)
#define BCSR4_TPSQEL             ((uint)0x20000000)
#define BCSR4_SIGNAL_LAMP        ((uint)0x10000000)
#ifdef CONFIG_MPC823
#define BCSR4_USB_EN             ((uint)0x08000000)
#endif /* CONFIG_MPC823 */
#ifdef CONFIG_MPC860SAR
#define BCSR4_UTOPIA_EN          ((uint)0x08000000)
#endif /* CONFIG_MPC860SAR */
#ifdef CONFIG_MPC860T
#define BCSR4_FETH_EN            ((uint)0x08000000)
#endif /* CONFIG_MPC860T */
#define BCSR4_USB_SPEED          ((uint)0x04000000)
#define BCSR4_VCCO               ((uint)0x02000000)
#define BCSR4_VIDEO_ON           ((uint)0x00800000)
#define BCSR4_VDO_EKT_CLK_EN     ((uint)0x00400000)
#define BCSR4_VIDEO_RST          ((uint)0x00200000)
#define BCSR4_MODEM_EN           ((uint)0x00100000)
#define BCSR4_DATA_VOICE         ((uint)0x00080000)

#define CONFIG_DRAM_40MHZ		1

#ifdef CONFIG_MPC860T

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT	SIU_LEVEL1	/* FEC interrupt */

#endif /* CONFIG_MPC860T */

/* We don't use the 8259.
*/
#define NR_8259_INTS	0

#define CONFIG_CMD_NET
/*
 * MPC8xx CPM Options
 */
#define CONFIG_SCC_ENET 1
#define CONFIG_SCC1_ENET 1
#define CONFIG_FEC_ENET 1
#undef  CONFIG_CPM_IIC
#undef  CONFIG_UCODE_PATCH


#define CONFIG_DISK_SPINUP_TIME 1000000


/* PCMCIA configuration */

#define PCMCIA_MAX_SLOTS    2

#ifdef CONFIG_MPC860
#define PCMCIA_SLOT_A 1
#endif

#endif	/* __CONFIG_H */
