/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * P010 RDB board configuration file
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_36BIT
#define CONFIG_PHYS_64BIT
#endif

#ifdef CONFIG_P1010RDB
#define CONFIG_P1010
#define CONFIG_NAND_FSL_IFC
#endif

#ifdef CONFIG_SDCARD
#define CONFIG_RAMBOOT_SDCARD
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#ifdef CONFIG_SPIFLASH
#define CONFIG_RAMBOOT_SPIFLASH
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#ifdef CONFIG_NAND	/* NAND Boot */
#define CONFIG_RAMBOOT_NAND
#define CONFIG_NAND_U_BOOT
#define CONFIG_SYS_TEXT_BASE_SPL	0xff800000
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE_SPL
#else
#define CONFIG_SYS_TEXT_BASE		0x11001000
#endif /* CONFIG_NAND_SPL */
#endif


#ifdef CONFIG_NAND_SECBOOT	/* NAND Boot */
#define CONFIG_RAMBOOT_NAND
#define CONFIG_SYS_TEXT_BASE		0x11000000
#define CONFIG_RESET_VECTOR_ADDRESS	0x1107fffc
#endif

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0xeff80000
#endif

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE    /* start of monitor */
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE			/* BOOKE */
#define CONFIG_E500			/* BOOKE e500 family */
#define CONFIG_MPC85xx
#define CONFIG_FSL_IFC			/* Enable IFC Support */
#define CONFIG_SYS_HAS_SERDES		/* common SERDES init code */

#define CONFIG_PCI			/* Enable PCI/PCIE */
#if defined(CONFIG_PCI)
#define CONFIG_PCIE1			/* PCIE controler 1 (slot 1) */
#define CONFIG_PCIE2			/* PCIE controler 2 (slot 2) */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_FSL_PCIE_RESET		/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI

#define CONFIG_E1000			/*  E1000 pci Ethernet card*/

/*
 * PCI Windows
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
/* controller 1, Slot 1, tgtid 1, Base address a000 */
#define CONFIG_SYS_PCIE1_NAME		"mini PCIe Slot"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xffc00000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xfffc00000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xffc00000
#endif

/* controller 2, Slot 2, tgtid 2, Base address 9000 */
#define CONFIG_SYS_PCIE2_NAME		"PCIe Slot"
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xfffc10000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xffc10000
#endif

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION
#endif

#define CONFIG_FSL_LAW			/* Use common FSL init code */
#define CONFIG_TSEC_ENET
#define CONFIG_ENV_OVERWRITE

#define CONFIG_DDR_CLK_FREQ	66666666 /* DDRCLK on P1010 RDB */
#define CONFIG_SYS_CLK_FREQ	66666666 /* SYSCLK for P1010 RDB */

#ifndef CONFIG_SDCARD
#define CONFIG_MISC_INIT_R
#endif

#define CONFIG_HWCONFIG
/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE			/* toggle L2 cache */
#define CONFIG_BTB			/* toggle branch predition */

#define CONFIG_ADDR_STREAMING		/* toggle addr streaming */

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP			1
#define CONFIG_SYS_NUM_ADDR_MAP		16	/* number of TLB1 entries */
#endif

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x1fffffff
#define CONFIG_PANIC_HANG		/* do not reset board on panic */

/* DDR Setup */
#define CONFIG_FSL_DDR3
#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_DDR_SPD
#define CONFIG_SYS_SPD_BUS_NUM		1
#define SPD_EEPROM_ADDRESS		0x52

#define CONFIG_MEM_INIT_VALUE		0xDeadBeef

#ifndef __ASSEMBLY__
extern unsigned long get_sdram_size(void);
#endif
#define CONFIG_SYS_SDRAM_SIZE		get_sdram_size() /* DDR size */
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	1

/* DDR3 Controller Settings */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000003f
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014302
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_DATA_INIT	0xdeadbeef
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000

