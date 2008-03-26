/* Configuration header file for Gaisler GR-CPCI-AX2000
 * AX board. Note that since the AX is removable the configuration
 * for this board must be edited below.
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2008
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_LEON3		/* This is an LEON3 CPU */
#define CONFIG_LEON		1	/* This is an LEON CPU */
#define CONFIG_CPCI_AX2000	1	/* ... on GR-CPCI-AX2000 board */

#define CONFIG_LEON_RAM_SRAM 1
#define CONFIG_LEON_RAM_SDRAM 2
#define CONFIG_LEON_RAM_SDRAM_NOSRAM 3

/* Select Memory to run from
 *
 * SRAM          - UBoot is run in SRAM, SRAM-0x40000000, SDRAM-0x60000000
 * SDRAM         - UBoot is run in SDRAM, SRAM-0x40000000 and SDRAM-0x60000000
 * SDRAM_NOSRAM  - UBoot is run in SDRAM, SRAM not available, SDRAM at 0x40000000
 *
 * Note, if Linux is to be used, SDRAM or SDRAM_NOSRAM is required since
 * it doesn't fit into the 4Mb SRAM.
 *
 * SRAM is default since it will work for all systems, however will not
 * be able to boot linux.
 */
#define CONFIG_LEON_RAM_SELECT CONFIG_LEON_RAM_SRAM

/* CPU / AMBA BUS configuration */
#define CONFIG_SYS_CLK_FREQ 	20000000	/* 20MHz */

/* Number of SPARC register windows */
#define CFG_SPARC_NWINDOWS 8

/*
 * Serial console configuration
 */
#define CONFIG_BAUDRATE		38400	/* ... at 38400 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/* Partitions */
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION

/*
 * Supported commands
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_AMBAPP
#define CONFIG_CMD_PING
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_IRQ

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS_BASE					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs console=ttyS0,38400 root=/dev/nfs rw "	\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs console=ttyS0,${baudrate} root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"getkernel=tftpboot \$\(scratch\)\ \$\(bootfile\)\0" \
	"bootargs=console=ttyS0,38400 root=/dev/nfs rw nfsroot=192.168.0.20:/export/rootfs ip=192.168.0.206:192.168.0.20:192.168.0.1:255.255.255.0:ax2000:eth0\0"

#if CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SRAM
#define CONFIG_EXTRA_ENV_SETTINGS_SELECT \
	"net_nfs=tftp 40000000 ${bootfile};run nfsargs addip;bootm\0"	\
	"scratch=40200000\0"					\
	""
#elif CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SDRAM
#define CONFIG_EXTRA_ENV_SETTINGS_SELECT \
	"net_nfs=tftp 60000000 ${bootfile};run nfsargs addip;bootm\0"	\
	"scratch=60800000\0"					\
	""
#else
/* More than 4Mb is assumed when running from SDRAM */
#define CONFIG_EXTRA_ENV_SETTINGS_SELECT \
	"net_nfs=tftp 40000000 ${bootfile};run nfsargs addip;bootm\0"	\
	"scratch=40800000\0"					\
	""
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS CONFIG_EXTRA_ENV_SETTINGS_BASE CONFIG_EXTRA_ENV_SETTINGS_SELECT

#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_SERVERIP 192.168.0.20
#define CONFIG_IPADDR 192.168.0.206
#define CONFIG_ROOTPATH /export/rootfs
#define CONFIG_HOSTNAME  ax2000
#define CONFIG_BOOTFILE  /uImage

#define CONFIG_BOOTCOMMAND	"run flash_self"

/* Memory MAP
 *
 *  Flash:
 *  |--------------------------------|
 *  | 0x00000000 Text & Data & BSS   | *
 *  |            for Monitor         | *
 *  | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~| *
 *  | UNUSED / Growth                | * 256kb
 *  |--------------------------------|
 *  | 0x00050000 Base custom area    | *
 *  |            kernel / FS         | *
 *  |                                | * Rest of Flash
 *  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *  | END-0x00008000 Environment     | * 32kb
 *  |--------------------------------|
 *
 *
 *
 *  Main Memory (4Mb SRAM or XMb SDRAM):
 *  |--------------------------------|
 *  | UNUSED / scratch area          |
 *  |                                |
 *  |                                |
 *  |                                |
 *  |                                |
 *  |--------------------------------|
 *  | Monitor .Text / .DATA / .BSS   | * 256kb
 *  | Relocated!                     | *
 *  |--------------------------------|
 *  | Monitor Malloc                 | * 128kb (contains relocated environment)
 *  |--------------------------------|
 *  | Monitor/kernel STACK           | * 64kb
 *  |--------------------------------|
 *  | Page Table for MMU systems     | * 2k
 *  |--------------------------------|
 *  | PROM Code accessed from Linux  | * 6kb-128b
 *  |--------------------------------|
 *  | Global data (avail from kernel)| * 128b
 *  |--------------------------------|
 *
 */

/*
 * Flash configuration (8,16 or 32 MB)
 * TEXT base always at 0xFFF00000
 * ENV_ADDR always at  0xFFF40000
 * FLASH_BASE at 0xFC000000 for 64 MB
 *               0xFE000000 for 32 MB
 *               0xFF000000 for 16 MB
 *               0xFF800000 for  8 MB
 */
/*#define CFG_NO_FLASH		1*/
#define CFG_FLASH_BASE		0x00000000
#define CFG_FLASH_SIZE		0x00800000

#define PHYS_FLASH_SECT_SIZE	0x00020000	/* 128 KB sectors */
#define CFG_MAX_FLASH_SECT	64	/* max num of sects on one chip */
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks      */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */

