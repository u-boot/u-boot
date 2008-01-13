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

#undef DEBUG

/*
 * High Level Configuration Options
 */
#define CONFIG_E300		1 /* E300 family */
#define CONFIG_QE		1 /* Has QE */
#define CONFIG_MPC83XX		1 /* MPC83XX family */
#define CONFIG_MPC8360		1 /* MPC8360 CPU specific */
#define CONFIG_MPC8360ERDK	1 /* MPC8360ERDK board specific */

/*
 * System Clock Setup
 */
#ifdef CONFIG_CLKIN_33MHZ
#define CONFIG_83XX_CLKIN		33000000
#define CONFIG_SYS_CLK_FREQ		33000000
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
#define CFG_HRCW_LOW (\
	HRCWL_LCL_BUS_TO_SCB_CLK_1X1 |\
	HRCWL_DDR_TO_SCB_CLK_1X1 |\
	HRCWL_CSB_TO_CLKIN_MPC8360ERDK |\
	HRCWL_CORE_TO_CSB_2X1 |\
	HRCWL_CE_TO_PLL_1X15)

#define CFG_HRCW_HIGH (\
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
#define CFG_SICRH		0x00000000
#define CFG_SICRL		0x40000000

#define CONFIG_BOARD_EARLY_INIT_F /* call board_pre_init */
#define CONFIG_BOARD_EARLY_INIT_R

/*
 * IMMR new address
 */
#define CFG_IMMR		0xE0000000

/*
 * DDR Setup
 */
#define CFG_DDR_BASE		0x00000000 /* DDR is system memory */
#define CFG_SDRAM_BASE		CFG_DDR_BASE
#define CFG_DDR_SDRAM_BASE	CFG_DDR_BASE
#define CFG_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
				DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

#undef CONFIG_DDR_ECC		/* support DDR ECC function */
#undef CONFIG_DDR_ECC_CMD	/* Use DDR ECC user commands */

/*
 * DDRCDR - DDR Control Driver Register
 */
#define CFG_DDRCDR_VALUE	0x80080001

#undef CONFIG_SPD_EEPROM	/* Do not use SPD EEPROM for DDR setup */

/*
 * Manually set up DDR parameters
 */
#define CONFIG_DDR_II
#define CFG_DDR_SIZE		256 /* MB */
#define CFG_DDRCDR		0x80080001
#define CFG_DDR_CS0_BNDS	0x0000000f
#define CFG_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_ROW_BIT_13 | \
				 CSCONFIG_COL_BIT_10)
#define CFG_DDR_TIMING_0	0x00330903
#define CFG_DDR_TIMING_1	0x3835a322
#define CFG_DDR_TIMING_2	0x00104909
#define CFG_DDR_TIMING_3	0x00000000
#define CFG_DDR_CLK_CNTL	0x02000000
#define CFG_DDR_MODE		0x47800432
#define CFG_DDR_MODE2		0x8000c000
#define CFG_DDR_INTERVAL	0x045b0100
#define CFG_DDR_SDRAM_CFG	0x03000000
#define CFG_DDR_SDRAM_CFG2	0x00001000

/*
 * Memory test
 */
#undef CFG_DRAM_TEST		/* memory test, takes time */
#define CFG_MEMTEST_START	0x00000000 /* memtest region */
#define CFG_MEMTEST_END		0x00100000

/*
 * The reserved memory
 */
#define CFG_MONITOR_BASE	TEXT_BASE /* start of monitor */
#define CFG_FLASH_BASE		0xFF800000 /* FLASH base address */

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#else
#undef	CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 * 1024) /* Reserve 256 kB for Mon */
#define CFG_MALLOC_LEN		(128 * 1024) /* Reserved for malloc */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_INIT_RAM_LOCK	1
#define CFG_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_INIT_RAM_END	0x1000 /* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	0x100 /* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)

/*
 * Local Bus Configuration & Clock Setup
 */
#define CFG_LCRR		(LCRR_DBYP | LCRR_CLKDIV_4)
#define CFG_LBC_LBCR		0x00000000

