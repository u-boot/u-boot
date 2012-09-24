/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Corenet DS style board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include "../board/freescale/common/ics307_clk.h"

#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_RAMBOOT_TEXT_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_RESET_VECTOR_ADDRESS	0xfffffffc
#endif

#ifdef CONFIG_SRIOBOOT_SLAVE
/* Set 1M boot space */
#define CONFIG_SYS_SRIOBOOT_SLAVE_ADDR (CONFIG_SYS_TEXT_BASE & 0xfff00000)
#define CONFIG_SYS_SRIOBOOT_SLAVE_ADDR_PHYS \
		(0x300000000ull | CONFIG_SYS_SRIOBOOT_SLAVE_ADDR)
#define CONFIG_RESET_VECTOR_ADDRESS 0xfffffffc
#define CONFIG_SYS_NO_FLASH
#endif

/* High Level Configuration Options */
#define CONFIG_BOOKE
#define CONFIG_E500			/* BOOKE e500 family */
#define CONFIG_E500MC			/* BOOKE e500mc family */
#define CONFIG_SYS_BOOK3E_HV		/* Category E.HV supported */
#define CONFIG_MPC85xx			/* MPC85xx/PQ3 platform */
#define CONFIG_FSL_CORENET		/* Freescale CoreNet platform */
#define CONFIG_MP			/* support multiple processors */

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xeff80000
#endif

#ifndef CONFIG_RESET_VECTOR_ADDRESS
#define CONFIG_RESET_VECTOR_ADDRESS	0xeffffffc
#endif

#define CONFIG_SYS_FSL_CPC		/* Corenet Platform Cache */
#define CONFIG_SYS_NUM_CPC		CONFIG_NUM_DDR_CONTROLLERS
#define CONFIG_FSL_ELBC			/* Has Enhanced localbus controller */
#define CONFIG_PCI			/* Enable PCI/PCIE */
#define CONFIG_PCIE1			/* PCIE controler 1 */
#define CONFIG_PCIE2			/* PCIE controler 2 */
#define CONFIG_FSL_PCI_INIT		/* Use common FSL init code */
#define CONFIG_SYS_PCI_64BIT		/* enable 64-bit PCI resources */

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */
#define CONFIG_SRIO2			/* SRIO port 2 */

#define CONFIG_FSL_LAW			/* Use common FSL init code */

#define CONFIG_ENV_OVERWRITE

#ifdef CONFIG_SYS_NO_FLASH
#ifndef CONFIG_SRIOBOOT_SLAVE
#define CONFIG_ENV_IS_NOWHERE
#endif
#else
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#endif

#if defined(CONFIG_SPIFLASH)
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_BUS              0
#define CONFIG_ENV_SPI_CS               0
#define CONFIG_ENV_SPI_MAX_HZ           10000000
#define CONFIG_ENV_SPI_MODE             0
#define CONFIG_ENV_SIZE                 0x2000          /* 8KB */
#define CONFIG_ENV_OFFSET               0x100000        /* 1MB */
#define CONFIG_ENV_SECT_SIZE            0x10000
#elif defined(CONFIG_SDCARD)
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_FSL_FIXED_MMC_LOCATION
#define CONFIG_SYS_MMC_ENV_DEV          0
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		(512 * 1097)
#elif defined(CONFIG_NAND)
#define CONFIG_SYS_EXTRA_ENV_RELOC
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE			CONFIG_SYS_NAND_BLOCK_SIZE
#define CONFIG_ENV_OFFSET		(5 * CONFIG_SYS_NAND_BLOCK_SIZE)
#elif defined(CONFIG_SRIOBOOT_SLAVE)
#define CONFIG_ENV_IS_IN_REMOTE
#define CONFIG_ENV_ADDR		0xffe20000
#define CONFIG_ENV_SIZE		0x2000
#elif defined(CONFIG_ENV_IS_NOWHERE)
#define CONFIG_ENV_SIZE		0x2000
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K (one sector) */
#endif

#define CONFIG_SYS_CLK_FREQ	get_board_sys_clk() /* sysclk for MPC85xx */

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_SYS_CACHE_STASHING
#define CONFIG_BACKSIDE_L2_CACHE
#define CONFIG_SYS_INIT_L2CSR0		L2CSR0_L2E
#define CONFIG_BTB			/* toggle branch predition */
#define	CONFIG_DDR_ECC
#ifdef CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#endif

