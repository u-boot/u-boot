/* Configuration header file for Gaisler Research AB's Template
 * design (GPL Open Source SPARC/LEON3 96MHz) for Altera NIOS
 * Development board Stratix II edition, with the FPGA device
 * EP2S60.
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
/* Altera NIOS Development board, Stratix II board */
#define CONFIG_GR_EP2S60	1

/* CPU / AMBA BUS configuration */
#define CONFIG_SYS_CLK_FREQ	96000000	/* 96MHz */

/* Number of SPARC register windows */
#define CONFIG_SYS_SPARC_NWINDOWS 8

/* Define this is the GR-2S60-MEZZ mezzanine is available and you
 * want to use the USB and GRETH functionality of the board
 */
#undef GR_2S60_MEZZ

#ifdef GR_2S60_MEZZ
#define USE_GRETH 1
#define USE_GRUSB 1
#endif

/*
 * Serial console configuration
 */
#define CONFIG_BAUDRATE		38400	/* ... at 38400 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

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

/* USB support */
#if USE_GRUSB
#define CONFIG_USB_UHCI
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
/* Enable needed helper functions */
#define CONFIG_SYS_STDIO_DEREGISTER	/* needs stdio_deregister */
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
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
	"net_nfs=tftp 40000000 ${bootfile};run nfsargs addip;bootm\0"	\
	"scratch=40800000\0"					\
	"getkernel=tftpboot \$\(scratch\)\ \$\(bootfile\)\0" \
	"bootargs=console=ttyS0,38400 root=/dev/nfs rw nfsroot=192.168.0.20:/export/rootfs ip=192.168.0.207:192.168.0.20:192.168.0.1:255.255.255.0:ml401:eth0\0" \
	""

#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_SERVERIP 192.168.0.20
#define CONFIG_IPADDR 192.168.0.207
#define CONFIG_ROOTPATH /export/rootfs
#define CONFIG_HOSTNAME  ml401
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
 *  Main Memory:
 *  |--------------------------------|
 *  | UNUSED / scratch area          |
 *  |                                |
 *  |                                |
 *  |                                |
 *  |                                |
 *  |--------------------------------|
 *  | Monitor .Text / .DATA / .BSS   | * 512kb
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
/*#define CONFIG_SYS_NO_FLASH		1*/
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_FLASH_SIZE		0x00400000	/* FPGA Bit file is in top of FLASH, we only ues the bottom 4Mb */

#define PHYS_FLASH_SECT_SIZE	0x00010000	/* 64 KB sectors */
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max num of sects on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#define CONFIG_SYS_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CONFIG_SYS_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */

/*** CFI CONFIG ***/
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
/* Bypass cache when reading regs from flash memory */
#define CONFIG_SYS_FLASH_CFI_BYPASS_READ
/* Buffered writes (32byte/go) instead of single accesses */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/*
 * Environment settings
 */
/*#define CONFIG_ENV_IS_NOWHERE 1*/
#define CONFIG_ENV_IS_IN_FLASH	1
/* CONFIG_ENV_ADDR need to be at sector boundary */
#define CONFIG_ENV_SIZE		0x8000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE+CONFIG_SYS_FLASH_SIZE-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_SDRAM_SIZE		0x02000000
#define CONFIG_SYS_SDRAM_END		(CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_SDRAM_SIZE)

/* no SRAM available */
#undef CONFIG_SYS_SRAM_BASE
#undef CONFIG_SYS_SRAM_SIZE

#define CONFIG_SYS_RAM_BASE CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_RAM_SIZE CONFIG_SYS_SDRAM_SIZE
#define CONFIG_SYS_RAM_END CONFIG_SYS_SDRAM_END

#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_END - CONFIG_SYS_GBL_DATA_SIZE)

#define CONFIG_SYS_PROM_SIZE		(8192-CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_PROM_OFFSET		(CONFIG_SYS_GBL_DATA_OFFSET-CONFIG_SYS_PROM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_PROM_OFFSET-32)
#define CONFIG_SYS_STACK_SIZE		(0x10000-32)