/*** CFI CONFIG ***/
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
/* Bypass cache when reading regs from flash memory */
#define CFG_FLASH_CFI_BYPASS_READ
/* Buffered writes (32byte/go) instead of single accesses */
#define CFG_FLASH_USE_BUFFER_WRITE

/*
 * Environment settings
 */
/*#define CFG_ENV_IS_NOWHERE 1*/
#define CFG_ENV_IS_IN_FLASH	1
/* CFG_ENV_ADDR need to be at sector boundary */
#define CFG_ENV_SIZE		0x8000
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR		(CFG_FLASH_BASE+CFG_FLASH_SIZE-CFG_ENV_SECT_SIZE)
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 *
 * Always 4Mb SRAM available
 * SDRAM module may be available on 0x60000000, SDRAM
 * is configured as if a 128Mb SDRAM module is available.
 */

#if CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SDRAM_NOSRAM
#define CFG_SDRAM_BASE		0x40000000
#else
#define CFG_SDRAM_BASE		0x60000000
#endif

#define CFG_SDRAM_SIZE		0x08000000
#define CFG_SDRAM_END		(CFG_SDRAM_BASE+CFG_SDRAM_SIZE)

/* 4Mb SRAM available */
#if CONFIG_LEON_RAM_SELECT != CONFIG_LEON_RAM_SDRAM_NOSRAM
#define CFG_SRAM_BASE 0x40000000
#define CFG_SRAM_SIZE 0x400000
#define CFG_SRAM_END  (CFG_SRAM_BASE+CFG_SRAM_SIZE)
#endif

/* Select RAM used to run U-BOOT from... */
#if CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SRAM
#define CFG_RAM_BASE CFG_SRAM_BASE
#define CFG_RAM_SIZE CFG_SRAM_SIZE
#define CFG_RAM_END CFG_SRAM_END
#else
#define CFG_RAM_BASE CFG_SDRAM_BASE
#define CFG_RAM_SIZE CFG_SDRAM_SIZE
#define CFG_RAM_END CFG_SDRAM_END
#endif

#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_RAM_END - CFG_GBL_DATA_SIZE)

#define CFG_PROM_SIZE		(8192-CFG_GBL_DATA_SIZE)
#define CFG_PROM_OFFSET		(CFG_GBL_DATA_OFFSET-CFG_PROM_SIZE)

#define CFG_INIT_SP_OFFSET	(CFG_PROM_OFFSET-32)
#define CFG_STACK_SIZE		(0x10000-32)

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor   */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()  */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CFG_MALLOC_END		(CFG_INIT_SP_OFFSET-CFG_STACK_SIZE)
#define CFG_MALLOC_BASE		(CFG_MALLOC_END-CFG_MALLOC_LEN)

/* relocated monitor area */
#define CFG_RELOC_MONITOR_MAX_END   CFG_MALLOC_BASE
#define CFG_RELOC_MONITOR_BASE     (CFG_RELOC_MONITOR_MAX_END-CFG_MONITOR_LEN)

/* make un relocated address from relocated address */
#define UN_RELOC(address) (address-(CFG_RELOC_MONITOR_BASE-TEXT_BASE))

/*
 * Ethernet configuration uses on board SMC91C111
 */
#define CONFIG_DRIVER_SMC91111          1
#define CONFIG_SMC91111_BASE		0x20000300	/* chip select 3         */
#define CONFIG_SMC_USE_32_BIT		1	/* 32 bit bus  */
#undef  CONFIG_SMC_91111_EXT_PHY	/* we use internal phy   */
/*#define CONFIG_SHOW_ACTIVITY*/
#define CONFIG_NET_RETRY_COUNT		10	/* # of retries          */

#define CONFIG_ETHADDR   00:00:7a:cc:00:13
#define CONFIG_PHY_ADDR	 0x00

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory     */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM  */

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Various low-level settings
 */

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00005000

/***** Gaisler GRLIB IP-Cores Config ********/

/* AMBA Plug & Play info display on startup */
/*#define CFG_AMBAPP_PRINT_ON_STARTUP*/

#define CFG_GRLIB_SDRAM    0

/* See, GRLIB Docs (grip.pdf) on how to set up
 * These the memory controller registers.
 */
#define CFG_GRLIB_MEMCFG1   (0x10f800ff | (1<<11))
#if CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SDRAM_NOSRAM
#define CFG_GRLIB_MEMCFG2   0x82206000
#else
#define CFG_GRLIB_MEMCFG2   0x82205260
#endif
#define CFG_GRLIB_MEMCFG3   0x0809a000

#define CFG_GRLIB_FT_MEMCFG1   (0x10f800ff | (1<<11))
#if CONFIG_LEON_RAM_SELECT == CONFIG_LEON_RAM_SDRAM_NOSRAM
#define CFG_GRLIB_FT_MEMCFG2   0x82206000
#else
#define CFG_GRLIB_FT_MEMCFG2   0x82205260
#endif
#define CFG_GRLIB_FT_MEMCFG3   0x0809a000

/* no DDR controller */
#define CFG_GRLIB_DDR_CFG   0x00000000

/* no DDR2 Controller */
#define CFG_GRLIB_DDR2_CFG1 0x00000000
#define CFG_GRLIB_DDR2_CFG3 0x00000000

/* Calculate scaler register value from default baudrate */
#define CFG_GRLIB_APBUART_SCALER \
 ((((CONFIG_SYS_CLK_FREQ*10)/(CONFIG_BAUDRATE*8))-5)/10)

/* Identification string */
#define CONFIG_IDENT_STRING "GAISLER LEON3 GR-CPCI-AX2000"

/* default kernel command line */
#define CONFIG_DEFAULT_KERNEL_COMMAND_LINE "console=ttyS0,38400\0\0"

#endif				/* __CONFIG_H */