#define CONFIG_ENABLE_36BIT_PHYS

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_ADDR_MAP
#define CONFIG_SYS_NUM_ADDR_MAP		64	/* number of TLB1 entries */
#endif

#define CONFIG_POST CONFIG_SYS_POST_MEMORY	/* test POST memory test */
#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000
#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_PANIC_HANG	/* do not reset board on panic */

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CONFIG_SYS_INIT_L3_ADDR		CONFIG_RAMBOOT_TEXT_BASE
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	(0xf00000000ull | CONFIG_RAMBOOT_TEXT_BASE)
#else
#define CONFIG_SYS_INIT_L3_ADDR_PHYS	CONFIG_SYS_INIT_L3_ADDR
#endif
#define CONFIG_SYS_L3_SIZE		(1024 << 10)
#define CONFIG_SYS_INIT_L3_END (CONFIG_SYS_INIT_L3_ADDR + CONFIG_SYS_L3_SIZE)

#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_DCSRBAR		0xf0000000
#define CONFIG_SYS_DCSRBAR_PHYS		0xf00000000ull
#endif

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM	0
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(4 * CONFIG_DIMM_SLOTS_PER_CTLR)

#define CONFIG_DDR_SPD
#define CONFIG_FSL_DDR3

#ifdef CONFIG_P3060QDS
#define CONFIG_SYS_SPD_BUS_NUM	0
#else
#define CONFIG_SYS_SPD_BUS_NUM	1
#endif
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1	/* for p3041/p5010 */
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * Local Bus Definitions
 */

/* Set the local bus clock 1/8 of platform clock */
#define CONFIG_SYS_LBC_LCRR		LCRR_CLKDIV_8

#define CONFIG_SYS_FLASH_BASE		0xe0000000	/* Start of PromJet */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_FLASH_BASE_PHYS	0xfe0000000ull
#else
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE
#endif

#define CONFIG_SYS_FLASH_BR_PRELIM \
		(BR_PHYS_ADDR((CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000)) \
		 | BR_PS_16 | BR_V)
#define CONFIG_SYS_FLASH_OR_PRELIM ((0xf8000ff7 & ~OR_GPCM_SCY & ~OR_GPCM_EHTR) \
					| OR_GPCM_SCY_8 | OR_GPCM_EHTR_CLEAR)

#define CONFIG_SYS_BR1_PRELIM \
	(BR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | BR_PS_16 | BR_V)
#define CONFIG_SYS_OR1_PRELIM	0xf8000ff7

#define PIXIS_BASE		0xffdf0000	/* PIXIS registers */
#ifdef CONFIG_PHYS_64BIT
#define PIXIS_BASE_PHYS		0xfffdf0000ull
#else
#define PIXIS_BASE_PHYS		PIXIS_BASE
#endif

#define CONFIG_SYS_BR3_PRELIM	(BR_PHYS_ADDR(PIXIS_BASE_PHYS) | BR_PS_8 | BR_V)
#define CONFIG_SYS_OR3_PRELIM	0xffffeff7	/* 32KB but only 4k mapped */

#define PIXIS_LBMAP_SWITCH	7
#define PIXIS_LBMAP_MASK	0xf0
#define PIXIS_LBMAP_SHIFT	4
#define PIXIS_LBMAP_ALTBANK	0x40

#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS 	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	2		/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024		/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000		/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500		/* Flash Write Timeout (ms) */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE	/* start of monitor */

#if defined(CONFIG_RAMBOOT_PBL)
#define CONFIG_SYS_RAMBOOT
#endif

/* Nand Flash */
#ifdef CONFIG_NAND_FSL_ELBC
#define CONFIG_SYS_NAND_BASE		0xffa00000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_NAND_BASE_PHYS	0xfffa00000ull
#else
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#endif

#define CONFIG_SYS_NAND_BASE_LIST     {CONFIG_SYS_NAND_BASE}
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_BLOCK_SIZE    (128 * 1024)

