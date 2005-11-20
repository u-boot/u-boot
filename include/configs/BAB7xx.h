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

#define CONFIG_BAB7xx           1       /* this is an BAB740/BAB750 board */

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

#define CONFIG_COMMANDS         (CONFIG_CMD_DFL | CFG_CMD_PCI | CFG_CMD_JFFS2 |\
				 CFG_CMD_SCSI   | CFG_CMD_IDE | CFG_CMD_DATE  |\
				 CFG_CMD_FDC    | CFG_CMD_ELF)

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
 * Choose the address mapping scheme for the MPC106 mem controller.
 * Default is mapping B (CHRP), set this define to choose mapping A (PReP).
 */
#define CFG_ADDRESS_MAP_A
#ifdef  CFG_ADDRESS_MAP_A

#define CFG_PCI_MEMORY_BUS      0x80000000
#define CFG_PCI_MEMORY_PHYS     0x00000000
#define CFG_PCI_MEMORY_SIZE     0x80000000

#define CFG_PCI_MEM_BUS         0x00000000
#define CFG_PCI_MEM_PHYS        0xc0000000
#define CFG_PCI_MEM_SIZE        0x3f000000

#define CFG_ISA_MEM_BUS         0
#define CFG_ISA_MEM_PHYS        0
#define CFG_ISA_MEM_SIZE        0

#define CFG_PCI_IO_BUS          0x1000
#define CFG_PCI_IO_PHYS         0x81000000
#define CFG_PCI_IO_SIZE         0x01000000-CFG_PCI_IO_BUS

#define CFG_ISA_IO_BUS          0x00000000
#define CFG_ISA_IO_PHYS         0x80000000
#define CFG_ISA_IO_SIZE         0x00800000

#else

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

#endif /*CFG_ADDRESS_MAP_A */

#define CFG_60X_PCI_MEM_OFFSET  0x00000000

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
#define CFG_FLASH_BASE          0xfff00000

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
/* No command line, one static partition */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support
 *
 * Note: fake mtd_id used, no linux mtd map file
 */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=bab7xx-0"
#define MTDPARTS_DEFAULT	"mtdparts=bab7xx-0:-(jffs2)"
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
#define CFG_NVRAM_SIZE          0x1ff0      /* NVRAM size (8kB), we must protect the clock data (16 bytes) */
#define CFG_ENV_SIZE            0x400       /* Size of Environment vars (1kB) */
/*
 * We store the environment and an image of revision eeprom in the upper part of the NVRAM. Thus,
 * user applications can use the remaining space for other purposes.
 */
#define CFG_ENV_ADDR            (CFG_NVRAM_SIZE +0x10 -0x800)
#define CFG_NV_SROM_COPY_ADDR   (CFG_NVRAM_SIZE +0x10 -0x400)
#define CFG_NVRAM_ACCESS_ROUTINE            /* This board needs a special routine to access the NVRAM */
#define CFG_SROM_SIZE           0x100       /* shadow of revision info is in nvram */

/*
 * Serial devices
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE    1
#define CFG_NS16550_CLK         1843200
#define CFG_NS16550_COM1        (CFG_ISA_IO + CFG_NS87308_UART1_BASE)
#define CFG_NS16550_COM2        (CFG_ISA_IO + CFG_NS87308_UART2_BASE)

/*
 * PCI stuff
 */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_PCI_HOST         PCI_HOST_AUTO
#undef  CONFIG_PCI_SCAN_SHOW

/*
 * Video console (graphic: SMI LynxEM, keyboard: i8042)
 */
#define CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_VIDEO_SMI_LYNXEM
#define CONFIG_I8042_KBD
#define CONFIG_VIDEO_LOGO
#define CONFIG_CONSOLE_TIME
#define CONFIG_CONSOLE_EXTRA_INFO
#define CONFIG_CONSOLE_CURSOR
#define CFG_CONSOLE_BLINK_COUNT         30000    /* approx. 2 HZ */

/*
 * IDE/SCSI globals
 */
#ifndef __ASSEMBLY__
extern unsigned int    eltec_board;
extern unsigned int    ata_reset_time;
extern unsigned int    scsi_reset_time;
extern unsigned short  scsi_dev_id;
extern unsigned int    scsi_max_scsi_id;
extern unsigned char   scsi_sym53c8xx_ccf;
#endif

/*
 * ATAPI Support (experimental)
 */
#define CONFIG_ATAPI
#define CFG_IDE_MAXBUS          1                       /* max. 2 IDE busses    */
#define CFG_IDE_MAXDEVICE       (CFG_IDE_MAXBUS*2)      /* max. 2 drives per IDE bus */

