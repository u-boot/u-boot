/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Keymile AG
 * Rainer Boschung <rainer.boschung@keymile.com>
 *
 */

#ifndef __KMCENT2_H
#define __KMCENT2_H

#define CONFIG_HOSTNAME		"kmcent2"
#define KM_BOARD_NAME	CONFIG_HOSTNAME

/*
 * The Linux fsl_fman driver needs to be able to process frames with more
 * than just the VLAN tag (i.e. eDSA tag). It is passed as a kernel boot
 * parameters
 */
#define CONFIG_KM_DEF_BOOT_ARGS_CPU	"fsl_dpaa_fman.fsl_fm_max_frm=1558"

#include "km/keymile-common.h"

/* Application IFC chip selects */
#define SYS_LAWAPP_BASE		0xc0000000
#define SYS_LAWAPP_BASE_PHYS	(0xf00000000ull | SYS_LAWAPP_BASE)

/* Application IFC CS4 MRAM */
#define CONFIG_SYS_MRAM_BASE		SYS_LAWAPP_BASE
#define SYS_MRAM_BASE_PHYS	SYS_LAWAPP_BASE_PHYS
#define SYS_MRAM_CSPR_EXT	(0x0f)
#define SYS_MRAM_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_MRAM_BASE) | \
				CSPR_PORT_SIZE_8 | /* 8 bit */		\
				CSPR_MSEL_GPCM   | /* msel = gpcm */	\
				CSPR_V /* bank is valid */)
#define SYS_MRAM_AMASK		IFC_AMASK(2 * 1024 * 1024) /* 2 MiB */
#define SYS_MRAM_CSOR		CSOR_GPCM_TRHZ_40
/* MRAM Timing parameters for IFC CS4 */
#define SYS_MRAM_FTIM0	(FTIM0_GPCM_TACSE(0x6) | \
			FTIM0_GPCM_TEADC(0x8)  | \
			FTIM0_GPCM_TEAHC(0x2))
#define SYS_MRAM_FTIM1	(FTIM1_GPCM_TACO(0x2) | \
			FTIM1_GPCM_TRAD(0xe))
#define SYS_MRAM_FTIM2	(FTIM2_GPCM_TCS(0x2) | \
			FTIM2_GPCM_TCH(0x2)  | \
			FTIM2_GPCM_TWP(0x8))
#define SYS_MRAM_FTIM3	0x04000000
#define CONFIG_SYS_CSPR4_EXT	SYS_MRAM_CSPR_EXT
#define CONFIG_SYS_CSPR4	SYS_MRAM_CSPR
#define CONFIG_SYS_AMASK4	SYS_MRAM_AMASK
#define CONFIG_SYS_CSOR4	SYS_MRAM_CSOR
#define CONFIG_SYS_CS4_FTIM0	SYS_MRAM_FTIM0
#define CONFIG_SYS_CS4_FTIM1	SYS_MRAM_FTIM1
#define CONFIG_SYS_CS4_FTIM2	SYS_MRAM_FTIM2
#define CONFIG_SYS_CS4_FTIM3	SYS_MRAM_FTIM3

/* Application IFC CS6: BFTIC */
#define SYS_BFTIC_BASE		0xd0000000
#define SYS_BFTIC_BASE_PHYS	(0xf00000000ull | SYS_BFTIC_BASE)
#define SYS_BFTIC_CSPR_EXT	(0x0f)
#define SYS_BFTIC_CSPR	(CSPR_PHYS_ADDR(SYS_BFTIC_BASE) | \
				CSPR_PORT_SIZE_8 | /* Port size = 8 bit */\
				CSPR_MSEL_GPCM |   /* MSEL = GPCM */\
				CSPR_V)            /* valid */
#define SYS_BFTIC_AMASK IFC_AMASK(64 * 1024)  /* 64kB */
#define SYS_BFTIC_CSOR  CSOR_GPCM_TRHZ_40
/* BFTIC Timing parameters for IFC CS6 */
#define SYS_BFTIC_FTIM0 (FTIM0_GPCM_TACSE(0x6) | \
				FTIM0_GPCM_TEADC(0x8) | \
				FTIM0_GPCM_TEAHC(0x2))