/* NAND flash config */
#define CONFIG_SYS_NAND_BR_PRELIM  (BR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
			       | (2<<BR_DECC_SHIFT)    /* Use HW ECC */ \
			       | BR_PS_8	       /* Port Size = 8 bit */ \
			       | BR_MS_FCM	       /* MSEL = FCM */ \
			       | BR_V)		       /* valid */
#define CONFIG_SYS_NAND_OR_PRELIM  (0xFFFC0000	      /* length 256K */ \
			       | OR_FCM_PGS	       /* Large Page*/ \
			       | OR_FCM_CSCT \
			       | OR_FCM_CST \
			       | OR_FCM_CHT \
			       | OR_FCM_SCY_1 \
			       | OR_FCM_TRLX \
			       | OR_FCM_EHTR)

#ifdef CONFIG_NAND
#define CONFIG_SYS_BR0_PRELIM  CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */
#define CONFIG_SYS_BR2_PRELIM  CONFIG_SYS_FLASH_BR_PRELIM /* NOR Base Address */
#define CONFIG_SYS_OR2_PRELIM  CONFIG_SYS_FLASH_OR_PRELIM /* NOR Options */
#else
#define CONFIG_SYS_BR0_PRELIM  CONFIG_SYS_FLASH_BR_PRELIM /* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_SYS_FLASH_OR_PRELIM /* NOR Options */
#define CONFIG_SYS_BR2_PRELIM  CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR2_PRELIM  CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */
#endif
#else
#define CONFIG_SYS_BR0_PRELIM  CONFIG_SYS_FLASH_BR_PRELIM /* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM  CONFIG_SYS_FLASH_OR_PRELIM /* NOR Options */
#endif /* CONFIG_NAND_FSL_ELBC */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_AMD_CHECK_DQ7
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS + 0x8000000, CONFIG_SYS_FLASH_BASE_PHYS}

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_EARLY_INIT_R	/* call board_early_init_r function */
#define CONFIG_MISC_INIT_R

#define CONFIG_HWCONFIG

/* define to use L1 as initial stack */
#define CONFIG_L1_INIT_RAM
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#else
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS	CONFIG_SYS_INIT_RAM_ADDR /* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH 0
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW CONFIG_SYS_INIT_RAM_ADDR_PHYS
#endif
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000	/* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024)	/* Reserved for malloc */

/* Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 */
#define CONFIG_CONS_INDEX	1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0)/2)

#define CONFIG_SYS_BAUDRATE_TABLE	\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR+0x11C500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_CCSRBAR+0x11C600)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_CCSRBAR+0x11D500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_CCSRBAR+0x11D600)

/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT
#define CONFIG_OF_BOARD_SETUP
#define CONFIG_OF_STDOUT_VIA_ALIAS

/* new uImage format support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE	/* enable fit_format_{error,warning}() */

/* I2C */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CONFIG_SYS_I2C_SPEED		400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x118000
#define CONFIG_SYS_I2C2_OFFSET		0x118100

/*
 * RapidIO
 */
#define CONFIG_SYS_SRIO1_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_SRIO1_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x10000000	/* 256M */

#define CONFIG_SYS_SRIO2_MEM_VIRT	0xb0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_SRIO2_MEM_PHYS	0xc30000000ull
#else
#define CONFIG_SYS_SRIO2_MEM_PHYS	0xb0000000
#endif
#define CONFIG_SYS_SRIO2_MEM_SIZE	0x10000000	/* 256M */

/*
 * SRIOBOOT - MASTER
 */
#ifdef CONFIG_SRIOBOOT_MASTER
/* master port for srioboot*/
#define CONFIG_SRIOBOOT_MASTER_PORT 0
/* #define CONFIG_SRIOBOOT_MASTER_PORT 1 */
/*
 * for slave u-boot IMAGE instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS1 0xfef080000ull
#define CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS1 0xfff80000ull
#define CONFIG_SRIOBOOT_SLAVE_IMAGE_SIZE 0x80000	/* 512K */
#define CONFIG_SRIOBOOT_SLAVE_IMAGE_LAW_PHYS2 0xfef080000ull
#define CONFIG_SRIOBOOT_SLAVE_IMAGE_SRIO_PHYS2 0x3fff80000ull
/*
 * for slave UCODE instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CONFIG_SRIOBOOT_SLAVE_UCODE_LAW_PHYS 0xfef020000ull
#define CONFIG_SRIOBOOT_SLAVE_UCODE_SRIO_PHYS 0x3ffe00000ull
#define CONFIG_SRIOBOOT_SLAVE_UCODE_SIZE 0x10000	/* 64K */
/*
 * for slave ENV instored in master memory space,
 * PHYS must be aligned based on the SIZE
 */