#define CFG_ATA_BASE_ADDR       CFG_60X_PCI_IO_OFFSET   /* base address */
#define CFG_ATA_IDE0_OFFSET     0x1F0                   /* default ide0 offste */
#define CFG_ATA_IDE1_OFFSET     0x170                   /* default ide1 offset */
#define CFG_ATA_DATA_OFFSET     0                       /* data reg offset    */
#define CFG_ATA_REG_OFFSET      0                       /* reg offset */
#define CFG_ATA_ALT_OFFSET      0x200                   /* alternate register offset */

#define ATA_RESET_TIME          (ata_reset_time)

#undef  CONFIG_IDE_PCMCIA                               /* no pcmcia interface required */
#undef  CONFIG_IDE_LED                                  /* no led for ide supported */

/*
 * SCSI support (experimental) only SYM53C8xx supported
 */
#define CONFIG_SCSI_SYM53C8XX
#define CONFIG_SCSI_DEV_ID      (scsi_dev_id)           /* 875 or 860 */
#define CFG_SCSI_SYM53C8XX_CCF  (scsi_sym53c8xx_ccf)    /* value for none 40 mhz clocks */
#define CFG_SCSI_MAX_LUN        8                       /* number of supported LUNs */
#define CFG_SCSI_MAX_SCSI_ID    (scsi_max_scsi_id)      /* max SCSI ID (0-6) */
#define CFG_SCSI_MAX_DEVICE     (15 * CFG_SCSI_MAX_LUN) /* max. Target devices */
#define CFG_SCSI_SPIN_UP_TIME   (scsi_reset_time)

/*
 * Partion suppport
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION

/*
 * Winbond Configuration
 */
#define CFG_WINBOND_83C553      1                       /* has a winbond bridge */
#define CFG_USE_WINBOND_IDE     0                       /* use winbond 83c553 internal ide */
#define CFG_WINBOND_ISA_CFG_ADDR    0x80005800          /* pci-isa bridge config addr */
#define CFG_WINBOND_IDE_CFG_ADDR    0x80005900          /* ide config addr */

/*
 * NS87308 Configuration
 */
#define CFG_NS87308                    /* Nat Semi super-io cntr on ISA bus */
#define CFG_NS87308_BADDR_10    1
#define CFG_NS87308_DEVS        (CFG_NS87308_UART1   | \
				 CFG_NS87308_UART2   | \
				 CFG_NS87308_KBC1    | \
				 CFG_NS87308_MOUSE   | \
				 CFG_NS87308_FDC     | \
				 CFG_NS87308_RARP    | \
				 CFG_NS87308_GPIO    | \
				 CFG_NS87308_POWRMAN | \
				 CFG_NS87308_RTC_APC )

#define CFG_NS87308_PS2MOD
#define CFG_NS87308_GPIO_BASE   0x0220
#define CFG_NS87308_PWMAN_BASE  0x0460
#define CFG_NS87308_PMC2        0x00        /* SuperI/O clock source is 24MHz via X1 */

/*
 * set up the NVRAM access registers
 * NVRAM's controlled by the configurable CS line from the 87308
 */
#define CFG_NS87308_CS0_BASE    0x0076
#define CFG_NS87308_CS0_CONF    0x40
#define CFG_NS87308_CS1_BASE    0x0070
#define CFG_NS87308_CS1_CONF    0x1C
#define CFG_NS87308_CS2_BASE    0x0071
#define CFG_NS87308_CS2_CONF    0x1C

#define CONFIG_RTC_MK48T59

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
#ifndef __ASSEMBLY__
extern  unsigned long           bab7xx_get_bus_freq (void);
extern  unsigned long           bab7xx_get_gclk_freq (void);
#endif
#define CFG_BUS_HZ              bab7xx_get_bus_freq()
#define CFG_BUS_CLK             CFG_BUS_HZ
#define CFG_CPU_CLK             bab7xx_get_gclk_freq()

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
 * L2 Cache Configuration is board specific for BAB740/BAB750
 * Init values read from revision srom.
 */
#undef  CFG_L2
#define L2_INIT     (L2CR_L2SIZ_HM | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		     L2CR_L2OH_5   | L2CR_L2CTL   | L2CR_L2WT)
#define L2_ENABLE   (L2_INIT | L2CR_L2E)

#define CFG_L2_BAB7xx

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD           0x01    /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM           0x02    /* Software reboot */


#define CONFIG_NET_MULTI                /* Multi ethernet cards support */
#define CONFIG_TULIP
#define CONFIG_TULIP_SELECT_MEDIA

#endif    /* __CONFIG_H */
