/*
 * Copyright (C) 2006 Mihai Georgian <u-boot@linuxnotincluded.org.uk>
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

#if 0
#define DEBUG
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

/*-----------------------------------------------------------------------
 * User configurable settings:
 *   Mandatory settings:
 *     CONFIG_IPADDR_LS		- the IP address of the LinkStation
 *     CONFIG_SERVERIP_LS	- the address of the server for NFS/TFTP/DHCP/BOOTP
 *   Optional settins:
 *     CONFIG_NCIP_LS		- the adress of the computer running net console
 *							  if not configured, it will be set to
 *							  CONFIG_SERVERIP_LS
 */


#define CONFIG_IPADDR_LS	192.168.11.150
#define CONFIG_SERVERIP_LS	192.168.11.149

#if !defined(CONFIG_IPADDR_LS) || !defined(CONFIG_SERVERIP_LS)
#error Both CONFIG_IPADDR_LS and CONFIG_SERVERIP_LS must be defined
#endif

#if !defined(CONFIG_NCIP_LS)
#define CONFIG_NCIP_LS		CONFIG_SERVERIP_LS
#endif

/*----------------------------------------------------------------------
 * DO NOT CHANGE ANYTHING BELOW, UNLESS YOU KNOW WHAT YOU ARE DOING
 *---------------------------------------------------------------------*/

#define CONFIG_MPC8245		1
#define CONFIG_LINKSTATION	1

/*---------------------------------------
 * Supported models
 *
 * LinkStation HDLAN /KuroBox Standard (CONFIG_HLAN)
 * LinkStation old model               (CONFIG_LAN) - totally untested
 * LinkStation HGLAN / KuroBox HG      (CONFIG_HGLAN)
 *
 * Models not supported yet
 * TeraStatin                          (CONFIG_HTGL)
 */

#if defined(CONFIG_HLAN) || defined(CONFIG_LAN)
#define CONFIG_IDENT_STRING		" LinkStation / KuroBox"
#elif defined(CONFIG_HGLAN)
#define CONFIG_IDENT_STRING		" LinkStation HG / KuroBox HG"
#elif defined(CONFIG_HTGL)
#define CONFIG_IDENT_STRING		" TeraStation"
#else
#error No LinkStation model defined
#endif

#define CONFIG_BOOTDELAY	5
#define CONFIG_ZERO_BOOTDELAY_CHECK
#undef CONFIG_BOOT_RETRY_TIME

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		\
	"Boot in %02d seconds ('s' to stop)...", bootdelay
#define CONFIG_AUTOBOOT_STOP_STR	"s"

#define CONFIG_CMD_IDE
#define CONFIG_CMD_PCI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_EXT2

#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_ALL

#define CONFIG_OF_LIBFDT	1

#define OF_STDOUT_PATH		"/soc10x/serial@80004600"

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <config_cmd_default.h>

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/

#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16		/* Max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_LOAD_ADDR		0x00800000	/* Default load address: 8 MB	*/

#define CONFIG_BOOTCOMMAND	"run bootcmd1"
#define CONFIG_BOOTARGS		"root=/dev/sda1 console=ttyS1,57600 netconsole=@192.168.1.7/eth0,@192.168.1.1/00:50:BF:A4:59:71 rtc-rs5c372.probe=0,0x32 debug"
#define CONFIG_NFSBOOTCOMMAND	"bootp;run nfsargs;bootm"

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define XMK_STR(x)		#x
#define MK_STR(x)		XMK_STR(x)

#if defined(CONFIG_HLAN) || defined(CONFIG_LAN)
#define UBFILE			"share/u-boot/u-boot-hd.flash.bin"
#elif defined(CONFIG_HGLAN)
#define UBFILE			"share/u-boot/u-boot-hg.flash.bin"
#elif defined(CONFIG_HTGL)
#define UBFILE			"share/u-boot/u-boot-ht.flash.bin"
#else
#error No LinkStation model defined
#endif

