/*
 * ddr_defs.h
 *
 * ddr specific header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DDR_DEFS_H
#define _DDR_DEFS_H

#include <asm/arch/hardware.h>
#include <asm/emif.h>

/* AM335X EMIF Register values */
#define VTP_CTRL_READY		(0x1 << 5)
#define VTP_CTRL_ENABLE		(0x1 << 6)
#define VTP_CTRL_START_EN	(0x1)
#define PHY_DLL_LOCK_DIFF	0x0
#define DDR_CKE_CTRL_NORMAL	0x1

#define DDR2_EMIF_READ_LATENCY	0x100005	/* Enable Dynamic Power Down */
#define DDR2_EMIF_TIM1		0x0666B3C9
#define DDR2_EMIF_TIM2		0x243631CA
#define DDR2_EMIF_TIM3		0x0000033F
#define DDR2_EMIF_SDCFG		0x41805332
#define DDR2_EMIF_SDREF		0x0000081a
#define DDR2_DLL_LOCK_DIFF	0x0
#define DDR2_RATIO		0x80
#define DDR2_INVERT_CLKOUT	0x00
#define DDR2_RD_DQS		0x12
#define DDR2_WR_DQS		0x00
#define DDR2_PHY_WRLVL		0x00
#define DDR2_PHY_GATELVL	0x00
#define DDR2_PHY_WR_DATA	0x40
#define DDR2_PHY_FIFO_WE	0x80
#define DDR2_PHY_RANK0_DELAY	0x1
#define DDR2_IOCTRL_VALUE	0x18B

/* Micron MT41J128M16JT-125 */
#define DDR3_EMIF_READ_LATENCY	0x06
#define DDR3_EMIF_TIM1		0x0888A39B
#define DDR3_EMIF_TIM2		0x26337FDA
#define DDR3_EMIF_TIM3		0x501F830F
#define DDR3_EMIF_SDCFG		0x61C04AB2
#define DDR3_EMIF_SDREF		0x0000093B
#define DDR3_ZQ_CFG		0x50074BE4
#define DDR3_DLL_LOCK_DIFF	0x1
#define DDR3_RATIO		0x40
#define DDR3_INVERT_CLKOUT	0x1
#define DDR3_RD_DQS		0x3B
#define DDR3_WR_DQS		0x85
#define DDR3_PHY_WR_DATA	0xC1
#define DDR3_PHY_FIFO_WE	0x100
#define DDR3_IOCTRL_VALUE	0x18B

/**
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs);

/**
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs);

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs);

/**
 * This structure represents the DDR registers on AM33XX devices.
 * We make use of DDR_PHY_BASE_ADDR2 to address the DATA1 registers that
 * correspond to DATA1 registers defined here.
 */
struct ddr_regs {
	unsigned int resv0[7];
	unsigned int cm0csratio;	/* offset 0x01C */
	unsigned int resv1[2];
	unsigned int cm0dldiff;		/* offset 0x028 */
	unsigned int cm0iclkout;	/* offset 0x02C */
	unsigned int resv2[8];
	unsigned int cm1csratio;	/* offset 0x050 */
	unsigned int resv3[2];
	unsigned int cm1dldiff;		/* offset 0x05C */
	unsigned int cm1iclkout;	/* offset 0x060 */
	unsigned int resv4[8];
	unsigned int cm2csratio;	/* offset 0x084 */
	unsigned int resv5[2];
	unsigned int cm2dldiff;		/* offset 0x090 */
	unsigned int cm2iclkout;	/* offset 0x094 */
	unsigned int resv6[12];
	unsigned int dt0rdsratio0;	/* offset 0x0C8 */
	unsigned int resv7[4];
	unsigned int dt0wdsratio0;	/* offset 0x0DC */
	unsigned int resv8[4];
	unsigned int dt0wiratio0;	/* offset 0x0F0 */
	unsigned int resv9;
	unsigned int dt0wimode0;	/* offset 0x0F8 */
	unsigned int dt0giratio0;	/* offset 0x0FC */
	unsigned int resv10;
	unsigned int dt0gimode0;	/* offset 0x104 */
	unsigned int dt0fwsratio0;	/* offset 0x108 */
	unsigned int resv11[4];
	unsigned int dt0dqoffset;	/* offset 0x11C */
	unsigned int dt0wrsratio0;	/* offset 0x120 */
	unsigned int resv12[4];
	unsigned int dt0rdelays0;	/* offset 0x134 */
	unsigned int dt0dldiff0;	/* offset 0x138 */
};

/**
 * Encapsulates DDR CMD control registers.
 */
struct cmd_control {
	unsigned long cmd0csratio;
	unsigned long cmd0csforce;
	unsigned long cmd0csdelay;
	unsigned long cmd0dldiff;
	unsigned long cmd0iclkout;
	unsigned long cmd1csratio;
	unsigned long cmd1csforce;
	unsigned long cmd1csdelay;
	unsigned long cmd1dldiff;
	unsigned long cmd1iclkout;
	unsigned long cmd2csratio;
	unsigned long cmd2csforce;
	unsigned long cmd2csdelay;
	unsigned long cmd2dldiff;
	unsigned long cmd2iclkout;
};

/**
 * Encapsulates DDR DATA registers.
 */
struct ddr_data {
	unsigned long datardsratio0;
	unsigned long datawdsratio0;
	unsigned long datawiratio0;
	unsigned long datagiratio0;
	unsigned long datafwsratio0;
	unsigned long datawrsratio0;
	unsigned long datauserank0delay;
	unsigned long datadldiff0;
};

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd);

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(int data_macrono, const struct ddr_data *data);

/**
 * This structure represents the DDR io control on AM33XX devices.
 */
struct ddr_cmdtctrl {
	unsigned int resv1[1];
	unsigned int cm0ioctl;
	unsigned int cm1ioctl;
	unsigned int cm2ioctl;
	unsigned int resv2[12];
	unsigned int dt0ioctl;
	unsigned int dt1ioctl;
};

/**
 * Configure DDR io control registers
 */
void config_io_ctrl(unsigned long val);

struct ddr_ctrl {
	unsigned int ddrioctrl;
	unsigned int resv1[325];
	unsigned int ddrckectrl;
};

void config_ddr(short ddr_type);

#endif  /* _DDR_DEFS_H */
