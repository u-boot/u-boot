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

/*
 * Imported from global configuration:
 *	CONFIG_MPC8255
 *	CONFIG_MPC8265
 *	CONFIG_200MHz
 *	CONFIG_266MHz
 *	CONFIG_300MHz
 *	CONFIG_L2_CACHE
 *	CONFIG_BUSMODE_60x
 */

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#define CONFIG_MPC8260		1	/* This is a MPC8260 CPU		*/

#if 0
#define CONFIG_TQM8260		100	/* ...on a TQM8260 module Rev.100	*/
#else
#define CONFIG_TQM8260		200	/* ...on a TQM8260 module Rev.200	*/
#endif

#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_82xx_CONS_SMC1	1	/* console on SMC1			*/

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define	CONFIG_BOOTCOUNT_LIMIT

#define CONFIG_BAUDRATE		115200

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=tqm8260/uImage\0"				\
	"kernel_addr=400C0000\0"					\
	"ramdisk_addr=40240000\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

/* enable I2C and select the hardware/software driver */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SOFT		/* I2C bit-banged */
#define CONFIG_SYS_I2C_SOFT_SPEED	400000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x7F

/*
 * Software (bit-bang) I2C driver configuration
 */

/* TQM8260 Rev.100 has the clock and data pins swapped (!!!) on EEPROM */
#if (CONFIG_TQM8260 <= 100)

#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00020000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00020000)
#define I2C_READ	((iop->pdat & 0x00020000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#else

#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */
#endif

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

#define CONFIG_I2C_X

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#define CONFIG_CONS_ON_SMC		/* define if console on SMC */
#undef  CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef  CONFIG_CONS_NONE		/* define if console on something else*/
#ifdef CONFIG_82xx_CONS_SMC1
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */
#endif
#ifdef CONFIG_82xx_CONS_SMC2
#define CONFIG_CONS_INDEX	2	/* which serial channel for console */
#endif

#undef  CONFIG_CONS_USE_EXTC		/* SMC/SCC use ext clock not brg_clk */
#define CONFIG_CONS_EXTC_RATE	3686400	/* SMC/SCC ext clk rate in Hz */
#define CONFIG_CONS_EXTC_PINSEL	0	/* pin select 0=CLK3/CLK9 */

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 *
 * (On TQM8260 either SCC1 or FCC2 may be chosen: SCC1 is hardwired to the
 * X.29 connector, and FCC2 is hardwired to the X.1 connector)
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC       */
#define	CONFIG_ETHER_ON_FCC		/* define if ether on FCC       */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define	CONFIG_ETHER_INDEX    2		/* which SCC/FCC channel for ethernet */

#if defined(CONFIG_ETHER_ON_SCC) && (CONFIG_ETHER_INDEX == 1)

/*
 *  - RX clk is CLK11
 *  - TX clk is CLK12
 */
# define CONFIG_SYS_CMXSCR_VALUE1	(CMXSCR_RS1CS_CLK11 | CMXSCR_TS1CS_CLK12)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK2	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */


/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#if defined(CONFIG_MPC8255) || defined(CONFIG_MPC8265)
#  define CONFIG_8260_CLKIN	66666666	/* in Hz */
#else	/* !CONFIG_MPC8255 && !CONFIG_MPC8265 */
# ifndef CONFIG_300MHz
#  define CONFIG_8260_CLKIN	66666666	/* in Hz */
# else
#  define CONFIG_8260_CLKIN	83333000	/* in Hz */
# endif
#endif	/* CONFIG_MPC8255 */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP


/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_SYS_HUSH_PARSER		1	/* Use the HUSH parser		*/

#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END	0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define	CONFIG_SYS_RESET_ADDRESS 0xFFFFFFFC	/* "bad" address		*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */


/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain CONFIG_SYS_TEXT_BASE from board/tqm8260/config.mk
 * The main FLASH is whichever is connected to *CS0.
 */
#define CONFIG_SYS_FLASH0_BASE 0x40000000
#define CONFIG_SYS_FLASH1_BASE 0x60000000
#define CONFIG_SYS_FLASH0_SIZE 32
#define CONFIG_SYS_FLASH1_SIZE 32

/* Flash bank size (for preliminary settings)
 */