#define CONFIG_SRIOBOOT_SLAVE_ENV_LAW_PHYS 0xfef060000ull
#define CONFIG_SRIOBOOT_SLAVE_ENV_SRIO_PHYS 0x3ffe20000ull
#define CONFIG_SRIOBOOT_SLAVE_ENV_SIZE 0x20000	/* 128K */
/* slave core release by master*/
#define CONFIG_SRIOBOOT_SLAVE_HOLDOFF
#define CONFIG_SRIOBOOT_SLAVE_BRR_OFFSET 0xe00e4
#define CONFIG_SRIOBOOT_SLAVE_RELEASE_MASK 0x00000001 /* release core 0 */
#endif

/*
 * SRIOBOOT - SLAVE
 */
#ifdef CONFIG_SRIOBOOT_SLAVE
/* slave port for srioboot */
#define CONFIG_SRIOBOOT_SLAVE_PORT0
/* #define CONFIG_SRIOBOOT_SLAVE_PORT1 */
#define CONFIG_SYS_SRIOBOOT_UCODE_ENV_ADDR 0xFFE00000
#define CONFIG_SYS_SRIOBOOT_UCODE_ENV_ADDR_PHYS \
		(0x300000000ull | CONFIG_SYS_SRIOBOOT_UCODE_ENV_ADDR)
#endif

/*
 * eSPI - Enhanced SPI
 */
#define CONFIG_FSL_ESPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED         10000000
#define CONFIG_SF_DEFAULT_MODE          0

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */

/* controller 1, direct to uli, tgtid 3, Base address 20000 */
#define CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#else
#define CONFIG_SYS_PCIE1_MEM_BUS	0x80000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0x80000000
#endif
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE1_IO_PHYS	0xff8000000ull
#else
#define CONFIG_SYS_PCIE1_IO_PHYS	0xf8000000
#endif
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00010000	/* 64k */

/* controller 2, Slot 2, tgtid 2, Base address 201000 */
#define CONFIG_SYS_PCIE2_MEM_VIRT	0xa0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xc20000000ull
#else
#define CONFIG_SYS_PCIE2_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE2_MEM_PHYS	0xa0000000
#endif
#define CONFIG_SYS_PCIE2_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE2_IO_VIRT	0xf8010000
#define CONFIG_SYS_PCIE2_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE2_IO_PHYS	0xff8010000ull
#else
#define CONFIG_SYS_PCIE2_IO_PHYS	0xf8010000
#endif
#define CONFIG_SYS_PCIE2_IO_SIZE	0x00010000	/* 64k */

/* controller 3, Slot 1, tgtid 1, Base address 202000 */
#define CONFIG_SYS_PCIE3_MEM_VIRT	0xc0000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc40000000ull
#else
#define CONFIG_SYS_PCIE3_MEM_BUS	0xc0000000
#define CONFIG_SYS_PCIE3_MEM_PHYS	0xc0000000
#endif
#define CONFIG_SYS_PCIE3_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE3_IO_VIRT	0xf8020000
#define CONFIG_SYS_PCIE3_IO_BUS		0x00000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_PCIE3_IO_PHYS	0xff8020000ull
#else
#define CONFIG_SYS_PCIE3_IO_PHYS	0xf8020000
#endif
#define CONFIG_SYS_PCIE3_IO_SIZE	0x00010000	/* 64k */

/* controller 4, Base address 203000 */
#define CONFIG_SYS_PCIE4_MEM_BUS	0xe0000000
#define CONFIG_SYS_PCIE4_MEM_PHYS	0xc60000000ull
#define CONFIG_SYS_PCIE4_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE4_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE4_IO_PHYS	0xff8030000ull
#define CONFIG_SYS_PCIE4_IO_SIZE	0x00010000	/* 64k */