/*
 * FLASH on the Local Bus
 */
#define CFG_FLASH_CFI		/* use the Common Flash Interface */
#define CFG_FLASH_CFI_DRIVER	/* use the CFI driver */
#define CFG_FLASH_SIZE		8 /* max FLASH size is 32M */
#define CFG_FLASH_PROTECTION	1 /* Use intel Flash protection. */

#define CFG_LBLAWBAR0_PRELIM	CFG_FLASH_BASE /* Window base at flash base */
#define CFG_LBLAWAR0_PRELIM	0x80000018 /* 32MB window size */

#define CFG_BR0_PRELIM	(CFG_FLASH_BASE | /* Flash Base address */ \
			(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
			BR_V)	/* valid */
#define CFG_OR0_PRELIM		((~(CFG_FLASH_SIZE - 1) << 20) | OR_UPM_XAM | \
				OR_GPCM_CSNT | OR_GPCM_ACS_0b11 | \
				OR_GPCM_XACS | OR_GPCM_SCY_15 | \
				OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)

#define CFG_MAX_FLASH_BANKS	1 /* number of banks */
#define CFG_MAX_FLASH_SECT	256 /* max sectors per device */

#undef	CFG_FLASH_CHECKSUM

/*
 * NAND flash on the local bus
 */
#define CFG_NAND_BASE		0x60000000

#define CFG_LBLAWBAR1_PRELIM	CFG_NAND_BASE
#define CFG_LBLAWAR1_PRELIM	0x8000001b /* Access window size 4K */

/* Port size 8 bit, UPMA */
#define CFG_BR1_PRELIM		(CFG_NAND_BASE | 0x00000881)
#define CFG_OR1_PRELIM		0xfc000001

/*
 * Fujitsu MB86277 (MINT) graphics controller
 */
#define CFG_VIDEO_BASE		0x70000000

#define CFG_LBLAWBAR2_PRELIM	CFG_VIDEO_BASE
#define CFG_LBLAWAR2_PRELIM	0x80000019 /* Access window size 64MB */

/* Port size 32 bit, UPMB */
#define CFG_BR2_PRELIM		(CFG_VIDEO_BASE | 0x000018a1) /* PS=11, UPMB */
#define CFG_OR2_PRELIM		0xfc000001 /* (64MB, EAD=1) */

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	1
#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200,}

#define CFG_NS16550_COM1	(CFG_IMMR+0x4500)
#define CFG_NS16550_COM2	(CFG_IMMR+0x4600)

#define CONFIG_CMDLINE_EDITING	1	/* add command line history */
/* Use the HUSH parser */
#define CFG_HUSH_PARSER
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "> "
#endif

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_FSL_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_I2C_CMD_TREE
#define CFG_I2C_SPEED	400000	/* I2C speed and slave address */
#define CFG_I2C_SLAVE	0x7F
#define CFG_I2C_NOPROBES	{{0x52}} /* Don't probe these addrs */
#define CFG_I2C_OFFSET	0x3000
#define CFG_I2C2_OFFSET 0x3100

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#define CONFIG_PCI
#define CONFIG_83XX_GENERIC_PCI	1

#define CFG_PCI1_MEM_BASE	0x80000000
#define CFG_PCI1_MEM_PHYS	CFG_PCI1_MEM_BASE
#define CFG_PCI1_MEM_SIZE	0x10000000 /* 256M */
#define CFG_PCI1_MMIO_BASE	0x90000000
#define CFG_PCI1_MMIO_PHYS	CFG_PCI1_MMIO_BASE
#define CFG_PCI1_MMIO_SIZE	0x10000000 /* 256M */
#define CFG_PCI1_IO_BASE	0xE0300000
#define CFG_PCI1_IO_PHYS	0xE0300000
#define CFG_PCI1_IO_SIZE	0x100000 /* 1M */

#ifdef CONFIG_PCI

#define CONFIG_NET_MULTI
#define CONFIG_PCI_PNP		/* do pci plug-and-play */