#define CONFIG_SYS_FLASH_SIZE CONFIG_SYS_FLASH0_SIZE

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

/* use CFI flash driver */
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER		1	/* Use the common driver */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_FLASH_EMPTY_INFO	1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1

#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x40000)
#define CONFIG_ENV_SIZE		0x08000
#define CONFIG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_ADDR_REDUND (CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#define CONFIG_SYS_HWINFO_OFFSET	0x0003FFC0	/* offset of HW Info block */
#define CONFIG_SYS_HWINFO_SIZE		0x00000040	/* size   of HW Info block */
#define CONFIG_SYS_HWINFO_MAGIC	0x54514D38	/* 'TQM8' */

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CONFIG_SYS_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CONFIG_SYS_SIUMCR.
 */
#define	__HRCW__ALL__		(HRCW_CIP | HRCW_ISB111 | HRCW_BMS)

#if defined(CONFIG_MPC8255) || defined(CONFIG_MPC8265)
#  define CONFIG_SYS_HRCW_MASTER	(__HRCW__ALL__ | HRCW_MODCK_H0111)
#else	/* ! MPC8255 && !MPC8265 */
# if defined(CONFIG_266MHz)
#  define CONFIG_SYS_HRCW_MASTER	(__HRCW__ALL__ | HRCW_MODCK_H0111)
# elif defined(CONFIG_300MHz)
#  define CONFIG_SYS_HRCW_MASTER	(__HRCW__ALL__ | HRCW_MODCK_H0110)
# else
#  define CONFIG_SYS_HRCW_MASTER	(__HRCW__ALL__)
# endif
#endif	/* CONFIG_MPC8255 */

/* no slaves so just fill with zeros */
#define CONFIG_SYS_HRCW_SLAVE1		0
#define CONFIG_SYS_HRCW_SLAVE2		0
#define CONFIG_SYS_HRCW_SLAVE3		0
#define CONFIG_SYS_HRCW_SLAVE4		0
#define CONFIG_SYS_HRCW_SLAVE5		0
#define CONFIG_SYS_HRCW_SLAVE6		0
#define CONFIG_SYS_HRCW_SLAVE7		0

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000  /* Size of used area in DPRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 *
 * 60x SDRAM is mapped at CONFIG_SYS_SDRAM_BASE, local SDRAM
 * is mapped at SDRAM_BASE2_PRELIM.
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_FLASH0_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE      32      /* For MPC8260 CPU              */
#if defined(CONFIG_CMD_KGDB)
# define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers                    2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CONFIG_SYS_HID0_INIT   (HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CONFIG_SYS_HID0_FINAL  (HID0_IFEM|HID0_ABE)
#define CONFIG_SYS_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register                                     5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CONFIG_SYS_RMR         RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#ifdef	CONFIG_BUSMODE_60x
#define CONFIG_SYS_BCR         (BCR_EBM|BCR_L2C|BCR_LETM|\
			 BCR_NPQM0|BCR_NPQM1|BCR_NPQM2)	/* 60x mode  */
