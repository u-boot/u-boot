/*
 * (C) Copyright 2000, 2001, 2002
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
 * board/config.h - configuration options, board specific,
 *                  for SinoVee Microsystems SC8xx series SBC
 *                  http://www.fel.com.cn (Chinese)
 *                  http://www.sinovee.com (English)
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	CONFIG_SYS_TEXT_BASE	0x40000000

/* Custom configuration */
/* SC823,SC850,SC860SAR, FEL8xx-AT(823/850/860) */
/* SC85T,SC860T, FEL8xx-AT(855T/860T) */
/*#define CONFIG_FEL8xx_AT */
/*#define CONFIG_LCD */
/* if core > 50MHz , un-comment CONFIG_BUS_DIV2 */
/* #define CONFIG_50MHz */
/* #define CONFIG_66MHz */
/* #define CONFIG_75MHz */
#define CONFIG_80MHz
/*#define CONFIG_100MHz */
/* #define CONFIG_BUS_DIV2	1 */
/* for BOOT device port size */
/* #define CONFIG_BOOT_8B */
#define CONFIG_BOOT_16B
/* #define CONFIG_BOOT_32B */
/* #define CONFIG_CAN_DRIVER */
/* #define DEBUG */
#define CONFIG_FEC_ENET

/* #define CONFIG_SDRAM_16M */
#define CONFIG_SDRAM_32M
/* #define CONFIG_SDRAM_64M */
#define CONFIG_SYS_RESET_ADDRESS 0xffffffff
/*
 * High Level Configuration Options
 * (easy to change)
 */

/* #define CONFIG_MPC823		1 */
/* #define CONFIG_MPC850		1 */
#define CONFIG_MPC855		1
/* #define CONFIG_MPC860		1 */
/* #define CONFIG_MPC860T		1 */

#undef	CONFIG_WATCHDOG			/* watchdog */

#define CONFIG_SVM_SC8xx		1	/* ...on SVM SC8xx series	*/

#ifdef	CONFIG_LCD			/* with LCD controller ?	*/
/* #define CONFIG_NEC_NL6448BC20 1 / * use NEC NL6448BC20 display	*/
#endif

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		19200	/* console baudrate = 115kbps	*/
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	1	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_BOARD_TYPES      1       /* support board types          */

#define CONFIG_PREBOOT	"echo;echo Welcome to U-Boot SVM port;echo;echo Type \"? or help\" to get on-line help;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_EXTRA_ENV_SETTINGS                                       \
	"nfsargs=setenv bootargs root=/dev/nfs rw "                     \
	 "nfsroot=${serverip}:${rootpath}\0"                     \
	"ramargs=setenv bootargs root=/dev/ram rw\0"                    \
	"addip=setenv bootargs ${bootargs} "                            \
	       "ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"      \
		":${hostname}:${netdev}:off panic=1\0"                  \
		"flash_nfs=run nfsargs addip;"                                  \
	     "bootm ${kernel_addr}\0"                                \
	"flash_self=run ramargs addip;"                                 \
	       "bootm ${kernel_addr} ${ramdisk_addr}\0"                \
	"net_nfs=tftp 0x210000 ${bootfile};run nfsargs addip;bootm\0"     \
	"rootpath=/opt/sinovee/ppc8xx-linux-2.0/target\0"                                  \
	"bootfile=pImage-sc855t\0"                           \
	"kernel_addr=48000000\0"                                        \
	"ramdisk_addr=48100000\0"                                       \
	""
#define CONFIG_BOOTCOMMAND							\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"tftpboot 0x210000 pImage-sc855t;bootm 0x210000"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/


#ifdef CONFIG_LCD
# undef	 CONFIG_STATUS_LED		/* disturbs display		*/
#else
# define CONFIG_STATUS_LED	1	/* Status LED enabled		*/
#endif	/* CONFIG_LCD */

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DATE

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000

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
#define	CONFIG_SYS_MONITOR_LEN		(384 << 10)	/* Reserve 192 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	67	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1

#ifdef CONFIG_BOOT_8B
#define	CONFIG_ENV_OFFSET		0x10000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#elif defined (CONFIG_BOOT_16B)
#define	CONFIG_ENV_OFFSET		0x10000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#elif defined (CONFIG_BOOT_32B)
#define	CONFIG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#endif

/* Address and size of Redundant Environment Sector     */
#define CONFIG_ENV_OFFSET_REDUND   (CONFIG_ENV_OFFSET+CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND     (CONFIG_ENV_SIZE)


