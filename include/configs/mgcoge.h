/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8247		1
#define CONFIG_MPC8272_FAMILY   1
#define CONFIG_MGCOGE		1

#define CONFIG_CPM2		1	/* Has a CPM2 */

#undef DEBUG

/*
 * Select serial console configuration
 *
 * If either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 */
#define	CONFIG_CONS_ON_SMC		/* Console is on SMC         */
#undef  CONFIG_CONS_ON_SCC		/* It's not on SCC           */
#undef	CONFIG_CONS_NONE		/* It's not on external UART */
#define CONFIG_CONS_INDEX	2	/* SMC2 is used for console  */

/*
 * Select ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected,
 * then CONFIG_ETHER_INDEX must be set to the channel number (1-4 for
 * SCC, 1-3 for FCC)
 *
 * If CONFIG_ETHER_NONE is defined, then either the ethernet routines
 * must be defined elsewhere (as for the console), or CONFIG_CMD_NET
 * must be unset.
 */
#define	CONFIG_ETHER_ON_SCC		/* Ethernet is on SCC */
#undef	CONFIG_ETHER_ON_FCC		/* Ethernet is not on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */

#define CONFIG_ETHER_INDEX	4
#define CFG_SCC_TOUT_LOOP	10000000

# define CFG_CMXSCR_VALUE	(CMXSCR_RS4CS_CLK7 | CMXSCR_TS4CS_CLK8)

#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	66000000	/* in Hz */
#endif

#define CONFIG_BAUDRATE		115200

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ECHO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

/*
 * Default environment settings
 */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"u-boot_addr=100000\0"						\
	"kernel_addr=200000\0"						\
	"fdt_addr=400000\0"						\
	"rootpath=/opt/eldk-4.2/ppc_82xx\0"				\
	"u-boot=/tftpboot/mgcoge/u-boot.bin\0"				\
	"bootfile=/tftpboot/mgcoge/uImage\0"				\
	"fdt_file=/tftpboot/mgcoge/mgcoge.dtb\0"			\
	"load=tftp ${u-boot_addr} ${u-boot}\0"				\
	"update=prot off fe000000 fe03ffff; era fe000000 fe03ffff; "	\
		"cp.b ${u-boot_addr} fe000000 ${filesize};"		\
		"prot on fe000000 fe03ffff\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"addcon=setenv bootargs ${bootargs} console=ttyCPM0,,${baudrate}\0"	\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
		"${netmask}:${hostname}:${netdev}:off panic=1 "		\
		"console=${console}\0"					\
	"net_nfs=tftp ${kernel_addr} ${bootfile}; "			\
		"tftp ${fdt_addr} ${fdt_file}; run nfsargs addip addcon;"\
		"bootm ${kernel_addr} - ${fdt_addr}\0"			\
	"net_self=tftp ${kernel_addr} ${bootfile}; "			\
		"tftp ${fdt_addr} ${fdt_file}; "			\
		"tftp ${ramdisk_addr} ${ramdisk_file}; "		\
		"run ramargs addip; "					\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0"	\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size  */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size  */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_SIZE		32
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks	*/
#define CFG_MAX_FLASH_SECT	256	/* max num of sects on one chip */

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256KB for Monitor */

#define CFG_ENV_IS_IN_FLASH

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#endif /* CFG_ENV_IS_IN_FLASH */

#define CFG_IMMR		0xF0000000

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/* Hard reset configuration word */
#define CFG_HRCW_MASTER		0x0604b211

/* No slaves */
#define CFG_HRCW_SLAVE1 	0
#define CFG_HRCW_SLAVE2 	0
#define CFG_HRCW_SLAVE3 	0
#define CFG_HRCW_SLAVE4 	0
#define CFG_HRCW_SLAVE5 	0
#define CFG_HRCW_SLAVE6 	0
#define CFG_HRCW_SLAVE7 	0

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot                  */

#define CFG_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE)

#define CFG_HID2		0

#define CFG_SIUMCR		0x4020c200
#define CFG_SYPCR		0xFFFFFFC3
#define CFG_BCR			0x10000000
#define CFG_SCCR		(SCCR_PCI_MODE | SCCR_PCI_MODCK)

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register                                     5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR         0

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
 * RCCR - RISC Controller Configuration                         13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR        0

/*
 * Init Memory Controller:
 *
 * Bank Bus     Machine PortSz  Device
 * ---- ---     ------- ------  ------
 *  0   60x     GPCM     8 bit  FLASH
 *  1   60x     SDRAM   32 bit  SDRAM
 *
 */
/* Bank 0 - FLASH
 */
#define CFG_BR0_PRELIM  ((CFG_FLASH_BASE & BRx_BA_MSK)	|\
			 BRx_PS_8			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CFG_OR0_PRELIM  (MEG_TO_AM(CFG_FLASH_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV2			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_TRLX )


/* Bank 1 - 60x bus SDRAM
 */
#define SDRAM_MAX_SIZE	0x08000000	/* max. 128 MB		*/
#define CFG_GLOBAL_SDRAM_LIMIT	(256 << 20)	/* less than 256 MB */

#define CFG_MPTPR       0x1800

/*-----------------------------------------------------------------------------
 * Address for Mode Register Set (MRS) command
 *-----------------------------------------------------------------------------
 */
#define CFG_MRS_OFFS	0x00000110
#define CFG_PSRT        0x0e

#define CFG_BR1_PRELIM  ((CFG_SDRAM_BASE & BRx_BA_MSK)  |\
			 BRx_PS_64                      |\
			 BRx_MS_SDRAM_P                 |\
			 BRx_V)

#define CFG_OR1_PRELIM	CFG_OR1

/* SDRAM initialization values
*/

#define CFG_OR1    ((~(CFG_GLOBAL_SDRAM_LIMIT-1) & ORxS_SDAM_MSK) |\
			 ORxS_BPD_8                     |\
			 ORxS_ROWST_PBI0_A7		|\
			 ORxS_NUMR_13)

#define CFG_PSDMR  (PSDMR_SDAM_A14_IS_A5 |\
			 PSDMR_BSMA_A14_A16           |\
			 PSDMR_SDA10_PBI0_A9		|\
			 PSDMR_RFRC_5_CLK               |\
			 PSDMR_PRETOACT_2W              |\
			 PSDMR_ACTTORW_2W               |\
			 PSDMR_LDOTOPRE_1C              |\
			 PSDMR_WRC_1C                   |\
			 PSDMR_CL_2)

#define	CFG_RESET_ADDRESS 0xFDFFFFFC	/* "bad" address		*/

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,8247@0"
#define OF_SOC			"soc@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc/cpm/serial@11a90"

#endif /* __CONFIG_H */
