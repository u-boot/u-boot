/*
 * (C) Copyright 2005
 * Heiko Schocher, DENX Software Engineering, <hs@denx.de>
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
#define CONFIG_MPC8272_FAMILY	1
#define CONFIG_IDS8247		1
#define CPU_ID_STR		"MPC8247"
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define	CONFIG_SYS_TEXT_BASE	0xfff00000

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define	CONFIG_BOOTCOUNT_LIMIT

#define CONFIG_PREBOOT	"echo;echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw "			\
	"console=ttyS0,115200\0"					\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/IDS8247/uImage\0"				\
	"kernel_addr=ff800000\0"					\
	"ramdisk_addr=ffa00000\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_MISC_INIT_R	1

/* enable I2C and select the hardware/software driver */
#undef  CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * Software (bit-bang) I2C driver configuration
 */

#define I2C_PORT	0		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00000080)
#define I2C_TRISTATE	(iop->pdir &= ~0x00000080)
#define I2C_READ	((iop->pdat & 0x00000080) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00000080; \
			else    iop->pdat &= ~0x00000080
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00000100; \
			else    iop->pdat &= ~0x00000100
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

#if 0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

#define CONFIG_I2C_X
#endif

/*
 * select serial console configuration
 * use the extern UART for the console
 */
#define	CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE    1

#define CONFIG_SYS_NS16550_CLK         14745600

#define	CONFIG_SYS_UART_BASE	0xE0000000
#define CONFIG_SYS_UART_SIZE	0x10000

#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_UART_BASE + 0x8000)


/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_TBCLK	(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH	"/soc@f0000000/serial8250@e0008000"


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
#define CONFIG_ETHER_ON_FCC1
#define FCC_ENET

/*
 * - Rx-CLK is CLK10
 * - Tx-CLK is CLK9
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC1|CMXFCR_RF1CS_MSK|CMXFCR_TF1CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF1CS_CLK10|CMXFCR_TF1CS_CLK9)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)


/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	66666666	/* in Hz */

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

#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_I2C
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

#define	CONFIG_SYS_RESET_ADDRESS 0xFDFFFFFC	/* "bad" address		*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ        (8 << 20)       /* Initial Memory map for Linux */

#define CONFIG_SYS_FLASH_CFI				/* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver        */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ 0xFF800000 }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks	      */
/* What should the base address of the main FLASH be and how big is
 * it (in MBytes)? This must contain CONFIG_SYS_TEXT_BASE from board/ids8247/config.mk
 * The main FLASH is whichever is connected to *CS0.
 */
#define CONFIG_SYS_FLASH0_BASE 0xFFF00000
#define CONFIG_SYS_FLASH0_SIZE 8

/* Flash bank size (for preliminary settings)
 */
#define CONFIG_SYS_FLASH_SIZE CONFIG_SYS_FLASH0_SIZE

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

/* Environment in flash */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+0x60000)
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_SECT_SIZE	0x20000

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------
 */
#if defined(CONFIG_CMD_NAND)

#define CONFIG_SYS_NAND0_BASE 0xE1000000
#define CONFIG_SYS_MAX_NAND_DEVICE     1       /* Max number of NAND devices           */

#endif /* CONFIG_CMD_NAND */

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CONFIG_SYS_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CONFIG_SYS_SIUMCR.
 */
#define CONFIG_SYS_HRCW_MASTER	(HRCW_BPS01 | HRCW_BMS | HRCW_ISB100 | HRCW_APPC10 | HRCW_MODCK_H1000)

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
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000  /* Size of used area in DPRAM    */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 *
 * 60x SDRAM is mapped at CONFIG_SYS_SDRAM_BASE
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_FLASH0_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

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

