/*
 * (C) Copyright 2001-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
#define CONFIG_CPU86		1	/* ...on a CPU86 board	*/
#define CONFIG_CPM2		1	/* Has a CPM2 */

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
#undef  CONFIG_CONS_ON_SMC		/* define if console on SMC */
#define CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef  CONFIG_CONS_NONE		/* define if console on something else*/
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */

#if defined(CONFIG_CONS_NONE) || defined(CONFIG_CONS_USE_EXTC)
#define CONFIG_BAUDRATE		230400
#else
#define CONFIG_BAUDRATE		9600
#endif

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
#define	CONFIG_ETHER_ON_FCC		/* define if ether on FCC       */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define	CONFIG_ETHER_INDEX	1	/* which SCC/FCC channel for ethernet */

#if defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 1)

/*
 * - Rx-CLK is CLK11
 * - Tx-CLK is CLK12
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC1|CMXFCR_RF1CS_MSK|CMXFCR_TF1CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF1CS_CLK11|CMXFCR_TF1CS_CLK12)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	64000000	/* in Hz */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_PREBOOT								\
	"echo; "								\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS; "	\
	"echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"

/*-----------------------------------------------------------------------
 * I2C/EEPROM/RTC configuration
 */
#define	CONFIG_SOFT_I2C			/* Software I2C support enabled	*/

# define CONFIG_SYS_I2C_SPEED		50000
# define CONFIG_SYS_I2C_SLAVE		0xFE
/*
 * Software (bit-bang) I2C driver configuration
 */
#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else    iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else    iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR	0x51

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

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

#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
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

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END	0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CONFIG_SYS_RESET_ADDRESS 0xFFF00100	/* "bad" address		*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Flash configuration
 */

#define CONFIG_SYS_BOOTROM_BASE	0xFF800000
#define CONFIG_SYS_BOOTROM_SIZE	0x00080000
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_SIZE		0x00800000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of memory banks      */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

/*-----------------------------------------------------------------------
 * Other areas to be mapped
 */

/* CS3: Dual ported SRAM */
#define CONFIG_SYS_DPSRAM_BASE		0x40000000
#define CONFIG_SYS_DPSRAM_SIZE		0x00020000

/* CS4: DiskOnChip */
#define CONFIG_SYS_DOC_BASE		0xF4000000
#define CONFIG_SYS_DOC_SIZE		0x00100000

/* CS5: FDC37C78 controller */
#define CONFIG_SYS_FDC37C78_BASE	0xF1000000
#define CONFIG_SYS_FDC37C78_SIZE	0x00100000

/* CS6: Board configuration registers */
#define CONFIG_SYS_BCRS_BASE		0xF2000000
#define CONFIG_SYS_BCRS_SIZE		0x00010000

/* CS7: VME Extended Access Range */
#define CONFIG_SYS_VMEEAR_BASE		0x80000000
#define CONFIG_SYS_VMEEAR_SIZE		0x01000000

/* CS8: VME Standard Access Range */
#define CONFIG_SYS_VMESAR_BASE		0xFE000000
#define CONFIG_SYS_VMESAR_SIZE		0x01000000

/* CS9: VME Short I/O Access Range */
#define CONFIG_SYS_VMESIOAR_BASE	0xFD000000
#define CONFIG_SYS_VMESIOAR_SIZE	0x01000000

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CONFIG_SYS_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CONFIG_SYS_SIUMCR.
 */
#if defined(CONFIG_BOOT_ROM)
#define CONFIG_SYS_HRCW_MASTER		(HRCW_CIP | HRCW_ISB100 | HRCW_BMS | \
				 HRCW_BPS01 | HRCW_CS10PC01)
#else
#define CONFIG_SYS_HRCW_MASTER		(HRCW_CIP | HRCW_ISB100 | HRCW_BMS | HRCW_CS10PC01)
#endif

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
#define CONFIG_SYS_INIT_RAM_END	0x4000  /* End of used area in DPRAM    */
#define CONFIG_SYS_GBL_DATA_SIZE	128 /* size in bytes reserved for initial data*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
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
#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
# define CONFIG_SYS_RAMBOOT
#endif

#if 0
/* environment is in Flash */
#define CONFIG_ENV_IS_IN_FLASH	1
#ifdef CONFIG_BOOT_ROM
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+0x70000)
# define CONFIG_ENV_SIZE		0x10000
# define CONFIG_ENV_SECT_SIZE	0x10000
#endif
#else
/* environment is in EEPROM */
#define CONFIG_ENV_IS_IN_EEPROM	1
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x58	/* EEPROM X24C16		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */
#define CONFIG_ENV_OFFSET		512
#define CONFIG_ENV_SIZE		(2048 - 512)
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH*/
#define BOOTFLAG_WARM		0x02	/* Software reboot                 */


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
#define CONFIG_SYS_SIUMCR      (SIUMCR_BBD|SIUMCR_DPPC00|SIUMCR_APPC10|\
			 SIUMCR_CS10PC01|SIUMCR_BCTLC10)

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
			 ORxS_ROWST_PBI0_A9		|\
			 ORxS_NUMR_12)

