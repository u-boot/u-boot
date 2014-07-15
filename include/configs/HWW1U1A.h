/*
 * Copyright 2009-2010 eXMeritus, A Boeing Company
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * HardwareWall HWW-1U-1A airborne unit configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* High-level system configuration options */
#define CONFIG_BOOKE		/* Power/PowerPC Book-E			*/
#define CONFIG_E500		/* e500 (Power ISA v2.03 with SPE)	*/
#define CONFIG_FSL_ELBC		/* FreeScale Enhanced LocalBus Cntlr	*/
#define CONFIG_FSL_LAW		/* FreeScale Local Access Window	*/
#define CONFIG_P2020		/* FreeScale P2020			*/
#define CONFIG_HWW1U1A		/* eXMeritus HardwareWall HWW-1U-1A	*/
#define CONFIG_MP		/* Multiprocessing support		*/
#define CONFIG_HWCONFIG		/* Use hwconfig from environment	*/

#define CONFIG_L2_CACHE			/* L2 cache enabled		*/
#define CONFIG_BTB			/* Branch predition enabled	*/

#define CONFIG_PANIC_HANG		/* No board reset on panic	*/
#define CONFIG_BOARD_EARLY_INIT_R	/* Call board_early_init_r()	*/
#define CONFIG_CMD_REGINFO		/* Dump various CPU regs	*/

/*
 * Allow the use of 36-bit physical addresses.  Device-trees with 64-bit
 * addresses have known compatibility issues with some existing kernels.
 */
#define CONFIG_ENABLE_36BIT_PHYS
#define CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP 16 /* Number of entries in TLB1 */

/* Reserve plenty of RAM for malloc (we have 2GB+) */
#define CONFIG_SYS_MALLOC_LEN (1024 * 1024)

/* How much L2 cache do we map so we can use it as RAM */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_SIZE 0x00004000

/* This is our temporary global data area just above the stack */
#define CONFIG_SYS_GBL_DATA_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

/* The stack grows down from the global data area */
#define CONFIG_SYS_INIT_SP_OFFSET CONFIG_SYS_GBL_DATA_OFFSET

/* Enable IRQs and watchdog with a 1000Hz system decrementer */
#define CONFIG_CMD_IRQ

/* -------------------------------------------------------------------- */

/*
 * Clock crystal configuration:
 *  (1) SYS: 66.666MHz +/- 50ppm (drives CPU/PCI/DDR)
 *  (2) CCB: Multiplier from SYS_CLK
 *  (3) RTC: 25.000MHz +/- 50ppm (sampled against CCB clock)
 */
#define CONFIG_SYS_CLK_FREQ 66666000/*Hz*/
#define CONFIG_DDR_CLK_FREQ 66666000/*Hz*/


/* -------------------------------------------------------------------- */

/*
 * Memory map
 *
 * 0x0000_0000  0x7fff_ffff    2G  DDR2 ECC SDRAM
 * 0x8000_0000  0x9fff_ffff  512M  PCI-E Bus 1
 * 0xa000_0000  0xbfff_ffff  512M  PCI-E Bus 2 (unused)
 * 0xc000_0000  0xdfff_ffff  512M  PCI-E Bus 3
 * 0xe000_0000  0xe7ff_ffff  128M  Spansion FLASH
 * 0xe800_0000  0xefff_ffff  128M  Spansion FLASH
 * 0xffd0_0000  0xffd0_3fff   16K  L1 boot stack (TLB0)
 * 0xffe0_0000  0xffef_ffff    1M  CCSR
 * 0xffe0_5000  0xffe0_5fff    4K    Enhanced LocalBus Controller
 */

/* Virtual Memory Map */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_PCIE3_MEM_VIRT	0x80000000
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xc0000000
#define CONFIG_SYS_FLASH_BASE		0xe0000000
#define CONFIG_SYS_PCIE3_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc20000
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000
#define CONFIG_SYS_CCSRBAR		0xffe00000 /* CCSRBAR @ runtime */

#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000 /* 512M */
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000 /* 512M */
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000 /* 512M */
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000 /* 64k */
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000 /* 64k */
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000 /* 64k */

