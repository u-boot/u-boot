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
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
#define CONFIG_KMETER1		1 /* KMETER1 board specific */
#define CONFIG_HOSTNAME		kmeter1

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"

#define CONFIG_MISC_INIT_R	1
/*
 * System Clock Setup
 */
#define CONFIG_83XX_CLKIN		66000000
#define CONFIG_SYS_CLK_FREQ		66000000
#define CONFIG_83XX_PCICLK		66000000

/*
 * Hardware Reset Configuration Word
 */
#define CONFIG_SYS_HRCW_LOW (\
	HRCWL_CSB_TO_CLKIN_4X1 | \
	HRCWL_CORE_TO_CSB_2X1 | \
	HRCWL_CE_PLL_VCO_DIV_2 | \
	HRCWL_CE_TO_PLL_1X6 )

#define CONFIG_SYS_HRCW_HIGH (\
	HRCWH_CORE_ENABLE | \
	HRCWH_FROM_0X00000100 | \
	HRCWH_BOOTSEQ_DISABLE | \
	HRCWH_SW_WATCHDOG_DISABLE | \
	HRCWH_ROM_LOC_LOCAL_16BIT | \
	HRCWH_BIG_ENDIAN | \
	HRCWH_LALE_EARLY | \
	HRCWH_LDP_CLEAR )

/*
 * System IO Config
 */
#define CONFIG_SYS_SICRH		0x00000006
#define CONFIG_SYS_SICRL		0x00000000

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

#define CFG_83XX_DDR_USES_CS0

#undef CONFIG_DDR_ECC

/*
 * DDRCDR - DDR Control Driver Register
 */

#undef CONFIG_SPD_EEPROM	/* Do not use SPD EEPROM for DDR setup */

/*
 * Manually set up DDR parameters
 */
#define CONFIG_DDR_II
#define CONFIG_SYS_DDR_SIZE		2048 /* MB */
#define CONFIG_SYS_DDR_CS0_BNDS		0x0000007f
#define CONFIG_SYS_DDR_CS0_CONFIG	(CSCONFIG_EN | CSCONFIG_AP | \
					 CSCONFIG_ROW_BIT_13 | \
					 CSCONFIG_COL_BIT_10 | CSCONFIG_ODT_WR_ACS)

#define CONFIG_SYS_DDR_SDRAM_CFG	(SDRAM_CFG_SDRAM_TYPE_DDR2 | \
					 SDRAM_CFG_SREN)
#define CONFIG_SYS_DDR_SDRAM_CFG2	0x00401000
#define CONFIG_SYS_DDR_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)
#define CONFIG_SYS_DDR_INTERVAL	((0x080 << SDRAM_INTERVAL_BSTOPRE_SHIFT) | \
				 (0x3cf << SDRAM_INTERVAL_REFINT_SHIFT))

#define	CONFIG_SYS_DDRCDR		0x40000001
#define CONFIG_SYS_DDR_MODE		0x47860452
#define CONFIG_SYS_DDR_MODE2		0x8080c000

#define CONFIG_SYS_DDR_TIMING_0	((2 << TIMING_CFG0_MRS_CYC_SHIFT) | \
				 (8 << TIMING_CFG0_ODT_PD_EXIT_SHIFT) | \
				 (6 << TIMING_CFG0_PRE_PD_EXIT_SHIFT) | \
				 (2 << TIMING_CFG0_ACT_PD_EXIT_SHIFT) | \
				 (0 << TIMING_CFG0_WWT_SHIFT) | \
				 (0 << TIMING_CFG0_RRT_SHIFT) | \
				 (0 << TIMING_CFG0_WRT_SHIFT) | \
				 (0 << TIMING_CFG0_RWT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_1	((      TIMING_CFG1_CASLAT_50) | \
				 ( 2 << TIMING_CFG1_WRTORD_SHIFT) | \
				 ( 2 << TIMING_CFG1_ACTTOACT_SHIFT) | \
				 ( 3 << TIMING_CFG1_WRREC_SHIFT) | \
				 ( 7 << TIMING_CFG1_REFREC_SHIFT) | \
				 ( 3 << TIMING_CFG1_ACTTORW_SHIFT) | \
				 ( 8 << TIMING_CFG1_ACTTOPRE_SHIFT) | \
				 ( 3 << TIMING_CFG1_PRETOACT_SHIFT))

#define CONFIG_SYS_DDR_TIMING_2	((8 << TIMING_CFG2_FOUR_ACT_SHIFT) | \
				 (3 << TIMING_CFG2_CKE_PLS_SHIFT) | \
				 (2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT) | \
				 (2 << TIMING_CFG2_RD_TO_PRE_SHIFT) | \
				 (4 << TIMING_CFG2_WR_LAT_DELAY_SHIFT) | \
				 (0 << TIMING_CFG2_ADD_LAT_SHIFT) | \
				 (5 << TIMING_CFG2_CPO_SHIFT))

