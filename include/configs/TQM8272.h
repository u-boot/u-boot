/*
 * (C) Copyright 2006
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

#define CONFIG_MPC8260		1	/* This is a MPC8260 CPU		*/
#define CONFIG_MPC8272_FAMILY   1
#define CONFIG_TQM8272		1

#define	CONFIG_GET_CPU_STR_F	1	/* Get the CPU ID STR */
#define CONFIG_BOARD_GET_CPU_CLK_F	1 /* Get the CLKIN from board fct */

#define	STK82xx_150		1	/* on a STK82xx.150 */

#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_82xx_CONS_SMC1	1	/* console on SMC1		*/

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BOARD_EARLY_INIT_R	1

#if defined(CONFIG_CONS_NONE) || defined(CONFIG_CONS_USE_EXTC)
#define CONFIG_BAUDRATE		230400
#else
#define CONFIG_BAUDRATE		115200
#endif

#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consdev=ttyCPM0\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"hostname=tqm8272\0"						\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=$(consdev),$(baudrate)\0"			\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addcons;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 300000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/tqm8272/uImage\0"				\
	"kernel_addr=40080000\0"					\
	"ramdisk_addr=40100000\0"					\
	"load=tftp 300000 /tftpboot/tqm8272/u-boot.bin\0"		\
	"update=protect off 40000000 4003ffff;era 40000000 4003ffff;"	\
		"cp.b 300000 40000000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"cphwib=cp.b 4003fc00 33fc00 400\0"				\
	"upd=run load;run cphwib;run update\0"				\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_I2C	1

#if CONFIG_I2C
/* enable I2C and select the hardware/software driver */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
#define ADD_CMD_I2C		CFG_CMD_I2C	| \
				CFG_CMD_DATE	|\
				CFG_CMD_DTT	|\
				CFG_CMD_EEPROM
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

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

#define CONFIG_I2C_X

/* EEPROM */
#define CFG_I2C_EEPROM_ADDR_LEN 2
#define CFG_EEPROM_PAGE_WRITE_BITS	4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE	/* necessary for the LM75 chip */
#define CFG_I2C_MULTI_EEPROMS		1	/* more than one eeprom */

/* I2C RTC */
#define CONFIG_RTC_DS1337		/* Use ds1337 rtc via i2c	*/
#define CFG_I2C_RTC_ADDR	0x68	/* at address 0x68		*/

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define CFG_DTT_HYSTERESIS	3

#else
#undef CONFIG_HARD_I2C
#undef CONFIG_SOFT_I2C
#define ADD_CMD_I2C		0
#endif

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
 * (On TQM8272 either SCC1 or FCC2 may be chosen: SCC1 is hardwired to the
 * X.29 connector, and FCC2 is hardwired to the X.1 connector)
 */
#define CFG_FCC_ETHERNET

#if defined(CFG_FCC_ETHERNET)
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC       */
#define	CONFIG_ETHER_ON_FCC		/* define if ether on FCC       */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define	CONFIG_ETHER_INDEX    2		/* which SCC/FCC channel for ethernet */
#else
#define	CONFIG_ETHER_ON_SCC		/* define if ether on SCC       */
#undef	CONFIG_ETHER_ON_FCC		/* define if ether on FCC       */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define	CONFIG_ETHER_INDEX    1		/* which SCC/FCC channel for ethernet */
#endif

#if defined(CONFIG_ETHER_ON_SCC) && (CONFIG_ETHER_INDEX == 1)

/*
 *  - RX clk is CLK11
 *  - TX clk is CLK12
 */
# define CFG_CMXSCR_VALUE	(CMXSCR_RS1CS_CLK11 | CMXSCR_TS1CS_CLK12)

#elif defined(CONFIG_ETHER_ON_FCC) && (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

#endif /* CONFIG_ETHER_ON_FCC, CONFIG_ETHER_INDEX */

#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT	2		/* Port C */

#if STK82xx_150
#define CFG_MDIO_PIN	0x00008000	/* PC16 */
#define CFG_MDC_PIN	0x00004000	/* PC17 */
#endif

#if STK82xx_100
#define CFG_MDIO_PIN	0x00000002	/* PC30 */
#define CFG_MDC_PIN	0x00000001	/* PC31 */
#endif

#if 1
#define MDIO_ACTIVE	(iop->pdir |=  CFG_MDIO_PIN)
#define MDIO_TRISTATE	(iop->pdir &= ~CFG_MDIO_PIN)
#define MDIO_READ	((iop->pdat &  CFG_MDIO_PIN) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  CFG_MDIO_PIN; \
			else	iop->pdat &= ~CFG_MDIO_PIN

#define MDC(bit)	if(bit) iop->pdat |=  CFG_MDC_PIN; \
			else	iop->pdat &= ~CFG_MDC_PIN
