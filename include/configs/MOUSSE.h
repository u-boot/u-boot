/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * James F. Dougherty (jfd@cs.stanford.edu)
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

/*
 *
 * Configuration settings for the MOUSSE board.
 * See also: http://www.vooha.com/
 *
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC824X      1
#define CONFIG_MPC8240      1
#define CONFIG_MOUSSE       1

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000
#define	CONFIG_SYS_LDSCRIPT	"board/mousse/u-boot.lds"

#define CONFIG_SYS_ADDR_MAP_B      1

#define CONFIG_CONS_INDEX   1
#define CONFIG_BAUDRATE     9600
#if 1
#define CONFIG_BOOTCOMMAND  "tftp 100000 vmlinux.img;bootm"    /* autoboot command */
#else
#define CONFIG_BOOTCOMMAND  "bootm ffe10000"
#endif
#define CONFIG_BOOTARGS      "console=ttyS0 root=/dev/nfs rw nfsroot=209.128.93.133:/boot nfsaddrs=209.128.93.133:209.128.93.138"
#define CONFIG_BOOTDELAY     3


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE


#define CONFIG_ENV_OVERWRITE 1
#define CONFIG_ETH_ADDR      "00:10:18:10:00:06"

#define CONFIG_DOS_PARTITION  1 /* MSDOS bootable partitiion support */

#include "../board/mousse/mousse.h"

/*
 * Miscellaneous configurable options
 */
#undef CONFIG_SYS_LONGHELP                /* undef to save memory     */
#define CONFIG_SYS_PROMPT      "=>"  /* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE      256        /* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE      (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS     8           /* Max number of command args   */

#define CONFIG_SYS_BARGSIZE    CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size    */
#define CONFIG_SYS_LOAD_ADDR   0x00100000  /* Default load address         */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE      0x00000000

#ifdef DEBUG
#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_SDRAM_BASE
#else
#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_FLASH_BASE
#endif

#ifdef DEBUG
#define CONFIG_SYS_MONITOR_LEN     (4 << 20)	/* lots of mem ... */
#else
#define CONFIG_SYS_MONITOR_LEN     (512 << 10)	/* 512K PLCC bootrom */
#endif
#define CONFIG_SYS_MALLOC_LEN      (2*(4096 << 10))    /* 2*4096kB for malloc()  */

#define CONFIG_SYS_MEMTEST_START   0x00004000	/* memtest works on      */
#define CONFIG_SYS_MEMTEST_END     0x02000000	/* 0 ... 32 MB in DRAM   */


#define CONFIG_SYS_EUMB_ADDR       0xFC000000

#define CONFIG_SYS_ISA_MEM         0xFD000000
#define CONFIG_SYS_ISA_IO          0xFE000000

#define CONFIG_SYS_FLASH_BASE      0xFFF00000
#define CONFIG_SYS_FLASH_SIZE      ((uint)(512 * 1024))
#define CONFIG_SYS_RESET_ADDRESS   0xFFF00100
#define FLASH_BASE0_PRELIM  0xFFF00000  /* 512K PLCC FLASH/AM29F040*/
#define FLASH_BASE0_SIZE    0x80000     /* 512K */
#define FLASH_BASE1_PRELIM  0xFFE10000  /* AMD 29LV160DB
					   1MB - 64K FLASH0 SEG =960K
					   (size=0xf0000)*/

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL

#define CONFIG_SYS_NS16550_REG_SIZE	1

#define CONFIG_SYS_NS16550_CLK		18432000

#define CONFIG_SYS_NS16550_COM1	0xFFE08080

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_INIT_RAM_SIZE   0x2F00  /* Size of used area in DPRAM  */
#define CONFIG_SYS_GBL_DATA_OFFSET  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET  CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33000000	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  2
#define CONFIG_SYS_HZ               1000

#define CONFIG_SYS_ETH_DEV_FN       0x00
#define CONFIG_SYS_ETH_IOBASE       0x00104000


	/* Bit-field values for MCCR1.
	 */
#define CONFIG_SYS_ROMNAL          8
#define CONFIG_SYS_ROMFAL          8

	/* Bit-field values for MCCR2.
	 */
#define CONFIG_SYS_REFINT          0xf5     /* Refresh interval               */

	/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.
	 */
#define CONFIG_SYS_BSTOPRE         0x79

#ifdef INCLUDE_ECC
#define USE_ECC				1
#else /* INCLUDE_ECC */
#define USE_ECC				0
#endif /* INCLUDE_ECC */


	/* Bit-field values for MCCR3.
	 */
#define CONFIG_SYS_REFREC          8       /* Refresh to activate interval   */
#define CONFIG_SYS_RDLAT           (4+USE_ECC)   /* Data latancy from read command */

	/* Bit-field values for MCCR4.
	 */