#undef CONFIG_EEPRO100
#undef CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup */
#define CFG_PCI_SUBSYS_VENDORID 0x1957	/* Freescale */

#endif	/* CONFIG_PCI */


#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"Freescale GETH"

#define CONFIG_UEC_ETH1		/* GETH1 */

#ifdef CONFIG_UEC_ETH1
#define CFG_UEC1_UCC_NUM	0	/* UCC1 */
#define CFG_UEC1_RX_CLK		QE_CLK_NONE
#define CFG_UEC1_TX_CLK		QE_CLK9
#define CFG_UEC1_ETH_TYPE	GIGA_ETH
#define CFG_UEC1_PHY_ADDR	2
#define CFG_UEC1_INTERFACE_MODE ENET_1000_GMII
#endif

#define CONFIG_UEC_ETH2		/* GETH2 */

#ifdef CONFIG_UEC_ETH2
#define CFG_UEC2_UCC_NUM	1	/* UCC2 */
#define CFG_UEC2_RX_CLK		QE_CLK_NONE
#define CFG_UEC2_TX_CLK		QE_CLK4
#define CFG_UEC2_ETH_TYPE	GIGA_ETH
#define CFG_UEC2_PHY_ADDR	4
#define CFG_UEC2_INTERFACE_MODE ENET_1000_GMII
#endif

/*
 * Environment
 */

#ifndef CFG_RAMBOOT
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + 0x40000)
#define CFG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#define CFG_ENV_SIZE		0x20000
#else /* CFG_RAMBOOT */
#define CFG_NO_FLASH		1	/* Flash is not usable now */
#define CFG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#define CFG_ENV_SIZE		0x2000
#endif /* CFG_RAMBOOT */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

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

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
#undef CONFIG_CMD_ENV
#undef CONFIG_CMD_LOADS
#endif

#undef CONFIG_WATCHDOG		/* watchdog disabled */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		/* undef to save memory */
#define CFG_LOAD_ADDR		0x2000000 /* default load address */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt */

#if defined(CONFIG_CMD_KGDB)
	#define CFG_CBSIZE	1024 /* Console I/O Buffer Size */
#else
	#define CFG_CBSIZE	256 /* Console I/O Buffer Size */
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_HZ		1000		/* decrementer freq: 1ms ticks */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * Core HID Setup
 */
#define CFG_HID0_INIT		0x000000000
#define CFG_HID0_FINAL		HID0_ENABLE_MACHINE_CHECK
#define CFG_HID2		HID2_HBE

/*
 * Cache Config
 */
#define CFG_DCACHE_SIZE		32768
#define CFG_CACHELINE_SIZE	32
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5 /*log base 2 of the above value */
#endif

/*
 * MMU Setup
 */

/* DDR: cache cacheable */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U

/* IMMRBAR & PCI IO: cache-inhibit and guarded */
#define CFG_IBAT1L	(CFG_IMMR | BATL_PP_10 | \
			BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT1U	(CFG_IMMR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U

/* NAND: cache-inhibit and guarded */
#define CFG_IBAT2L	(CFG_NAND_BASE | BATL_PP_10 | BATL_CACHEINHIBIT |\
			 BATL_GUARDEDSTORAGE)
#define CFG_IBAT2U	(CFG_NAND_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CFG_IBAT3L	(CFG_FLASH_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT3U	(CFG_FLASH_BASE | BATU_BL_32M | BATU_VS | BATU_VP)
#define CFG_DBAT3L	(CFG_FLASH_BASE | BATL_PP_10 | \
			 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_DBAT3U	CFG_IBAT3U

/* Stack in dcache: cacheable, no memory coherence */
#define CFG_IBAT4L	(CFG_INIT_RAM_ADDR | BATL_PP_10)
#define CFG_IBAT4U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CFG_DBAT4L	CFG_IBAT4L
#define CFG_DBAT4U	CFG_IBAT4U

#define CFG_IBAT5L	(CFG_VIDEO_BASE | BATL_PP_10 | BATL_CACHEINHIBIT | \
			 BATL_GUARDEDSTORAGE)