#define SYS_BFTIC_FTIM1 (FTIM1_GPCM_TACO(0x2) | \
				FTIM1_GPCM_TRAD(0x12))
#define SYS_BFTIC_FTIM2 (FTIM2_GPCM_TCS(0x3) | \
				FTIM2_GPCM_TCH(0x1) | \
				FTIM2_GPCM_TWP(0x12))
#define SYS_BFTIC_FTIM3	0x04000000
#define CONFIG_SYS_CSPR6_EXT	SYS_BFTIC_CSPR_EXT
#define CONFIG_SYS_CSPR6	SYS_BFTIC_CSPR
#define CONFIG_SYS_AMASK6	SYS_BFTIC_AMASK
#define CONFIG_SYS_CSOR6	SYS_BFTIC_CSOR
#define CONFIG_SYS_CS6_FTIM0	SYS_BFTIC_FTIM0
#define CONFIG_SYS_CS6_FTIM1	SYS_BFTIC_FTIM1
#define CONFIG_SYS_CS6_FTIM2	SYS_BFTIC_FTIM2
#define CONFIG_SYS_CS6_FTIM3	SYS_BFTIC_FTIM3

/* Application IFC CS7 PAXE */
#define CONFIG_SYS_PAXE_BASE		0xd8000000
#define SYS_PAXE_BASE_PHYS	(0xf00000000ull | CONFIG_SYS_PAXE_BASE)
#define SYS_PAXE_CSPR_EXT	(0x0f)
#define SYS_PAXE_CSPR  (CSPR_PHYS_ADDR(CONFIG_SYS_PAXE_BASE) | \
				CSPR_PORT_SIZE_8 | /* Port size = 8 bit */\
				CSPR_MSEL_GPCM |   /* MSEL = GPCM */\
				CSPR_V)            /* valid */
#define SYS_PAXE_AMASK IFC_AMASK(64 * 1024)  /* 64kB */
#define SYS_PAXE_CSOR  CSOR_GPCM_TRHZ_40
/* PAXE Timing parameters for IFC CS7 */
#define SYS_PAXE_FTIM0 (FTIM0_GPCM_TACSE(0x6) | \
			FTIM0_GPCM_TEADC(0x8) | \
			FTIM0_GPCM_TEAHC(0x2))
#define SYS_PAXE_FTIM1 (FTIM1_GPCM_TACO(0x2) | \
			FTIM1_GPCM_TRAD(0x12))
#define SYS_PAXE_FTIM2 (FTIM2_GPCM_TCS(0x3) | \
			FTIM2_GPCM_TCH(0x1) | \
			FTIM2_GPCM_TWP(0x12))
#define SYS_PAXE_FTIM3	0x04000000
#define CONFIG_SYS_CSPR7_EXT	SYS_PAXE_CSPR_EXT
#define CONFIG_SYS_CSPR7	SYS_PAXE_CSPR
#define CONFIG_SYS_AMASK7	SYS_PAXE_AMASK
#define CONFIG_SYS_CSOR7	SYS_PAXE_CSOR
#define CONFIG_SYS_CS7_FTIM0	SYS_PAXE_FTIM0
#define CONFIG_SYS_CS7_FTIM1	SYS_PAXE_FTIM1
#define CONFIG_SYS_CS7_FTIM2	SYS_PAXE_FTIM2
#define CONFIG_SYS_CS7_FTIM3	SYS_PAXE_FTIM3

/* PRST */
#define KM_BFTIC4_RST		0
#define KM_DPAXE_RST		1
#define KM_FEMT_RST		3
#define KM_FOAM_RST		4
#define KM_EFE_RST		5
#define KM_ES_PHY_RST		6
#define KM_XES_PHY_RST		7
#define KM_ZL30158_RST		8
#define KM_ZL30364_RST		9
#define KM_BOBCAT_RST		10
#define KM_ETHSW_DDR_RST		12
#define KM_CFE_RST		13
#define KM_PEXSW_RST		14
#define KM_PEXSW_NT_RST		15

