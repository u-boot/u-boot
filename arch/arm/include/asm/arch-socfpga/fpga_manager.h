/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 */

#ifndef	_FPGA_MANAGER_H_
#define	_FPGA_MANAGER_H_

#include <altera.h>

struct socfpga_fpga_manager {
	/* FPGA Manager Module */
	u32	stat;			/* 0x00 */
	u32	ctrl;
	u32	dclkcnt;
	u32	dclkstat;
	u32	gpo;			/* 0x10 */
	u32	gpi;
	u32	misci;			/* 0x18 */
	u32	_pad_0x1c_0x82c[517];

	/* Configuration Monitor (MON) Registers */
	u32	gpio_inten;		/* 0x830 */
	u32	gpio_intmask;
	u32	gpio_inttype_level;
	u32	gpio_int_polarity;
	u32	gpio_intstatus;		/* 0x840 */
	u32	gpio_raw_intstatus;
	u32	_pad_0x848;
	u32	gpio_porta_eoi;
	u32	gpio_ext_porta;		/* 0x850 */
	u32	_pad_0x854_0x85c[3];
	u32	gpio_1s_sync;		/* 0x860 */
	u32	_pad_0x864_0x868[2];
	u32	gpio_ver_id_code;
	u32	gpio_config_reg2;	/* 0x870 */
	u32	gpio_config_reg1;
};

#define FPGAMGRREGS_STAT_MODE_MASK		0x7
#define FPGAMGRREGS_STAT_MSEL_MASK		0xf8
#define FPGAMGRREGS_STAT_MSEL_LSB		3

#define FPGAMGRREGS_CTRL_CFGWDTH_MASK		0x200
#define FPGAMGRREGS_CTRL_AXICFGEN_MASK		0x100
#define FPGAMGRREGS_CTRL_NCONFIGPULL_MASK	0x4
#define FPGAMGRREGS_CTRL_NCE_MASK		0x2
#define FPGAMGRREGS_CTRL_EN_MASK		0x1
#define FPGAMGRREGS_CTRL_CDRATIO_LSB		6

#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_CRC_MASK	0x8
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_ID_MASK	0x4
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_CD_MASK	0x2
#define FPGAMGRREGS_MON_GPIO_EXT_PORTA_NS_MASK	0x1

/* FPGA Mode */
#define FPGAMGRREGS_MODE_FPGAOFF		0x0
#define FPGAMGRREGS_MODE_RESETPHASE		0x1
#define FPGAMGRREGS_MODE_CFGPHASE		0x2
#define FPGAMGRREGS_MODE_INITPHASE		0x3
#define FPGAMGRREGS_MODE_USERMODE		0x4
#define FPGAMGRREGS_MODE_UNKNOWN		0x5

/* FPGA CD Ratio Value */
#define CDRATIO_x1				0x0
#define CDRATIO_x2				0x1
#define CDRATIO_x4				0x2
#define CDRATIO_x8				0x3

/* SoCFPGA support functions */
int fpgamgr_test_fpga_ready(void);
int fpgamgr_poll_fpga_ready(void);
int fpgamgr_get_mode(void);

#endif /* _FPGA_MANAGER_H_ */
