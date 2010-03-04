/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_QE		1 /* Has QE */
#define CONFIG_MPC83xx		1 /* MPC83xx family */
#define CONFIG_MPC8360		1 /* MPC8360 CPU specific */
#define CONFIG_MPC8360ERDK	1 /* MPC8360ERDK board specific */

/*
 * System Clock Setup
 */
#ifdef CONFIG_CLKIN_33MHZ
#define CONFIG_83XX_CLKIN		33333333
#define CONFIG_SYS_CLK_FREQ		33333333
#define PCI_33M				1
#define HRCWL_CSB_TO_CLKIN_MPC8360ERDK	HRCWL_CSB_TO_CLKIN_10X1
#else
#define CONFIG_83XX_CLKIN		66000000
#define CONFIG_SYS_CLK_FREQ		66000000
#define PCI_66M				1
#define HRCWL_CSB_TO_CLKIN_MPC8360ERDK	HRCWL_CSB_TO_CLKIN_5X1
#endif /* CONFIG_CLKIN_33MHZ */

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_MPC8360ERDK |\
	HRCWL_CORE_TO_CSB_2X1 |\
	HRCWL_CE_TO_PLL_1X15)

#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_PCI_HOST |\
	HRCWH_PCI1_ARBITER_ENABLE |\
	HRCWH_PCICKDRV_ENABLE |\
	HRCWH_CORE_ENABLE |\
	HRCWH_FROM_0X00000100 |\
	HRCWH_BOOTSEQ_DISABLE |\
	HRCWH_SW_WATCHDOG_DISABLE |\
	HRCWH_ROM_LOC_LOCAL_16BIT |\
	HRCWH_SECONDARY_DDR_DISABLE |\
	HRCWH_BIG_ENDIAN |\
	HRCWH_LALE_EARLY)

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH		0x00000000
#define CONFIG_SYS_SICRL		0x40000000

#define CONFIG_BOARD_EARLY_INIT_F /* call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CONFIG_SYS_83XX_DDR_USES_CS0

#define CONFIG_DDR_ECC		/* support DDR ECC function */
#define CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

/*
 * DDRCDR - DDR Control Driver Register
 */
#define CONFIG_SYS_DDRCDR_VALUE	0x80080001

#undef CONFIG_SPD_EEPROM	/* Do not use SPD EEPROM for DDR setup */

/*
 * Manually set up DDR parameters
 */
#define CONFIG_DDR_II
#define CONFIG_SYS_DDR_SIZE		256 /* MB */
#define CONFIG_SYS_DDR_CS0_BNDS	0x0000000f
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | \
				 CSCONFIG_COL_BIT_10 | CSCONFIG_ODT_WR_ACS)
#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SDRAM_TYPE_DDR2 | SDRAM_CFG_ECC_EN)
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00001000
#define CONFIG_SYS_DDR_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)
#define CONFIG_SYS_DDR_INTERVAL	((256 << SDRAM_INTERVAL_BSTOPRE_SHIFT) | \
				 (1115 << SDRAM_INTERVAL_REFINT_SHIFT))
#define CONFIG_SYS_DDR_MODE		0x47800432
#define CONFIG_SYS_DDR_MODE2		0x8000c000