#define CONFIG_EXTRA_ENV_SETTINGS						\
	"autoload=no\0"								\
	"stdin=nc\0"								\
	"stdout=nc\0"								\
	"stderr=nc\0"								\
	"ipaddr="MK_STR(CONFIG_IPADDR_LS)"\0"					\
	"netmask=255.255.255.0\0"						\
	"serverip="MK_STR(CONFIG_SERVERIP_LS)"\0"				\
	"ncip="MK_STR(CONFIG_NCIP_LS)"\0"					\
	"netretry=no\0"								\
	"nc=setenv stdin nc;setenv stdout nc;setenv stderr nc\0"		\
	"ser=setenv stdin serial;setenv stdout serial;setenv stderr serial\0"	\
	"ldaddr=800000\0"							\
	"hdpart=0:1\0"								\
	"hdfile=boot/uImage\0"							\
	"hdload=echo Loading ${hdpart}:${hdfile};ext2load ide ${hdpart} ${ldaddr} ${hdfile};ext2load ide ${hdpart} 7f0000 boot/kuroboxHG.dtb\0"	\
	"boothd=setenv bootargs " CONFIG_BOOTARGS ";bootm ${ldaddr} - 7f0000\0"	\
	"hdboot=run hdload;run boothd\0"					\
	"flboot=setenv bootargs root=/dev/hda1;bootm ffc00000\0"		\
	"emboot=setenv bootargs root=/dev/ram0;bootm ffc00000\0"		\
	"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off\0"	\
	"bootretry=30\0"							\
	"bootcmd1=run hdboot;run flboot\0"					\
	"bootcmd2=run flboot\0"							\
	"bootcmd3=run emboot\0"							\
	"writeng=protect off fff70000 fff7ffff;era fff70000 fff7ffff;mw.l 800000 4e474e47 1;cp.b 800000 fff70000 4\0" \
	"writeok=protect off fff70000 fff7ffff;era fff70000 fff7ffff;mw.l 800000 4f4b4f4b 1;cp.b 800000 fff70000 4\0" \
	"ubpart=0:3\0"								\
	"ubfile="UBFILE"\0"							\
	"ubload=echo Loading ${ubpart}:${ubfile};ext2load ide ${ubpart} ${ldaddr} ${ubfile}\0" \
	"ubsaddr=fff00000\0"							\
	"ubeaddr=fff2ffff\0"							\
	"ubflash=protect off ${ubsaddr} ${ubeaddr};era ${ubsaddr} ${ubeaddr};cp.b ${ldaddr} ${ubsaddr} ${filesize};cmp.b ${ldaddr} ${ubsaddr} ${filesize}\0" \
	"upgrade=run ubload ubflash\0"

/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI
/* Verified: CONFIG_PCI_PNP doesn't work */
#undef CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW

#ifndef CONFIG_PCI_PNP
/* Keep the following defines in sync with the BAT mappings */

#define PCI_ETH_IOADDR      0xbfff00
#define PCI_ETH_MEMADDR     0xbffffc00
#define PCI_IDE_IOADDR      0xbffed0
#define PCI_IDE_MEMADDR     0xbffffb00
#define PCI_USB0_IOADDR     0
#define PCI_USB0_MEMADDR    0xbfffe000
#define PCI_USB1_IOADDR     0
#define PCI_USB1_MEMADDR    0xbfffd000
#define PCI_USB2_IOADDR     0
#define PCI_USB2_MEMADDR    0xbfffcf00

#endif

/*-----------------------------------------------------------------------
 * Ethernet stuff
 */
#define CONFIG_NET_MULTI

#if defined(CONFIG_LAN) || defined(CONFIG_HLAN)
#define CONFIG_TULIP
#define CONFIG_TULIP_USE_IO
#elif defined(CONFIG_HGLAN) || defined(CONFIG_HTGL)
#define CONFIG_RTL8169
#endif

#define CONFIG_NET_RETRY_COUNT		5

#define CONFIG_NETCONSOLE

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

#define CONFIG_SYS_FLASH_BASE		0xFFC00000
#define CONFIG_SYS_FLASH_SIZE		0x00400000
#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100
#define CONFIG_SYS_EUMB_ADDR		0x80000000
#define CONFIG_SYS_PCI_MEM_ADDR	0xB0000000
#define CONFIG_SYS_MISC_REGION_ADDR	0xFE000000

#define CONFIG_SYS_MONITOR_LEN		0x00040000	/* 256 kB			*/
#define CONFIG_SYS_MALLOC_LEN		(512 << 10)	/* Reserve some kB for malloc()	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x00800000	/* 1M ... 8M in DRAM		*/