/* QRIO GPIOs used for deblocking */
#define KM_I2C_DEBLOCK_PORT	QRIO_GPIO_A
#define KM_I2C_DEBLOCK_SCL	20
#define KM_I2C_DEBLOCK_SDA	21

/* High Level Configuration Options */
#define CONFIG_SYS_BOOK3E_HV		/* Category E.HV supported */
#define CONFIG_FSL_CORENET		/* Freescale CoreNet platform */

#define CONFIG_RESET_VECTOR_ADDRESS	0xebfffffc

#define CONFIG_SYS_FSL_CPC		/* Corenet Platform Cache */
#define CONFIG_SYS_NUM_CPC		CONFIG_SYS_NUM_DDR_CTLRS
#define CONFIG_PCIE1			/* PCIE controller 1 */

/* Environment in parallel NOR-Flash */
#define CONFIG_ENV_TOTAL_SIZE		0x040000
#define ENV_DEL_ADDR		0xebf00000	/*direct for newenv*/

/*
 * These can be toggled for performance analysis, otherwise use default.
 */
#define CONFIG_SYS_CACHE_STASHING
#define CONFIG_SYS_INIT_L2CSR0		L2CSR0_L2E

#define CONFIG_ENABLE_36BIT_PHYS

/* POST memory regions test */
#define CONFIG_POST CONFIG_SYS_POST_MEM_REGIONS

/*
 *  Config the L3 Cache as L3 SRAM
 */
#define CONFIG_SYS_INIT_L3_ADDR		0xFFFC0000
#define CONFIG_SYS_L3_SIZE		256 << 10

#define CONFIG_SYS_DCSRBAR		0xf0000000
#define CONFIG_SYS_DCSRBAR_PHYS		0xf00000000ull

/*
 * DDR Setup
 */
#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x00000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_SYS_SPD_BUS_NUM	0
#define SPD_EEPROM_ADDRESS	0x54
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/******************************************************************************
 * (PRAM usage)
 * ... -------------------------------------------------------
 * ... |ROOTFSSIZE | PNVRAM |PHRAM |RESERVED_PRAM | END_OF_RAM
 * ... |<------------------- pram -------------------------->|
 * ... -------------------------------------------------------
 * @END_OF_RAM:
 * @CONFIG_KM_RESERVED_PRAM: reserved pram for special purpose
 * @CONFIG_KM_PHRAM: address for /var
 * @CONFIG_KM_PNVRAM: address for PNVRAM (for the application)
 * @CONFIG_KM_ROOTFSSIZE: address for rootfilesystem in RAM
 */

/* size of rootfs in RAM */
#define CONFIG_KM_ROOTFSSIZE	0x0
/* set the default PRAM value to at least PNVRAM + PHRAM when pram env variable
 * is not valid yet, which is the case for when u-boot copies itself to RAM
 */
#define CONFIG_PRAM		((CONFIG_KM_PNVRAM + CONFIG_KM_PHRAM) >> 10)

/*
 * IFC Definitions
 */
/* NOR flash on IFC CS0 */
#define CONFIG_SYS_FLASH_BASE		0xe8000000
#define CONFIG_SYS_FLASH_BASE_PHYS	(0xf00000000ull | \
					CONFIG_SYS_FLASH_BASE)

#define CONFIG_SYS_NOR_CSPR_EXT	(0x0f)
#define CONFIG_SYS_NOR_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE) | \
				CSPR_PORT_SIZE_16 | /* Port size = 16 bit */\
				0x00000010 |	    /* drive TE high */\
				CSPR_MSEL_NOR |	    /* MSEL = NOR */\
				CSPR_V)		    /* valid */
