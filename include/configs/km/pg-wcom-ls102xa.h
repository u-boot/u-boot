/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Hitachi Power Grids. All rights reserved.
 */

#ifndef __CONFIG_PG_WCOM_LS102XA_H
#define __CONFIG_PG_WCOM_LS102XA_H

/* include common defines/options for all Keymile boards */
#include "keymile-common.h"

#define CONFIG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

#define CONFIG_PRAM			((CONFIG_KM_PNVRAM + \
					  CONFIG_KM_PHRAM + \
					  CONFIG_KM_RESERVED_PRAM) >> 10)

#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define SPD_EEPROM_ADDRESS		0x54

/* POST memory regions test */
#define CONFIG_POST			(CONFIG_SYS_POST_MEM_REGIONS)
#define CONFIG_POST_EXTERNAL_WORD_FUNCS

/*
 * IFC Definitions
 */
/* NOR Flash Definitions */
#define CONFIG_SYS_FLASH_BASE		0x60000000
#define CONFIG_SYS_FLASH_BASE_PHYS	CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_NOR0_CSPR_EXT	(0x0)
#define CONFIG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS) | \
				CSPR_PORT_SIZE_16 | \
				CSPR_TE | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CONFIG_SYS_NOR_AMASK		IFC_AMASK(64 * 1024 * 1024)

#define CONFIG_SYS_NOR_CSOR		(CSOR_NOR_AVD_TGL_PGM_EN | \
					CSOR_NOR_ADM_SHIFT(0x4) | \
					CSOR_NOR_NOR_MODE_ASYNC_NOR | \
					CSOR_NOR_TRHZ_20 | \
					CSOR_NOR_BCTLD)
#define CONFIG_SYS_NOR_FTIM0		(FTIM0_NOR_TACSE(0x1) | \
					FTIM0_NOR_TEADC(0x7) | \
					FTIM0_NOR_TAVDS(0x0) | \
					FTIM0_NOR_TEAHC(0x1))
#define CONFIG_SYS_NOR_FTIM1		(FTIM1_NOR_TACO(0x1) | \
					FTIM1_NOR_TRAD_NOR(0x21) | \
					FTIM1_NOR_TSEQRAD_NOR(0x21))
#define CONFIG_SYS_NOR_FTIM2		(FTIM2_NOR_TCS(0x1) | \
					FTIM2_NOR_TCH(0x1) | \
					FTIM2_NOR_TWPH(0x6) | \
					FTIM2_NOR_TWP(0xb))
#define CONFIG_SYS_NOR_FTIM3		0

#define CONFIG_FLASH_SHOW_PROGRESS	45	/* count down from 45/5: 9..1 */

#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE_PHYS }

#define CONFIG_SYS_WRITE_SWAPPED_DATA

#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3

/* NAND Flash Definitions */
#define CONFIG_SYS_NAND_BASE		0x68000000
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE) | \
				CSPR_PORT_SIZE_8 | \
				CSPR_TE | \
				CSPR_MSEL_NAND | \
				CSPR_V)
#define CONFIG_SYS_NAND_AMASK		IFC_AMASK(64 * 1024)
#define CONFIG_SYS_NAND_CSOR		(CSOR_NAND_ECC_ENC_EN \
					| CSOR_NAND_ECC_DEC_EN \
					| CSOR_NAND_ECC_MODE_4 \
					| CSOR_NAND_RAL_3 \
					| CSOR_NAND_PGS_2K \
					| CSOR_NAND_SPRZ_64 \
					| CSOR_NAND_PB(64) \
					| CSOR_NAND_TRHZ_40 \
					| CSOR_NAND_BCTLD)

#define CONFIG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x3) | \
					FTIM0_NAND_TWP(0x8) | \
					FTIM0_NAND_TWCHT(0x3) | \
					FTIM0_NAND_TWH(0x5))
#define CONFIG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x1e) | \
					FTIM1_NAND_TWBE(0x1e) | \
					FTIM1_NAND_TRR(0x6) | \
					FTIM1_NAND_TRP(0x8))
#define CONFIG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x9) | \
					FTIM2_NAND_TREH(0x5) | \
					FTIM2_NAND_TWHRE(0x3c))
#define CONFIG_SYS_NAND_FTIM3		(FTIM3_NAND_TWW(0x1e))

