/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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
 * mpc8569mds board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_BOOKE		1	/* BOOKE */
#define CONFIG_E500		1	/* BOOKE e500 family */
#define CONFIG_MPC85xx		1	/* MPC8540/60/55/41/48/68 */
#define CONFIG_MPC8569		1	/* MPC8569 specific */
#define CONFIG_MPC8569MDS	1	/* MPC8569MDS board specific */

#define CONFIG_FSL_ELBC		1	/* Has Enhance localbus controller */

#define CONFIG_SYS_SRIO
#define CONFIG_SRIO1			/* SRIO port 1 */

#define CONFIG_PCI		1	/* Disable PCI/PCIE */
#define CONFIG_PCIE1		1	/* PCIE controller */
#define CONFIG_FSL_PCI_INIT	1	/* use common fsl pci init code */
#define CONFIG_FSL_PCIE_RESET	1	/* need PCIe reset errata */
#define CONFIG_SYS_PCI_64BIT	1	/* enable 64-bit PCI resources */
#define CONFIG_QE			/* Enable QE */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_FSL_LAW		1	/* Use common FSL init code */

#ifndef __ASSEMBLY__
extern unsigned long get_clock_freq(void);
#endif
/* Replace a call to get_clock_freq (after it is implemented)*/
#define CONFIG_SYS_CLK_FREQ	66666666
#define CONFIG_DDR_CLK_FREQ	CONFIG_SYS_CLK_FREQ

#ifdef CONFIG_ATM
#define CONFIG_PQ_MDS_PIB
#define CONFIG_PQ_MDS_PIB_ATM
#endif

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_L2_CACHE				/* toggle L2 cache	*/
#define CONFIG_BTB				/* toggle branch predition */

#ifdef CONFIG_NAND
#define CONFIG_NAND_U_BOOT		1
#define CONFIG_RAMBOOT_NAND		1
#ifdef CONFIG_NAND_SPL
#define CONFIG_SYS_TEXT_BASE_SPL 0xfff00000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE_SPL /* start of monitor */
#else
#define CONFIG_SYS_LDSCRIPT $(TOPDIR)/$(CPUDIR)/u-boot-nand.lds
#define CONFIG_SYS_TEXT_BASE	0xf8f82000
#endif
#endif

#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE	0xfff80000
#endif

#ifndef CONFIG_SYS_MONITOR_BASE
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE	/* start of monitor */
#endif

/*
 * Only possible on E500 Version 2 or newer cores.
 */
#define CONFIG_ENABLE_36BIT_PHYS	1

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R	1
#define CONFIG_HWCONFIG

#define CONFIG_SYS_MEMTEST_START	0x00200000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00400000

/*
 * Config the L2 Cache as L2 SRAM
 */
#define CONFIG_SYS_INIT_L2_ADDR		0xf8f80000
#define CONFIG_SYS_INIT_L2_ADDR_PHYS	CONFIG_SYS_INIT_L2_ADDR
#define CONFIG_SYS_L2_SIZE		(512 << 10)
#define CONFIG_SYS_INIT_L2_END	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_L2_SIZE)

#define CONFIG_SYS_CCSRBAR		0xe0000000
#define CONFIG_SYS_CCSRBAR_PHYS_LOW	CONFIG_SYS_CCSRBAR

#if defined(CONFIG_NAND_SPL)
#define CONFIG_SYS_CCSR_DO_NOT_RELOCATE
#endif

/* DDR Setup */
#define CONFIG_FSL_DDR3
#undef CONFIG_FSL_DDR_INTERACTIVE
#define CONFIG_SPD_EEPROM		/* Use SPD EEPROM for DDR setup*/
#define CONFIG_DDR_SPD
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER	/* DDR controller or DMA? */

#define CONFIG_MEM_INIT_VALUE	0xDeadBeef

#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
					/* DDR is system memory*/
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_NUM_DDR_CONTROLLERS	1
#define CONFIG_DIMM_SLOTS_PER_CTLR	1
#define CONFIG_CHIP_SELECTS_PER_CTRL	(2 * CONFIG_DIMM_SLOTS_PER_CTLR)