/* Physical Memory Map */
#define CONFIG_SYS_PCIE3_MEM_PHYS     0xc00000000ull
#define CONFIG_SYS_PCIE2_MEM_PHYS     0xc20000000ull
#define CONFIG_SYS_PCIE1_MEM_PHYS     0xc40000000ull
#define CONFIG_SYS_FLASH_BASE_PHYS    0xfe0000000ull
#define CONFIG_SYS_PCIE3_IO_PHYS      0xfffc00000ull
#define CONFIG_SYS_PCIE2_IO_PHYS      0xfffc10000ull
#define CONFIG_SYS_PCIE1_IO_PHYS      0xfffc20000ull
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS 0xfffd00000ull
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf		/* for ASM code */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW  0xffd00000	/* for ASM code */
#define CONFIG_SYS_CCSRBAR_PHYS_HIGH       0xf		/* for ASM code */
#define CONFIG_SYS_CCSRBAR_PHYS_LOW        0xffe00000	/* for ASM code */


/* -------------------------------------------------------------------- */

/* U-Boot image (MONITOR_BASE == TEXT_BASE) */
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc /* Top address in flash */
#define CONFIG_SYS_TEXT_BASE		0xeff80000 /* Start of U-Boot image */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		0x80000 /* 512kB (4 flash sectors) */

/*
 * U-Boot Environment Image:  The two sectors immediately below U-Boot
 * form the U-Boot environment (regular and redundant).
 */
#define CONFIG_ENV_IS_IN_FLASH	/* The environment image is stored in FLASH */
#define CONFIG_ENV_OVERWRITE	/* Allow "protected" variables to be erased */
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128kB (1 flash sector) */
#define CONFIG_ENV_ADDR        (CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_ADDR_REDUND (CONFIG_ENV_ADDR         - CONFIG_ENV_SECT_SIZE)

/* Only use 8kB of each environment sector for data */
#define CONFIG_ENV_SIZE		0x2000 /* 8kB */
#define CONFIG_ENV_SIZE_REDUND	0x2000 /* 8kB */


/* -------------------------------------------------------------------- */

/* Serial Console Configuration */
#define CONFIG_CONS_INDEX 1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE 1
#define CONFIG_SYS_NS16550_CLK get_bus_freq(0)

#define CONFIG_BAUDRATE 115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Echo back characters received during a serial download */
#define CONFIG_LOADS_ECHO

/* Allow a serial-download to temporarily change baud */
#define CONFIG_SYS_LOADS_BAUD_CHANGE


/* -------------------------------------------------------------------- */

/* PCI and PCI-Express Support */
#define CONFIG_PCI		/* Enable PCI/PCIE			*/
#define CONFIG_PCI_PNP		/* Scan PCI busses			*/
#define CONFIG_CMD_PCI		/* Enable the "pci" command		*/
#define CONFIG_FSL_PCI_INIT	/* Common FreeScale PCI initialization	*/
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_FSL_PCIE_RESET	/* We have PCI-E reset errata		*/
#define CONFIG_SYS_PCI_64BIT	/* PCI resources are 64-bit		*/
#define CONFIG_PCI_SCAN_SHOW	/* Display PCI scan during boot		*/

/* Enable 2 of the 3 PCI-E controllers */
#define CONFIG_PCIE3
#undef  CONFIG_PCIE2
#define CONFIG_PCIE1

/* Display human-readable names when initializing */
#define CONFIG_SYS_PCIE3_NAME "Intel 82571EB"
#define CONFIG_SYS_PCIE2_NAME "Unused"
#define CONFIG_SYS_PCIE1_NAME "Silicon Image SIL3531"

/*
 * PCI bus addresses
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
#define CONFIG_SYS_PCIE3_IO_BUS  0x00000000
#define CONFIG_SYS_PCIE2_IO_BUS  0x00000000
#define CONFIG_SYS_PCIE1_IO_BUS  0x00000000
#define CONFIG_SYS_PCIE3_MEM_BUS 0xe0000000
#define CONFIG_SYS_PCIE2_MEM_BUS 0xe0000000
#define CONFIG_SYS_PCIE1_MEM_BUS 0xe0000000


/* -------------------------------------------------------------------- */

/* Generic FreeScale hardware I2C support */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	400000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	400000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x29} }
#define CONFIG_CMD_I2C

/* DDR2 SO-RDIMM SPD EEPROM is at I2C0-0x51 */
#define CONFIG_SYS_SPD_BUS_NUM 0
#define SPD_EEPROM_ADDRESS 0x51

/* DS1339 RTC is at I2C0-0x68 (I know it says "DS1337", it's a DS1339) */
#define CONFIG_CMD_DATE
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_RTC_BUS_NUM 0
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
/* Turn off RTC square-wave output to save battery */
#define CONFIG_SYS_RTC_DS1337_NOOSC

