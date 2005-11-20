/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef  DEBUG
#define GTREGREAD(x) 0xffffffff         /* needed for debug */

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* these hardware addresses are pretty bogus, please change them to
   suit your needs */

/* first ethernet */
#define CONFIG_ETHADDR          00:00:5b:ee:de:ad

#define CONFIG_IPADDR           192.168.0.105
#define CONFIG_SERVERIP         192.168.0.100

#define CONFIG_ELPPC            1       /* this is an BAB740/BAB750 board */

#define CONFIG_BAUDRATE         9600    /* console baudrate */

#undef  CONFIG_WATCHDOG

#define CONFIG_BOOTDELAY        5       /* autoboot after 5 seconds */

#define CONFIG_ZERO_BOOTDELAY_CHECK

#undef  CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND                                  \
    "bootp 1000000; "                                       \
    "setenv bootargs root=ramfs console=ttyS00,9600 "       \
    "ip=${ipaddr}:${serverip}:${rootpath}:${gatewayip}:"    \
    "${netmask}:${hostname}:eth0:none; "                    \
    "bootm"

#define CONFIG_LOADS_ECHO       0       /* echo off for serial download */
#define CFG_LOADS_BAUD_CHANGE           /* allow baudrate changes */

#define CONFIG_BOOTP_MASK       (CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_COMMANDS         (CONFIG_CMD_DFL | CFG_CMD_PCI | CFG_CMD_JFFS2)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP                    /* undef to save memory */
#define CFG_PROMPT              "=> "   /* Monitor Command Prompt */

/*
 * choose between COM1 and COM2 as serial console
 */
#define CONFIG_CONS_INDEX       1

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE              1024        /* Console I/O Buffer Size */
#else
#define CFG_CBSIZE              256         /* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS             16          /* max number of command args    */
#define CFG_BARGSIZE            CFG_CBSIZE  /* Boot Argument Buffer Size    */

#define CFG_MEMTEST_START       0x00000000  /* memtest works on    */
#define CFG_MEMTEST_END         0x04000000  /* 0 ... 64 MB in DRAM    */

#define CFG_LOAD_ADDR           0x1000000   /* default load address    */

#define CFG_HZ                  1000        /* dec. freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define CFG_BOARD_ASM_INIT
#define CONFIG_MISC_INIT_R

/*
 * Address mapping scheme for the MPC107 mem controller is mapping B (CHRP)
 */
#undef  CFG_ADDRESS_MAP_A

#define CFG_PCI_MEMORY_BUS      0x00000000
#define CFG_PCI_MEMORY_PHYS     0x00000000
#define CFG_PCI_MEMORY_SIZE     0x40000000

#define CFG_PCI_MEM_BUS         0x80000000
#define CFG_PCI_MEM_PHYS        0x80000000
#define CFG_PCI_MEM_SIZE        0x7d000000

#define CFG_ISA_MEM_BUS         0x00000000
#define CFG_ISA_MEM_PHYS        0xfd000000
#define CFG_ISA_MEM_SIZE        0x01000000

#define CFG_PCI_IO_BUS          0x00800000
#define CFG_PCI_IO_PHYS         0xfe800000
#define CFG_PCI_IO_SIZE         0x00400000

#define CFG_ISA_IO_BUS          0x00000000
#define CFG_ISA_IO_PHYS         0xfe000000
#define CFG_ISA_IO_SIZE         0x00800000

/* driver defines FDC,IDE,... */
#define CFG_ISA_IO_BASE_ADDRESS CFG_ISA_IO_PHYS
#define CFG_ISA_IO              CFG_ISA_IO_PHYS
#define CFG_60X_PCI_IO_OFFSET   CFG_ISA_IO_PHYS

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE          0x00000000

#define CFG_USR_LED_BASE        0x78000000
#define CFG_NVRAM_BASE          0xff000000
#define CFG_UART_BASE           0xff400000
#define CFG_FLASH_BASE          0xfff00000

#define MPC107_EUMB_ADDR        0xfce00000
#define MPC107_EUMB_PI          0xfce41090
#define MPC107_EUMB_GCR         0xfce41020
#define MPC107_EUMB_IACKR       0xfce600a0
#define MPC107_I2C_ADDR         0xfce03000

/*
 * Definitions for initial stack pointer and data area
 */
#define CFG_INIT_RAM_ADDR       0x00fd0000  /* above the memtest region */
#define CFG_INIT_RAM_END        0x4000
#define CFG_GBL_DATA_SIZE       64          /* size in bytes reserved for init data */
#define CFG_GBL_DATA_OFFSET     (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*
 * Flash mapping/organization on the MPC10x.
 */
#define FLASH_BASE0_PRELIM      0xff800000
#define FLASH_BASE1_PRELIM      0xffc00000

#define CFG_MAX_FLASH_BANKS     2           /* max number of memory banks    */
#define CFG_MAX_FLASH_SECT      67          /* max number of sectors on one chip */