/* I2C addresses of SPD EEPROMs */
#define SPD_EEPROM_ADDRESS    0x51    /* CTLR 0 DIMM 0 */

/* These are used when DDR doesn't use SPD.  */
#define CONFIG_SYS_SDRAM_SIZE           1024		/* DDR is 1024MB */
#define CONFIG_SYS_DDR_CS0_BNDS         0x0000003F
#define CONFIG_SYS_DDR_CS0_CONFIG       0x80014202
#define CONFIG_SYS_DDR_TIMING_3         0x00020000
#define CONFIG_SYS_DDR_TIMING_0         0x00330004
#define CONFIG_SYS_DDR_TIMING_1         0x6F6B4644
#define CONFIG_SYS_DDR_TIMING_2         0x002888D0
#define CONFIG_SYS_DDR_SDRAM_CFG	0x47000000
#define CONFIG_SYS_DDR_SDRAM_CFG_2	0x04401040
#define CONFIG_SYS_DDR_SDRAM_MODE	0x40401521
#define CONFIG_SYS_DDR_SDRAM_MODE_2	0x8000C000
#define CONFIG_SYS_DDR_SDRAM_INTERVAL	0x03E00000
#define CONFIG_SYS_DDR_DATA_INIT        0xdeadbeef
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	0x01000000
#define CONFIG_SYS_DDR_TIMING_4         0x00220001
#define CONFIG_SYS_DDR_TIMING_5         0x03402400
#define CONFIG_SYS_DDR_ZQ_CNTL		0x89080600
#define CONFIG_SYS_DDR_WRLVL_CNTL	0x0655A604
#define CONFIG_SYS_DDR_CDR_1		0x80040000
#define CONFIG_SYS_DDR_CDR_2		0x00000000
#define CONFIG_SYS_DDR_OCD_CTRL         0x00000000
#define CONFIG_SYS_DDR_OCD_STATUS       0x00000000
#define CONFIG_SYS_DDR_CONTROL          0xc7000000      /* Type = DDR3 */
#define CONFIG_SYS_DDR_CONTROL2         0x24400000

#define CONFIG_SYS_DDR_ERR_INT_EN       0x0000000d
#define CONFIG_SYS_DDR_ERR_DIS          0x00000000
#define CONFIG_SYS_DDR_SBE              0x00010000

#undef CONFIG_CLOCKS_IN_MHZ

/*
 * Local Bus Definitions
 */

#define CONFIG_SYS_FLASH_BASE		0xfe000000	/* start of FLASH 32M */
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_BCSR_BASE		0xf8000000
#define CONFIG_SYS_BCSR_BASE_PHYS	CONFIG_SYS_BCSR_BASE

/*Chip select 0 - Flash*/
#define CONFIG_FLASH_BR_PRELIM		0xfe000801
#define	CONFIG_FLASH_OR_PRELIM		0xfe000ff7

/*Chip select 1 - BCSR*/
#define CONFIG_SYS_BR1_PRELIM		0xf8000801
#define	CONFIG_SYS_OR1_PRELIM		0xffffe9f7

/*Chip select 4 - PIB*/
#define CONFIG_SYS_BR4_PRELIM		0xf8008801
#define CONFIG_SYS_OR4_PRELIM		0xffffe9f7

/*Chip select 5 - PIB*/
#define CONFIG_SYS_BR5_PRELIM		0xf8010801
#define CONFIG_SYS_OR5_PRELIM		0xffffe9f7

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* sectors per device */
#undef	CONFIG_SYS_FLASH_CHECKSUM
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#if defined(CONFIG_RAMBOOT_NAND)
#define CONFIG_SYS_RAMBOOT
#define CONFIG_SYS_EXTRA_ENV_RELOC
#else
#undef CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* Chip select 3 - NAND */
#ifndef CONFIG_NAND_SPL
#define CONFIG_SYS_NAND_BASE		0xFC000000
#else
#define CONFIG_SYS_NAND_BASE		0xFFF00000
#endif

