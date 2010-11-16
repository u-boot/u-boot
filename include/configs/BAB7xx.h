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

#define GTREGREAD(x) 0xffffffff         /* needed for debug */

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

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
#define CONFIG_SYS_LOADS_BAUD_CHANGE           /* allow baudrate changes */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH

#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PCI
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_IDE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_FDC
#define CONFIG_CMD_ELF


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP                    /* undef to save memory */
#define CONFIG_SYS_PROMPT              "=> "   /* Monitor Command Prompt */

/*
 * choose between COM1 and COM2 as serial console
 */
#define CONFIG_CONS_INDEX       1

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE              1024        /* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE              256         /* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS             16          /* max number of command args    */
#define CONFIG_SYS_BARGSIZE            CONFIG_SYS_CBSIZE  /* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START       0x00000000  /* memtest works on    */
#define CONFIG_SYS_MEMTEST_END         0x04000000  /* 0 ... 64 MB in DRAM    */

#define CONFIG_SYS_LOAD_ADDR           0x1000000   /* default load address    */

#define CONFIG_SYS_HZ                  1000        /* dec. freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define CONFIG_SYS_BOARD_ASM_INIT
#define CONFIG_MISC_INIT_R

/*
 * Choose the address mapping scheme for the MPC106 mem controller.
 * Default is mapping B (CHRP), set this define to choose mapping A (PReP).
 */
#define CONFIG_SYS_ADDRESS_MAP_A
#ifdef  CONFIG_SYS_ADDRESS_MAP_A

#define CONFIG_SYS_PCI_MEMORY_BUS      0x80000000
#define CONFIG_SYS_PCI_MEMORY_PHYS     0x00000000
#define CONFIG_SYS_PCI_MEMORY_SIZE     0x80000000

#define CONFIG_SYS_PCI_MEM_BUS         0x00000000
#define CONFIG_SYS_PCI_MEM_PHYS        0xc0000000
#define CONFIG_SYS_PCI_MEM_SIZE        0x3f000000

#define CONFIG_SYS_ISA_MEM_BUS         0
#define CONFIG_SYS_ISA_MEM_PHYS        0
#define CONFIG_SYS_ISA_MEM_SIZE        0

#define CONFIG_SYS_PCI_IO_BUS          0x1000
#define CONFIG_SYS_PCI_IO_PHYS         0x81000000
#define CONFIG_SYS_PCI_IO_SIZE         0x01000000-CONFIG_SYS_PCI_IO_BUS

#define CONFIG_SYS_ISA_IO_BUS          0x00000000
#define CONFIG_SYS_ISA_IO_PHYS         0x80000000
#define CONFIG_SYS_ISA_IO_SIZE         0x00800000

#else

#define CONFIG_SYS_PCI_MEMORY_BUS      0x00000000
#define CONFIG_SYS_PCI_MEMORY_PHYS     0x00000000
#define CONFIG_SYS_PCI_MEMORY_SIZE     0x40000000

#define CONFIG_SYS_PCI_MEM_BUS         0x80000000
#define CONFIG_SYS_PCI_MEM_PHYS        0x80000000
#define CONFIG_SYS_PCI_MEM_SIZE        0x7d000000

#define CONFIG_SYS_ISA_MEM_BUS         0x00000000
#define CONFIG_SYS_ISA_MEM_PHYS        0xfd000000
#define CONFIG_SYS_ISA_MEM_SIZE        0x01000000

#define CONFIG_SYS_PCI_IO_BUS          0x00800000
#define CONFIG_SYS_PCI_IO_PHYS         0xfe800000
#define CONFIG_SYS_PCI_IO_SIZE         0x00400000

#define CONFIG_SYS_ISA_IO_BUS          0x00000000
#define CONFIG_SYS_ISA_IO_PHYS         0xfe000000
#define CONFIG_SYS_ISA_IO_SIZE         0x00800000

#endif /*CONFIG_SYS_ADDRESS_MAP_A */

#define CONFIG_SYS_60X_PCI_MEM_OFFSET  0x00000000

/* driver defines FDC,IDE,... */
#define CONFIG_SYS_ISA_IO_BASE_ADDRESS CONFIG_SYS_ISA_IO_PHYS
#define CONFIG_SYS_ISA_IO              CONFIG_SYS_ISA_IO_PHYS
#define CONFIG_SYS_60X_PCI_IO_OFFSET   CONFIG_SYS_ISA_IO_PHYS

/*
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE          0x00000000
#define CONFIG_SYS_FLASH_BASE          0xfff00000

/*
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR       0x00fd0000  /* above the memtest region */
#define CONFIG_SYS_INIT_RAM_SIZE        0x4000
#define CONFIG_SYS_GBL_DATA_OFFSET     (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET      CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Flash mapping/organization on the MPC10x.
 */
#define FLASH_BASE0_PRELIM      0xff800000
#define FLASH_BASE1_PRELIM      0xffc00000

