/*
 * Copyright 2011 Alex Dubov <oakad@yahoo.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Merury Computers MPQ101 board configuration file
 *
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_36BIT
# define CONFIG_PHYS_64BIT
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE      /* BOOKE */
#define CONFIG_E500       /* BOOKE e500 family */
#define CONFIG_MPC85xx    /* MPC8540/60/55/41/48 */
#define CONFIG_MPC8548    /* MPC8548 specific */
#define CONFIG_MPQ101     /* MPQ101 board specific */

#define CONFIG_SYS_SRIO   /* enable serial RapidIO */
#define CONFIG_TSEC_ENET  /* tsec ethernet support */
#define CONFIG_INTERRUPTS /* enable pci, srio, ddr interrupts */
#define CONFIG_FSL_LAW    /* Use common FSL init code */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE   /* toggle L2 cache */
#define CONFIG_BTB        /* toggle branch predition */

#define CONFIG_PANIC_HANG

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
# define CONFIG_ADDR_MAP
# define CONFIG_SYS_NUM_ADDR_MAP 16 /* number of TLB1 entries */
#endif


#define CONFIG_SYS_CLK_FREQ      33000000 /* sysclk for MPC85xx */

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

/* DDR Setup */
#define CONFIG_FSL_DDR2

#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER /* DDR controller or DMA? */

#define CONFIG_MEM_INIT_VALUE        0xDeadBeef
#define CONFIG_SYS_DDR_SDRAM_BASE    0x00000000 /* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE        CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS   1
#define CONFIG_DIMM_SLOTS_PER_CTLR   1
#define CONFIG_CHIP_SELECTS_PER_CTRL (2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/* Fixed 512MB DDR2 parameters */
#define CONFIG_SYS_SDRAM_SIZE_LOG    29 /* DDR is 512MB */
#define CONFIG_SYS_DDR_CS0_BNDS      0x0000001f
#define CONFIG_SYS_DDR_CS0_CONFIG    0x80014102
#define CONFIG_SYS_DDR_TIMING_3      0x00010000
#define CONFIG_SYS_DDR_TIMING_0      0x00260802
#define CONFIG_SYS_DDR_TIMING_1      0x5c47a432
#define CONFIG_SYS_DDR_TIMING_1_PERF 0x49352322
#define CONFIG_SYS_DDR_TIMING_2      0x03984cce
#define CONFIG_SYS_DDR_TIMING_2_PERF 0x14904cca
#define CONFIG_SYS_DDR_MODE_1        0x00400442
#define CONFIG_SYS_DDR_MODE_1_PERF   0x00480432
#define CONFIG_SYS_DDR_MODE_2        0x00000000
#define CONFIG_SYS_DDR_MODE_2_PERF   0x00000000
#define CONFIG_SYS_DDR_INTERVAL      0x08200100
#define CONFIG_SYS_DDR_INTERVAL_PERF 0x06180100
#define CONFIG_SYS_DDR_CLK_CTRL      0x03800000
#define CONFIG_SYS_DDR_CONTROL       0xc3008000 /* Type = DDR2 */
#define CONFIG_SYS_DDR_CONTROL2      0x04400000

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_START     0x0ff00000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END       0x0ffffffc

/*
 * RAM definitions
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR   0xe4010000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE   0x4000     /* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_INIT_RAM_SIZE \
				    - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET  CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN     (256 * 1024)  /* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN      (1024 * 1024) /* Reserved for malloc */

/*
 * Local Bus Definitions
 */
#define CONFIG_SYS_LBC_LCRR 0x00000004 /* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR 0x00000000 /* LB config reg */


/*
 * FLASH on the Local Bus
 * One bank, 128M, using the CFI driver.
 */
#define CONFIG_SYS_BOOT_BLOCK 0xf8000000            /* boot TLB block */
#define CONFIG_SYS_FLASH_BASE CONFIG_SYS_BOOT_BLOCK /* start of FLASH 128M */

#ifdef CONFIG_PHYS_64BIT
# define CONFIG_SYS_FLASH_BASE_PHYS 0xff8000000ull
#else
# define CONFIG_SYS_FLASH_BASE_PHYS CONFIG_SYS_FLASH_BASE
#endif

/* 0xf8001801 */
#define CONFIG_SYS_BR0_PRELIM (BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) \
			       | BR_PS_32 | BR_V)

/* 0xf8006ff7 */
#define CONFIG_SYS_OR0_PRELIM (MEG_TO_AM(128) | OR_GPCM_XAM | OR_GPCM_CSNT \
			       | OR_GPCM_ACS_DIV2 | OR_GPCM_XACS \
			       | OR_GPCM_SCY_15 | OR_GPCM_TRLX \
			       | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CONFIG_SYS_FLASH_BANKS_LIST    {CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_FLASH_SHOW_PROGRESS     45   /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS     1    /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT      512  /* sectors per device */

#define CONFIG_SYS_FLASH_ERASE_TOUT    60000 /* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT    500   /* Flash Write Timeout (ms) */

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_AMD_CHECK_DQ7
/*
 * When initializing flash, if we cannot find the manufacturer ID,
 * assume this is the AMD flash.
 */
#define CONFIG_ASSUME_AMD_FLASH

/*
 * Environment parameters
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_USE_PPCENV
#define ENV_IS_EMBEDDED
#define CONFIG_ENV_SECT_SIZE 0x40000   /* 256K */
#define CONFIG_ENV_SIZE      0x800

/* Environment at the start of flash sector, before text. */
#define CONFIG_ENV_ADDR         (CONFIG_SYS_TEXT_BASE - CONFIG_ENV_SIZE)
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE /* start of monitor */
#define CONFIG_SYS_TEXT_BASE    0xfffc0800
#define CONFIG_SYS_LDSCRIPT     "board/mercury/mpq101/u-boot.lds"

/*
 * Cypress CY7C67200 USB controller on the Local Bus.
 * Not supported by u-boot at present.
 */
#define CONFIG_SYS_LBC_OPTION_BASE 0xf0000000

#ifdef CONFIG_PHYS_64BIT
# define CONFIG_SYS_LBC_OPTION_BASE_PHYS 0xff0000000ull
#else
# define CONFIG_SYS_LBC_OPTION_BASE_PHYS CONFIG_SYS_LBC_OPTION_BASE
#endif

/* 0xf0001001 */
#define CONFIG_SYS_BR1_PRELIM (BR_PHYS_ADDR(CONFIG_SYS_LBC_OPTION_BASE_PHYS) \
			       | BR_PS_16 | BR_V)