#define CFG_IBAT5U	(CFG_VIDEO_BASE | BATU_BL_64M | BATU_VS | BATU_VP)
#define CFG_DBAT5L	CFG_IBAT5L
#define CFG_DBAT5U	CFG_IBAT5U

#ifdef CONFIG_PCI
/* PCI MEM space: cacheable */
#define CFG_IBAT6L	(CFG_PCI1_MEM_PHYS | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT6U	(CFG_PCI1_MEM_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
/* PCI MMIO space: cache-inhibit and guarded */
#define CFG_IBAT7L	(CFG_PCI1_MMIO_PHYS | BATL_PP_10 | \
			 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CFG_IBAT7U	(CFG_PCI1_MMIO_PHYS | BATU_BL_256M | BATU_VS | BATU_VP)
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U
#else /* CONFIG_PCI */
#define CFG_IBAT6L	(0)
#define CFG_IBAT6U	(0)
#define CFG_IBAT7L	(0)
#define CFG_IBAT7U	(0)
#define CFG_DBAT6L	CFG_IBAT6L
#define CFG_DBAT6U	CFG_IBAT6U
#define CFG_DBAT7L	CFG_IBAT7L
#define CFG_DBAT7U	CFG_IBAT7U
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
#define CONFIG_ETHADDR	00:04:9f:ef:01:01
#define CONFIG_ETH1ADDR	00:04:9f:ef:01:02
#define CONFIG_ETH2ADDR	00:04:9f:ef:01:03
#define CONFIG_ETH3ADDR	00:04:9f:ef:01:04
#endif

#define CONFIG_BAUDRATE 115200

#define CONFIG_LOADADDR	a00000
#define CONFIG_HOSTNAME	mpc8360erdk
#define CONFIG_BOOTFILE	uImage

#define CONFIG_IPADDR		10.0.0.99
#define CONFIG_SERVERIP		10.0.0.2
#define CONFIG_GATEWAYIP	10.0.0.2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_ROOTPATH		/nfsroot/

#define	CONFIG_BOOTDELAY 2	/* -1 disables auto-boot */
#undef	CONFIG_BOOTARGS		/* the boot command will set bootargs */

#define CONFIG_EXTRA_ENV_SETTINGS \
   "netdev=eth0\0"\
   "consoledev=ttyS0\0"\
   "loadaddr=a00000\0"\
   "fdtaddr=900000\0"\
   "bootfile=uImage\0"\
   "fdtfile=dtb\0"\
   "fsfile=fs\0"\
   "ubootfile=u-boot.bin\0"\
   "setbootargs=setenv bootargs console=$consoledev,$baudrate "\
   		"$mtdparts panic=1\0"\
   "adddhcpargs=setenv bootargs $bootargs ip=on\0"\
   "addnfsargs=setenv bootargs $bootargs ip=$ipaddr:$serverip:"\
   		"$gatewayip:$netmask:$hostname:$netdev:off "\
   		"root=/dev/nfs rw nfsroot=$serverip:$rootpath\0"\
   "tftp_get_uboot=tftp 100000 $ubootfile\0"\
   "tftp_get_kernel=tftp $loadaddr $bootfile\0"\
   "tftp_get_dtb=tftp $fdtaddr $fdtfile\0"\
   "tftp_get_fs=tftp c00000 $fsfile\0"\
   "nor_reflash=protect off ff800000 ff87ffff ; erase ff800000 ff87ffff ; "\
   		"cp.b 100000 ff800000 $filesize\0"\
   "boot_m=bootm $loadaddr - $fdtaddr\0"\
   "dhcpboot=run setbootargs adddhcpargs tftp_get_kernel tftp_get_dtb "\
   		"boot_m\0"\
   "nfsboot=run setbootargs addnfsargs tftp_get_kernel tftp_get_dtb "\
   		"boot_m\0"\
   ""

#define CONFIG_BOOTCOMMAND "run dhcpboot"

#endif /* __CONFIG_H */