#define CONFIG_SYS_MAX_FLASH_BANKS     2           /* max number of memory banks    */
#define CONFIG_SYS_MAX_FLASH_SECT      67          /* max number of sectors on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT    120000      /* Timeout for Flash Erase (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT    500         /* Timeout for Flash Write (in ms) */

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support
 *
 * Note: fake mtd_id used, no linux mtd map file
 */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		"nor0=bab7xx-0"
#define MTDPARTS_DEFAULT	"mtdparts=bab7xx-0:-(jffs2)"
*/

#define CONFIG_SYS_MONITOR_BASE        CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN         0x40000     /* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MALLOC_LEN          0x20000     /* Reserve 128 kB for malloc() */
#undef  CONFIG_SYS_MEMTEST

/*
 * Environment settings
 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_IS_IN_NVRAM     1           /* use NVRAM for environment vars */
#define CONFIG_SYS_NVRAM_SIZE          0x1ff0      /* NVRAM size (8kB), we must protect the clock data (16 bytes) */
#define CONFIG_ENV_SIZE            0x400       /* Size of Environment vars (1kB) */
/*
 * We store the environment and an image of revision eeprom in the upper part of the NVRAM. Thus,
 * user applications can use the remaining space for other purposes.
 */
#define CONFIG_ENV_ADDR            (CONFIG_SYS_NVRAM_SIZE +0x10 -0x800)
#define CONFIG_SYS_NV_SROM_COPY_ADDR   (CONFIG_SYS_NVRAM_SIZE +0x10 -0x400)
#define CONFIG_SYS_NVRAM_ACCESS_ROUTINE            /* This board needs a special routine to access the NVRAM */
#define CONFIG_SYS_SROM_SIZE           0x100       /* shadow of revision info is in nvram */

/*
 * Serial devices
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE    1
#define CONFIG_SYS_NS16550_CLK         1843200
#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_ISA_IO + CONFIG_SYS_NS87308_UART1_BASE)
#define CONFIG_SYS_NS16550_COM2        (CONFIG_SYS_ISA_IO + CONFIG_SYS_NS87308_UART2_BASE)

/*
 * PCI stuff
 */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_SYS_EARLY_PCI_INIT
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
#define CONFIG_SYS_CONSOLE_BLINK_COUNT         30000    /* approx. 2 HZ */

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
#define CONFIG_SYS_IDE_MAXBUS          1                       /* max. 2 IDE busses    */
#define CONFIG_SYS_IDE_MAXDEVICE       (CONFIG_SYS_IDE_MAXBUS*2)      /* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR       CONFIG_SYS_60X_PCI_IO_OFFSET   /* base address */
#define CONFIG_SYS_ATA_IDE0_OFFSET     0x1F0                   /* default ide0 offste */
#define CONFIG_SYS_ATA_IDE1_OFFSET     0x170                   /* default ide1 offset */
#define CONFIG_SYS_ATA_DATA_OFFSET     0                       /* data reg offset    */
#define CONFIG_SYS_ATA_REG_OFFSET      0                       /* reg offset */
#define CONFIG_SYS_ATA_ALT_OFFSET      0x200                   /* alternate register offset */

#define ATA_RESET_TIME          (ata_reset_time)

#undef  CONFIG_IDE_PCMCIA                               /* no pcmcia interface required */
#undef  CONFIG_IDE_LED                                  /* no led for ide supported */

/*
 * SCSI support (experimental) only SYM53C8xx supported
 */
#define CONFIG_SCSI_SYM53C8XX
#define CONFIG_SCSI_DEV_ID      (scsi_dev_id)           /* 875 or 860 */
#define CONFIG_SYS_SCSI_SYM53C8XX_CCF  (scsi_sym53c8xx_ccf)    /* value for none 40 mhz clocks */
#define CONFIG_SYS_SCSI_MAX_LUN        8                       /* number of supported LUNs */
#define CONFIG_SYS_SCSI_MAX_SCSI_ID    (scsi_max_scsi_id)      /* max SCSI ID (0-6) */
#define CONFIG_SYS_SCSI_MAX_DEVICE     (15 * CONFIG_SYS_SCSI_MAX_LUN) /* max. Target devices */
#define CONFIG_SYS_SCSI_SPIN_UP_TIME   (scsi_reset_time)

/*
 * Partion suppport
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_MAC_PARTITION
#define CONFIG_ISO_PARTITION

/*
 * Winbond Configuration
 */
#define CONFIG_WINBOND_83C553      1                       /* has a winbond bridge */
#define CONFIG_SYS_USE_WINBOND_IDE     0                       /* use winbond 83c553 internal ide */
#define CONFIG_SYS_WINBOND_ISA_CFG_ADDR    0x80005800          /* pci-isa bridge config addr */
#define CONFIG_SYS_WINBOND_IDE_CFG_ADDR    0x80005900          /* ide config addr */

/*
 * NS87308 Configuration
 */