#define CONFIG_SYS_DDR_ZQ_CONTROL	0x89080600
#define CONFIG_SYS_DDR_SR_CNTR		0x00000000
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#define CONFIG_SYS_DDR_CONTROL		0x470C0000	/* Type = DDR3  */
#define CONFIG_SYS_DDR_CONTROL_2	0x04401010
#define CONFIG_SYS_DDR_TIMING_4		0x00000001
#define CONFIG_SYS_DDR_TIMING_5		0x03402400

#define CONFIG_SYS_DDR_TIMING_3_800	0x00020000
#define CONFIG_SYS_DDR_TIMING_0_800	0x00330004
#define CONFIG_SYS_DDR_TIMING_1_800	0x6f6B4644
#define CONFIG_SYS_DDR_TIMING_2_800	0x0FA888CF
#define CONFIG_SYS_DDR_CLK_CTRL_800	0x03000000
#define CONFIG_SYS_DDR_MODE_1_800	0x40461520
#define CONFIG_SYS_DDR_MODE_2_800	0x8000c000
#define CONFIG_SYS_DDR_INTERVAL_800	0x0C300100
#define CONFIG_SYS_DDR_WRLVL_CONTROL_800	0x8655A608

/* settings for DDR3 at 667MT/s */
#define CONFIG_SYS_DDR_TIMING_3_667		0x00010000
#define CONFIG_SYS_DDR_TIMING_0_667		0x00110004
#define CONFIG_SYS_DDR_TIMING_1_667		0x5d59e544
#define CONFIG_SYS_DDR_TIMING_2_667		0x0FA890CD
#define CONFIG_SYS_DDR_CLK_CTRL_667		0x03000000
#define CONFIG_SYS_DDR_MODE_1_667		0x00441210
#define CONFIG_SYS_DDR_MODE_2_667		0x00000000
#define CONFIG_SYS_DDR_INTERVAL_667		0x0a280000
#define CONFIG_SYS_DDR_WRLVL_CONTROL_667	0x8675F608

#define CONFIG_SYS_CCSRBAR			0xffe00000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW		CONFIG_SYS_CCSRBAR

/* Don't relocate CCSRBAR while in NAND_SPL */
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_CCSR_DO_NOT_RELOCATE
#endif

/*
 * Memory map
 *
 * 0x0000_0000	0x3fff_ffff	DDR			1G cacheable
 * 0x8000_0000  0xbfff_ffff	PCI Express Mem		1.5G non-cacheable
 * 0xffc0_0000  0xffc3_ffff	PCI IO range		256k non-cacheable
 *
 * Localbus non-cacheable
 * 0xff80_0000	0xff8f_ffff	NAND Flash		1M non-cacheable
 * 0xffb0_0000	0xffbf_ffff	Board CPLD		1M non-cacheable
 * 0xffd0_0000	0xffd0_3fff	L1 for stack		16K Cacheable TLB0
 * 0xffe0_0000	0xffef_ffff	CCSR			1M non-cacheable
 */

/* In case of SD card boot, IFC interface is not available because of muxing */
#ifdef CONFIG_SDCARD
#define CONFIG_SYS_NO_FLASH
#else
/*
 * IFC Definitions
 */
/* NOR Flash on IFC */
#define CONFIG_SYS_FLASH_BASE		0xee000000
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* 32M */

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | CONFIG_SYS_FLASH_BASE)
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_SYS_NOR_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CONFIG_SYS_NOR_AMASK	IFC_AMASK(32*1024*1024)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(7)
/* NOR Flash Timing Params */
#define CONFIG_SYS_NOR_FTIM0	FTIM0_NOR_TACSE(0x4) | \
				FTIM0_NOR_TEADC(0x5) | \
				FTIM0_NOR_TEAHC(0x5)
#define CONFIG_SYS_NOR_FTIM1	FTIM1_NOR_TACO(0x1e) | \
				FTIM1_NOR_TRAD_NOR(0x0f)