/* Maximum amount of RAM */
#if defined(CONFIG_HLAN) || defined(CONFIG_LAN)
#define CONFIG_SYS_MAX_RAM_SIZE	0x04000000	/* 64MB of SDRAM  */
#elif defined(CONFIG_HGLAN) || defined(CONFIG_HTGL)
#define CONFIG_SYS_MAX_RAM_SIZE	0x08000000	/* 128MB of SDRAM */
#else
#error Unknown LinkStation type
#endif

/*-----------------------------------------------------------------------
 * Change TEXT_BASE in bord/linkstation/config.mk to get a RAM build
 *
 * RAM based builds are for testing purposes. A Linux module, uloader.o,
 * exists to load U-Boot and pass control to it
 *
 * Always do "make clean" after changing the build type
 */
#if CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_RAMBOOT
#endif

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#if 1 /* RAM is available when the first C function is called */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_MAX_RAM_SIZE - 0x1000)
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#endif
#define CONFIG_SYS_INIT_RAM_END	0x1000
#define CONFIG_SYS_GBL_DATA_SIZE	128
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)

/*----------------------------------------------------------------------
 * Serial configuration
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		57600
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_EUMB_ADDR + 0x4600)	/* Console port	*/
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_EUMB_ADDR + 0x4500)	/* AVR port	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8245 user's manual.
 *
 * Unless indicated otherwise, the values are
 * taken from the orignal Linkstation boot code
 *
 * Most of the low level configuration setttings are normally used
 * in cpu/mpc824x/cpu_init.c which is NOT used by this implementation.
 * Low level initialisation is done in board/linkstation/early_init.S
 * The values below are included for reference purpose only
 */

/* FIXME: 32.768 MHz is the crystal frequency but */
/* the real frequency is lower by about 0.75%     */
#define CONFIG_SYS_CLK_FREQ	32768000
#define CONFIG_SYS_HZ			1000

/* Bit-field values for MCCR1.  */
#define CONFIG_SYS_ROMNAL      0
#define CONFIG_SYS_ROMFAL      11

#define CONFIG_SYS_BANK0_ROW	2       /* Only bank 0 used: 13 x n x 4 */
#define CONFIG_SYS_BANK1_ROW	0
#define CONFIG_SYS_BANK2_ROW	0
#define CONFIG_SYS_BANK3_ROW	0
#define CONFIG_SYS_BANK4_ROW	0
#define CONFIG_SYS_BANK5_ROW	0
#define CONFIG_SYS_BANK6_ROW	0
#define CONFIG_SYS_BANK7_ROW	0

/* Bit-field values for MCCR2.  */
#define CONFIG_SYS_TSWAIT      0
#if defined(CONFIG_LAN) || defined(CONFIG_HLAN)
#define CONFIG_SYS_REFINT      0x15e0
#elif defined(CONFIG_HGLAN) || defined(CONFIG_HTGL)
#define CONFIG_SYS_REFINT      0x1580
#endif

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4. */
#define CONFIG_SYS_BSTOPRE	0x91c

/* Bit-field values for MCCR3.  */
#define CONFIG_SYS_REFREC      7

/* Bit-field values for MCCR4.  */
#define CONFIG_SYS_PRETOACT		2
#define CONFIG_SYS_ACTTOPRE		2	/* Original value was 2	*/
#define CONFIG_SYS_ACTORW		2
#if defined(CONFIG_LAN) || defined(CONFIG_HLAN)
#define CONFIG_SYS_SDMODE_CAS_LAT	2	/* For 100MHz bus	*/
/*#define CONFIG_SYS_SDMODE_BURSTLEN	3*/
#elif defined(CONFIG_HGLAN) || defined(CONFIG_HTGL)
#define CONFIG_SYS_SDMODE_CAS_LAT	3	/* For 133MHz bus	*/
/*#define CONFIG_SYS_SDMODE_BURSTLEN	2*/
#endif
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER 1
#define CONFIG_SYS_EXTROM		1	/* Original setting but there is no EXTROM */
#define CONFIG_SYS_REGDIMM		0
#define CONFIG_SYS_DBUS_SIZE2		1
#define CONFIG_SYS_SDMODE_WRAP		0

#define CONFIG_SYS_PGMAX		0x32	/* All boards use this setting. Original 0x92 */
#define CONFIG_SYS_SDRAM_DSCD		0x30

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */

#define CONFIG_SYS_BANK0_START	    0x00000000
#define CONFIG_SYS_BANK0_END	    (CONFIG_SYS_MAX_RAM_SIZE - 1)
#define CONFIG_SYS_BANK0_ENABLE    1
#define CONFIG_SYS_BANK1_START     0x3ff00000
#define CONFIG_SYS_BANK1_END       0x3fffffff
#define CONFIG_SYS_BANK1_ENABLE    0
#define CONFIG_SYS_BANK2_START     0x3ff00000
#define CONFIG_SYS_BANK2_END       0x3fffffff
#define CONFIG_SYS_BANK2_ENABLE    0
#define CONFIG_SYS_BANK3_START     0x3ff00000
#define CONFIG_SYS_BANK3_END       0x3fffffff
#define CONFIG_SYS_BANK3_ENABLE    0
#define CONFIG_SYS_BANK4_START     0x3ff00000
#define CONFIG_SYS_BANK4_END       0x3fffffff
#define CONFIG_SYS_BANK4_ENABLE    0
#define CONFIG_SYS_BANK5_START     0x3ff00000
#define CONFIG_SYS_BANK5_END       0x3fffffff
#define CONFIG_SYS_BANK5_ENABLE    0
#define CONFIG_SYS_BANK6_START     0x3ff00000
#define CONFIG_SYS_BANK6_END       0x3fffffff
#define CONFIG_SYS_BANK6_ENABLE    0
#define CONFIG_SYS_BANK7_START     0x3ff00000
#define CONFIG_SYS_BANK7_END       0x3fffffff
#define CONFIG_SYS_BANK7_ENABLE    0

#define CONFIG_SYS_ODCR	    0x15

/*----------------------------------------------------------------------
 * Initial BAT mappings
 */

/* NOTES:
 * 1) GUARDED and WRITETHROUGH not allowed in IBATS
 * 2) CACHEINHIBIT and WRITETHROUGH not allowed together in same BAT
 */

/* SDRAM */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_128M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

/* EUMB: 1MB of address space */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_EUMB_ADDR | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_EUMB_ADDR | BATU_BL_1M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT1L	(CONFIG_SYS_IBAT1L | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* PCI Mem: 256MB of address space */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PCI_MEM_ADDR | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PCI_MEM_ADDR | BATU_BL_256M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_IBAT2L | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* PCI and local ROM/Flash: last 32MB of address space */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_MISC_REGION_ADDR | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_MISC_REGION_ADDR | BATU_BL_32M | BATU_VS | BATU_VP)

#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_IBAT3L | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 *
 * FIXME: This doesn't appear to be true for the newer kernels
 * which map more that 8 MB
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_CFI			/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/

#undef  CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	72	/* Max number of sectors per flash	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	12000
#define CONFIG_SYS_FLASH_WRITE_TOUT	1000

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CONFIG_ENV_IS_IN_FLASH
/*
 * The original LinkStation flash organisation uses
 * 448 kB (0xFFF00000 - 0xFFF6FFFF) for the boot loader
 * We use the last sector of this area to store the environment
 * which leaves max. 384 kB for the U-Boot itself
 */
#define CONFIG_ENV_ADDR		0xFFF60000
#define CONFIG_ENV_SIZE		0x00010000
#define CONFIG_ENV_SECT_SIZE	0x00010000

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * IDE/ATA definitions
 */
#undef  CONFIG_IDE_LED				/* No IDE LED			*/
#define CONFIG_IDE_RESET			/* no reset for ide supported	*/
#define CONFIG_IDE_PREINIT			/* check for units		*/
#define CONFIG_LBA48				/* 48 bit LBA supported		*/

#if defined(CONFIG_LAN) || defined(CONFIG_HLAN) || defined(CONFIG_HGLAN)
#define CONFIG_SYS_IDE_MAXBUS		1		/* Scan only 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1		/* Only 1 drive per IDE bus	*/
#elif defined(CONFIG_HGTL)
#define CONFIG_SYS_IDE_MAXBUS		2		/* Max. 2 IDE busses		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2		/* max. 2 drives per IDE bus	*/
#else
#error Config IDE: Unknown LinkStation type
#endif

#define CONFIG_SYS_ATA_BASE_ADDR	0

#define CONFIG_SYS_ATA_DATA_OFFSET	0		/* Offset for data I/O		*/
#define CONFIG_SYS_ATA_REG_OFFSET	0		/* Offset for normal registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0		/* Offset for alternate registers */

/*-----------------------------------------------------------------------
 * Partitions and file system
 */
#define CONFIG_DOS_PARTITION

/*-----------------------------------------------------------------------
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#endif	/* __CONFIG_H */