/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#define CONFIG_SYS_HWINFO_OFFSET	0x0003FFC0	/* offset of HW Info block */
#define CONFIG_SYS_HWINFO_SIZE		0x00000040	/* size   of HW Info block */
#define CONFIG_SYS_HWINFO_MAGIC	0x46454C38	/* 'SVM8' */

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
/*#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
*/
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR 0xffffff88
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#ifndef	CONFIG_CAN_DRIVER
/*#define CONFIG_SYS_SIUMCR 0x00610c00	*/
#define CONFIG_SYS_SIUMCR 0x00000000
#else	/* we must activate GPL5 in the SIUMCR for CAN */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01)
#endif	/* CONFIG_CAN_DRIVER */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	0x0001

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	0x00c3

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	0x0000

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 */
#if defined (CONFIG_100MHz)
#define CONFIG_SYS_PLPRCR 0x06301000
#define CONFIG_8xx_GCLK_FREQ 100000000
#elif defined (CONFIG_80MHz)
#define CONFIG_SYS_PLPRCR 0x04f01000
#define CONFIG_8xx_GCLK_FREQ 80000000
#elif defined(CONFIG_75MHz)
#define CONFIG_SYS_PLPRCR 0x04a00100
#define CONFIG_8xx_GCLK_FREQ 75000000
#elif defined(CONFIG_66MHz)
#define CONFIG_SYS_PLPRCR 0x04101000
#define CONFIG_8xx_GCLK_FREQ 66000000
#elif defined(CONFIG_50MHz)
#define CONFIG_SYS_PLPRCR 0x03101000
#define CONFIG_8xx_GCLK_FREQ 50000000
#endif

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#ifdef	CONFIG_BUS_DIV2
#define CONFIG_SYS_SCCR	0x02020000 | SCCR_RTSEL
#else			/* up to 50 MHz we use a 1:1 clock */
#define CONFIG_SYS_SCCR    0x02000000 | SCCR_RTSEL
#endif

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
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
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#undef	CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#define	CONFIG_IDE_8xx_DIRECT	1	/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CONFIG_SYS_ATA_BASE_ADDR       0xFE100010
#define CONFIG_SYS_ATA_IDE0_OFFSET     0x0000
/*#define CONFIG_SYS_ATA_IDE1_OFFSET     0x0C00 */
#define CONFIG_SYS_ATA_DATA_OFFSET     0x0000  /* Offset for data I/O
					   */
#define CONFIG_SYS_ATA_REG_OFFSET      0x0200  /* Offset for normal register accesses
					   */
#define CONFIG_SYS_ATA_ALT_OFFSET      0x0210  /* Offset for alternate registers
					   */
#define CONFIG_ATAPI
#define CONFIG_SYS_PIO_MODE 0

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CONFIG_SYS_DER	0x2002000F*/
#define CONFIG_SYS_DER	0x0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x60000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
#if defined(CONFIG_100MHz)
#define CONFIG_SYS_OR_TIMING_FLASH 0x000002f4
#define CONFIG_SYS_OR_TIMING_DOC   0x000002f4
#define CONFIG_SYS_MxMR_PTx 0x61000000
#define CONFIG_SYS_MPTPR 0x400

#elif  defined(CONFIG_80MHz)
#define CONFIG_SYS_OR_TIMING_FLASH 0x00000ff4
#define CONFIG_SYS_OR_TIMING_DOC   0x000001f4
#define CONFIG_SYS_MxMR_PTx 0x4e000000
#define CONFIG_SYS_MPTPR 0x400

#elif defined(CONFIG_75MHz)
#define CONFIG_SYS_OR_TIMING_FLASH 0x000008f4
#define CONFIG_SYS_OR_TIMING_DOC   0x000002f4
#define CONFIG_SYS_MxMR_PTx 0x49000000
#define CONFIG_SYS_MPTPR 0x400

#elif defined(CONFIG_66MHz)
#define CONFIG_SYS_OR_TIMING_FLASH     (OR_ACS_DIV1  | OR_TRLX | OR_CSNT_SAM | \
	OR_SCY_3_CLK | OR_EHTR | OR_BI)
/*#define CONFIG_SYS_OR_TIMING_FLASH 0x000001f4 */
#define CONFIG_SYS_OR_TIMING_DOC   0x000003f4
#define CONFIG_SYS_MxMR_PTx  0x40000000
#define CONFIG_SYS_MPTPR 0x400

#else		/*   50 MHz */
#define CONFIG_SYS_OR_TIMING_FLASH 0x00000ff4
#define CONFIG_SYS_OR_TIMING_DOC   0x000001f4
#define CONFIG_SYS_MxMR_PTx  0x30000000
#define CONFIG_SYS_MPTPR 0x400
#endif	/*CONFIG_??MHz */


#if  defined (CONFIG_BOOT_8B)   /* 512K X 8 ,29F040 , 2MB space */
#define CONFIG_SYS_OR0_PRELIM	(0xffe00000 | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V | BR_PS_8)
#elif  defined (CONFIG_BOOT_16B)   /* 29lv160 X 16 , 4MB space */
#define CONFIG_SYS_OR0_PRELIM	(0xffc00000 | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V | BR_PS_16)
#elif defined( CONFIG_BOOT_32B )  /* 29lv160 X 2 X 32, 4/8/16MB , 64MB space */
#define CONFIG_SYS_OR0_PRELIM	(0xfc000000 | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )
#else
#error Boot device port size missing.
#endif

/*
 * Disk-On-Chip configuration
 */

#define CONFIG_SYS_DOC_SHORT_TIMEOUT
#define CONFIG_SYS_MAX_DOC_DEVICE      1       /* Max number of DOC devices    */

#define CONFIG_SYS_DOC_SUPPORT_2000
#define CONFIG_SYS_DOC_SUPPORT_MILLENNIUM
#define CONFIG_SYS_DOC_BASE 0x80000000

#endif	/* __CONFIG_H */