#define CONFIG_SYS_NOR_AMASK	IFC_AMASK(64 * 1024 * 1024) /* 64MB */
#define CONFIG_SYS_NOR_CSOR	(CSOR_NOR_AVD_TGL_PGM_EN | /* AVD toggle */\
				CSOR_NOR_TRHZ_20 | \
				CSOR_NOR_BCTLD)

/* NOR Flash Timing Params */
#define CONFIG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x1) | \
				FTIM0_NOR_TEADC(0x7) | \
				FTIM0_NOR_TEAHC(0x1))
#define CONFIG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x1) | \
				FTIM1_NOR_TRAD_NOR(0x21) | \
				FTIM1_NOR_TSEQRAD_NOR(0x21))
#define CONFIG_SYS_NOR_FTIM2	(FTIM2_NOR_TCH(0x1) | \
				FTIM2_NOR_TCS(0x1) | \
				FTIM2_NOR_TWP(0xb) | \
				FTIM2_NOR_TWPH(0x6))
#define CONFIG_SYS_NOR_FTIM3	0x0

#define CONFIG_SYS_CSPR0_EXT	CONFIG_SYS_NOR_CSPR_EXT
#define CONFIG_SYS_CSPR0	CONFIG_SYS_NOR_CSPR
#define CONFIG_SYS_AMASK0	CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0	CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0	CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1	CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2	CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3	CONFIG_SYS_NOR_FTIM3

/* More NOR Flash params */
#define CONFIG_SYS_FLASH_QUIET_TEST

#define CONFIG_SYS_MAX_FLASH_SECT	512	/* sectors per device */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE_PHYS}

/* NAND Flash on IFC CS1*/
#define CONFIG_SYS_NAND_BASE		0xfa000000
#define CONFIG_SYS_NAND_BASE_PHYS	(0xf00000000ull | CONFIG_SYS_NAND_BASE)

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0f)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE) | \
				CSPR_PORT_SIZE_8 | /* Port Size = 8 bit */\
				0x00000010 |	   /* drive TE high */\
				CSPR_MSEL_NAND |   /* MSEL = NAND */\
				CSPR_V)		   /* valid */
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024) /* 64kB */

#define CONFIG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN | /* ECC encoder on */ \
				CSOR_NAND_ECC_DEC_EN | /* ECC decoder on */   \
				CSOR_NAND_ECC_MODE_4 | /* 4-bit ECC */        \
				CSOR_NAND_RAL_3      | /* RAL = 3Bytes */     \
				CSOR_NAND_PGS_2K     | /* Page size = 2K */   \
				CSOR_NAND_SPRZ_128   | /* Spare size = 128 */ \
				CSOR_NAND_PB(64)     | /* 64 Pages/Block */   \
				CSOR_NAND_TRHZ_40    | /**/                   \
				CSOR_NAND_BCTLD)       /**/

/* ONFI NAND Flash mode0 Timing Params */
#define CONFIG_SYS_NAND_FTIM0	(FTIM0_NAND_TCCST(0x3) | \
				FTIM0_NAND_TWP(0x8) | \
				FTIM0_NAND_TWCHT(0x3) | \
				FTIM0_NAND_TWH(0x5))
#define CONFIG_SYS_NAND_FTIM1	(FTIM1_NAND_TADLE(0x1e) | \
				FTIM1_NAND_TWBE(0x1e) | \
				FTIM1_NAND_TRR(0x6) | \
				FTIM1_NAND_TRP(0x8))
#define CONFIG_SYS_NAND_FTIM2	(FTIM2_NAND_TRAD(0x9) | \
				FTIM2_NAND_TREH(0x5) | \
				FTIM2_NAND_TWHRE(0x3c))
#define CONFIG_SYS_NAND_FTIM3	(FTIM3_NAND_TWW(0x1e))

#define CONFIG_SYS_CSPR1_EXT	CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR1	CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1	CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1	CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0	CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1	CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2	CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3	CONFIG_SYS_NAND_FTIM3