#define CONFIG_SYS_DDR_TIMING_3	0x00000000

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	TEXT_BASE /* start of monitor */
#define CONFIG_SYS_FLASH_BASE		0xF0000000
#define CONFIG_SYS_FLASH_BASE_1		0xF2000000
#define CONFIG_SYS_PIGGY_BASE		0xE8000000
#define	CONFIG_SYS_PIGGY_SIZE		128
#define CONFIG_SYS_PAXE_BASE		0xA0000000
#define	CONFIG_SYS_PAXE_SIZE		512

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#else
#undef	CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(384 * 1024) /* Reserve 384 kB for Mon */

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
#define CONFIG_SYS_LCRR_EADC	LCRR_EADC_2
#define CONFIG_SYS_LCRR_CLKDIV	LCRR_CLKDIV_4

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  0   Local   GPCM    16 bit  256MB FLASH
 *  1   Local   GPCM     8 bit  128MB GPIO/PIGGY
 *  3   Local   GPCM     8 bit  512MB PAXE
 *
 */
/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#define CONFIG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */
#define CONFIG_SYS_FLASH_PROTECTION	1
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE /* Window base at flash base */
#define CONFIG_SYS_LBLAWAR0_PRELIM	0x8000001b /* 256MB window size */

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE | \
				(2 << BR_PS_SHIFT) | /* 16 bit port size */ \
				BR_V)

#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_5 | \
				OR_GPCM_TRLX | OR_GPCM_EAD)

#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE_1 }

#undef	CONFIG_SYS_FLASH_CHECKSUM

/*
 * PRIO1/PIGGY on the local bus CS1
 */
#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_PIGGY_BASE /* Window base at flash base */
#define CONFIG_SYS_LBLAWAR1_PRELIM	0x8000001A /* 128MB window size */

#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_PIGGY_BASE | \
				(1 << BR_PS_SHIFT) | /* 8 bit port size */ \
				BR_V)
#define CONFIG_SYS_OR1_PRELIM		(MEG_TO_AM(CONFIG_SYS_PIGGY_SIZE) | /* 128MB */ \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_2 | \
				OR_GPCM_TRLX | OR_GPCM_EAD)

/*
 * PAXE on the local bus CS3
 */
#define CONFIG_SYS_LBLAWBAR3_PRELIM	CONFIG_SYS_PAXE_BASE /* Window base at flash base */
#define CONFIG_SYS_LBLAWAR3_PRELIM	0x8000001C /* 512MB window size */

#define CONFIG_SYS_BR3_PRELIM	(CONFIG_SYS_PAXE_BASE | \
				(1 << BR_PS_SHIFT) | /* 8 bit port size */ \
				BR_V)
#define CONFIG_SYS_OR3_PRELIM	(MEG_TO_AM(CONFIG_SYS_PAXE_SIZE) | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_2 | \
				OR_GPCM_TRLX | OR_GPCM_EAD)

/*
 * Serial Port
 */
#define CONFIG_CONS_INDEX	1
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/* Pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1
#define CONFIG_OF_STDOUT_VIA_ALIAS

/*
 * General PCI
 * Addresses are mapped 1-1.
 */
#undef CONFIG_PCI		/* No PCI */

