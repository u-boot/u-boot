/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_A38X_H
#define _DDR3_A38X_H

#define MAX_INTERFACE_NUM		1
#define MAX_BUS_NUM			5

#include "ddr3_hws_hw_training_def.h"

#define ECC_SUPPORT

/* right now, we're not supporting this in mainline */
#undef SUPPORT_STATIC_DUNIT_CONFIG

/* Controler bus divider 1 for 32 bit, 2 for 64 bit */
#define DDR_CONTROLLER_BUS_WIDTH_MULTIPLIER	1

/* Tune internal training params values */
#define TUNE_TRAINING_PARAMS_CK_DELAY		160
#define TUNE_TRAINING_PARAMS_CK_DELAY_16	160
#define TUNE_TRAINING_PARAMS_PFINGER		41
#define TUNE_TRAINING_PARAMS_NFINGER		43
#define TUNE_TRAINING_PARAMS_PHYREG3VAL		0xa

#define MARVELL_BOARD				MARVELL_BOARD_ID_BASE


#define REG_DEVICE_SAR1_ADDR			0xe4204
#define RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET	17
#define RST2_CPU_DDR_CLOCK_SELECT_IN_MASK	0x1f

/* DRAM Windows */
#define REG_XBAR_WIN_5_CTRL_ADDR		0x20050
#define REG_XBAR_WIN_5_BASE_ADDR		0x20054

/* DRAM Windows */
#define REG_XBAR_WIN_4_CTRL_ADDR                0x20040
#define REG_XBAR_WIN_4_BASE_ADDR                0x20044
#define REG_XBAR_WIN_4_REMAP_ADDR               0x20048
#define REG_XBAR_WIN_7_REMAP_ADDR               0x20078
#define REG_XBAR_WIN_16_CTRL_ADDR               0x200d0
#define REG_XBAR_WIN_16_BASE_ADDR               0x200d4
#define REG_XBAR_WIN_16_REMAP_ADDR              0x200dc
#define REG_XBAR_WIN_19_CTRL_ADDR               0x200e8

#define REG_FASTPATH_WIN_BASE_ADDR(win)         (0x20180 + (0x8 * win))
#define REG_FASTPATH_WIN_CTRL_ADDR(win)         (0x20184 + (0x8 * win))

/* SatR defined too change topology busWidth and ECC configuration */
#define DDR_SATR_CONFIG_MASK_WIDTH		0x8
#define DDR_SATR_CONFIG_MASK_ECC		0x10
#define DDR_SATR_CONFIG_MASK_ECC_PUP		0x20

#define	REG_SAMPLE_RESET_HIGH_ADDR		0x18600

#define MV_BOARD_REFCLK				MV_BOARD_REFCLK_25MHZ

/* Matrix enables DRAM modes (bus width/ECC) per boardId */
#define TOPOLOGY_UPDATE_32BIT			0
#define TOPOLOGY_UPDATE_32BIT_ECC		1
#define TOPOLOGY_UPDATE_16BIT			2
#define TOPOLOGY_UPDATE_16BIT_ECC		3
#define TOPOLOGY_UPDATE_16BIT_ECC_PUP3		4
#define TOPOLOGY_UPDATE { \
		/* 32Bit, 32bit ECC, 16bit, 16bit ECC PUP4, 16bit ECC PUP3 */ \
		{1, 1, 1, 1, 1},	/* RD_NAS_68XX_ID */ \
		{1, 1, 1, 1, 1},	/* DB_68XX_ID	  */ \
		{1, 0, 1, 0, 1},	/* RD_AP_68XX_ID  */ \
		{1, 0, 1, 0, 1},	/* DB_AP_68XX_ID  */ \
		{1, 0, 1, 0, 1},	/* DB_GP_68XX_ID  */ \
		{0, 0, 1, 1, 0},	/* DB_BP_6821_ID  */ \
		{1, 1, 1, 1, 1}		/* DB_AMC_6820_ID */ \
	};

enum {
	CPU_1066MHZ_DDR_400MHZ,
	CPU_RESERVED_DDR_RESERVED0,
	CPU_667MHZ_DDR_667MHZ,
	CPU_800MHZ_DDR_800MHZ,
	CPU_RESERVED_DDR_RESERVED1,
	CPU_RESERVED_DDR_RESERVED2,
	CPU_RESERVED_DDR_RESERVED3,
	LAST_FREQ
};

#define ACTIVE_INTERFACE_MASK			0x1

#endif /* _DDR3_A38X_H */