#else
#define MDIO_ACTIVE	({unsigned long tmp; tmp = iop->pdir; tmp |=  CFG_MDIO_PIN; iop->pdir = tmp;})
#define MDIO_TRISTATE	({unsigned long tmp; tmp = iop->pdir; tmp &= ~CFG_MDIO_PIN; iop->pdir = tmp;})
#define MDIO_READ	((iop->pdat &  CFG_MDIO_PIN) != 0)

#define MDIO(bit)	if(bit) {unsigned long tmp; tmp = iop->pdat; tmp |=  CFG_MDIO_PIN; iop->pdat = tmp;}\
			else	{unsigned long tmp; tmp = iop->pdat; tmp &= ~CFG_MDIO_PIN; iop->pdat = tmp;}

#define MDC(bit)	if(bit) {unsigned long tmp; tmp = iop->pdat; tmp |=  CFG_MDC_PIN; iop->pdat = tmp;}\
			else	{unsigned long tmp; tmp = iop->pdat; tmp &= ~CFG_MDC_PIN; iop->pdat = tmp;}
#endif

#define MIIDELAY	udelay(1)


/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	66666666	/* in Hz */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT|CONFIG_BOOTP_BOOTFILESIZE)


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_I2C
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SNTP


/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#if 0
#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CFG_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif
#endif

#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END	0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CFG_LOAD_ADDR	0x300000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define	CFG_RESET_ADDRESS 0x40000104	/* "bad" address		*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * CAN stuff
 *-----------------------------------------------------------------------
 */
#define CFG_CAN_BASE	0x51000000
#define	CFG_CAN_SIZE	1
#define CFG_CAN_BR	((CFG_CAN_BASE & BRx_BA_MSK)	|\
			 BRx_PS_8			|\
			 BRx_MS_UPMC			|\
			 BRx_V)

#define CFG_CAN_OR	(MEG_TO_AM(CFG_CAN_SIZE)	|\
			 ORxU_BI)


/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain TEXT_BASE from board/tqm8272/config.mk
 * The main FLASH is whichever is connected to *CS0.
 */
#define CFG_FLASH0_BASE 0x40000000
#define CFG_FLASH0_SIZE 32	/* 32 MB */

/* Flash bank size (for preliminary settings)
 */
#define CFG_FLASH_SIZE CFG_FLASH0_SIZE

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CFG_FLASH_CFI				/* flash is CFI compat.	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector	*/
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash*/

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

#define CFG_UPDATE_FLASH_SIZE

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x40000)
#define CFG_ENV_SIZE		0x20000
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	0x20000

/* Where is the Hardwareinformation Block (from Monitor Sources) */
#define MON_RES_LENGTH		(0x0003FC00)
#define HWIB_INFO_START_ADDR    (CFG_FLASH_BASE + MON_RES_LENGTH)
#define HWIB_INFO_LEN           512
#define CIB_INFO_START_ADDR     (CFG_FLASH_BASE + MON_RES_LENGTH + HWIB_INFO_LEN)
#define CIB_INFO_LEN            512

#define CFG_HWINFO_OFFSET	0x3fc00	/* offset of HW Info block */
#define CFG_HWINFO_SIZE		0x00000060	/* size   of HW Info block */
#define CFG_HWINFO_MAGIC	0x54514D38	/* 'TQM8' */

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_NAND)

#define CFG_NAND_CS_DIST		0x80
#define CFG_NAND_UPM_WRITE_CMD_OFS	0x20
#define CFG_NAND_UPM_WRITE_ADDR_OFS	0x40

#define CFG_NAND_BR	((CFG_NAND0_BASE & BRx_BA_MSK)	|\
			 BRx_PS_8			|\
			 BRx_MS_UPMB			|\
			 BRx_V)

#define CFG_NAND_OR	(MEG_TO_AM(CFG_NAND_SIZE)	|\
			 ORxU_BI			|\
			 ORxU_EHTR_8IDLE)

#define CFG_NAND_SIZE	1
#define CFG_NAND0_BASE 0x50000000
#define CFG_NAND1_BASE (CFG_NAND0_BASE + CFG_NAND_CS_DIST)
#define CFG_NAND2_BASE (CFG_NAND1_BASE + CFG_NAND_CS_DIST)
#define CFG_NAND3_BASE (CFG_NAND2_BASE + CFG_NAND_CS_DIST)

#define CFG_MAX_NAND_DEVICE     4       /* Max number of NAND devices           */
#define NAND_MAX_CHIPS 1

#define CFG_NAND_BASE_LIST { CFG_NAND0_BASE, \
			     CFG_NAND1_BASE, \
			     CFG_NAND2_BASE, \
			     CFG_NAND3_BASE, \
			   }

