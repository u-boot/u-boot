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
#define CFG_ADDR_MAP_B      1
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
#undef CFG_LONGHELP                /* undef to save memory     */
#define CFG_PROMPT      "=>"  /* Monitor Command Prompt   */
#define CFG_CBSIZE      256        /* Console I/O Buffer Size  */
#define CFG_PBSIZE      (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS     8           /* Max number of command args   */

#define CFG_BARGSIZE    CFG_CBSIZE  /* Boot Argument Buffer Size    */
#define CFG_LOAD_ADDR   0x00100000  /* Default load address         */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE      0x00000000

#ifdef DEBUG
#define CFG_MONITOR_BASE    CFG_SDRAM_BASE
#else
#define CFG_MONITOR_BASE    CFG_FLASH_BASE
#endif

#ifdef DEBUG
#define CFG_MONITOR_LEN     (4 << 20)	/* lots of mem ... */
#else
#define CFG_MONITOR_LEN     (512 << 10)	/* 512K PLCC bootrom */
#endif
#define CFG_MALLOC_LEN      (2*(4096 << 10))    /* 2*4096kB for malloc()  */

#define CFG_MEMTEST_START   0x00004000	/* memtest works on      */
#define CFG_MEMTEST_END     0x02000000	/* 0 ... 32 MB in DRAM   */


#define CFG_EUMB_ADDR       0xFC000000

#define CFG_ISA_MEM         0xFD000000
#define CFG_ISA_IO          0xFE000000

#define CFG_FLASH_BASE      0xFFF00000
#define CFG_FLASH_SIZE      ((uint)(512 * 1024))
#define CFG_RESET_ADDRESS   0xFFF00100
#define FLASH_BASE0_PRELIM  0xFFF00000  /* 512K PLCC FLASH/AM29F040*/
#define FLASH_BASE0_SIZE    0x80000     /* 512K */
#define FLASH_BASE1_PRELIM  0xFFE10000  /* AMD 29LV160DB
					   1MB - 64K FLASH0 SEG =960K
					   (size=0xf0000)*/

#define CFG_BAUDRATE_TABLE  { 9600, 19200, 38400, 57600, 115200 }

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE	1

#define CFG_NS16550_CLK		18432000

#define CFG_NS16550_COM1	0xFFE08080

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR CFG_SDRAM_BASE + CFG_MONITOR_LEN
#define CFG_INIT_RAM_END   0x2F00  /* End of used area in DPRAM  */
#define CFG_GBL_DATA_SIZE  64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET  CFG_GBL_DATA_OFFSET

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33000000	/* external frequency to pll */
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  2
#define CFG_HZ               1000

#define CFG_ETH_DEV_FN       0x00
#define CFG_ETH_IOBASE       0x00104000


	/* Bit-field values for MCCR1.
	 */
#define CFG_ROMNAL          8
#define CFG_ROMFAL          8

	/* Bit-field values for MCCR2.
	 */
#define CFG_REFINT          0xf5     /* Refresh interval               */

	/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.
	 */
#define CFG_BSTOPRE         0x79

#ifdef INCLUDE_ECC
#define USE_ECC				1
#else /* INCLUDE_ECC */
#define USE_ECC				0
#endif /* INCLUDE_ECC */


	/* Bit-field values for MCCR3.
	 */
#define CFG_REFREC          8       /* Refresh to activate interval   */
#define CFG_RDLAT           (4+USE_ECC)   /* Data latancy from read command */

	/* Bit-field values for MCCR4.
	 */
#define CFG_PRETOACT        3       /* Precharge to activate interval */
#define CFG_ACTTOPRE        5       /* Activate to Precharge interval */
#define CFG_SDMODE_CAS_LAT  3       /* SDMODE CAS latancy             */
#define CFG_SDMODE_WRAP     0       /* SDMODE wrap type               */
#define CFG_SDMODE_BURSTLEN 2       /* SDMODE Burst length            */
#define CFG_ACTORW          2
#define CFG_REGISTERD_TYPE_BUFFER (1-USE_ECC)

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */
#define CFG_RAM_SIZE        0x04000000  /* 64MB */