#define CONFIG_SYS_NOR_FTIM2	FTIM2_NOR_TCS(0x4) | \
				FTIM2_NOR_TCH(0x4) | \
				FTIM2_NOR_TWP(0x1c)
#define CONFIG_SYS_NOR_FTIM3	0x0

#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS}
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */

#undef CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

/* CFI for NOR Flash */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

/* NAND Flash on IFC */
#define CONFIG_SYS_NAND_BASE		0xff800000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_NAND_BASE_PHYS	0xfff800000ull
#else
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#endif

#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8	\
				| CSPR_MSEL_NAND	\
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64*1024)
#define CONFIG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN	/* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_2	/* RAL = 2 Bytes */ \
				| CSOR_NAND_PGS_512	/* Page Size = 512b */ \
				| CSOR_NAND_SPRZ_16	/* Spare size = 16 */ \
				| CSOR_NAND_PB(32))	/* 32 Pages Per Block */

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_BLOCK_SIZE	(16 * 1024)

/* NAND Flash Timing Params */
#define CONFIG_SYS_NAND_FTIM0		FTIM0_NAND_TCCST(0x01) | \
					FTIM0_NAND_TWP(0x0C)   | \
					FTIM0_NAND_TWCHT(0x04) | \
					FTIM0_NAND_TWH(0x05)
#define CONFIG_SYS_NAND_FTIM1		FTIM1_NAND_TADLE(0x1d) | \
					FTIM1_NAND_TWBE(0x1d)  | \
					FTIM1_NAND_TRR(0x07)   | \
					FTIM1_NAND_TRP(0x0c)
#define CONFIG_SYS_NAND_FTIM2		FTIM2_NAND_TRAD(0x0c) | \
					FTIM2_NAND_TREH(0x05) | \
					FTIM2_NAND_TWHRE(0x0f)
#define CONFIG_SYS_NAND_FTIM3		FTIM3_NAND_TWW(0x04)

#define CONFIG_SYS_NAND_DDR_LAW		11

/* Set up IFC registers for boot location NOR/NAND */
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SECBOOT)
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NOR_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NOR_FTIM3
#else
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NAND_FTIM3
#endif

/* NAND boot: 8K NAND loader config */
#define CONFIG_SYS_NAND_SPL_SIZE	0x2000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 << 10)
#define CONFIG_SYS_NAND_U_BOOT_DST	(0x11000000 - CONFIG_SYS_NAND_SPL_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_START	0x11000000
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(0)
#define CONFIG_SYS_NAND_U_BOOT_RELOC	0x10000
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP	(CONFIG_SYS_NAND_U_BOOT_RELOC + 0x10000)

/* CPLD on IFC */
#define CONFIG_SYS_CPLD_BASE		0xffb00000

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_CPLD_BASE_PHYS	0xfffb00000ull
#else
#define CONFIG_SYS_CPLD_BASE_PHYS	CONFIG_SYS_CPLD_BASE
#endif

#define CONFIG_SYS_CSPR3	(CSPR_PHYS_ADDR(CONFIG_SYS_CPLD_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 \
				| CSPR_MSEL_GPCM \
				| CSPR_V)
#define CONFIG_SYS_AMASK3		IFC_AMASK(64*1024)
#define CONFIG_SYS_CSOR3		0x0
/* CPLD Timing parameters for IFC CS3 */
#define CONFIG_SYS_CS3_FTIM0		(FTIM0_GPCM_TACSE(0x0e) | \
					FTIM0_GPCM_TEADC(0x0e) | \
					FTIM0_GPCM_TEAHC(0x0e))
#define CONFIG_SYS_CS3_FTIM1		(FTIM1_GPCM_TACO(0x0e) | \
					FTIM1_GPCM_TRAD(0x1f))
#define CONFIG_SYS_CS3_FTIM2		(FTIM2_GPCM_TCS(0x0e) | \
					FTIM2_GPCM_TCH(0x0) | \
					FTIM2_GPCM_TWP(0x1f))