#define WRITE_NAND(d, adr) do{ *(volatile __u8 *)((unsigned long)(adr)) = (__u8)d; } while(0)
#define READ_NAND(adr) ((volatile unsigned char)(*(volatile __u8 *)(unsigned long)(adr)))
#define WRITE_NAND_UPM(d, adr, off) do \
{ \
	volatile unsigned char *addr = (unsigned char *) (adr + off); \
	WRITE_NAND(d, addr); \
} while(0)

#endif /* CFG_CMD_NAND */

#define	CONFIG_PCI
#ifdef CONFIG_PCI
#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_PCI_PNP
#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8		/* use 8 rx buffer on eepro100	*/
#define CONFIG_PCI_SCAN_SHOW
#endif

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CFG_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CFG_SIUMCR.
 */
#if 0
#define	__HRCW__ALL__		(HRCW_CIP | HRCW_ISB111 | HRCW_BMS)

#  define CFG_HRCW_MASTER	(__HRCW__ALL__ | HRCW_MODCK_H0111)
#else
#define CFG_HRCW_MASTER	(HRCW_BPS11 | HRCW_ISB111 | HRCW_BMS | HRCW_MODCK_H0111)
#endif

/* no slaves so just fill with zeros */
#define CFG_HRCW_SLAVE1		0
#define CFG_HRCW_SLAVE2		0
#define CFG_HRCW_SLAVE3		0
#define CFG_HRCW_SLAVE4		0
#define CFG_HRCW_SLAVE5		0
#define CFG_HRCW_SLAVE6		0
#define CFG_HRCW_SLAVE7		0

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2000  /* End of used area in DPRAM    */
#define CFG_GBL_DATA_SIZE	128 /* size in bytes reserved for initial data*/
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		CFG_FLASH0_BASE
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

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
#define CFG_CACHELINE_SIZE      32      /* For MPC8260 CPU              */
#if defined(CONFIG_CMD_KGDB)
# define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
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
#define CFG_HID0_INIT   (HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL  (HID0_IFEM|HID0_ABE)
#define CFG_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register                                     5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR         RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR_60x         (BCR_EBM|BCR_NPQM0|BCR_NPQM2)	/* 60x mode  */
#define BCR_APD01	0x10000000
#define CFG_BCR_SINGLE		(BCR_APD01|BCR_ETM)	/* 8260 mode */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_BOARD_GET_CPU_CLK_F)
#define CFG_SIUMCR_LOW		(SIUMCR_DPPC00)
#define CFG_SIUMCR_HIGH		(SIUMCR_DPPC00 | SIUMCR_ABE)
#else
#define CFG_SIUMCR		(SIUMCR_DPPC00)
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control                             4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR       (SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR       (SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control                     4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC     (TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control                 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR       (PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control                                   9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CFG_SCCR        SCCR_DFBRG01

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR        0

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM    32 bit  FLASH
 *  1   60x     SDRAM   64 bit  SDRAM
 *  2   60x	UPMB	 8 bit	NAND
 *  3   60x	UPMC	 8 bit	CAN
 *
 */

/* Initialize SDRAM
	 */
#undef CFG_INIT_LOCAL_SDRAM		/* No SDRAM on Local Bus */

#define SDRAM_MAX_SIZE	0x20000000	/* max. 512 MB		*/

/* Minimum mask to separate preliminary
 * address ranges for CS[0:2]
 */
#define CFG_GLOBAL_SDRAM_LIMIT	(512<<20)	/* less than 512 MB */

#define CFG_MPTPR       0x4000

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
#define CFG_MRS_OFFS	0x00000110

/* Bank 0 - FLASH
 */
#define CFG_BR0_PRELIM  ((CFG_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_32                      |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CFG_OR0_PRELIM  (MEG_TO_AM(CFG_FLASH_SIZE)      |\
			 ORxG_CSNT                      |\
			 ORxG_ACS_DIV4                  |\
			 ORxG_SCY_8_CLK                 |\
			 ORxG_TRLX)

/* SDRAM on TQM8272 can have either 8 or 9 columns.
 * The number affects configuration values.
 */

/* Bank 1 - 60x bus SDRAM
 */
#define CFG_PSRT        0x20	/* Low Value */
/* #define CFG_PSRT        0x10	 Fast Value */
#define CFG_LSRT        0x20	/* Local Bus */
#ifndef CFG_RAMBOOT
#define CFG_BR1_PRELIM  ((CFG_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CFG_OR1_PRELIM	CFG_OR1_8COL

/* SDRAM initialization values for 8-column chips
 */
#define CFG_OR1_8COL    ((~(CFG_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A7             |\
			 ORxS_NUMR_12)

#define CFG_PSDMR_8COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A15_IS_A5           |\
			 PSDMR_BSMA_A12_A14             |\
			 PSDMR_SDA10_PBI1_A8            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_EAMUX                    |\
			 PSDMR_BUFCMD			|\
			 PSDMR_CL_2)


/* SDRAM initialization values for 9-column chips
 */
#define CFG_OR1_9COL    ((~(CFG_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A5             |\
			 ORxS_NUMR_13)

