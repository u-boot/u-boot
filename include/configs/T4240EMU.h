/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * T4240 EMU board configuration file
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_T4240EMU
#define CONFIG_PHYS_64BIT

#define CONFIG_SYS_NO_FLASH		1
#define CONFIG_SYS_FSL_DDR_EMU		1
#define CONFIG_SYS_FSL_NO_QIXIS		1
#define CONFIG_SYS_FSL_NO_SERDES	1

#include "t4qds.h"

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CACHE_FLUSH

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE         0x2000

#define CONFIG_SYS_CLK_FREQ	100000000
#define CONFIG_DDR_CLK_FREQ	133333333
#define CONFIG_FSL_TBCLK_EXTRA_DIV 100

/*
 * DDR Setup
 */
#define CONFIG_SYS_SPD_BUS_NUM	1
#define SPD_EEPROM_ADDRESS1	0x51
#define SPD_EEPROM_ADDRESS2	0x52
#define SPD_EEPROM_ADDRESS3	0x53
#define SPD_EEPROM_ADDRESS4	0x54
#define SPD_EEPROM_ADDRESS5	0x55
#define SPD_EEPROM_ADDRESS6	0x56
#define SPD_EEPROM_ADDRESS	SPD_EEPROM_ADDRESS1	/* for p3041/p5010 */
#define CONFIG_SYS_SDRAM_SIZE	4096	/* for fixed parameter use */

/*
 * IFC Definitions
 */
#define CONFIG_SYS_NOR0_CSPR_EXT	(0xf)
#define CONFIG_SYS_NOR_AMASK	IFC_AMASK(128*1024*1024)
/* NOR Flash Timing Params */
#define CONFIG_SYS_NOR0_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS \
				+ 0x8000000) | \
				CSPR_PORT_SIZE_32 | \
				CSPR_MSEL_NOR | \
				CSPR_V)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(0)
#define CONFIG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x1) | \
				FTIM0_NOR_TEADC(0x1) | \
				FTIM0_NOR_TEAHC(0x1))
#define CONFIG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x1) | \
				FTIM1_NOR_TRAD_NOR(0x1))
#define CONFIG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x0) | \
				FTIM2_NOR_TCH(0x0) | \
				FTIM2_NOR_TWP(0x1))
#define CONFIG_SYS_NOR_FTIM3	0x04000000
#define CONFIG_SYS_IFC_CCR	0x01000000

#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NOR0_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NOR0_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NOR_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NOR_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NOR_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NOR_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NOR_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NOR_FTIM3

/* I2C */
#define CONFIG_SYS_FSL_I2C_SPEED	4000000	/* faster speed for emulator */
#define CONFIG_SYS_FSL_I2C2_SPEED	4000000

/* Qman/Bman */
#define CONFIG_SYS_DPAA_QBMAN		/* Support Q/Bman */
#define CONFIG_SYS_BMAN_NUM_PORTALS	50
#define CONFIG_SYS_BMAN_MEM_BASE	0xf4000000
#define CONFIG_SYS_BMAN_MEM_PHYS	0xff4000000ull
#define CONFIG_SYS_BMAN_MEM_SIZE	0x02000000
#define CONFIG_SYS_QMAN_NUM_PORTALS	50
#define CONFIG_SYS_QMAN_MEM_BASE	0xf6000000
#define CONFIG_SYS_QMAN_MEM_PHYS	0xff6000000ull
#define CONFIG_SYS_QMAN_MEM_SIZE	0x02000000

#define CONFIG_SYS_DPAA_FMAN
#define CONFIG_SYS_DPAA_PME
#define CONFIG_SYS_PMAN
#define CONFIG_SYS_DPAA_DCE
#define CONFIG_SYS_DPAA_RMAN
#define CONFIG_SYS_INTERLAKEN

#define CONFIG_SYS_QE_FMAN_FW_IN_NOR
#define CONFIG_SYS_QE_FMAN_FW_ADDR		0xEFF40000
#define CONFIG_SYS_QE_FMAN_FW_LENGTH	0x10000
#define CONFIG_SYS_FDT_PAD		(0x3000 + CONFIG_SYS_QE_FMAN_FW_LENGTH)

#define CONFIG_BOOTDELAY	0

/*
 * T4240 has 3 DDR controllers. Default to 3-way interleaving. It can be
 * 3way_1KB, 3way_4KB, 3way_8KB. T4160 has 2 DDR controllers. Default to 2-way
 * interleaving. It can be cacheline, page, bank, superbank.
 * See doc/README.fsl-ddr for details.
 */
#ifdef CONFIG_PPC_T4240
#define CTRL_INTLV_PREFERED 3way_4KB
#else
#define CTRL_INTLV_PREFERED cacheline
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"hwconfig=fsl_ddr:"					\
	"ctlr_intlv=" __stringify(CTRL_INTLV_PREFERED) ","	\
	"bank_intlv=auto;"					\
	"netdev=eth0\0"						\
	"uboot=" __stringify(CONFIG_UBOOTPATH) "\0"			\
	"ubootaddr=" __stringify(CONFIG_SYS_TEXT_BASE) "\0"		\
	"consoledev=ttyS0\0"					\
	"ramdiskaddr=2000000\0"					\
	"ramdiskfile=t4240emu/ramdisk.uboot\0"			\
	"fdtaddr=c00000\0"					\
	"fdtfile=t4240emu/t4240emu.dtb\0"				\
	"bdev=sda3\0"						\
	"c=ffe\0"

/*
 * For emulation this causes u-boot to jump to the start of the proof point
 * app code automatically
 */
#define CONFIG_PROOF_POINTS			\
	"setenv bootargs root=/dev/$bdev rw "		\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"cpu 1 release 0x29000000 - - -;"		\
	"cpu 2 release 0x29000000 - - -;"		\
	"cpu 3 release 0x29000000 - - -;"		\
	"cpu 4 release 0x29000000 - - -;"		\
	"cpu 5 release 0x29000000 - - -;"		\
	"cpu 6 release 0x29000000 - - -;"		\
	"cpu 7 release 0x29000000 - - -;"		\
	"go 0x29000000"

#define CONFIG_HVBOOT				\
	"setenv bootargs config-addr=0x60000000; "	\
	"bootm 0x01000000 - 0x00f00000"

#define CONFIG_LINUX					\
	"errata;"					\
	"setenv othbootargs ignore_loglevel;"		\
	"setenv bootargs root=/dev/ram rw "		\
	"console=$consoledev,$baudrate $othbootargs;"	\
	"setenv ramdiskaddr 0x02000000;"		\
	"setenv fdtaddr 0x00c00000;"			\
	"setenv loadaddr 0x1000000;"			\
	"bootm $loadaddr $ramdiskaddr $fdtaddr"

#define CONFIG_BOOTCOMMAND		CONFIG_LINUX

#endif	/* __CONFIG_H */