/*
 * AT24C128N EEPROM at I2C0-0x53.
 *
 * That Atmel EEPROM has 128kbit of memory (16kByte) divided into 256 pages
 * of 64 bytes per page.  The chip uses 2-byte addresses and has a max write
 * cycle time of 20ms according to the datasheet.
 *
 * NOTE: Our environment is stored on regular direct-attached FLASH, this
 * chip is only used as a write-protected backup for certain key settings
 * such as the serial# and macaddr values.  (EG: "env import")
 */
#define CONFIG_CMD_EEPROM
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR 0x53
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6 /* 1 << 6 == 64 byte pages */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 21

/*
 * PCA9554 is at I2C1-0x3f (I know it says "PCA953X", it's a PCA9554).  You
 * must first select the I2C1 bus with "i2c dev 1" or the "pca953x" command
 * will not be able to access the chip.
 */
#define CONFIG_PCA953X
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO
#define CONFIG_SYS_I2C_PCA953X_ADDR 0x3f


/* -------------------------------------------------------------------- */

/* FreeScale DDR2/3 SDRAM Controller */
#define CONFIG_SYS_FSL_DDR2		/* Our SDRAM slot is DDR2		*/
#define CONFIG_DDR_ECC		/* Enable ECC by default		*/
#define CONFIG_DDR_SPD		/* Detect DDR config from SPD EEPROM	*/
#define CONFIG_SPD_EEPROM	/* ...why 2 config variables for this?	*/
#define CONFIG_VERY_BIG_RAM	/* Allow 2GB+ of RAM			*/
#define CONFIG_CMD_SDRAM

/* Standard P2020 DDR controller parameters */
#define CONFIG_NUM_DDR_CONTROLLERS 1
#define CONFIG_DIMM_SLOTS_PER_CTLR 1
#define CONFIG_CHIP_SELECTS_PER_CTRL 2

/* Make sure to tell the DDR controller to preinitialze all of RAM */
#define CONFIG_MEM_INIT_VALUE 0xDEADBEEF
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER


/* -------------------------------------------------------------------- */

/* FLASH Memory Configuration (2x 128MB SPANSION FLASH) */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_AMD_CHECK_DQ7

/* Flash banks (2x 128MB) */
#define FLASH0_PHYS (CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000ull)
#define FLASH1_PHYS (CONFIG_SYS_FLASH_BASE_PHYS + 0x0000000ull)
#define CONFIG_SYS_MAX_FLASH_BANKS 2
#define CONFIG_SYS_MAX_FLASH_SECT 1024
#define CONFIG_SYS_FLASH_BANKS_LIST { FLASH0_PHYS, FLASH1_PHYS }

/*
 * Flash access modes and timings (values are the defaults after a RESET).
 *
 * NOTE: These could probably be optimized but are more than sufficient for
 * this particular system for the moment.
 */
#define FLASH_BRx (BR_PS_16 | BR_MS_GPCM | BR_V)
#define FLASH_ORx (OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_XACS \
		| OR_GPCM_SCY_15 | OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

/* Configure both flash banks */
#define CONFIG_SYS_BR0_PRELIM (FLASH_BRx | BR_PHYS_ADDR(FLASH0_PHYS))
#define CONFIG_SYS_BR1_PRELIM (FLASH_BRx | BR_PHYS_ADDR(FLASH1_PHYS))
#define CONFIG_SYS_OR0_PRELIM (FLASH_ORx | OR_AM_128MB)
#define CONFIG_SYS_OR1_PRELIM (FLASH_ORx | OR_AM_128MB)

/* Flash timeouts (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT 60000UL /* Erase (60s)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT   500UL /* Write (0.5s)	*/

/* Quiet flash testing */
#define CONFIG_SYS_FLASH_QUIET_TEST

/* Make program/erase count down from 45/5 (9....8....7....) */
#define CONFIG_FLASH_SHOW_PROGRESS 45


/* -------------------------------------------------------------------- */

/* Ethernet Device Support */
#define CONFIG_MII			/* Enable MII PHY code		*/
#define CONFIG_MII_DEFAULT_TSEC		/* ??? Copied from P2020DS	*/
#define CONFIG_PHY_GIGE			/* Support Gigabit PHYs		*/
#define CONFIG_ETHPRIME "e1000#0"	/* Default to external ports	*/

/* Turn on various helpful networking commands */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING

/* On-chip FreeScale P2020 "tsec" Ethernet (oneway fibers and peer) */
#define CONFIG_TSEC_ENET
#define CONFIG_TSEC1
#define CONFIG_TSEC2
#define CONFIG_TSEC3
#define CONFIG_TSEC1_NAME "owt0"
#define CONFIG_TSEC2_NAME "owt1"
#define CONFIG_TSEC3_NAME "peer"
#define TSEC1_FLAGS (TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS (TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS (TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC1_PHYIDX 0
#define TSEC2_PHYIDX 0
#define TSEC3_PHYIDX 0
#define TSEC1_PHY_ADDR      2
#define TSEC2_PHY_ADDR      3
#define TSEC3_PHY_ADDR      4
#define TSEC3_PHY_ADDR_CPUA 4
#define TSEC3_PHY_ADDR_CPUB 5

/* PCI-E dual-port E1000 (external ethernet ports) */
#define CONFIG_E1000
#define CONFIG_E1000_SPI
#define CONFIG_E1000_SPI_GENERIC
#define CONFIG_CMD_E1000

/* We need the SPI infrastructure to poke the E1000's EEPROM */
#define CONFIG_SPI
#define CONFIG_SPI_X
#define CONFIG_CMD_SPI
#define MAX_SPI_BYTES 32


/* -------------------------------------------------------------------- */

/* USB Thumbdrive Device Support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_STORAGE
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_CMD_USB

/* Partition and Filesystem support */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT


/* -------------------------------------------------------------------- */

/* Command line configuration. */
#define CONFIG_CMDLINE_EDITING		/* Enable command editing	*/
#define CONFIG_COMMAND_HISTORY		/* Enable command history	*/
#define CONFIG_AUTO_COMPLETE		/* Enable command completion	*/
#define CONFIG_SYS_LONGHELP		/* Enable detailed command help	*/
#define CONFIG_SYS_MAXARGS 128		/* Up to 128 command-line args	*/
#define CONFIG_SYS_PBSIZE 8192		/* Allow up to 8k printed lines	*/
#define CONFIG_SYS_CBSIZE 4096		/* Allow up to 4k command lines	*/
#define CONFIG_SYS_BARGSIZE 4096	/* Allow up to 4k boot args	*/
#define CONFIG_SYS_HUSH_PARSER		/* Enable a fancier shell	*/

/* A little extra magic here for the prompt */
#define CONFIG_SYS_PROMPT hww1u1a_get_ps1()
#ifndef __ASSEMBLY__
const char *hww1u1a_get_ps1(void);
#endif

/* Include a bunch of default commands we probably want */
#include <config_cmd_default.h>

/* Other helpful shell-like commands */
#define CONFIG_MD5
#define CONFIG_SHA1
#define CONFIG_CMD_MD5SUM
#define CONFIG_CMD_SHA1SUM
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_SETEXPR


/* -------------------------------------------------------------------- */

/* Image manipulation and booting */

/* We use the OpenFirmware-esque "Flattened Device Tree" */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_CMD_ELF
#define CONFIG_SYS_BOOTMAPSZ (64 << 20)	/* Maximum kernel memory map	*/
#define CONFIG_SYS_BOOTM_LEN (64 << 20)	/* Maximum kernel size of 64MB	*/

/* This is the default address for commands with an optional address arg */
#define CONFIG_LOADADDR		  100000
#define CONFIG_SYS_LOAD_ADDR	0x100000

/* Test memory starting from the default load address to just below 2GB */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_MEMTEST_END		0x7f000000

#define CONFIG_BOOTDELAY 20
#define CONFIG_BOOTCOMMAND "echo Not yet flashed"
#define CONFIG_BOOTARGS ""
#define CONFIG_BOOTARGS_DYNAMIC "console=ttyS0,${baudrate}n1"

/* Extra environment parameters */
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"ethprime=e1000#0\0"						\
	"ethrotate=no\0"						\
	"setbootargs=setenv bootargs "					\
			"\"${bootargs} "CONFIG_BOOTARGS_DYNAMIC"\"\0"	\
	"perf_mode=performance\0"					\
	"hwconfig="	"fsl_ddr:ctlr_intlv=bank,bank_intlv=cs0_cs1;"	\
			"usb1:dr_mode=host,phy_type=ulpi\0"		\
	"flkernel=0xe8000000\0"						\
	"flinitramfs=0xe8800000\0"					\
	"fldevicetree=0xeff20000\0"					\
	"flbootm=bootm ${flkernel} ${flinitramfs} ${fldevicetree}\0"	\
	"flboot=run preboot; run flbootm\0"				\
	"restore_eeprom=i2c dev 0 && "					\
			"eeprom read $loadaddr 0x0000 0x2000 && "	\
			"env import -c $loadaddr 0x2000\0"

#endif	/* __CONFIG_H */