#define CFG_PSDMR_9COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A16_IS_A5           |\
			 PSDMR_BSMA_A12_A14             |\
			 PSDMR_SDA10_PBI1_A7            |\
			 PSDMR_RFRC_7_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_EAMUX                    |\
			 PSDMR_BUFCMD			|\
			 PSDMR_CL_2)

#define CFG_OR1_10COL    ((~(CFG_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI1_A4             |\
			 ORxS_NUMR_13)

#define CFG_PSDMR_10COL  (PSDMR_PBI                      |\
			 PSDMR_SDAM_A17_IS_A5           |\
			 PSDMR_BSMA_A12_A14             |\
			 PSDMR_SDA10_PBI1_A4            |\
			 PSDMR_RFRC_6_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_2C                   |\
			 PSDMR_EAMUX                    |\
			 PSDMR_BUFCMD			|\
			 PSDMR_CL_2)

#define PSDMR_RFRC_66MHZ_SINGLE         0x00028000  /* PSDMR[RFRC] at 66 MHz single mode */
#define PSDMR_RFRC_100MHZ_SINGLE        0x00030000  /* PSDMR[RFRC] at 100 MHz single mode */
#define PSDMR_RFRC_133MHZ_SINGLE        0x00030000  /* PSDMR[RFRC] at 133 MHz single mode */
#define PSDMR_RFRC_66MHZ_60X            0x00030000  /* PSDMR[RFRC] at 66 MHz 60x mode */
#define PSDMR_RFRC_100MHZ_60X           0x00028000  /* PSDMR[RFRC] at 100 MHz 60x mode */
#define PSDMR_RFRC_DEFAULT              PSDMR_RFRC_133MHZ_SINGLE  /* PSDMR[RFRC] default value */

#define PSDMR_PRETOACT_66MHZ_SINGLE     0x00002000  /* PSDMR[PRETOACT] at 66 MHz single mode */
#define PSDMR_PRETOACT_100MHZ_SINGLE    0x00002000  /* PSDMR[PRETOACT] at 100 MHz single mode */
#define PSDMR_PRETOACT_133MHZ_SINGLE    0x00002000  /* PSDMR[PRETOACT] at 133 MHz single mode */
#define PSDMR_PRETOACT_66MHZ_60X        0x00001000  /* PSDMR[PRETOACT] at 66 MHz 60x mode */
#define PSDMR_PRETOACT_100MHZ_60X       0x00001000  /* PSDMR[PRETOACT] at 100 MHz 60x mode */
#define PSDMR_PRETOACT_DEFAULT          PSDMR_PRETOACT_133MHZ_SINGLE  /* PSDMR[PRETOACT] default value */

#define PSDMR_WRC_66MHZ_SINGLE          0x00000020  /* PSDMR[WRC] at 66 MHz single mode */
#define PSDMR_WRC_100MHZ_SINGLE         0x00000020  /* PSDMR[WRC] at 100 MHz single mode */
#define PSDMR_WRC_133MHZ_SINGLE         0x00000010  /* PSDMR[WRC] at 133 MHz single mode */
#define PSDMR_WRC_66MHZ_60X             0x00000010  /* PSDMR[WRC] at 66 MHz 60x mode */
#define PSDMR_WRC_100MHZ_60X            0x00000010  /* PSDMR[WRC] at 100 MHz 60x mode */
#define PSDMR_WRC_DEFAULT               PSDMR_WRC_133MHZ_SINGLE  /* PSDMR[WRC] default value */

#define PSDMR_BUFCMD_66MHZ_SINGLE       0x00000000  /* PSDMR[BUFCMD] at 66 MHz single mode */
#define PSDMR_BUFCMD_100MHZ_SINGLE      0x00000000  /* PSDMR[BUFCMD] at 100 MHz single mode */
#define PSDMR_BUFCMD_133MHZ_SINGLE      0x00000004  /* PSDMR[BUFCMD] at 133 MHz single mode */
#define PSDMR_BUFCMD_66MHZ_60X          0x00000000  /* PSDMR[BUFCMD] at 66 MHz 60x mode */
#define PSDMR_BUFCMD_100MHZ_60X         0x00000000  /* PSDMR[BUFCMD] at 100 MHz 60x mode */
#define PSDMR_BUFCMD_DEFAULT            PSDMR_BUFCMD_133MHZ_SINGLE  /* PSDMR[BUFCMD] default value */

#endif /* CFG_RAMBOOT */

#endif	/* __CONFIG_H */