/* Qman/Bman */
#define CONFIG_SYS_DPAA_QBMAN		/* Support Q/Bman */
#define CONFIG_SYS_BMAN_NUM_PORTALS	10
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#else
#define CONFIG_SYS_BMAN_MEM_PHYS	CONFIG_SYS_BMAN_MEM_BASE
#endif
#define CONFIG_SYS_BMAN_MEM_SIZE	0x00200000
#define CONFIG_SYS_QMAN_NUM_PORTALS	10
#define CONFIG_SYS_QMAN_MEM_BASE	0xf4200000
#ifdef CONFIG_PHYS_64BIT
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff4200000ull
#else
#define CONFIG_SYS_QMAN_MEM_PHYS	CONFIG_SYS_QMAN_MEM_BASE
#endif
#define CONFIG_SYS_QMAN_MEM_SIZE	0x00200000

#define CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_DPAA_PME
/* Default address of microcode for the Linux Fman driver */
#if defined(CONFIG_SPIFLASH)
/*
 * env is stored at 0x100000, sector size is 0x10000, ucode is stored after
 * env, so we got 0x110000.
 */
#define CONFIG_SYS_QE_FW_IN_SPIFLASH
#define CONFIG_SYS_QE_FMAN_FW_ADDR	0x110000
#elif defined(CONFIG_SDCARD)
/*
 * PBL SD boot image should stored at 0x1000(8 blocks), the size of the image is
 * about 545KB (1089 blocks), Env is stored after the image, and the env size is
 * 0x2000 (16 blocks), 8 + 1089 + 16 = 1113, enlarge it to 1130.
 */
#define CONFIG_SYS_QE_FMAN_FW_IN_MMC
#define CONFIG_SYS_QE_FMAN_FW_ADDR	(512 * 1130)
#elif defined(CONFIG_NAND)
#define CONFIG_SYS_QE_FMAN_FW_IN_NAND
#define CONFIG_SYS_QE_FMAN_FW_ADDR	(6 * CONFIG_SYS_NAND_BLOCK_SIZE)
#elif defined(CONFIG_SRIOBOOT_SLAVE)
/*
 * Slave has no ucode locally, it can fetch this from remote. When implementing
 * in two corenet boards, slave's ucode could be stored in master's memory
 * space, the address can be mapped from slave TLB->slave LAW->
 * slave SRIO outbound window->master inbound window->master LAW->
 * the ucode address in master's NOR flash.
 */
#define CONFIG_SYS_QE_FMAN_FW_IN_REMOTE
#define CONFIG_SYS_QE_FMAN_FW_ADDR	0xFFE00000
#else
#define CONFIG_SYS_QE_FMAN_FW_IN_NOR
#define CONFIG_SYS_QE_FMAN_FW_ADDR		0xEF000000
#endif
#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

#ifdef CONFIG_SYS_DPAA_FMAN
#define CONFIG_FMAN_ENET
#define CONFIG_PHYLIB_10G
#define CONFIG_PHY_VITESSE
#define CONFIG_PHY_TERANETICS
#endif

#ifdef CONFIG_PCI
#define CONFIG_PCI_PNP			/* do pci plug-and-play */
#define CONFIG_E1000

#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */
#define CONFIG_DOS_PARTITION
#endif	/* CONFIG_PCI */

/* SATA */
#ifdef CONFIG_FSL_SATA_V2
#define CONFIG_LIBATA
#define CONFIG_FSL_SATA

#define CONFIG_SYS_SATA_MAX_DEVICE	2
#define CONFIG_SATA1
#define CONFIG_SYS_SATA1		CONFIG_SYS_MPC85xx_SATA1_ADDR
#define CONFIG_SYS_SATA1_FLAGS		FLAGS_DMA
#define CONFIG_SATA2
#define CONFIG_SYS_SATA2		CONFIG_SYS_MPC85xx_SATA2_ADDR
#define CONFIG_SYS_SATA2_FLAGS		FLAGS_DMA

#define CONFIG_LBA48
#define CONFIG_CMD_SATA
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_EXT2
#endif

