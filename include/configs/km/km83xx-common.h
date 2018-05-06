/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_KM83XX_H
#define __CONFIG_KM83XX_H

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"
#include "km-powerpc.h"

#define CONFIG_MISC_INIT_R
/*
 * System Clock Setup
 */
#define CONFIG_83XX_CLKIN		66000000
#define CONFIG_SYS_CLK_FREQ		66000000
#define CONFIG_83XX_PCICLK		66000000

/*
 * IMMR new address
 */
#define CONFIG_SYS_IMMR		0xE0000000

/*
 * Bus Arbitration Configuration Register (ACR)
 */
#define CONFIG_SYS_ACR_PIPE_DEP 3       /* pipeline depth 4 transactions */
#define CONFIG_SYS_ACR_RPTCNT   3       /* 4 consecutive transactions */
#define CONFIG_SYS_ACR_APARK    0       /* park bus to master (below) */
#define CONFIG_SYS_ACR_PARKM    3       /* parking master = QuiccEngine */

/*
 * DDR Setup
 */
#define CONFIG_SYS_DDR_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_SDRAM_BASE2	(CONFIG_SYS_SDRAM_BASE + 0x10000000) /* +256M */

#define CONFIG_SYS_DDR_SDRAM_BASE	CONFIG_SYS_DDR_BASE
#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
					DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

/*
 * Manually set up DDR parameters
 */
#define CONFIG_DDR_II
#define CONFIG_SYS_DDR_SIZE		2048 /* MB */

/*
 * The reserved memory
 */
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE /* start of monitor */
#define CONFIG_SYS_FLASH_BASE		0xF0000000

#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

/* Reserve 768 kB for Mon */
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  0   Local   GPCM    16 bit  256MB FLASH
 *  1   Local   GPCM     8 bit  128MB GPIO/PIGGY
 *
 */
/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_CFI		/* use the Common Flash Interface */
#define CONFIG_FLASH_CFI_DRIVER		/* use the CFI driver */
#define CONFIG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

#define CONFIG_SYS_LBLAWBAR0_PRELIM	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_LBLAWAR0_PRELIM	(LBLAWAR_EN | LBLAWAR_256MB)

#define CONFIG_SYS_BR0_PRELIM	(CONFIG_SYS_FLASH_BASE | \
				BR_PS_16 | /* 16 bit port size */ \
				BR_MS_GPCM | /* MSEL = GPCM */ \
				BR_V)

#define CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(CONFIG_SYS_FLASH_SIZE) | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_5 | \
				OR_GPCM_TRLX_SET | OR_GPCM_EAD)

#define CONFIG_SYS_MAX_FLASH_BANKS	1   /* max num of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512 /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }

/*
 * PRIO1/PIGGY on the local bus CS1
 */
/* Window base at flash base */
#define CONFIG_SYS_LBLAWBAR1_PRELIM	CONFIG_SYS_KMBEC_FPGA_BASE
#define CONFIG_SYS_LBLAWAR1_PRELIM	(LBLAWAR_EN | LBLAWAR_128MB)

#define CONFIG_SYS_BR1_PRELIM	(CONFIG_SYS_KMBEC_FPGA_BASE | \
				BR_PS_8 | /* 8 bit port size */ \
				BR_MS_GPCM | /* MSEL = GPCM */ \
				BR_V)
#define CONFIG_SYS_OR1_PRELIM	(MEG_TO_AM(CONFIG_SYS_KMBEC_FPGA_SIZE) | \
				OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | \
				OR_GPCM_SCY_2 | \
				OR_GPCM_TRLX_SET | OR_GPCM_EAD)

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_bus_freq(0)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_IMMR+0x4500)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_IMMR+0x4600)

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
#define CONFIG_ETHPRIME		"UEC0"

#if !defined(CONFIG_MPC8309)
#define CONFIG_UEC_ETH1		/* GETH1 */
#define UEC_VERBOSE_DEBUG	1
#endif

#ifdef CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM	3	/* UCC4 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK_NONE /* not used in RMII Mode */
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK17
#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	0
#define CONFIG_SYS_UEC1_INTERFACE_TYPE	PHY_INTERFACE_MODE_RMII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100
#endif

/*
 * Environment
 */

#ifndef CONFIG_SYS_RAMBOOT
#ifndef CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
					CONFIG_SYS_MONITOR_LEN)
#endif
#define CONFIG_ENV_SECT_SIZE	0x20000 /* 128K(one sector) for env */
#ifndef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET	(CONFIG_SYS_MONITOR_LEN)
#endif

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
						CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#else /* CFG_SYS_RAMBOOT */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE - 0x1000)