#ifndef CONFIG_NET_MULTI
#define CONFIG_NET_MULTI	1
#endif
/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"FSL UEC0"

#define CONFIG_UEC_ETH1		/* GETH1 */
#define UEC_VERBOSE_DEBUG	1

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	3	/* UCC4 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK_NONE	/* not used in RMII Mode */
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK17
#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	0
#define CONFIG_SYS_UEC1_INTERFACE_MODE ENET_100_RMII
#endif

/*
 * Environment
 */

#ifndef CONFIG_SYS_RAMBOOT
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#define CONFIG_ENV_OFFSET	(CONFIG_SYS_MONITOR_LEN)

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET+CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#else /* CFG_RAMBOOT */
#define CONFIG_SYS_NO_FLASH		1	/* Flash is not usable now */
#define CONFIG_ENV_IS_NOWHERE	1	/* Store ENV in memory only */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif /* CFG_RAMBOOT */

/* I2C */
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged */
#define CONFIG_FSL_I2C
#define CONFIG_SYS_I2C_SPEED	200000	/* I2C speed and slave address */
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SYS_I2C_OFFSET	0x3000
#define CONFIG_I2C_MULTI_BUS	1
#define CONFIG_I2C_MUX		1

/* EEprom support */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1	/* ON Semi's LM75		*/
#define CONFIG_DTT_SENSORS	{0, 1, 2, 3}	/* Sensor addresses		*/
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define CONFIG_SYS_DTT_HYSTERESIS	3
#define CONFIG_SYS_DTT_BUS_NUM		(CONFIG_SYS_MAX_I2C_BUS)

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		CONFIG_SYS_PIGGY_BASE
#endif

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif

#if defined(CFG_RAMBOOT)
#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_LOADS
#endif

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
#define CONFIG_SYS_HID2			HID2_HBE

/*
 * MMU Setup
 */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR: cache cacheable */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_10 | \
				BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

/* IMMRBAR & PCI IO: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_IMMR | BATL_PP_10 | \
				BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_IMMR | BATU_BL_4M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* PRIO1, PIGGY:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_PIGGY_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_PIGGY_BASE | BATU_BL_128M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_PIGGY_BASE | BATL_PP_10 | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_FLASH_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_10 | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/* Stack in dcache: cacheable, no memory coherence */
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_10)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U

/* PAXE:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT5L	(CONFIG_SYS_PAXE_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT5U	(CONFIG_SYS_PAXE_BASE | BATU_BL_256M | BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT5L	(CONFIG_SYS_PAXE_BASE | BATL_PP_10 | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT5U	CONFIG_SYS_IBAT5U

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

#define BOOTFLASH_START	F0000000

#define CONFIG_PRAM	512	/* protected RAM [KBytes] */

#define MTDIDS_DEFAULT		"nor2=app"
#define MTDPARTS_DEFAULT \
	"mtdparts=app:256k(u-boot),128k(env),128k(envred),"	\
	"1536k(esw0),8704k(rootfs0),1536k(esw1),2432k(rootfs1),640k(var),768k(cfg)"

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV "km-common=empty\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
       CONFIG_KM_DEF_ENV						\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"addcon=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"ramdisk_file=/tftpboot/kmeter1/uRamdisk\0"			\
	"loadram=tftp ${ramdisk_addr_r} ${ramdisk_file}\0"		\
	"loadfdt=tftp ${fdt_addr_r} ${fdt_file}\0"			\
	"loadkernel=tftp ${kernel_addr_r} ${bootfile}\0"		\
	"unlock=yes\0"							\
	"fdt_addr=F0080000\0"						\
	"kernel_addr=F00a0000\0"					\
	"ramdisk_addr=F03a0000\0"					\
	"ramdisk_addr_r=F10000\0"					\
	"EEprom_ivm=pca9547:70:9\0"					\
	"dtt_bus=pca9547:70:a\0"					\
	"mtdids=nor0=app \0"						\
	"mtdparts=" MK_STR(MTDPARTS_DEFAULT) "\0"			\
   ""

#if defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#endif

#endif /* __CONFIG_H */