#ifdef CONFIG_FMAN_ENET
#define CONFIG_SYS_FM1_DTSEC1_PHY_ADDR	0x1c
#define CONFIG_SYS_FM1_DTSEC2_PHY_ADDR	0x1d
#define CONFIG_SYS_FM1_DTSEC3_PHY_ADDR	0x1e
#define CONFIG_SYS_FM1_DTSEC4_PHY_ADDR	0x1f
#define CONFIG_SYS_FM1_10GEC1_PHY_ADDR	4

#define CONFIG_SYS_FM2_DTSEC1_PHY_ADDR	0x1c
#define CONFIG_SYS_FM2_DTSEC2_PHY_ADDR	0x1d
#define CONFIG_SYS_FM2_DTSEC3_PHY_ADDR	0x1e
#define CONFIG_SYS_FM2_DTSEC4_PHY_ADDR	0x1f
#define CONFIG_SYS_FM2_10GEC1_PHY_ADDR	0

#define CONFIG_SYS_TBIPA_VALUE	8
#define CONFIG_MII		/* MII PHY management */
#define CONFIG_ETHPRIME		"FM1@DTSEC1"
#define CONFIG_PHY_GIGE		/* Include GbE speed/duplex detection */
#endif

/*
 * Environment
 */
#define CONFIG_LOADS_ECHO		/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change */

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_ERRATA
#define CONFIG_CMD_GREPENV
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#ifdef CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_CMD_NET
#endif

/*
* USB
*/
#define CONFIG_HAS_FSL_DR_USB
#define CONFIG_HAS_FSL_MPH_USB

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_FSL
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_CMD_EXT2
#endif

#ifdef CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR       CONFIG_SYS_MPC85xx_ESDHC_ADDR
#define CONFIG_SYS_FSL_ESDHC_BROKEN_TIMEOUT
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
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
#ifdef CONFIG_CMD_KGDB
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ROOTPATH		"/opt/nfsroot"
#define CONFIG_BOOTFILE		"uImage"
#define CONFIG_UBOOTPATH	u-boot.bin	/* U-Boot image on TFTP server */

/* default location for tftp and bootm */
#define CONFIG_LOADADDR		1000000

#define CONFIG_BOOTDELAY 	10	/* -1 disables auto-boot */

#define CONFIG_BAUDRATE	115200

#if defined(CONFIG_P4080DS) || defined(CONFIG_P3060QDS)
#define __USB_PHY_TYPE	ulpi
#else
#define __USB_PHY_TYPE	utmi
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"hwconfig=fsl_ddr:ctlr_intlv=cacheline,"		\
	"bank_intlv=cs0_cs1;"					\
	"usb1:dr_mode=host,phy_type=" MK_STR(__USB_PHY_TYPE) "\0"\
	"netdev=eth0\0"						\
	"uboot=" MK_STR(CONFIG_UBOOTPATH) "\0"			\
	"ubootaddr=" MK_STR(CONFIG_SYS_TEXT_BASE) "\0"			\
	"tftpflash=tftpboot $loadaddr $uboot && "		\
	"protect off $ubootaddr +$filesize && "			\
	"erase $ubootaddr +$filesize && "			\
	"cp.b $loadaddr $ubootaddr $filesize && "		\
	"protect on $ubootaddr +$filesize && "			\
	"cmp.b $loadaddr $ubootaddr $filesize\0"		\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=p4080ds/ramdisk.uboot\0"			\
	"fdtaddr=c00000\0"					\
	"fdtfile=p4080ds/p4080ds.dtb\0"				\
	"bdev=sda3\0"						\
	"c=ffe\0"

#define CONFIG_HDBOOT					\
	"setenv bootargs root=/dev/$bdev rw "		\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $loadaddr $bootfile;"			\
	"tftp $fdtaddr $fdtfile;"			\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_NFSBOOTCOMMAND			\
	"setenv bootargs root=/dev/nfs rw "	\
	"nfsroot=$serverip:$rootpath "		\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $loadaddr $bootfile;"		\
	"tftp $fdtaddr $fdtfile;"		\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND				\
	"setenv bootargs root=/dev/ram rw "		\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $ramdiskaddr $ramdiskfile;"		\
	"tftp $loadaddr $bootfile;"			\
	"tftp $fdtaddr $fdtfile;"			\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_HDBOOT

#ifdef CONFIG_SECURE_BOOT
#include <asm/fsl_secure_boot.h>
#endif

#endif	/* __CONFIG_H */