#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }

/* QRIO FPGA Definitions */
#define CONFIG_SYS_QRIO_BASE		0x70000000
#define CONFIG_SYS_QRIO_BASE_PHYS	CONFIG_SYS_QRIO_BASE

#define CONFIG_SYS_CSPR2_EXT		(0x00)
#define CONFIG_SYS_CSPR2	(CSPR_PHYS_ADDR(CONFIG_SYS_QRIO_BASE) | \
					CSPR_PORT_SIZE_8 | \
					CSPR_TE | \
					CSPR_MSEL_GPCM | \
					CSPR_V)
#define CONFIG_SYS_AMASK2		IFC_AMASK(64 * 1024)
#define CONFIG_SYS_CSOR2		(CSOR_GPCM_ADM_SHIFT(0x4) | \
					CSOR_GPCM_TRHZ_20 | \
					CSOR_GPCM_BCTLD)
#define CONFIG_SYS_CS2_FTIM0		(FTIM0_GPCM_TACSE(0x2) | \
					FTIM0_GPCM_TEADC(0x8) | \
					FTIM0_GPCM_TEAHC(0x2))
#define CONFIG_SYS_CS2_FTIM1		(FTIM1_GPCM_TACO(0x2) | \
					FTIM1_GPCM_TRAD(0x6))
#define CONFIG_SYS_CS2_FTIM2		(FTIM2_GPCM_TCS(0x1) | \
					FTIM2_GPCM_TCH(0x1) | \
					FTIM2_GPCM_TWP(0x7))
#define CONFIG_SYS_CS2_FTIM3		0x04000000

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

/*
 * I2C
 */

#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	3
#define I2C_MUX_PCA_ADDR		0x70
#define I2C_MUX_CH_DEFAULT		0x0
#define CONFIG_SYS_I2C_BUSES	{	{0, {I2C_NULL_HOP} }, \
					{0, {{I2C_MUX_PCA9547, 0x70, 1 } } }, \
					{1, {I2C_NULL_HOP}                 }, \
				}

#define CONFIG_SMP_PEN_ADDR		0x01ee0200

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		256
#define CONFIG_FSL_DEVICE_DISABLE

/*
 * Miscellaneous configurable options
 */

#define CONFIG_LS102XA_STREAM_ID

/*
 * Environment
 */

#define CONFIG_ENV_TOTAL_SIZE		0x40000
#define ENV_DEL_ADDR		CONFIG_ENV_ADDR_REDUND  /* direct for newenv */

#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV
#endif

#ifndef CONFIG_KM_DEF_BOOT_ARGS_CPU
#define CONFIG_KM_DEF_BOOT_ARGS_CPU		""
#endif

#define CONFIG_KM_DEF_ENV_CPU						\
	"boot=bootm ${load_addr_r} - ${fdt_addr_r}\0"			\
	"cramfsloadfdt="						\
		"cramfsload ${fdt_addr_r} "				\
		"fdt_0x${IVM_BoardId}_0x${IVM_HWKey}.dtb\0"		\
	"u-boot=" CONFIG_HOSTNAME "/u-boot.bin\0"			\
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
		" +" __stringify(CONFIG_SYS_MONITOR_LEN)"\0"		\
	"set_fdthigh=true\0"			\
	"checkfdt=true\0"						\
	""

#define CONFIG_KM_NEW_ENV						\
	"newenv=protect off " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) " && "		\
		"erase " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) " && "		\
		"protect on " __stringify(ENV_DEL_ADDR)			\
		" +" __stringify(CONFIG_ENV_TOTAL_SIZE) "\0"

#define CONFIG_HW_ENV_SETTINGS						\
	"hwconfig=devdis:esdhc,usb3,usb2,sata,sec,dcu,duart2,qspi,"	\
			"can1,can2_4,ftm2_8,i2c2_3,sai1_4,lpuart2_6,"	\
			"asrc,spdif,lpuart1,ftm1\0"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_KM_NEW_ENV						\
	CONFIG_KM_DEF_ENV						\
	CONFIG_HW_ENV_SETTINGS						\
	"EEprom_ivm=pca9547:70:9\0"					\
	"ethrotate=no\0"						\
	""

#define CONFIG_SYS_BOOTMAPSZ	(256 << 20) /* Increase map for Linux */

#endif