/* More NAND Flash Params */
#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* QRIO on IFC CS2 */
#define CONFIG_SYS_QRIO_BASE		0xfb000000
#define CONFIG_SYS_QRIO_BASE_PHYS	(0xf00000000ull | CONFIG_SYS_QRIO_BASE)
#define SYS_QRIO_CSPR_EXT	(0x0f)
#define SYS_QRIO_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_QRIO_BASE) | \
				CSPR_PORT_SIZE_8 | /* Port size = 8 bit */\
				0x00000010 |	   /* drive TE high */\
				CSPR_MSEL_GPCM |   /* MSEL = GPCM */\
				CSPR_V)           /* valid */
#define SYS_QRIO_AMASK	IFC_AMASK(64 * 1024)  /* 64kB */
#define SYS_QRIO_CSOR	(CSOR_GPCM_TRHZ_20 |\
			CSOR_GPCM_BCTLD)
/* QRIO Timing parameters for IFC CS2 */
#define SYS_QRIO_FTIM0	(FTIM0_GPCM_TACSE(0x2) | \
			FTIM0_GPCM_TEADC(0x8) | \
			FTIM0_GPCM_TEAHC(0x2))
#define SYS_QRIO_FTIM1	(FTIM1_GPCM_TACO(0x2) | \
			FTIM1_GPCM_TRAD(0x6))
#define SYS_QRIO_FTIM2	(FTIM2_GPCM_TCS(0x1) | \
			FTIM2_GPCM_TCH(0x1) | \
			FTIM2_GPCM_TWP(0x7))
#define SYS_QRIO_FTIM3	0x04000000
#define CONFIG_SYS_CSPR2_EXT	SYS_QRIO_CSPR_EXT
#define CONFIG_SYS_CSPR2	SYS_QRIO_CSPR
#define CONFIG_SYS_AMASK2	SYS_QRIO_AMASK
#define CONFIG_SYS_CSOR2	SYS_QRIO_CSOR
#define CONFIG_SYS_CS2_FTIM0	SYS_QRIO_FTIM0
#define CONFIG_SYS_CS2_FTIM1	SYS_QRIO_FTIM1
#define CONFIG_SYS_CS2_FTIM2	SYS_QRIO_FTIM2
#define CONFIG_SYS_CS2_FTIM3	SYS_QRIO_FTIM3

#define CONFIG_HWCONFIG

/* define to use L1 as initial stack */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xfdd00000	/* Initial L1 address */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0xf
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW	0xfe03c000
/* The assembler doesn't like typecast */
#define CONFIG_SYS_INIT_RAM_ADDR_PHYS \
	((CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH * 1ull << 32) | \
	  CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW)
#define CONFIG_SYS_INIT_RAM_SIZE		0x00004000

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		0xc0000         /* 768k */

/*
 * Serial Port - controlled on board with jumper J8
 * open - index 2
 * shorted - index 1
 * Retain non-DM serial port for debug purposes.
 */
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		(get_bus_freq(0) / 2)
#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_CCSRBAR + 0x11C500)
#endif

#ifndef __ASSEMBLY__
void set_sda(int state);
void set_scl(int state);
int get_sda(void);
int get_scl(void);
#endif

/*
 * General PCI
 * Memory space is mapped 1-1, but I/O space must start from 0.
 */
/* controller 1 */
#define	CONFIG_SYS_PCIE1_MEM_VIRT	0x80000000
#define	CONFIG_SYS_PCIE1_MEM_PHYS	0xc00000000ull
#define CONFIG_SYS_PCIE1_IO_VIRT	0xf8000000
#define CONFIG_SYS_PCIE1_IO_PHYS	0xff8000000ull

#define CONFIG_SYS_BMAN_NUM_PORTALS	10
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#define CONFIG_SYS_BMAN_MEM_SIZE	0x02000000
#define CONFIG_SYS_BMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_BMAN_CENA_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_CINH_BASE       (CONFIG_SYS_BMAN_MEM_BASE + \
					CONFIG_SYS_BMAN_CENA_SIZE)