#define CONFIG_SYS_DDR_TIMING_0	((2 << TIMING_CFG0_MRS_CYC_SHIFT) | \
				 (9 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				 (3 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) | \
				 (3 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) | \
				 (0 << TIMING_CFG0_WWT_SHIFT) | \
				 (0 << TIMING_CFG0_RRT_SHIFT) | \
				 (0 << TIMING_CFG0_WRT_SHIFT) | \
				 (0 << TIMING_CFG0_RWT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_1	((      TIMING_CFG1_CASLAT_30) | \
				 ( 2 << TIMING_CFG1_WRTORD_SHIFT) | \
				 ( 2 << TIMING_CFG1_ACTTOACT_SHIFT) | \
				 ( 3 << TIMING_CFG1_WRREC_SHIFT) | \
				 (10 << TIMING_CFG1_REFREC_SHIFT) | \
				 ( 3 << TIMING_CFG1_ACTTORW_SHIFT) | \
				 ( 8 << TIMING_CFG1_ACTTOPRE_SHIFT) | \
				 ( 3 << TIMING_CFG1_PRETOACT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_2	((9 << TIMING_CFG2_FOUR_ACT_SHIFT) | \
				 (4 << TIMING_CFG2_CKE_PLS_SHIFT) | \
				 (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) | \
				 (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) | \
				 (2 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) | \
				 (0 << TIMING_CFG2_ADD_LAT_SHIFT) | \
				 (0 << TIMING_CFG2_CPO_SHIFT))

#define CONFIG_SYS_DDR_TIMING_3	0x00000000

/*
 * Memory test
 */
#undef CONFIG_SYS_DRAM_TEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00000000 /* memtest region */
#define CONFIG_SYS_MEMTEST_END		0x00100000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE /* start of monitor */
#define CONFIG_SYS_FLASH_BASE		0xFF800000 /* FLASH base address */

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(384 * 1024) /* Reserve 384 kB for Mon */
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK	1
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_END	0x1000 /* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_SIZE	0x100 /* num bytes initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)

/*
 * Local Bus Configuration & Clock Setup
 */
#define CONFIG_SYS_LCRR_DBYP	LCRR_DBYP
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4
#define CONFIG_SYS_LBC_LBCR		0x00000000

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CONFIG_SYS_FLASH_SIZE		8 /* max FLASH size is 32M */
#define CONFIG_SYS_FLASH_PROTECTION	1 /* Use intel Flash protection. */

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE /* Window base at flash base */
#define CONFIG_SYS_LBLAWAR0_PRELIM	0x80000018 /* 32MB window size */

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE | /* Flash Base address */ \
			(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
			BR_V)	/* valid */
#define CONFIG_SYS_OR0_PRELIM		((~(CONFIG_SYS_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CONFIG_SYS_MAX_FLASH_BANKS	1 /* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	256 /* max sectors per device */

#undef	CONFIG_SYS_FLASH_CHECKSUM

/*
 * NAND flash on the local bus
 */
#define CONFIG_SYS_NAND_BASE		0x60000000
#define CONFIG_CMD_NAND		1
#define CONFIG_NAND_FSL_UPM	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE

#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_NAND_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	0x8000001b /* Access window size 4K */

/* Port size 8 bit, UPMA */
#define CONFIG_SYS_BR1_PRELIM		(CONFIG_SYS_NAND_BASE | 0x00000881)
#define CONFIG_SYS_OR1_PRELIM		0xfc000001

/*
 * Fujitsu MB86277 (MINT) graphics controller
 */
#define CONFIG_SYS_VIDEO_BASE		0x70000000

#define CONFIG_SYS_LBLAWBAR2_PRELIM	CONFIG_SYS_VIDEO_BASE
#define CONFIG_SYS_LBLAWAR2_PRELIM	0x80000019 /* Access window size 64MB */

/* Port size 32 bit, UPMB */
#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_VIDEO_BASE | 0x000018a1) /* PS=11, UPMB */
#define CONFIG_SYS_OR2_PRELIM		0xfc000001 /* (64MB, EAD=1) */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200,}

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
/* Use the HUSH parser */
#define CONFIG_SYS_HUSH_PARSER
#ifdef	CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "> "
#endif

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED	400000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SYS_I2C_NOPROBES	{{0x52}} /* Don't probe these addrs */
#define CONFIG_SYS_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C2_OFFSET 0x3100

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_PCI

#define CONFIG_SYS_PCI1_MEM_BASE	0x80000000
#define CONFIG_SYS_PCI1_MEM_PHYS	CONFIG_SYS_PCI1_MEM_BASE
#define CONFIG_SYS_PCI1_MEM_SIZE	0x10000000 /* 256M */
#define CONFIG_SYS_PCI1_MMIO_BASE	0x90000000
#define CONFIG_SYS_PCI1_MMIO_PHYS	CONFIG_SYS_PCI1_MMIO_BASE
#define CONFIG_SYS_PCI1_MMIO_SIZE	0x10000000 /* 256M */
#define CONFIG_SYS_PCI1_IO_BASE	0xE0300000
#define CONFIG_SYS_PCI1_IO_PHYS	0xE0300000
#define CONFIG_SYS_PCI1_IO_SIZE	0x100000 /* 1M */

#ifdef CONFIG_PCI

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */

#endif	/* CONFIG_PCI */


#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"FSL UEC0"

#define CONFIG_UEC_ETH1		/* GETH1 */

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	0	/* UCC1 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK_NONE
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK9
#define CONFIG_SYS_UEC1_ETH_TYPE	GIGA_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	2
#define CONFIG_SYS_UEC1_INTERFACE_TYPE RGMII_RXID
#define CONFIG_SYS_UEC1_INTERFACE_SPEED 1000
#endif

#define CONFIG_UEC_ETH2		/* GETH2 */

#ifdef CONFIG_UEC_ETH2
#define CONFIG_SYS_UEC2_UCC_NUM	1	/* UCC2 */
#define CONFIG_SYS_UEC2_RX_CLK		QE_CLK_NONE
#define CONFIG_SYS_UEC2_TX_CLK		QE_CLK4
#define CONFIG_SYS_UEC2_ETH_TYPE	GIGA_ETH
#define CONFIG_SYS_UEC2_PHY_ADDR	4
#define CONFIG_SYS_UEC2_INTERFACE_TYPE RGMII_RXID
#define CONFIG_SYS_UEC2_INTERFACE_SPEED 1000
#endif

/*
 * Environment
 */

#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#define CONFIG_ENV_SIZE		0x20000
#else /* CONFIG_SYS_RAMBOOT */
#define CONFIG_SYS_NO_FLASH		1	/* Flash is not usable now */
#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif /* CONFIG_SYS_RAMBOOT */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

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
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if defined(CONFIG_SYS_RAMBOOT)
#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_LOADS
#endif

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_LOAD_ADDR		0x2000000 /* default load address */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
	#define CONFIG_SYS_CBSIZE	1024 /* Console I/O Buffer Size */
#else
	#define CONFIG_SYS_CBSIZE	256 /* Console I/O Buffer Size */
#endif

#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */
#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * Core HID Setup
 */
#define CONFIG_SYS_HID0_INIT		0x000000000
#define CONFIG_SYS_HID0_FINAL		HID0_ENABLE_MACHINE_CHECK
#define CONFIG_SYS_HID2		HID2_HBE

/*
 * MMU Setup
 */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR: cache cacheable */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

/* IMMRBAR & PCI IO: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_IMMR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_IMMR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* NAND: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_NAND_BASE | BATL_PP_10 | BATL_CACHEINHIBIT |\
			 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_NAND_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT2L	CONFIG_SYS_IBAT2L
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_FLASH_BASE | BATU_BL_32M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_10 | \
			 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/* Stack in dcache: cacheable, no memory coherence */
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U

#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_VIDEO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | \
			 BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_VIDEO_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	CONFIG_SYS_IBAT5L
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

#ifdef CONFIG_PCI
/* PCI MEM space: cacheable */
#define CONFIG_SYS_IBAT6L	(CONFIG_SYS_PCI1_MEM_PHYS | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT6U	(CONFIG_SYS_PCI1_MEM_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
/* PCI MMIO space: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT7L	(CONFIG_SYS_PCI1_MMIO_PHYS | BATL_PP_10 | \
			 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT7U	(CONFIG_SYS_PCI1_MMIO_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U
#else /* CONFIG_PCI */
#define CONFIG_SYS_IBAT6L	(0)
#define CONFIG_SYS_IBAT6U	(0)
#define CONFIG_SYS_IBAT7L	(0)
#define CONFIG_SYS_IBAT7U	(0)
#define CONFIG_SYS_DBAT6L	CONFIG_SYS_IBAT6L
#define CONFIG_SYS_DBAT6U	CONFIG_SYS_IBAT6U
#define CONFIG_SYS_DBAT7L	CONFIG_SYS_IBAT7L
#define CONFIG_SYS_DBAT7U	CONFIG_SYS_IBAT7U
#endif /* CONFIG_PCI */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01 /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02 /* Software reboot */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed of kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE

#if defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#endif

#define CONFIG_BAUDRATE 115200

#define CONFIG_LOADADDR	a00000
#define CONFIG_HOSTNAME	mpc8360erdk
#define CONFIG_BOOTFILE	uImage

#define CONFIG_ROOTPATH		/nfsroot/

#define	CONFIG_BOOTDELAY 2	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_EXTRA_ENV_SETTINGS \
   "netdev=eth0\0"\
   "consoledev=ttyS0\0"\
   "loadaddr=a00000\0"\
   "fdtaddr=900000\0"\
   "fdtfile=mpc836x_rdk.dtb\0"\
   "fsfile=fs\0"\
   "ubootfile=u-boot.bin\0"\
   "mtdparts=mtdparts=60000000.nand-flash:4096k(kernel),128k(dtb),-(rootfs)\0"\
   "setbootargs=setenv bootargs console=$consoledev,$baudrate "\
		"$mtdparts panic=1\0"\
   "adddhcpargs=setenv bootargs $bootargs ip=on\0"\
   "addnfsargs=setenv bootargs $bootargs ip=$ipaddr:$serverip:"\
		"$gatewayip:$netmask:$hostname:$netdev:off "\
		"root=/dev/nfs rw nfsroot=$serverip:$rootpath\0"\
   "addnandargs=setenv bootargs $bootargs root=/dev/mtdblock3 "\
		"rootfstype=jffs2 rw\0"\
   "tftp_get_uboot=tftp 100000 $ubootfile\0"\
   "tftp_get_kernel=tftp $loadaddr $bootfile\0"\
   "tftp_get_dtb=tftp $fdtaddr $fdtfile\0"\
   "tftp_get_fs=tftp c00000 $fsfile\0"\
   "nand_erase_kernel=nand erase 0 400000\0"\
   "nand_erase_dtb=nand erase 400000 20000\0"\
   "nand_erase_fs=nand erase 420000 3be0000\0"\
   "nand_write_kernel=nand write.jffs2 $loadaddr 0 400000\0"\
   "nand_write_dtb=nand write.jffs2 $fdtaddr 400000 20000\0"\
   "nand_write_fs=nand write.jffs2 c00000 420000 $filesize\0"\
   "nand_read_kernel=nand read.jffs2 $loadaddr 0 400000\0"\
   "nand_read_dtb=nand read.jffs2 $fdtaddr 400000 20000\0"\
   "nor_reflash=protect off ff800000 ff87ffff ; erase ff800000 ff87ffff ; "\
		"cp.b 100000 ff800000 $filesize\0"\
   "nand_reflash_kernel=run tftp_get_kernel nand_erase_kernel "\
		"nand_write_kernel\0"\
   "nand_reflash_dtb=run tftp_get_dtb nand_erase_dtb nand_write_dtb\0"\
   "nand_reflash_fs=run tftp_get_fs nand_erase_fs nand_write_fs\0"\
   "nand_reflash=run nand_reflash_kernel nand_reflash_dtb "\
		"nand_reflash_fs\0"\
   "boot_m=bootm $loadaddr - $fdtaddr\0"\
   "dhcpboot=dhcp ; run setbootargs adddhcpargs tftp_get_dtb boot_m\0"\
   "nfsboot=run setbootargs addnfsargs tftp_get_kernel tftp_get_dtb "\
		"boot_m\0"\
   "nandboot=run setbootargs addnandargs nand_read_kernel nand_read_dtb "\
		"boot_m\0"\
   ""

#define CONFIG_BOOTCOMMAND "run dhcpboot"

#endif /* __CONFIG_H */