#define CONFIG_SYS_HID0_INIT   (HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI)
#define CONFIG_SYS_HID0_FINAL  0
#define CONFIG_SYS_HID2        0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register                                     5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CONFIG_SYS_RMR         0

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration                                       4-25
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_BCR		0

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration                             4-31
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SIUMCR      (SIUMCR_DPPC00|SIUMCR_APPC10|SIUMCR_BCTLC01)

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
#define CONFIG_SYS_SCCR        (0x00000028 | SCCR_DFBRG01)

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
 *  0   60x     GPCM    16 bit  FLASH
 *  1   60x     GPCM     8 bit  NAND
 *  2   60x     SDRAM   32 bit  SDRAM
 *  3   60x     GPCM     8 bit  UART
 *
 */

#define SDRAM_MAX_SIZE	0x08000000	/* max. 128 MB		*/

/* Minimum mask to separate preliminary
 * address ranges for CS[0:2]
 */
#define CONFIG_SYS_GLOBAL_SDRAM_LIMIT	(32<<20)	/* less than 32 MB */

#define CONFIG_SYS_MPTPR       0x6600

/*-----------------------------------------------------------------------------
 * Address for Mode Register Set (MRS) command
 *-----------------------------------------------------------------------------
 */
#define CONFIG_SYS_MRS_OFFS	0x00000110


/* Bank 0 - FLASH
 */
#define CONFIG_SYS_BR0_PRELIM  ((CONFIG_SYS_FLASH_BASE & BRx_BA_MSK)  |\
			 BRx_PS_8                       |\
			 BRx_MS_GPCM_P                  |\
			 BRx_V)

#define CONFIG_SYS_OR0_PRELIM  (MEG_TO_AM(CONFIG_SYS_FLASH_SIZE)      |\
			 ORxG_SCY_6_CLK                 )

#if defined(CONFIG_CMD_NAND)
/* Bank 1 - NAND Flash
*/
#define	CONFIG_SYS_NAND_BASE		CONFIG_SYS_NAND0_BASE
#define	CONFIG_SYS_NAND_SIZE		0x8000

#define CONFIG_SYS_OR_TIMING_NAND	0x000036

#define CONFIG_SYS_BR1_PRELIM  ((CONFIG_SYS_NAND_BASE & BRx_BA_MSK) | BRx_PS_8 | BRx_MS_GPCM_P | BRx_V  )
#define CONFIG_SYS_OR1_PRELIM  (P2SZ_TO_AM(CONFIG_SYS_NAND_SIZE) | CONFIG_SYS_OR_TIMING_NAND )
#endif

/* Bank 2 - 60x bus SDRAM
 */
#define CONFIG_SYS_PSRT        0x20
#define CONFIG_SYS_LSRT        0x20

#define CONFIG_SYS_BR2_PRELIM  ((CONFIG_SYS_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_32                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CONFIG_SYS_OR2_PRELIM	CONFIG_SYS_OR2


/* SDRAM initialization values
*/
#define CONFIG_SYS_OR2    ((~(CONFIG_SYS_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_4                     |\
			 ORxS_ROWST_PBI0_A9		|\
			 ORxS_NUMR_12)

#define CONFIG_SYS_PSDMR  (PSDMR_SDAM_A14_IS_A5 |\
			 PSDMR_BSMA_A15_A17           |\
			 PSDMR_SDA10_PBI0_A10		|\
			 PSDMR_RFRC_5_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_BL                       |\
			 PSDMR_LDOTOPRE_2C              |\
			 PSDMR_WRC_3C                   |\
			 PSDMR_CL_3)

/* Bank 3 - UART
*/

#define CONFIG_SYS_BR3_PRELIM  ((CONFIG_SYS_UART_BASE & BRx_BA_MSK) | BRx_PS_8 | BRx_MS_GPCM_P | BRx_V  )
#define CONFIG_SYS_OR3_PRELIM  (((-CONFIG_SYS_UART_SIZE) & ORxG_AM_MSK) | ORxG_CSNT | ORxG_SCY_1_CLK | ORxG_TRLX )

#endif	/* __CONFIG_H */