#define CONFIG_ENV_SIZE		0x2000
#endif /* CFG_SYS_RAMBOOT */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_NUM_I2C_BUSES	4
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_I2C_FSL
#define CONFIG_SYS_FSL_I2C_SPEED	200000
#define CONFIG_SYS_FSL_I2C_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C_OFFSET	0x3000
#define CONFIG_SYS_I2C_OFFSET		0x3000
#define CONFIG_SYS_FSL_I2C2_SPEED	200000
#define CONFIG_SYS_FSL_I2C2_SLAVE	0x7F
#define CONFIG_SYS_FSL_I2C2_OFFSET	0x3100
#define CONFIG_SYS_I2C_BUSES	{{0, {I2C_NULL_HOP} }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 2} } }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 1} } }, \
		{1, {I2C_NULL_HOP} } }

#define CONFIG_KM_IVM_BUS		2	/* I2C2 (Mux-Port 1)*/

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		CONFIG_SYS_KMBEC_FPGA_BASE
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Core HID Setup
 */
#define CONFIG_SYS_HID0_INIT		0x000000000
#define CONFIG_SYS_HID0_FINAL		(HID0_ENABLE_MACHINE_CHECK | \
					 HID0_ENABLE_INSTRUCTION_CACHE)
#define CONFIG_SYS_HID2			HID2_HBE

/*
 * MMU Setup
 */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/* DDR: cache cacheable */
#define CONFIG_SYS_IBAT0L	(CONFIG_SYS_SDRAM_BASE | BATL_PP_RW | \
				BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT0U	(CONFIG_SYS_SDRAM_BASE | BATU_BL_256M | \
					BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT0L	CONFIG_SYS_IBAT0L
#define CONFIG_SYS_DBAT0U	CONFIG_SYS_IBAT0U

/* IMMRBAR & PCI IO: cache-inhibit and guarded */
#define CONFIG_SYS_IBAT1L	(CONFIG_SYS_IMMR | BATL_PP_RW | \
				BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_IBAT1U	(CONFIG_SYS_IMMR | BATU_BL_4M | BATU_VS \
					| BATU_VP)
#define CONFIG_SYS_DBAT1L	CONFIG_SYS_IBAT1L
#define CONFIG_SYS_DBAT1U	CONFIG_SYS_IBAT1U

/* PRIO1, PIGGY:  icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT2L	(CONFIG_SYS_KMBEC_FPGA_BASE | BATL_PP_RW | \
				BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT2U	(CONFIG_SYS_KMBEC_FPGA_BASE | BATU_BL_128M | \
				BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT2L	(CONFIG_SYS_KMBEC_FPGA_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT2U	CONFIG_SYS_IBAT2U

/* FLASH: icache cacheable, but dcache-inhibit and guarded */
#define CONFIG_SYS_IBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_RW | \
					BATL_MEMCOHERENCE)
#define CONFIG_SYS_IBAT3U	(CONFIG_SYS_FLASH_BASE | BATU_BL_256M | \
					BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT3L	(CONFIG_SYS_FLASH_BASE | BATL_PP_RW | \
				 BATL_CACHEINHIBIT | BATL_GUARDEDSTORAGE)
#define CONFIG_SYS_DBAT3U	CONFIG_SYS_IBAT3U

/* Stack in dcache: cacheable, no memory coherence */
#define CONFIG_SYS_IBAT4L	(CONFIG_SYS_INIT_RAM_ADDR | BATL_PP_RW)
#define CONFIG_SYS_IBAT4U	(CONFIG_SYS_INIT_RAM_ADDR | BATU_BL_128K | \
					BATU_VS | BATU_VP)
#define CONFIG_SYS_DBAT4L	CONFIG_SYS_IBAT4L
#define CONFIG_SYS_DBAT4U	CONFIG_SYS_IBAT4U

/*
 * Internal Definitions
 */
#define BOOTFLASH_START	0xF0000000

#define CONFIG_KM_CONSOLE_TTY	"ttyS0"

/*
 * Environment Configuration
 */
#define CONFIG_ENV_OVERWRITE
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV "km-common=empty\0"
#endif

#ifndef CONFIG_KM_DEF_ARCH
#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_KM_DEF_ENV						\
	CONFIG_KM_DEF_ARCH						\
	"newenv="							\
		"prot off "__stringify(CONFIG_ENV_ADDR)" +0x40000 && "	\
		"era "__stringify(CONFIG_ENV_ADDR)" +0x40000\0"		\
	"unlock=yes\0"							\
	""

#if defined(CONFIG_UEC_ETH)
#define CONFIG_HAS_ETH0
#endif

#endif /* __CONFIG_KM83XX_H */
