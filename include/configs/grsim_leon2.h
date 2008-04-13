/* Configuration header file for LEON2 GRSIM.
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
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
 *
 * Select between TSIM or GRSIM by setting CONFIG_GRSIM or CONFIG_TSIM to 1.
 *
 * TSIM command
 *  tsim-leon -sdram 0 -ram 32000 -rom 8192 -mmu
 *
 */

#define CONFIG_LEON2		/* This is an LEON2 CPU */
#define CONFIG_LEON		1	/* This is an LEON CPU */
#define CONFIG_GRSIM		0	/* ... not running on GRSIM */
#define CONFIG_TSIM		1	/* ... running on TSIM */

/* CPU / AMBA BUS configuration */
#define CONFIG_SYS_CLK_FREQ 	40000000	/* 40MHz */

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
#define CONFIG_CMD_AUTOSCRIPT	/* Autoscript Support           */
#define CONFIG_CMD_BDI		/* bdinfo                       */
#define CONFIG_CMD_CONSOLE	/* coninfo                      */
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ECHO		/* echo arguments               */
#define CONFIG_CMD_FPGA		/* FPGA configuration Support   */
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_ITEST	/* Integer (and string) test    */
#define CONFIG_CMD_LOADB	/* loadb                        */
#define CONFIG_CMD_LOADS	/* loads                        */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc */
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_RUN		/* run command in env variable  */
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx           */
#define CONFIG_CMD_XIMG		/* Load part of Multi Image     */

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS
/*#define CFG_HUSH_PARSER 0*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

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
	"net_nfs=tftp 40000000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/export/roofs\0"					\
	"scratch=40000000\0"					\
	"getkernel=tftpboot \$\(scratch\)\ \$\(bootfile\)\0" \
	"ethaddr=00:00:7A:CC:00:12\0" \
	"bootargs=console=ttyS0,38400" \
	""
#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_SERVERIP 192.168.0.81
#define CONFIG_IPADDR 192.168.0.80
#define CONFIG_ROOTPATH /export/rootfs
#define CONFIG_HOSTNAME  grxc3s1500
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
#define CFG_NO_FLASH		1
#define CFG_FLASH_BASE		0x00000000
#define CFG_FLASH_SIZE		0x00800000
#define CFG_ENV_SIZE		0x8000

#define CFG_ENV_ADDR		(CFG_FLASH_BASE+CFG_FLASH_SIZE-CFG_ENV_SIZE)

#define PHYS_FLASH_SECT_SIZE	0x00020000	/* 128 KB sectors */
#define CFG_MAX_FLASH_SECT	64	/* max num of sects on one chip */
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks      */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */

#ifdef ENABLE_FLASH_SUPPORT
/* For use with grsim FLASH emulation extension */
#define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */

#undef CONFIG_FLASH_8BIT	/* Flash is 32-bit */

/*** CFI CONFIG ***/
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#endif

/*
 * Environment settings
 */
#define CFG_ENV_IS_NOWHERE 1
/*#define CFG_ENV_IS_IN_FLASH	0*/
/*#define CFG_ENV_SIZE		0x8000*/
#define CFG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CFG_SDRAM_BASE		0x40000000
#define CFG_SDRAM_SIZE		0x00800000
#define CFG_SDRAM_END		(CFG_SDRAM_BASE+CFG_SDRAM_SIZE)

/* no SRAM available */
#undef CFG_SRAM_BASE
#undef CFG_SRAM_SIZE


/* Always Run U-Boot from SDRAM */
#define CFG_RAM_BASE CFG_SDRAM_BASE
#define CFG_RAM_SIZE CFG_SDRAM_SIZE
#define CFG_RAM_END CFG_SDRAM_END

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
 * Ethernet configuration
 */
/*#define CONFIG_GRETH	1*/
/*#define CONFIG_NET_MULTI	1*/

/* Default HARDWARE address */
#define GRETH_HWADDR_0 0x00
#define GRETH_HWADDR_1 0x00
#define GRETH_HWADDR_2 0x7A
#define GRETH_HWADDR_3 0xcc
#define GRETH_HWADDR_4 0x00
#define GRETH_HWADDR_5 0x12

#define CONFIG_ETHADDR   00:00:7a:cc:00:12

/*
 * Define CONFIG_GRETH_10MBIT to force GRETH at 10Mb/s
 */
/* #define CONFIG_GRETH_10MBIT 1 */
#define CONFIG_PHY_ADDR		0x00

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

/***** Gaisler GRLIB IP-Cores Config ********/

#define CFG_GRLIB_SDRAM    0
#define CFG_GRLIB_MEMCFG1  (0x000000ff | (1<<11))
#if CONFIG_GRSIM
#define CFG_GRLIB_MEMCFG2  0x82206000
#else
#define CFG_GRLIB_MEMCFG2  0x00001820
#endif
#define CFG_GRLIB_MEMCFG3  0x00136000

/*** LEON2 UART 1 ***/
#define CFG_LEON2_UART1_SCALER \
 ((((CONFIG_SYS_CLK_FREQ*10)/(CONFIG_BAUDRATE*8))-5)/10)
 
/* UART1 Define to 1 or 0 */
#define LEON2_UART1_LOOPBACK_ENABLE 0
#define LEON2_UART1_FLOWCTRL_ENABLE 0
#define LEON2_UART1_PARITY_ENABLE 0
#define LEON2_UART1_ODDPAR_ENABLE 0

/*** LEON2 UART 2 ***/

#define CFG_LEON2_UART2_SCALER \
 ((((CONFIG_SYS_CLK_FREQ*10)/(CONFIG_BAUDRATE*8))-5)/10)

/* UART2 Define to 1 or 0 */
#define LEON2_UART2_LOOPBACK_ENABLE 0
#define LEON2_UART2_FLOWCTRL_ENABLE 0
#define LEON2_UART2_PARITY_ENABLE 0
#define LEON2_UART2_ODDPAR_ENABLE 0

#define LEON_CONSOLE_UART1 1
#define LEON_CONSOLE_UART2 2

/* Use UART2 as console */
#define LEON2_CONSOLE_SELECT LEON_CONSOLE_UART1

/* LEON2 I/O Port */
/*#define LEON2_IO_PORT_DIR 0x0000aa00*/

/* default kernel command line */
#define CONFIG_DEFAULT_KERNEL_COMMAND_LINE "console=ttyS0,38400\0\0"

#define CONFIG_IDENT_STRING "Gaisler GRSIM LEON2"

#endif				/* __CONFIG_H */