#define CONFIG_SYS_CS3_FTIM3		0x0
#endif	/* CONFIG_SDCARD */

#if defined(CONFIG_RAMBOOT_SDCARD) || defined(CONFIG_RAMBOOT_SPIFLASH) || \
    defined(CONFIG_RAMBOOT_NAND)
#define CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_EXTRA_ENV_RELOC
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_BOARD_EARLY_INIT_F	/* Call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xffd00000 /* stack in RAM */
#define CONFIG_SYS_INIT_RAM_END		0x00004000 /* End of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END \
						- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon*/
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc*/

/* Serial Port */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#ifdef CONFIG_NAND_SPL
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SERIAL_MULTI		/* Enable both serial ports */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	/* determine from environment */

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/*
 * Pass open firmware flat tree
 */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

/* new uImage format support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE	/* enable fit_format_{error,warning}() */

#define CONFIG_FSL_I2C			/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C			/* I2C with hardware support */
#undef CONFIG_SOFT_I2C			/* I2C bit-banged */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_I2C_SPEED		400000 /* I2C speed and slave address*/
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x3000
#define CONFIG_SYS_I2C2_OFFSET		0x3100

/* I2C EEPROM */
#undef CONFIG_ID_EEPROM
/* enable read and write access to EEPROM */
#define CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

/* RTC */
#define CONFIG_RTC_PT7C4338
#define CONFIG_SYS_I2C_RTC_ADDR	0x68

#define CONFIG_CMD_I2C

/*
 * SPI interface will not be available in case of NAND boot SPI CS0 will be
 * used for SLIC
 */
#if !defined(CONFIG_NAND_U_BOOT) || !defined(CONFIG_NAND_SECBOOT)
/* eSPI - Enhanced SPI */
#define CONFIG_FSL_ESPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		10000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#endif

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_MII			/* MII PHY management */
#define CONFIG_MII_DEFAULT_TSEC	1	/* Allow unregistered phys */
#define CONFIG_TSEC1	1
#define CONFIG_TSEC1_NAME	"eTSEC1"
#define CONFIG_TSEC2	1
#define CONFIG_TSEC2_NAME	"eTSEC2"
#define CONFIG_TSEC3	1
#define CONFIG_TSEC3_NAME	"eTSEC3"

#define TSEC1_PHY_ADDR		1
#define TSEC2_PHY_ADDR		0
#define TSEC3_PHY_ADDR		2

#define TSEC1_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC3_FLAGS		(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX		0
#define TSEC2_PHYIDX		0
#define TSEC3_PHYIDX		0

#define CONFIG_ETHPRIME		"eTSEC1"

#define CONFIG_PHY_GIGE		/* Include GbE speed/duplex detection */

/* TBI PHY configuration for SGMII mode */
#define CONFIG_TSEC_TBICR_SETTINGS ( \
		TBICR_PHY_RESET \
		| TBICR_ANEG_ENABLE \
		| TBICR_FULL_DUPLEX \
		| TBICR_SPEED1_SET \
		)

#endif	/* CONFIG_TSEC_ENET */


/* SATA */
#define CONFIG_FSL_SATA
#define CONFIG_LIBATA

#ifdef CONFIG_FSL_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#define CONFIG_CMD_SATA
#define CONFIG_LBA48
#endif /* #ifdef CONFIG_FSL_SATA  */

/*  SD interface will only be available in case of SD boot */
#ifdef CONFIG_SDCARD
#define CONFIG_MMC
#define CONFIG_DEF_HWCONFIG		esdhc
#endif

#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_FSL_ESDHC
#define CONFIG_GENERIC_MMC
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
#endif

#define CONFIG_HAS_FSL_DR_USB

#if defined(CONFIG_HAS_FSL_DR_USB)
#define CONFIG_USB_EHCI

