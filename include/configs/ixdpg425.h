/*
 * (C) Copyright 2005-2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2003
 * Martijn de Gouw, Prodrive B.V., martijn.de.gouw@prodrive.nl
 *
 * Configuation settings for the IXDPG425 board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_IXP425           1       /* This is an IXP425 CPU	*/
#define CONFIG_IXDPG425         1       /* on an IXDPG425 Board		*/

#define CONFIG_DISPLAY_CPUINFO	1	/* display cpu info (and speed)	*/
#define CONFIG_DISPLAY_BOARDINFO 1	/* display board info		*/

/*
 * Ethernet
 */
#define CONFIG_IXP4XX_NPE	1	/* include IXP4xx NPE support	*/
#define CONFIG_NET_MULTI	1
#define	CONFIG_PHY_ADDR		5	/* NPE0 PHY address		*/
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	4	/* NPE1 PHY address		*/
#define CONFIG_MII		1	/* MII PHY management		*/
#define CFG_RX_ETH_BUFFER	16	/* Number of ethernet rx buffers & descriptors */

/*
 * Misc configuration options
 */
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define CONFIG_USE_IRQ          1	/* we need IRQ stuff for timer	*/

#define CONFIG_BOOTCOUNT_LIMIT		/* support for bootcount limit	*/
#define CFG_BOOTCOUNT_ADDR	0x60003000 /* inside qmrg sram		*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(256 << 10)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE         115200
#define CFG_IXP425_CONSOLE	IXP425_UART1   /* we use UART1 for console */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING


#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP                            /* undef to save memory         */
#define CFG_PROMPT              "=> "   /* Monitor Command Prompt       */
#define CFG_CBSIZE              256             /* Console I/O Buffer Size      */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS             16              /* max number of command args   */
#define CFG_BARGSIZE            CFG_CBSIZE      /* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START       0x00400000      /* memtest works on     */
#define CFG_MEMTEST_END         0x00800000      /* 4 ... 8 MB in DRAM   */
#define CFG_LOAD_ADDR           0x00010000      /* default load address */

#undef  CFG_CLKS_IN_HZ          /* everything, incl board info, in Hz */
#define CFG_HZ			1000		/* decrementer freq: 1 ms ticks */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE        (128*1024)      /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)        /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ    (4*1024)        /* FIQ stack */
#endif

/***************************************************************
 * Platform/Board specific defines start here.
 ***************************************************************/

/*-----------------------------------------------------------------------
 * Default configuration (environment varibles...)
 *----------------------------------------------------------------------*/
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=ixdpg425\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/eldk/arm\0"					\
	"bootfile=/tftpboot/ixdpg425/uImage\0"				\
	"kernel_addr=50080000\0"					\
	"ramdisk_addr=50200000\0"					\
	"load=tftp 100000 /tftpboot/ixdpg425/u-boot.bin\0"		\
	"update=protect off 50000000 5003ffff;era 50000000 5003ffff;"	\
		"cp.b 100000 50000000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load;run update\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1          /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1            0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x02000000 /* 32 MB */

#define PHYS_FLASH_1            0x50000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE         0x01000000 /* 16 MB */
#define PHYS_FLASH_BANK_SIZE    0x01000000 /* 16 MB Banks */
#define PHYS_FLASH_SECT_SIZE    0x00020000 /* 128 KB sectors (x1) */

#define CFG_DRAM_BASE           0x00000000
#define CFG_DRAM_SIZE           0x01000000

#define CFG_FLASH_BASE          PHYS_FLASH_1
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/

/*
 * Expansion bus settings
 */
#define CFG_EXP_CS0		0xbcd23c42

/*
 * SDRAM settings
 */
#define CFG_SDR_CONFIG		0x18
#define CFG_SDR_MODE_CONFIG	0x1
#define CFG_SDRAM_REFRESH_CNT 	0x81a

/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1       /* max number of memory banks           */
#define CFG_MAX_FLASH_SECT      128 	/* max number of sectors on one chip    */

#define CFG_FLASH_CFI				/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/
#define	CFG_ENV_IS_IN_FLASH	1

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* hardware flash protection		*/

#define CFG_FLASH_BANKS_LIST	{ PHYS_FLASH_1 }

#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_16BIT	/* no byte writes on IXP4xx	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CFG_ENV_SECT_SIZE	0x20000 	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x40000)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*
 * GPIO settings
 */
#define CFG_GPIO_PCI_INTA_N	6
#define CFG_GPIO_PCI_INTB_N	7
#define CFG_GPIO_SWITCH_RESET_N	8
#define CFG_GPIO_SLIC_RESET_N	13
#define CFG_GPIO_PCI_CLK	14
#define CFG_GPIO_EXTBUS_CLK	15

/*
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32

#endif  /* __CONFIG_H */