/* NAND boot: 4K NAND loader config */
#define CONFIG_SYS_NAND_SPL_SIZE	0x1000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	((512 << 10) - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_DST	(CONFIG_SYS_INIT_L2_ADDR)
#define CONFIG_SYS_NAND_U_BOOT_START \
	(CONFIG_SYS_INIT_L2_ADDR + CONFIG_SYS_NAND_SPL_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(0)
#define CONFIG_SYS_NAND_U_BOOT_RELOC	(CONFIG_SYS_INIT_L2_END - 0x2000)
#define CONFIG_SYS_NAND_U_BOOT_RELOC_SP	((CONFIG_SYS_INIT_L2_END - 1) & ~0xF)

#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE, }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE	1
#define CONFIG_CMD_NAND			1
#define CONFIG_NAND_FSL_ELBC		1
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BR_PRELIM	(CONFIG_SYS_NAND_BASE_PHYS \
				| (2<<BR_DECC_SHIFT) /* Use HW ECC */ \
				| BR_PS_8	     /* Port Size = 8 bit */ \
				| BR_MS_FCM	     /* MSEL = FCM */ \
				| BR_V)		     /* valid */
#define CONFIG_SYS_NAND_OR_PRELIM	(0xFFFC0000	     /* length 256K */ \
				| OR_FCM_CSCT \
				| OR_FCM_CST \
				| OR_FCM_CHT \
				| OR_FCM_SCY_1 \
				| OR_FCM_TRLX \
				| OR_FCM_EHTR)

#ifdef CONFIG_RAMBOOT_NAND
#define CONFIG_SYS_BR0_PRELIM	CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR0_PRELIM	CONFIG_SYS_NAND_OR_PRELIM/* NAND Options */
#define CONFIG_SYS_BR3_PRELIM	CONFIG_FLASH_BR_PRELIM	/* NOR Base Address */
#define CONFIG_SYS_OR3_PRELIM	CONFIG_FLASH_OR_PRELIM	/* NOR Options */
#else
#define CONFIG_SYS_BR0_PRELIM	CONFIG_FLASH_BR_PRELIM	/* NOR Base Address */
#define CONFIG_SYS_OR0_PRELIM	CONFIG_FLASH_OR_PRELIM	/* NOR Options */
#define CONFIG_SYS_BR3_PRELIM	CONFIG_SYS_NAND_BR_PRELIM /* NAND Base Address */
#define CONFIG_SYS_OR3_PRELIM	CONFIG_SYS_NAND_OR_PRELIM /* NAND Options */
#endif

#define CONFIG_SYS_LBC_LCRR	0x00000004	/* LB clock ratio reg */
#define CONFIG_SYS_LBC_LBCR	0x00040000	/* LB config reg */
#define CONFIG_SYS_LBC_LSRT	0x20000000	/* LB sdram refresh timer */
#define CONFIG_SYS_LBC_MRTPR	0x00000000	/* LB refresh timer prescal*/

#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xe4010000  /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000	    /* Size of used area in RAM */

#define CONFIG_SYS_GBL_DATA_OFFSET	\
			(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)	/* Reserve 256 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)	/* Reserved for malloc */

/* Serial Port */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE    1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)
#ifdef CONFIG_NAND_SPL
#define CONFIG_NS16550_MIN_FUNCTIONS
#endif

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400,115200}

#define CONFIG_SYS_NS16550_COM1        (CONFIG_SYS_CCSRBAR+0x4500)
#define CONFIG_SYS_NS16550_COM2        (CONFIG_SYS_CCSRBAR+0x4600)

/* Use the HUSH parser*/
#define CONFIG_SYS_HUSH_PARSER
#ifdef  CONFIG_SYS_HUSH_PARSER
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT		1
#define CONFIG_OF_BOARD_SETUP		1
#define CONFIG_OF_STDOUT_VIA_ALIAS	1

/*
 * I2C
 */
#define CONFIG_FSL_I2C		/* Use FSL common I2C driver */
#define CONFIG_HARD_I2C		/* I2C with hardware support*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED	400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SYS_I2C_NOPROBES	{{0,0x69}}	/* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C2_OFFSET	0x3100

/*
 * I2C2 EEPROM
 */
#define CONFIG_ID_EEPROM
#ifdef CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#endif
#define CONFIG_SYS_I2C_EEPROM_ADDR      0x52
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_BUS_NUM       1