#define CONFIG_SYS_MONITOR_BASE    TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 512 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()  */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CONFIG_SYS_MALLOC_END		(CONFIG_SYS_INIT_SP_OFFSET-CONFIG_SYS_STACK_SIZE)
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MALLOC_END-CONFIG_SYS_MALLOC_LEN)

/* relocated monitor area */
#define CONFIG_SYS_RELOC_MONITOR_MAX_END   CONFIG_SYS_MALLOC_BASE
#define CONFIG_SYS_RELOC_MONITOR_BASE     (CONFIG_SYS_RELOC_MONITOR_MAX_END-CONFIG_SYS_MONITOR_LEN)

/* make un relocated address from relocated address */
#define UN_RELOC(address) (address-(CONFIG_SYS_RELOC_MONITOR_BASE-TEXT_BASE))

/*
 * Ethernet configuration uses on board SMC91C111, however if a mezzanine
 * with a PHY is attached the GRETH can be used on this board.
 * Define USE_GRETH in order to use the mezzanine provided PHY with the
 * onchip GRETH network MAC, note that this is not supported by the
 * template design.
 */
#ifndef USE_GRETH

/* USE SMC91C111 MAC */
#define CONFIG_DRIVER_SMC91111          1
#define CONFIG_SMC91111_BASE		0x20000300	/* chip select 3         */
#define CONFIG_SMC_USE_32_BIT		1	/* 32 bit bus  */
#undef  CONFIG_SMC_91111_EXT_PHY	/* we use internal phy   */
/*#define CONFIG_SHOW_ACTIVITY*/
#define CONFIG_NET_RETRY_COUNT		10	/* # of retries          */

#else

/* USE GRETH Ethernet Driver */
#define CONFIG_NET_MULTI	1
#define CONFIG_GRETH	1

/* Default GRETH Ethernet HARDWARE address */
#define GRETH_HWADDR_0 0x00
#define GRETH_HWADDR_1 0x00
#define GRETH_HWADDR_2 0x7a
#define GRETH_HWADDR_3 0xcc
#define GRETH_HWADDR_4 0x00
#define GRETH_HWADDR_5 0x13
#endif

#define CONFIG_ETHADDR   00:00:7a:cc:00:13
#define CONFIG_PHY_ADDR	 0x00

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory     */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM  */

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00005000

/***** Gaisler GRLIB IP-Cores Config ********/

/* AMBA Plug & Play info display on startup */
/*#define CONFIG_SYS_AMBAPP_PRINT_ON_STARTUP*/

#define CONFIG_SYS_GRLIB_SDRAM    0

/* See, GRLIB Docs (grip.pdf) on how to set up
 * These the memory controller registers.
 */
#define CONFIG_SYS_GRLIB_MEMCFG1  (0x10f800ff | (1<<11))
#define CONFIG_SYS_GRLIB_MEMCFG2  0x00000000
#define CONFIG_SYS_GRLIB_MEMCFG3  0x00000000

#define CONFIG_SYS_GRLIB_FT_MEMCFG1  (0x10f800ff | (1<<11))
#define CONFIG_SYS_GRLIB_FT_MEMCFG2  0x00000000
#define CONFIG_SYS_GRLIB_FT_MEMCFG3  0x00000000

#define CONFIG_SYS_GRLIB_DDR_CFG  0xa900830a

#define CONFIG_SYS_GRLIB_DDR2_CFG1 0x00000000
#define CONFIG_SYS_GRLIB_DDR2_CFG3 0x00000000

/* Calculate scaler register value from default baudrate */
#define CONFIG_SYS_GRLIB_APBUART_SCALER \
 ((((CONFIG_SYS_CLK_FREQ*10)/(CONFIG_BAUDRATE*8))-5)/10)

/* Identification string */
#define CONFIG_IDENT_STRING "GAISLER LEON3 EP2S60"

/* default kernel command line */
#define CONFIG_DEFAULT_KERNEL_COMMAND_LINE "console=ttyS0,38400\0\0"

#endif				/* __CONFIG_H */