#define CONFIG_SYS_PSDMR_8COL	(PSDMR_SDAM_A13_IS_A5		|\
			 PSDMR_BSMA_A14_A16		|\
			 PSDMR_SDA10_PBI0_A10		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_2W		|\
			 PSDMR_ACTTORW_1W		|\
			 PSDMR_LDOTOPRE_1C		|\
			 PSDMR_WRC_1C			|\
			 PSDMR_CL_2)

	/* SDRAM initialization values for 9-column chips
	 */
#define CONFIG_SYS_OR2_9COL	(CONFIG_SYS_MIN_AM_MASK		|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A7		|\
			 ORxS_NUMR_13)

#define CONFIG_SYS_PSDMR_9COL	(PSDMR_SDAM_A14_IS_A5		|\
			 PSDMR_BSMA_A13_A15		|\
			 PSDMR_SDA10_PBI0_A9		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_2W		|\
			 PSDMR_ACTTORW_1W		|\
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

#ifdef CONFIG_BOOT_ROM
/* Bank 0 - Boot ROM
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_BOOTROM_BASE & BRx_BA_MSK)|\
			 BRx_PS_8                       |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_BOOTROM_SIZE)   |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxU_EHTR_8IDLE)

/* Bank 1 - FLASH
 */
#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR1_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_FLASH_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxU_EHTR_8IDLE)

#else /* CONFIG_BOOT_ROM */
/* Bank 0 - FLASH
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_FLASH_SIZE)     |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxU_EHTR_8IDLE)

/* Bank 1 - Boot ROM
 */
#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_BOOTROM_BASE & BRx_BA_MSK)|\
			 BRx_PS_8                       |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR1_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_BOOTROM_SIZE)   |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_3_CLK                 |\
			 ORxU_EHTR_8IDLE)

#endif /* CONFIG_BOOT_ROM */


/* Bank 2 - 60x bus SDRAM
 */
#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_BR2_PRELIM  ((CONFIG_SYS_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CONFIG_SYS_OR2_PRELIM	 CONFIG_SYS_OR2_9COL

#define CONFIG_SYS_PSDMR	 CONFIG_SYS_PSDMR_9COL
#endif /* CONFIG_SYS_RAMBOOT */

/* Bank 3 - Dual Ported SRAM
 */
#define CONFIG_SYS_BR3_PRELIM  ((CONFIG_SYS_DPSRAM_BASE & BRx_BA_MSK) |\
			 BRx_PS_16                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR3_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_DPSRAM_SIZE)    |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_5_CLK                 |\
			 ORxG_SETA)

/* Bank 4 - DiskOnChip
 */
#define CONFIG_SYS_BR4_PRELIM  ((CONFIG_SYS_DOC_BASE & BRx_BA_MSK)    |\
			 BRx_PS_8                       |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR4_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_DOC_SIZE)       |\
			 ORxG_ACS_DIV2                  |\
			 ORxG_SCY_5_CLK                 |\
			 ORxU_EHTR_8IDLE)

/* Bank 5 - FDC37C78 controller
 */
#define CONFIG_SYS_BR5_PRELIM  ((CONFIG_SYS_FDC37C78_BASE & BRx_BA_MSK) |\
			 BRx_PS_8                         |\
			 BRx_MS_GPCM_P                    |\
			 BRx_V)

#define CONFIG_SYS_OR5_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_FDC37C78_SIZE)    |\
			 ORxG_ACS_DIV2                    |\
			 ORxG_SCY_8_CLK                   |\
			 ORxU_EHTR_8IDLE)

/* Bank 6 - Board control registers
 */
#define CONFIG_SYS_BR6_PRELIM  ((CONFIG_SYS_BCRS_BASE & BRx_BA_MSK)   |\
			 BRx_PS_8                       |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR6_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_BCRS_SIZE)      |\
			 ORxG_CSNT                      |\
			 ORxG_SCY_5_CLK)

/* Bank 7 - VME Extended Access Range
 */
#define CONFIG_SYS_BR7_PRELIM  ((CONFIG_SYS_VMEEAR_BASE & BRx_BA_MSK) |\
			 BRx_PS_32                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR7_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_VMEEAR_SIZE)    |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_5_CLK                 |\
			 ORxG_SETA)

/* Bank 8 - VME Standard Access Range
 */
#define CONFIG_SYS_BR8_PRELIM  ((CONFIG_SYS_VMESAR_BASE & BRx_BA_MSK) |\
			 BRx_PS_16                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR8_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_VMESAR_SIZE)    |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV1                  |\
			 ORxG_SCY_5_CLK                 |\
			 ORxG_SETA)

/* Bank 9 - VME Short I/O Access Range
 */
#define CONFIG_SYS_BR9_PRELIM  ((CONFIG_SYS_VMESIOAR_BASE & BRx_BA_MSK) |\
			 BRx_PS_16                        |\
			 BRx_MS_GPCM_P                    |\
			 BRx_V)

#define CONFIG_SYS_OR9_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_VMESIOAR_SIZE)    |\
			 ORxG_CSNT                        |\
			 ORxG_ACS_DIV1                    |\
			 ORxG_SCY_5_CLK                   |\
			 ORxG_SETA)

#endif	/* __CONFIG_H */