#else
#define BCR_APD01	0x10000000
#define CONFIG_SYS_BCR		(BCR_APD01|BCR_ETM|BCR_LETM)	/* 8260 mode */
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */
#if 0
#define CONFIG_SYS_SIUMCR      (SIUMCR_DPPC10|SIUMCR_APPC10)
#else
#define CONFIG_SYS_SIUMCR      (SIUMCR_DPPC00|SIUMCR_APPC10)
#endif


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                             4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR       (SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CONFIG_SYS_SYPCR       (SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control                     4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CONFIG_SYS_TMCNTSC     (TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CONFIG_SYS_PISCR       (PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                                   9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CONFIG_SYS_SCCR        0

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR        0

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM    64 bit  FLASH
 *  1   60x     SDRAM   64 bit  SDRAM
 *  2   Local   SDRAM   32 bit  SDRAM
 *
 */

	/* Initialize SDRAM on local bus
	 */
#define CONFIG_SYS_INIT_LOCAL_SDRAM

#define SDRAM_MAX_SIZE	0x08000000	/* max. 128 MB		*/

/* Minimum mask to separate preliminary
 * address ranges for CS[0:2]
 */
#define CONFIG_SYS_GLOBAL_SDRAM_LIMIT	(512<<20)	/* less than 512 MB */
#define CONFIG_SYS_LOCAL_SDRAM_LIMIT	(128<<20)	/* less than 128 MB */

#define CONFIG_SYS_MPTPR       0x4000

/*-----------------------------------------------------------------------------
 * Address for Mode Register Set (MRS) command
 *-----------------------------------------------------------------------------
 * In fact, the address is rather configuration data presented to the SDRAM on
 * its address lines. Because the address lines may be mux'ed externally either
 * for 8 column or 9 column devices, some bits appear twice in the 8260's
 * address:
 *
 * |   (RFU)   |   (RFU)   | WBL |    TM    |     CL    |  BT | Burst Length |
 * | BA1   BA0 | A12 : A10 |  A9 |  A8   A7 |  A6 : A4  |  A3 |   A2 :  A0   |
 *  8 columns mux'ing:     |  A9 | A10  A21 | A22 : A24 | A25 |  A26 : A28   |
 *  9 columns mux'ing:     |  A8 | A20  A21 | A22 : A24 | A25 |  A26 : A28   |
 *  Settings:              |  0  |  0    0  |  0  1  0  |  0  |   0  1  0    |
 *-----------------------------------------------------------------------------
 */
#define CONFIG_SYS_MRS_OFFS	0x00000110


/* Bank 0 - FLASH
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (MEG_TO_AM(CONFIG_SYS_FLASH_SIZE)      |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxG_EHTR                      |\
			 ORxG_TRLX)

	/* SDRAM on TQM8260 can have either 8 or 9 columns.
	 * The number affects configuration values.
	 */

/* Bank 1 - 60x bus SDRAM
 */
#define CONFIG_SYS_PSRT        0x20
#define CONFIG_SYS_LSRT        0x20
#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR1_8COL


	/* SDRAM initialization values for 8-column chips
	 */
#define CONFIG_SYS_OR1_8COL    ((~(CONFIG_SYS_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A7             |\
			 ORxS_NUMR_12)

#define CONFIG_SYS_PSDMR_8COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A15_IS_A5           |\
			 PSDMR_BSMA_A12_A14             |\
			 PSDMR_SDA10_PBI1_A8            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_EAMUX                    |\
			 PSDMR_CL_2)

	/* SDRAM initialization values for 9-column chips
	 */
#define CONFIG_SYS_OR1_9COL    ((~(CONFIG_SYS_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A5             |\
			 ORxS_NUMR_13)

#define CONFIG_SYS_PSDMR_9COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A16_IS_A5           |\
			 PSDMR_BSMA_A12_A14             |\
			 PSDMR_SDA10_PBI1_A7            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_EAMUX                    |\
			 PSDMR_CL_2)

/* Bank 2 - Local bus SDRAM
 */
#ifdef CONFIG_SYS_INIT_LOCAL_SDRAM
#define CONFIG_SYS_BR2_PRELIM  ((SDRAM_BASE2_PRELIM & BRx_BA_MSK) |\
			 BRx_PS_32                      |\
			 BRx_MS_SDRAM_L                 |\
			 BRx_V)

#define CONFIG_SYS_OR2_PRELIM	CONFIG_SYS_OR2_8COL

#define SDRAM_BASE2_PRELIM	0x80000000

	/* SDRAM initialization values for 8-column chips
	 */
#define CONFIG_SYS_OR2_8COL    ((~(CONFIG_SYS_LOCAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A8             |\
			 ORxS_NUMR_12)

#define CONFIG_SYS_LSDMR_8COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A15_IS_A5           |\
			 PSDMR_BSMA_A13_A15             |\
			 PSDMR_SDA10_PBI1_A9            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_BL                       |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_CL_2)

	/* SDRAM initialization values for 9-column chips
	 */
#define CONFIG_SYS_OR2_9COL    ((~(CONFIG_SYS_LOCAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A6             |\
			 ORxS_NUMR_13)

#define CONFIG_SYS_LSDMR_9COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A16_IS_A5           |\
			 PSDMR_BSMA_A13_A15             |\
			 PSDMR_SDA10_PBI1_A8            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_BL                       |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_CL_2)

#endif /* CONFIG_SYS_INIT_LOCAL_SDRAM */

#endif /* CONFIG_SYS_RAMBOOT */

#endif	/* __CONFIG_H */