#define CONFIG_SYS_PRETOACT        3       /* Precharge to activate interval */
#define CONFIG_SYS_ACTTOPRE        5       /* Activate to Precharge interval */
#define CONFIG_SYS_SDMODE_CAS_LAT  3       /* SDMODE CAS latancy             */
#define CONFIG_SYS_SDMODE_WRAP     0       /* SDMODE wrap type               */
#define CONFIG_SYS_SDMODE_BURSTLEN 2       /* SDMODE Burst length            */
#define CONFIG_SYS_ACTORW          2
#define CONFIG_SYS_REGISTERD_TYPE_BUFFER (1-USE_ECC)

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */
#define CONFIG_SYS_RAM_SIZE        0x04000000  /* 64MB */


#define CONFIG_SYS_BANK0_START     0x00000000
#define CONFIG_SYS_BANK0_END       (CONFIG_SYS_RAM_SIZE - 1)
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

#define CONFIG_SYS_ODCR            0x7f


#define CONFIG_SYS_PGMAX           0x32 /* how long the 8240 reatins the currently accessed page in memory
				    see 8240 book for details*/
#define PCI_MEM_SPACE1_START	0x80000000
#define PCI_MEM_SPACE2_START	0xfd000000

/* IBAT/DBAT Configuration */
/* Ram: 64MB, starts at address-0, r/w instruction/data */
#define CONFIG_SYS_IBAT0U      (CONFIG_SYS_SDRAM_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT0L      (CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_DBAT0U      CONFIG_SYS_IBAT0U
#define CONFIG_SYS_DBAT0L      CONFIG_SYS_IBAT0L

/* MPLD/Port-X I/O Space : data and instruction read/write,  cache-inhibit */
#define CONFIG_SYS_IBAT1U      (PORTX_DEV_BASE | BATU_BL_128M | BATU_VS | BATU_VP)
#if 0
#define CONFIG_SYS_IBAT1L      (PORTX_DEV_BASE | BATL_PP_10  | BATL_MEMCOHERENCE |\
			 BATL_WRITETHROUGH | BATL_CACHEINHIBIT)
#else
#define CONFIG_SYS_IBAT1L      (PORTX_DEV_BASE | BATL_PP_10 |BATL_CACHEINHIBIT)
#endif
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L

/* PCI Memory region 1: 0x8XXX_XXXX PCI Mem space: EUMBAR, etc - 16MB */
#define CONFIG_SYS_IBAT2U	(PCI_MEM_SPACE1_START|BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT2L	(PCI_MEM_SPACE1_START|BATL_PP_10 | BATL_GUARDEDSTORAGE|BATL_CACHEINHIBIT)
#define CONFIG_SYS_DBAT2U      CONFIG_SYS_IBAT2U
#define CONFIG_SYS_DBAT2L      CONFIG_SYS_IBAT2L

/* PCI Memory region 2: PCI Devices in 0xFD space */
#define CONFIG_SYS_IBAT3U	(PCI_MEM_SPACE2_START|BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_IBAT3L	(PCI_MEM_SPACE2_START|BATL_PP_10 | BATL_GUARDEDSTORAGE | BATL_CACHEINHIBIT)
#define CONFIG_SYS_DBAT3U      CONFIG_SYS_IBAT3U
#define CONFIG_SYS_DBAT3L      CONFIG_SYS_IBAT3L


/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ       (8 << 20)   /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS     3       /* Max number of flash banks         */
#define CONFIG_SYS_MAX_FLASH_SECT      64      /* Max number of sectors in one bank */

#define CONFIG_SYS_FLASH_ERASE_TOUT    120000  /* Timeout for Flash Erase (in ms)   */
#define CONFIG_SYS_FLASH_WRITE_TOUT    500     /* Timeout for Flash Write (in ms)   */

#if 0
#define	CONFIG_ENV_IS_IN_FLASH	    1
#define CONFIG_ENV_OFFSET          0x8000  /* Offset of the Environment Sector	*/
#define CONFIG_ENV_SIZE            0x4000  /* Size of the Environment Sector    */
#else
#define CONFIG_ENV_IS_IN_NVRAM          1
#define CONFIG_ENV_ADDR            NV_OFF_U_BOOT_ADDR /* PortX NVM Free addr*/
#define CONFIG_ENV_OFFSET          CONFIG_ENV_ADDR
#define CONFIG_ENV_SIZE            NV_U_BOOT_ENV_SIZE /* 2K */
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE  16

/* Localizations */
#if 0
#define CONFIG_ETHADDR          0:0:0:0:1:d
#define CONFIG_IPADDR           172.16.40.113
#define CONFIG_SERVERIP         172.16.40.111
#else
#define CONFIG_ETHADDR          0:0:0:0:1:d
#define CONFIG_IPADDR           209.128.93.138
#define CONFIG_SERVERIP         209.128.93.133
#endif

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI			/* include pci support			*/
#undef CONFIG_PCI_PNP


#define CONFIG_TULIP

#endif  /* __CONFIG_H */