/* fffff002 */
#define CONFIG_SYS_OR1_PRELIM (P2SZ_TO_AM(0x8000) | OR_GPCM_XAM \
			       | OR_GPCM_BCTLD | OR_GPCM_EHTR)

/*
 * Serial Ports
 */
#define CONFIG_CONS_INDEX           2
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE 1
#define CONFIG_SYS_NS16550_CLK      get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE   {300, 600, 1200, 2400, 4800, 9600, \
				     19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1     (CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2     (CONFIG_SYS_CCSRBAR+0x4600)

/*
 * I2C buses and peripherals
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x69} }

/* I2C RTC - M41T81 */
#define CONFIG_RTC_M41T62
#define CONFIG_SYS_I2C_RTC_ADDR     0x68
#define CONFIG_SYS_M41T11_BASE_YEAR 2000

/* I2C EEPROM - 24C256 */
#define CONFIG_SYS_I2C_EEPROM_ADDR            0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS     6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 12
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN        2
#define CONFIG_SYS_EEPROM_BUS_NUM             1

/*
 * RapidIO MMU
 */
#ifdef CONFIG_SYS_SRIO
# define CONFIG_SRIO1
# define CONFIG_SYS_SRIO1_MEM_VIRT  0xc0000000
# define CONFIG_SYS_SRIO1_MEM_SIZE  0x20000000 /* 512M */

# ifdef CONFIG_PHYS_64BIT
#  define CONFIG_SYS_SRIO1_MEM_PHYS 0xfc0000000ull
# else
#  define CONFIG_SYS_SRIO1_MEM_PHYS CONFIG_SYS_SRIO1_MEM_VIRT
# endif
#endif

/*
 * Ethernet
 */
#ifdef CONFIG_TSEC_ENET

# define CONFIG_MII                /* MII PHY management */
# define CONFIG_MII_DEFAULT_TSEC 1 /* Allow unregistered phys */

# define CONFIG_TSEC1
# define CONFIG_TSEC1_NAME       "eTSEC0"
# define TSEC1_PHY_ADDR          0x10
# define TSEC1_PHYIDX            0
# define TSEC1_FLAGS             (TSEC_GIGABIT | TSEC_REDUCED)

# define CONFIG_TSEC2
# define CONFIG_TSEC2_NAME       "eTSEC1"
# define TSEC2_PHY_ADDR          0x11
# define TSEC2_PHYIDX            0
# define TSEC2_FLAGS             (TSEC_GIGABIT | TSEC_REDUCED)

# define CONFIG_TSEC3
# define CONFIG_TSEC3_NAME       "eTSEC2"
# define TSEC3_PHY_ADDR          0x12
# define TSEC3_PHYIDX            0
# define TSEC3_FLAGS             (TSEC_GIGABIT | TSEC_REDUCED)

# define CONFIG_TSEC4
# define CONFIG_TSEC4_NAME       "eTSEC3"
# define TSEC4_PHY_ADDR          0x13
# define TSEC4_PHYIDX            0
# define TSEC4_FLAGS             (TSEC_GIGABIT | TSEC_REDUCED)

/* Options are: eTSEC[0-3] */
# define CONFIG_ETHPRIME         "eTSEC0"
# define CONFIG_PHY_GIGE         /* Include GbE speed/duplex detection */
#endif

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_I2C
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_JFFS2

/*
 * Miscellaneous configurable options
 */

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

#define CONFIG_FIT         /* new uImage format support */
#define CONFIG_FIT_VERBOSE /* enable fit_format_{error,warning}() */

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER


#define CONFIG_LOADS_ECHO            /* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE /* allow baudrate change */

#define CONFIG_SYS_LONGHELP          /* undef to save memory */
#define CONFIG_CMDLINE_EDITING       /* Command-line editing */
#define CONFIG_AUTO_COMPLETE         /* add autocompletion support */

#define CONFIG_SYS_LOAD_ADDR         0x2000000    /* default load address */
#define CONFIG_SYS_PROMPT            "MPQ-101=> " /* Monitor Command Prompt */

/* Console I/O Buffer Size */
#ifdef CONFIG_CMD_KGDB
# define CONFIG_SYS_CBSIZE 1024
#else
# define CONFIG_SYS_CBSIZE 256
#endif

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT)+16)

#define CONFIG_SYS_MAXARGS  16                /* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ       1000              /* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ (16 << 20) /* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN (16 << 20) /* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
# define CONFIG_KGDB_BAUDRATE  230400 /* speed to run kgdb serial port */
# define CONFIG_KGDB_SER_INDEX 2      /* which serial port to use */
#endif

/*
 * Basic Environment Configuration
 */
#define CONFIG_BAUDRATE  115200
#define CONFIG_BOOTDELAY 5            /* -1 disables auto-boot */

/*default location for tftp and bootm*/
#define CONFIG_LOADADDR  CONFIG_SYS_LOAD_ADDR

#endif /* __CONFIG_H */