#define CFG_FLASH_ERASE_TOUT    120000      /* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT    500         /* Timeout for Flash Write (in ms) */

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=elppc-0,nor1=elppc-1"
#define MTDPARTS_DEFAULT	"mtdparts=elppc-0:-(jffs2),elppc-1:-(user)"
*/

#define CFG_MONITOR_BASE        CFG_FLASH_BASE
#define CFG_MONITOR_LEN         0x40000     /* Reserve 256 kB for Monitor */
#define CFG_MALLOC_LEN          0x20000     /* Reserve 128 kB for malloc() */
#undef  CFG_MEMTEST

/*
 * Environment settings
 */
#define CONFIG_ENV_OVERWRITE
#define CFG_ENV_IS_IN_NVRAM     1           /* use NVRAM for environment vars */
#define CFG_NVRAM_SIZE          0x800       /* NVRAM size (2kB) */
#define CFG_ENV_SIZE            0x400       /* Size of Environment vars (1kB) */
#define CFG_ENV_ADDR            0x0
#define CFG_ENV_MAP_ADRS        0xff000000
#define CFG_NV_SROM_COPY_ADDR   (CFG_ENV_ADDR + CFG_ENV_SIZE)
#define CFG_NVRAM_ACCESS_ROUTINE            /* only byte accsess alowed */
#define CFG_SROM_SIZE           0x100       /* shadow of revision info is in nvram */

/*
 * Serial devices
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK         24000000
#define CFG_NS16550_COM1        (CFG_UART_BASE + 0)
#define CFG_NS16550_COM2        (CFG_UART_BASE + 8)

/*
 * PCI stuff
 */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_PCI_HOST         PCI_HOST_AUTO
#undef  CONFIG_PCI_SCAN_SHOW

/*
 * Optional Video console (graphic: SMI LynxEM)
 */
#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define VIDEO_KBD_INIT_FCT    (simple_strtol (getenv("console"), NULL, 10))
#define VIDEO_TSTC_FCT        serial_tstc
#define VIDEO_GETC_FCT        serial_getc

#define CONFIG_VIDEO_SMI_LYNXEM
#define CONFIG_VIDEO_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO

/*
 * Initial BATs
 */
#if 1

#define CFG_IBAT0L 0
#define CFG_IBAT0U 0
#define CFG_DBAT0L CFG_IBAT1L
#define CFG_DBAT0U CFG_IBAT1U

#define CFG_IBAT1L 0
#define CFG_IBAT1U 0
#define CFG_DBAT1L CFG_IBAT1L
#define CFG_DBAT1U CFG_IBAT1U

#define CFG_IBAT2L 0
#define CFG_IBAT2U 0
#define CFG_DBAT2L CFG_IBAT2L
#define CFG_DBAT2U CFG_IBAT2U

#define CFG_IBAT3L 0
#define CFG_IBAT3U 0
#define CFG_DBAT3L CFG_IBAT3L
#define CFG_DBAT3U CFG_IBAT3U

#else

/* SDRAM */
#define CFG_IBAT0L (CFG_SDRAM_BASE | BATL_RW)
#define CFG_IBAT0U (CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L CFG_IBAT1L
#define CFG_DBAT0U CFG_IBAT1U

/* address range for flashes */
#define CFG_IBAT1L (CFG_FLASH_BASE | BATL_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT1U (CFG_FLASH_BASE | BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_DBAT1L CFG_IBAT1L
#define CFG_DBAT1U CFG_IBAT1U

/* ISA IO space */
#define CFG_IBAT2L (CFG_ISA_IO | BATL_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT2U (CFG_ISA_IO | BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_DBAT2L CFG_IBAT2L
#define CFG_DBAT2U CFG_IBAT2U

/* ISA memory space */
#define CFG_IBAT3L (CFG_ISA_MEM | BATL_RW | BATL_CACHEINHIBIT)
#define CFG_IBAT3U (CFG_ISA_MEM | BATU_BL_16M | BATU_VS | BATU_VP)
#define CFG_DBAT3L CFG_IBAT3L
#define CFG_DBAT3U CFG_IBAT3U

#endif

/*
 * Speed settings are board specific
 */
#define CFG_BUS_HZ              100000000
#define CFG_CPU_CLK             400000000
#define CFG_BUS_CLK             CFG_BUS_HZ

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ           (8 << 20)           /* Initial Memory map for Linux */

/*
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE        32    /* For all MPC74xx CPUs */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT        5    /* log base 2 of the above value */
#endif

/*
 * L2CR setup -- make sure this is right for your board!
 * look in include/74xx_7xx.h for the defines used here
 */

#define CFG_L2

#if 1
#define L2_INIT     0       /* cpu 750 CXe*/
#else
#define L2_INIT     (L2CR_L2SIZ_2M | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		     L2CR_L2OH_5   | L2CR_L2CTL | L2CR_L2WT)
#endif
#define L2_ENABLE   (L2_INIT | L2CR_L2E)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD           0x01    /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM           0x02    /* Software reboot */

#define CONFIG_NET_MULTI        /* Multi ethernet cards support */
#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8               /* use 8 rx buffer on eepro100  */
#define CONFIG_EEPRO100_SROM_WRITE

#endif    /* __CONFIG_H */