#define PLPPAR1_I2C_BIT_MASK		0x0000000F
#define PLPPAR1_I2C2_VAL		0x00000000
#define PLPPAR1_ESDHC_VAL		0x0000000A
#define PLPDIR1_I2C_BIT_MASK		0x0000000F
#define PLPDIR1_I2C2_VAL		0x0000000F
#define PLPDIR1_ESDHC_VAL		0x00000006
#define PLPPAR1_UART0_BIT_MASK		0x00000fc0
#define PLPPAR1_ESDHC_4BITS_VAL		0x00000a80
#define PLPDIR1_UART0_BIT_MASK		0x00000fc0
#define PLPDIR1_ESDHC_4BITS_VAL		0x00000a80

/*
 * General PCI
 * Memory Addresses are mapped 1-1. I/O is mapped from 0
 */
#define CONFIG_SYS_PCIE1_NAME		"Slot"
#define CONFIG_SYS_PCIE1_MEM_VIRT	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_BUS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_PHYS	0xa0000000
#define CONFIG_SYS_PCIE1_MEM_SIZE	0x20000000	/* 512M */
#define CONFIG_SYS_PCIE1_IO_VIRT	0xe2800000
#define CONFIG_SYS_PCIE1_IO_BUS		0x00000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xe2800000
#define CONFIG_SYS_PCIE1_IO_SIZE	0x00800000	/* 8M */

#define CONFIG_SYS_SRIO1_MEM_VIRT	0xC0000000
#define CONFIG_SYS_SRIO1_MEM_BUS	0xC0000000
#define CONFIG_SYS_SRIO1_MEM_PHYS	CONFIG_SYS_SRIO1_MEM_BUS
#define CONFIG_SYS_SRIO1_MEM_SIZE	0x20000000	/* 512M */

#ifdef CONFIG_QE
/*
 * QE UEC ethernet configuration
 */
#define CONFIG_SYS_UCC_RGMII_MODE	/* Set UCC work at RGMII by default */
#undef CONFIG_SYS_UCC_RMII_MODE		/* Set UCC work at RMII mode */

#define CONFIG_MIIM_ADDRESS	(CONFIG_SYS_CCSRBAR + 0x82120)
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME         "UEC0"
#define CONFIG_PHY_MODE_NEED_CHANGE

#define CONFIG_UEC_ETH1         /* GETH1 */
#define CONFIG_HAS_ETH0

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM        0       /* UCC1 */
#define CONFIG_SYS_UEC1_RX_CLK         QE_CLK_NONE
#if defined(CONFIG_SYS_UCC_RGMII_MODE)
#define CONFIG_SYS_UEC1_TX_CLK         QE_CLK12
#define CONFIG_SYS_UEC1_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR       7
#define CONFIG_SYS_UEC1_INTERFACE_TYPE PHY_INTERFACE_MODE_RGMII_ID
#define CONFIG_SYS_UEC1_INTERFACE_SPEED 1000
#elif defined(CONFIG_SYS_UCC_RMII_MODE)
#define CONFIG_SYS_UEC1_TX_CLK         QE_CLK16	/* CLK16 for RMII */
#define CONFIG_SYS_UEC1_ETH_TYPE       FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR       8	/* 0x8 for RMII */
#define CONFIG_SYS_UEC1_INTERFACE_TYPE PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED 100
#endif /* CONFIG_SYS_UCC_RGMII_MODE */
#endif /* CONFIG_UEC_ETH1 */

#define CONFIG_UEC_ETH2         /* GETH2 */
#define CONFIG_HAS_ETH1

