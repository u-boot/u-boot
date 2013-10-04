/*
 * (C) Copyright 2001
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

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU		*/
#define CONFIG_ATC		1	/* ...on a ATC board	*/
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define	CONFIG_SYS_TEXT_BASE	0xFF000000

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
#define  CONFIG_CONS_ON_SMC		/* define if console on SMC */
#undef CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef  CONFIG_CONS_NONE		/* define if console on something else*/
#define CONFIG_CONS_INDEX	2	/* which serial channel for console */

#define CONFIG_BAUDRATE		115200

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC       */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define CONFIG_ETHER_ON_FCC

#define CONFIG_ETHER_ON_FCC2

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

#define CONFIG_ETHER_ON_FCC3

/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK16
 * - RAM for BD/Buffers is on the local Bus (see 28-13)
 * - Enable Half Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK3	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE3	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	64000000	/* in Hz */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#undef	CONFIG_CLOCKS_IN_MHZ		/* clocks passsed to Linux in Hz */

#define CONFIG_PREBOOT							\
	"echo;"								\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;"\
	"echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND						\
	"bootp;"							\
	"setenv bootargs root=/dev/nfs rw "				\
	"nfsroot=${serverip}:${rootpath} "				\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"\
	"bootm"

/*-----------------------------------------------------------------------
 * Miscellaneous configuration options
 */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/


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

#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PCMCIA
#define CONFIG_CMD_DATE
#define CONFIG_CMD_IDE


#define CONFIG_DOS_PARTITION

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
#define CONFIG_SYS_MEMTEST_END	0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address	*/

#define CONFIG_SYS_PIO_MODE		0	/* IDE interface in PIO Mode 0	*/

#define	CONFIG_SYS_RESET_ADDRESS 0xFFF00100	/* "bad" address		*/

#define CONFIG_SYS_ALLOC_DPRAM

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_SPI

#define CONFIG_RTC_DS12887

#define RTC_BASE_ADDR		0xF5000000
#define RTC_PORT_ADDR		RTC_BASE_ADDR + 0x800
#define RTC_PORT_DATA		RTC_BASE_ADDR + 0x808

#define CONFIG_MISC_INIT_R

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Flash configuration
 */

#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_SIZE		0x00800000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

#define CONFIG_FLASH_16BIT

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CONFIG_SYS_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CONFIG_SYS_SIUMCR.
 */
#define CONFIG_SYS_HRCW_MASTER		(HRCW_CIP | HRCW_ISB100 | HRCW_BMS | \
				 HRCW_BPS10 |\
				 HRCW_APPC10)

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
#define CONFIG_SYS_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00  /* Size of used area in DPRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 *
 * 60x SDRAM is mapped at CONFIG_SYS_SDRAM_BASE.
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_MAX_SIZE	0x08000000	/* max. 128 MB		*/
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
# define CONFIG_SYS_RAMBOOT
#endif

#define	CONFIG_PCI
#define CONFIG_PCI_INDIRECT_BRIDGE
#define	CONFIG_PCI_PNP
#define	CONFIG_SYS_PCI_MSTR_IO_BUS	0x00000000	/* PCI base   */

#if 1
/* environment is in Flash */
#define CONFIG_ENV_IS_IN_FLASH	1
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+0x30000)
# define CONFIG_ENV_SIZE		0x10000
# define CONFIG_ENV_SECT_SIZE	0x10000
#else
#define CONFIG_ENV_IS_IN_EEPROM	1
#define CONFIG_ENV_OFFSET		0
#define CONFIG_ENV_SIZE		2048
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4	/* 16-byte page size	*/
#endif

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
#define CONFIG_SYS_HID0_INIT   (HID0_ICE|HID0_DCE|HID0_ICFI|\
			 HID0_DCI|HID0_IFEM|HID0_ABE)
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
#define BCR_APD01	0x10000000
#define CONFIG_SYS_BCR		(BCR_APD01|BCR_ETM|BCR_LETM)	/* 8260 mode */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SIUMCR      (SIUMCR_BBD|SIUMCR_APPC10|\
			 SIUMCR_CS10PC00|SIUMCR_BCTLC10)

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
#define CONFIG_SYS_SCCR        SCCR_DFBRG01

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR        0

#define CONFIG_SYS_MIN_AM_MASK	0xC0000000
/*-----------------------------------------------------------------------
 * MPTPR - Memory Refresh Timer Prescaler Register              10-18
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_MPTPR       0x1F00

/*-----------------------------------------------------------------------
 * PSRT - Refresh Timer Register                                10-16
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_PSRT        0x0f

/*-----------------------------------------------------------------------
 * PSRT - SDRAM Mode Register                                   10-10
 *-----------------------------------------------------------------------
 */

	/* SDRAM initialization values for 8-column chips
	 */
#define CONFIG_SYS_OR2_8COL	(CONFIG_SYS_MIN_AM_MASK		|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI1_A7		|\
			 ORxS_NUMR_12)

#define CONFIG_SYS_PSDMR_8COL	(PSDMR_PBI			|\
			 PSDMR_SDAM_A15_IS_A5		|\
			 PSDMR_BSMA_A15_A17		|\
			 PSDMR_SDA10_PBI1_A7		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_3W		|\
			 PSDMR_ACTTORW_2W		|\
			 PSDMR_LDOTOPRE_1C		|\
			 PSDMR_WRC_1C			|\
			 PSDMR_CL_2)

	/* SDRAM initialization values for 9-column chips
	 */
#define CONFIG_SYS_OR2_9COL	(CONFIG_SYS_MIN_AM_MASK		|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI1_A6		|\
			 ORxS_NUMR_12)

#define CONFIG_SYS_PSDMR_9COL	(PSDMR_PBI			|\
			 PSDMR_SDAM_A16_IS_A5		|\
			 PSDMR_BSMA_A15_A17		|\
			 PSDMR_SDA10_PBI1_A6		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_3W		|\
			 PSDMR_ACTTORW_2W		|\
			 PSDMR_LDOTOPRE_1C		|\
			 PSDMR_WRC_1C			|\
			 PSDMR_CL_2)

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM    8  bit  Boot ROM
 *  1   60x     GPCM    64 bit  FLASH
 *  2   60x     SDRAM   64 bit  SDRAM
 *
 */

#define CONFIG_SYS_MRS_OFFS	0x00000000

/* Bank 0 - FLASH
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_16                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_FLASH_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxU_EHTR_8IDLE)


/* Bank 2 - 60x bus SDRAM
 */
#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_BR2_PRELIM  ((CONFIG_SYS_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CONFIG_SYS_OR2_PRELIM	 CONFIG_SYS_OR2_8COL

#define CONFIG_SYS_PSDMR	 CONFIG_SYS_PSDMR_8COL
#endif /* CONFIG_SYS_RAMBOOT */

#define CONFIG_SYS_BR4_PRELIM  ((RTC_BASE_ADDR & BRx_BA_MSK)   |\
			 BRx_PS_8                       |\
			 BRx_MS_UPMA                    |\
			 BRx_V)

#define CONFIG_SYS_OR4_PRELIM  (ORxU_AM_MSK | ORxU_BI)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_I82365

#define CONFIG_SYS_PCMCIA_MEM_ADDR	0x81000000
#define CONFIG_SYS_PCMCIA_MEM_SIZE	0x1000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define CONFIG_IDE_PREINIT	1	/* Use preinit IDE hook */
#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#undef	CONFIG_IDE_LED			/* LED	 for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	0xa0000000

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	0x100

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	0x100

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x108

#endif	/* __CONFIG_H */