#define CONFIG_SYS_BMAN_CINH_SIZE       (CONFIG_SYS_BMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_BMAN_SWP_ISDR_REG	0xE08
#define CONFIG_SYS_QMAN_NUM_PORTALS	10
#define CONFIG_SYS_QMAN_MEM_BASE	0xf6000000
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff6000000ull
#define CONFIG_SYS_QMAN_MEM_SIZE	0x02000000
#define CONFIG_SYS_QMAN_SP_CINH_SIZE    0x1000
#define CONFIG_SYS_QMAN_CENA_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_CINH_BASE       (CONFIG_SYS_QMAN_MEM_BASE + \
					CONFIG_SYS_QMAN_CENA_SIZE)
#define CONFIG_SYS_QMAN_CINH_SIZE       (CONFIG_SYS_QMAN_MEM_SIZE >> 1)
#define CONFIG_SYS_QMAN_SWP_ISDR_REG	0xE08

#define CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_DPAA_PME

#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

/* Qman / Bman */
/* RGMII (FM1@DTESC5) is local managemant interface */
#define CONFIG_SYS_RGMII2_PHY_ADDR             0x11

/*
 * Hardware Watchdog
 */
#define CONFIG_WATCHDOG_PRESC 34	/* wdog prescaler 2^(64-34) ~10min */
#define CONFIG_WATCHDOG_RC WRC_CHIP	/* reset chip on watchdog event */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)	/* Initial map for Linux*/
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)	/* Increase max gunzip size */

/*
 * Environment Configuration
 */
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV
#endif

#define __USB_PHY_TYPE	utmi

#define CONFIG_KM_DEF_ENV_CPU						\
	"boot=bootm ${load_addr_r} - ${fdt_addr_r}\0"			\
	"cramfsloadfdt="						\
		"cramfsload ${fdt_addr_r} "				\
		"fdt_0x${IVM_BoardId}_0x${IVM_HWKey}.dtb\0"		\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.bin\0"		\
	"update=protect off " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize} && "					\
		"erase " __stringify(CONFIG_SYS_MONITOR_BASE)		\
		" +${filesize} && "					\
		"cp.b ${load_addr_r} "					\
		__stringify(CONFIG_SYS_MONITOR_BASE) " ${filesize} && "	\
		"protect on " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +${filesize}\0"					\
	"update-nor=protect off " __stringify(CONFIG_SYS_FLASH_BASE)	\
		" +${filesize} && "					\
		"erase " __stringify(CONFIG_SYS_FLASH_BASE)		\
		" +${filesize} && "					\
		"cp.b ${load_addr_r} "					\
		__stringify(CONFIG_SYS_FLASH_BASE) " ${filesize} && "	\
		"protect on " __stringify(CONFIG_SYS_MONITOR_BASE)	\
		" +" __stringify(CONFIG_SYS_MONITOR_LEN) "\0"		\
	"set_fdthigh=true\0"						\
	"checkfdt=true\0"						\
	"fpgacfg=true\0"						\
	""

#define CONFIG_HW_ENV_SETTINGS						\
	"hwconfig=fsl_ddr:ctlr_intlv=cacheline\0"			\
	"usb_phy_type=" __stringify(__USB_PHY_TYPE) "\0"		\
	"usb_dr_mode=host\0"

#define CONFIG_KM_NEW_ENV						\
	"newenv=protect off " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) " && "		\
		"erase " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) " && "		\
		"protect on " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) "\0"

/* ppc_82xx is the equivalent to ppc_6xx, the generic ppc toolchain */
#ifndef CONFIG_KM_DEF_ARCH
#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_DEF_ENV						\
	CONFIG_KM_DEF_ARCH						\
	CONFIG_KM_NEW_ENV						\
	CONFIG_HW_ENV_SETTINGS						\
	"EEprom_ivm=pca9547:70:9\0"					\
	""

#endif	/* __KMCENT2_H */
