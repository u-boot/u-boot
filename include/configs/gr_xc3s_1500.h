/* Configuration header file for Gaisler GR-XC3S-1500
 * spartan board.
 *
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_DISPLAY_BOARDINFO

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_GRXC3S1500	1	/* ... on GR-XC3S-1500 board */

/* CPU / AMBA BUS configuration */
#define CONFIG_SYS_CLK_FREQ	40000000	/* 40MHz */

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
#define CONFIG_CMD_REGINFO
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
	"scratch=40200000\0"					\
	"getkernel=tftpboot $(scratch) $(bootfile)\0" \
	"bootargs=console=ttyS0,38400 root=/dev/nfs rw nfsroot=192.168.0.20:/export/rootfs ip=192.168.0.206:192.168.0.20:192.168.0.1:255.255.255.0:grxc3s1500_daniel:eth0\0" \
	""

#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.0.1
#define CONFIG_SERVERIP 192.168.0.20
#define CONFIG_IPADDR 192.168.0.206
#define CONFIG_ROOTPATH "/export/rootfs"
#define CONFIG_HOSTNAME  grxc3s1500
#define CONFIG_BOOTFILE "/uImage"

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
/*#define CONFIG_SYS_NO_FLASH		1*/
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_FLASH_SIZE		0x00800000

#define PHYS_FLASH_SECT_SIZE	0x00020000	/* 128 KB sectors */
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max num of sects on one chip */
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
#define CONFIG_SYS_SDRAM_SIZE		0x4000000
#define CONFIG_SYS_SDRAM_END		(CONFIG_SYS_SDRAM_BASE+CONFIG_SYS_SDRAM_SIZE)

/* no SRAM available */
#undef CONFIG_SYS_SRAM_BASE
#undef CONFIG_SYS_SRAM_SIZE

/* Always Run U-Boot from SDRAM */
#define CONFIG_SYS_RAM_BASE CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_RAM_SIZE CONFIG_SYS_SDRAM_SIZE
#define CONFIG_SYS_RAM_END CONFIG_SYS_SDRAM_END

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_RAM_END - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_PROM_SIZE		(8192-GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_PROM_OFFSET		(CONFIG_SYS_GBL_DATA_OFFSET-CONFIG_SYS_PROM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_PROM_OFFSET-32)
#define CONFIG_SYS_STACK_SIZE		(0x10000-32)

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor   */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()  */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CONFIG_SYS_MALLOC_END		(CONFIG_SYS_INIT_SP_OFFSET-CONFIG_SYS_STACK_SIZE)
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MALLOC_END-CONFIG_SYS_MALLOC_LEN)

/* relocated monitor area */
#define CONFIG_SYS_RELOC_MONITOR_MAX_END   CONFIG_SYS_MALLOC_BASE
#define CONFIG_SYS_RELOC_MONITOR_BASE     (CONFIG_SYS_RELOC_MONITOR_MAX_END-CONFIG_SYS_MONITOR_LEN)

/* make un relocated address from relocated address */
#define UN_RELOC(address) (address-(CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE))

/*
 * Ethernet configuration
 */
#define CONFIG_GRETH	1

#define CONFIG_PHY_ADDR	 0x00

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory     */
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

#define CONFIG_SYS_GRLIB_SDRAM    0

/* No SDRAM Configuration */
#undef CONFIG_SYS_GRLIB_GAISLER_SDCTRL1

/* See, GRLIB Docs (grip.pdf) on how to set up
 * These the memory controller registers.
 */
#define CONFIG_SYS_GRLIB_ESA_MCTRL1
#define CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG1   (0x000000ff | (1<<11))
#define CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG2   0x82206000
#define CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG3   0x00136000

/* GRLIB FT-MCTRL configuration */
#define CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1
#define CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG1   (0x000000ff | (1<<11))
#define CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG2   0x82206000
#define CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG3   0x00136000

/* no DDR controller */
#undef CONFIG_SYS_GRLIB_GAISLER_DDRSPA1

/* no DDR2 Controller */
#undef CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1

/* Identification string */
#define CONFIG_IDENT_STRING " Gaisler LEON3 GR-XC3S-1500"

/* default kernel command line */
#define CONFIG_DEFAULT_KERNEL_COMMAND_LINE "console=ttyS0,38400\0\0"

#endif				/* __CONFIG_H */