#ifdef CONFIG_USB_EHCI
#define CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_EHCI_FSL
#define CONFIG_USB_STORAGE
#endif
#endif

/*
 * Environment
 */
#if defined(CONFIG_SYS_RAMBOOT)
#if defined(CONFIG_RAMBOOT_SDCARD)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x2000
#elif defined(CONFIG_RAMBOOT_SPIFLASH)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS	0
#define CONFIG_ENV_SPI_CS	0
#define CONFIG_ENV_SPI_MAX_HZ	10000000
#define CONFIG_ENV_SPI_MODE	0
#define CONFIG_ENV_OFFSET	0x100000	/* 1MB */
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_SIZE		0x2000
#elif defined(CONFIG_NAND_U_BOOT)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE		CONFIG_SYS_NAND_BLOCK_SIZE
#define CONFIG_ENV_OFFSET	CONFIG_SYS_NAND_U_BOOT_SIZE
#define CONFIG_ENV_RANGE	(3 * CONFIG_ENV_SIZE)
#else
#define CONFIG_ENV_IS_NOWHERE		/* Store ENV in memory only */
#define CONFIG_ENV_ADDR			(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE			0x2000
#endif
#else
#define CONFIG_ENV_IS_IN_FLASH
#if CONFIG_SYS_MONITOR_BASE > 0xfff80000
#define CONFIG_ENV_ADDR	0xfff80000
#else
#define CONFIG_ENV_ADDR	(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#endif
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DATE
#define CONFIG_CMD_ERRATA
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#if defined(CONFIG_MMC) || defined(CONFIG_USB_EHCI) \
		 || defined(CONFIG_FSL_SATA)
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_CMDLINE_EDITING			/* Command-line editing */
#define CONFIG_AUTO_COMPLETE			/* add autocompletion support */
#define CONFIG_SYS_LOAD_ADDR	0x2000000	/* default load address */
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* dec freq: 1ms ticks */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20) /* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20) /* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */

#if defined(CONFIG_TSEC_ENET)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

#define CONFIG_HOSTNAME		P1010RDB
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY	10	/* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_BAUDRATE		115200

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"hwconfig=" MK_STR(CONFIG_DEF_HWCONFIG)  "\0"	\
	"netdev=eth0\0"						\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"				\
	"loadaddr=1000000\0"			\
	"consoledev=ttyS0\0"				\
	"ramdiskaddr=2000000\0"			\
	"ramdiskfile=rootfs.ext2.gz.uboot\0"		\
	"fdtaddr=c00000\0"				\
	"fdtfile=p1010rdb.dtb\0"		\
	"bdev=sda1\0"	\
	"hwconfig=usb1:dr_mode=host,phy_type=utmi\0"	\
	"othbootargs=ramdisk_size=600000\0" \
	"usbfatboot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"fatload usb 0:2 $loadaddr $bootfile;"		\
	"fatload usb 0:2 $fdtaddr $fdtfile;"	\
	"fatload usb 0:2 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\
	"usbext2boot=setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"usb start;"			\
	"ext2load usb 0:4 $loadaddr $bootfile;"		\
	"ext2load usb 0:4 $fdtaddr $fdtfile;"	\
	"ext2load usb 0:4 $ramdiskaddr $ramdiskfile;"	\
	"bootm $loadaddr $ramdiskaddr $fdtaddr\0"		\

#define CONFIG_RAMBOOTCOMMAND		\
	"setenv bootargs root=/dev/ram rw "	\
	"console=$consoledev,$baudrate $othbootargs; "	\
	"tftp $ramdiskaddr $ramdiskfile;"	\
	"tftp $loadaddr $bootfile;"		\
	"tftp $fdtaddr $fdtfile;"		\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND CONFIG_RAMBOOTCOMMAND

#ifdef CONFIG_SECURE_BOOT
#include <asm/fsl_secure_boot.h>
#endif

#endif	/* __CONFIG_H */