#define CONFIG_NS87308                    /* Nat Semi super-io cntr on ISA bus */
#define CONFIG_SYS_NS87308_BADDR_10    1
#define CONFIG_SYS_NS87308_DEVS        (CONFIG_SYS_NS87308_UART1   | \
				 CONFIG_SYS_NS87308_UART2   | \
				 CONFIG_SYS_NS87308_KBC1    | \
				 CONFIG_SYS_NS87308_MOUSE   | \
				 CONFIG_SYS_NS87308_FDC     | \
				 CONFIG_SYS_NS87308_RARP    | \
				 CONFIG_SYS_NS87308_GPIO    | \
				 CONFIG_SYS_NS87308_POWRMAN | \
				 CONFIG_SYS_NS87308_RTC_APC )

#define CONFIG_SYS_NS87308_PS2MOD
#define CONFIG_SYS_NS87308_GPIO_BASE   0x0220
#define CONFIG_SYS_NS87308_PWMAN_BASE  0x0460
#define CONFIG_SYS_NS87308_PMC2        0x00        /* SuperI/O clock source is 24MHz via X1 */

/*
 * set up the NVRAM access registers
 * NVRAM's controlled by the configurable CS line from the 87308
 */
#define CONFIG_SYS_NS87308_CS0_BASE    0x0076
#define CONFIG_SYS_NS87308_CS0_CONF    0x40
#define CONFIG_SYS_NS87308_CS1_BASE    0x0070
#define CONFIG_SYS_NS87308_CS1_CONF    0x1C
#define CONFIG_SYS_NS87308_CS2_BASE    0x0071
#define CONFIG_SYS_NS87308_CS2_CONF    0x1C

#define CONFIG_RTC_MK48T59

/*
 * Initial BATs
 */
#if 1

#define CONFIG_SYS_IBAT0L 0
#define CONFIG_SYS_IBAT0U 0
#define CONFIG_SYS_DBAT0L CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT1U

#define CONFIG_SYS_IBAT1L 0
#define CONFIG_SYS_IBAT1U 0
#define CONFIG_SYS_DBAT1L CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U CONFIG_SYS_IBAT1U

#define CONFIG_SYS_IBAT2L 0
#define CONFIG_SYS_IBAT2U 0
#define CONFIG_SYS_DBAT2L CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U CONFIG_SYS_IBAT2U

#define CONFIG_SYS_IBAT3L 0
#define CONFIG_SYS_IBAT3U 0
#define CONFIG_SYS_DBAT3L CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U CONFIG_SYS_IBAT3U

#else

/* SDRAM */
#define CONFIG_SYS_IBAT0L (CONFIG_SYS_SDRAM_BASE | BATL_RW)
#define CONFIG_SYS_IBAT0U (CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT0U CONFIG_SYS_IBAT1U

/* address range for flashes */
#define CONFIG_SYS_IBAT1L (CONFIG_SYS_FLASH_BASE | BATL_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT1U (CONFIG_SYS_FLASH_BASE | BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U CONFIG_SYS_IBAT1U

/* ISA IO space */
#define CONFIG_SYS_IBAT2L (CONFIG_SYS_ISA_IO | BATL_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT2U (CONFIG_SYS_ISA_IO | BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT2L CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U CONFIG_SYS_IBAT2U

/* ISA memory space */
#define CONFIG_SYS_IBAT3L (CONFIG_SYS_ISA_MEM | BATL_RW | BATL_CACHEINHIBIT)
#define CONFIG_SYS_IBAT3U (CONFIG_SYS_ISA_MEM | BATU_BL_16M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT3L CONFIG_SYS_IBAT3L
#define CONFIG_SYS_DBAT3U CONFIG_SYS_IBAT3U

#endif

/*
 * Speed settings are board specific
 */
#ifndef __ASSEMBLY__
extern  unsigned long           bab7xx_get_bus_freq (void);
extern  unsigned long           bab7xx_get_gclk_freq (void);
#endif
#define CONFIG_SYS_BUS_CLK	bab7xx_get_bus_freq()
#define CONFIG_SYS_CPU_CLK	bab7xx_get_gclk_freq()

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ           (8 << 20)           /* Initial Memory map for Linux */

/*
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE        32    /* For all MPC74xx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT        5    /* log base 2 of the above value */
#endif

/*
 * L2 Cache Configuration is board specific for BAB740/BAB750
 * Init values read from revision srom.
 */
#undef  CONFIG_SYS_L2
#define L2_INIT     (L2CR_L2SIZ_HM | L2CR_L2CLK_3 | L2CR_L2RAM_BURST | \
		     L2CR_L2OH_5   | L2CR_L2CTL   | L2CR_L2WT)
#define L2_ENABLE   (L2_INIT | L2CR_L2E)

#define CONFIG_SYS_L2_BAB7xx

#define CONFIG_NET_MULTI                /* Multi ethernet cards support */
#define CONFIG_TULIP
#define CONFIG_TULIP_SELECT_MEDIA

#endif    /* __CONFIG_H */