#ifdef CONFIG_UEC_ETH2
#define CONFIG_SYS_UEC2_UCC_NUM        1       /* UCC2 */
#define CONFIG_SYS_UEC2_RX_CLK         QE_CLK_NONE
#if defined(CONFIG_SYS_UCC_RGMII_MODE)
#define CONFIG_SYS_UEC2_TX_CLK         QE_CLK17
#define CONFIG_SYS_UEC2_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC2_PHY_ADDR       1
#define CONFIG_SYS_UEC2_INTERFACE_TYPE PHY_INTERFACE_MODE_RGMII_ID
#define CONFIG_SYS_UEC2_INTERFACE_SPEED 1000
#elif defined(CONFIG_SYS_UCC_RMII_MODE)
#define CONFIG_SYS_UEC2_TX_CLK         QE_CLK16	/* CLK 16 for RMII */
#define CONFIG_SYS_UEC2_ETH_TYPE       FAST_ETH
#define CONFIG_SYS_UEC2_PHY_ADDR       0x9	/* 0x9 for RMII */
#define CONFIG_SYS_UEC2_INTERFACE_TYPE PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC2_INTERFACE_SPEED 100
#endif /* CONFIG_SYS_UCC_RGMII_MODE */
#endif /* CONFIG_UEC_ETH2 */

#define CONFIG_UEC_ETH3         /* GETH3 */
#define CONFIG_HAS_ETH2

#ifdef CONFIG_UEC_ETH3
#define CONFIG_SYS_UEC3_UCC_NUM        2       /* UCC3 */
#define CONFIG_SYS_UEC3_RX_CLK         QE_CLK_NONE
#if defined(CONFIG_SYS_UCC_RGMII_MODE)
#define CONFIG_SYS_UEC3_TX_CLK         QE_CLK12
#define CONFIG_SYS_UEC3_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC3_PHY_ADDR       2
#define CONFIG_SYS_UEC3_INTERFACE_TYPE PHY_INTERFACE_MODE_RGMII_ID
#define CONFIG_SYS_UEC3_INTERFACE_SPEED 1000
#elif defined(CONFIG_SYS_UCC_RMII_MODE)
#define CONFIG_SYS_UEC3_TX_CLK		QE_CLK16 /* CLK_16 for RMII */
#define CONFIG_SYS_UEC3_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC3_PHY_ADDR	0xA     /* 0xA for RMII */
#define CONFIG_SYS_UEC3_INTERFACE_TYPE PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC3_INTERFACE_SPEED 100
#endif /* CONFIG_SYS_UCC_RGMII_MODE */
#endif /* CONFIG_UEC_ETH3 */

#define CONFIG_UEC_ETH4         /* GETH4 */
#define CONFIG_HAS_ETH3

#ifdef CONFIG_UEC_ETH4
#define CONFIG_SYS_UEC4_UCC_NUM        3       /* UCC4 */
#define CONFIG_SYS_UEC4_RX_CLK         QE_CLK_NONE
#if defined(CONFIG_SYS_UCC_RGMII_MODE)
#define CONFIG_SYS_UEC4_TX_CLK         QE_CLK17
#define CONFIG_SYS_UEC4_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC4_PHY_ADDR       3
#define CONFIG_SYS_UEC4_INTERFACE_TYPE PHY_INTERFACE_MODE_RGMII_ID
#define CONFIG_SYS_UEC4_INTERFACE_SPEED 1000
#elif defined(CONFIG_SYS_UCC_RMII_MODE)
#define CONFIG_SYS_UEC4_TX_CLK		QE_CLK16 /* CLK16 for RMII */
#define CONFIG_SYS_UEC4_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC4_PHY_ADDR	0xB     /* 0xB for RMII */
#define CONFIG_SYS_UEC4_INTERFACE_TYPE PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC4_INTERFACE_SPEED 100
#endif /* CONFIG_SYS_UCC_RGMII_MODE */
#endif /* CONFIG_UEC_ETH4 */

#undef CONFIG_UEC_ETH6         /* GETH6 */
#define CONFIG_HAS_ETH5

#ifdef CONFIG_UEC_ETH6
#define CONFIG_SYS_UEC6_UCC_NUM        5       /* UCC6 */
#define CONFIG_SYS_UEC6_RX_CLK         QE_CLK_NONE
#define CONFIG_SYS_UEC6_TX_CLK         QE_CLK_NONE
#define CONFIG_SYS_UEC6_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC6_PHY_ADDR       4
#define CONFIG_SYS_UEC6_INTERFACE_TYPE PHY_INTERFACE_MODE_SGMII
#define CONFIG_SYS_UEC6_INTERFACE_SPEED 1000
#endif /* CONFIG_UEC_ETH6 */