#define CFG_BANK0_START     0x00000000
#define CFG_BANK0_END       (CFG_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE    1
#define CFG_BANK1_START     0x3ff00000
#define CFG_BANK1_END       0x3fffffff
#define CFG_BANK1_ENABLE    0
#define CFG_BANK2_START     0x3ff00000
#define CFG_BANK2_END       0x3fffffff
#define CFG_BANK2_ENABLE    0
#define CFG_BANK3_START     0x3ff00000
#define CFG_BANK3_END       0x3fffffff
#define CFG_BANK3_ENABLE    0
#define CFG_BANK4_START     0x3ff00000
#define CFG_BANK4_END       0x3fffffff
#define CFG_BANK4_ENABLE    0
#define CFG_BANK5_START     0x3ff00000
#define CFG_BANK5_END       0x3fffffff
#define CFG_BANK5_ENABLE    0
#define CFG_BANK6_START     0x3ff00000
#define CFG_BANK6_END       0x3fffffff
#define CFG_BANK6_ENABLE    0
#define CFG_BANK7_START     0x3ff00000
#define CFG_BANK7_END       0x3fffffff
#define CFG_BANK7_ENABLE    0

#define CFG_ODCR            0x7f


#define CFG_PGMAX           0x32 /* how long the 8240 reatins the currently accessed page in memory
				    see 8240 book for details*/
#define PCI_MEM_SPACE1_START	0x80000000
#define PCI_MEM_SPACE2_START	0xfd000000

/* IBAT/DBAT Configuration */
/* Ram: 64MB, starts at address-0, r/w instruction/data */
#define CFG_IBAT0U      (CFG_SDRAM_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CFG_IBAT0L      (CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_DBAT0U      CFG_IBAT0U
#define CFG_DBAT0L      CFG_IBAT0L

/* MPLD/Port-X I/O Space : data and instruction read/write,  cache-inhibit */
#define CFG_IBAT1U      (PORTX_DEV_BASE | BATU_BL_128M | BATU_VS | BATU_VP)
#if 0
#define CFG_IBAT1L      (PORTX_DEV_BASE | BATL_PP_10  | BATL_MEMCOHERENCE |\
			 BATL_WRITETHROUGH | BATL_CACHEINHIBIT)
#else
#define CFG_IBAT1L      (PORTX_DEV_BASE | BATL_PP_10 |BATL_CACHEINHIBIT)
#endif
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT1L	CFG_IBAT1L

/* PCI Memory region 1: 0x8XXX_XXXX PCI Mem space: EUMBAR, etc - 16MB */
#define CFG_IBAT2U	(PCI_MEM_SPACE1_START|BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_IBAT2L	(PCI_MEM_SPACE1_START|BATL_PP_10 | BATL_GUARDEDSTORAGE|BATL_CACHEINHIBIT)
#define CFG_DBAT2U      CFG_IBAT2U
#define CFG_DBAT2L      CFG_IBAT2L

/* PCI Memory region 2: PCI Devices in 0xFD space */
#define CFG_IBAT3U	(PCI_MEM_SPACE2_START|BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_IBAT3L	(PCI_MEM_SPACE2_START|BATL_PP_10 | BATL_GUARDEDSTORAGE | BATL_CACHEINHIBIT)
#define CFG_DBAT3U      CFG_IBAT3U
#define CFG_DBAT3L      CFG_IBAT3L


/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ       (8 << 20)   /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS     3       /* Max number of flash banks         */
#define CFG_MAX_FLASH_SECT      64      /* Max number of sectors in one bank */

#define CFG_FLASH_ERASE_TOUT    120000  /* Timeout for Flash Erase (in ms)   */
#define CFG_FLASH_WRITE_TOUT    500     /* Timeout for Flash Write (in ms)   */

#if 0
#define	CFG_ENV_IS_IN_FLASH	    1
#define CFG_ENV_OFFSET          0x8000  /* Offset of the Environment Sector	*/
#define CFG_ENV_SIZE            0x4000  /* Size of the Environment Sector    */
#else
#define CFG_ENV_IS_IN_NVRAM          1
#define CFG_ENV_ADDR            NV_OFF_U_BOOT_ADDR /* PortX NVM Free addr*/
#define CFG_ENV_OFFSET          CFG_ENV_ADDR
#define CFG_ENV_SIZE            NV_U_BOOT_ENV_SIZE /* 2K */
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE  16


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD           0x01    /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM           0x02    /* Software reboot                  */

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

#define CONFIG_NET_MULTI		/* Multi ethernet cards support		*/

#define CONFIG_TULIP

#endif  /* __CONFIG_H */