#undef CONFIG_UEC_ETH8         /* GETH8 */
#define CONFIG_HAS_ETH7

#ifdef CONFIG_UEC_ETH8
#define CONFIG_SYS_UEC8_UCC_NUM        7       /* UCC8 */
#define CONFIG_SYS_UEC8_RX_CLK         QE_CLK_NONE
#define CONFIG_SYS_UEC8_TX_CLK         QE_CLK_NONE
#define CONFIG_SYS_UEC8_ETH_TYPE       GIGA_ETH
#define CONFIG_SYS_UEC8_PHY_ADDR       6
#define CONFIG_SYS_UEC8_INTERFACE_TYPE PHY_INTERFACE_MODE_SGMII
#define CONFIG_SYS_UEC8_INTERFACE_SPEED 1000
#endif /* CONFIG_UEC_ETH8 */

#endif /* CONFIG_QE */

#if defined(CONFIG_PCI)

#define CONFIG_PCI_PNP			/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_TULIP
#define CONFIG_E1000			/* Define e1000 pci Ethernet card */

#undef CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup */

#endif	/* CONFIG_PCI */

/*
 * Environment
 */
#if defined(CONFIG_SYS_RAMBOOT)
#if defined(CONFIG_RAMBOOT_NAND)
#define CONFIG_ENV_IS_IN_NAND	1
#define CONFIG_ENV_SIZE		CONFIG_SYS_NAND_BLOCK_SIZE
#define CONFIG_ENV_OFFSET	((512 * 1024) + CONFIG_SYS_NAND_BLOCK_SIZE)
#endif
#else
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SECT_SIZE	0x20000	/* 128K(one sector) for env */
#define CONFIG_ENV_SIZE		0x2000
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/* QE microcode/firmware address */
#define CONFIG_SYS_QE_FMAN_FW_IN_NOR
#define CONFIG_SYS_QE_FMAN_FW_ADDR	0xfff00000

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

#define CONFIG_CMD_PING
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_SETEXPR
#define CONFIG_CMD_REGINFO

#if defined(CONFIG_PCI)
    #define CONFIG_CMD_PCI
#endif


#undef CONFIG_WATCHDOG			/* watchdog disabled */

#define CONFIG_MMC     1

#ifdef CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_ESDHC_PIN_MUX
#define CONFIG_SYS_FSL_ESDHC_ADDR	CONFIG_SYS_MPC85xx_ESDHC_ADDR
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
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	2048		/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE	512		/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
						/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	32		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE
						/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ	1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20) /* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_HOSTNAME mpc8569mds
#define CONFIG_ROOTPATH  "/nfsroot"
#define CONFIG_BOOTFILE  "your.uImage"

#define CONFIG_SERVERIP  192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_NETMASK   255.255.255.0

#define CONFIG_LOADADDR  200000   /*default location for tftp and bootm*/

#define CONFIG_BOOTDELAY 10       /* -1 disables auto-boot */
#undef  CONFIG_BOOTARGS           /* the boot command will set bootargs*/

#define CONFIG_BAUDRATE	115200

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"consoledev=ttyS0\0"						\
	"ramdiskaddr=600000\0"						\
	"ramdiskfile=your.ramdisk.u-boot\0"				\
	"fdtaddr=400000\0"						\
	"fdtfile=your.fdt.dtb\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
	"nfsroot=$serverip:$rootpath "					\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw "			\
	"console=$consoledev,$baudrate $othbootargs\0"			\

#define CONFIG_NFSBOOTCOMMAND						\
	"run nfsargs;"							\
	"tftp $loadaddr $bootfile;"					\
	"tftp $fdtaddr $fdtfile;"					\
	"bootm $loadaddr - $fdtaddr"

#define CONFIG_RAMBOOTCOMMAND						\
	"run ramargs;"							\
	"tftp $ramdiskaddr $ramdiskfile;"				\
	"tftp $loadaddr $bootfile;"					\
	"bootm $loadaddr $ramdiskaddr"

#define CONFIG_BOOTCOMMAND  CONFIG_NFSBOOTCOMMAND

#endif	/* __CONFIG_H */
