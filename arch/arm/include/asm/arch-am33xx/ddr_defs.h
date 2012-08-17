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

/* AM335X EMIF Register values */
#define EMIF_SDMGT		0x80000000
#define EMIF_SDRAM		0x00004650
#define EMIF_PHYCFG		0x2
#define DDR_PHY_RESET		(0x1 << 10)
#define DDR_FUNCTIONAL_MODE_EN	0x1
#define DDR_PHY_READY		(0x1 << 2)
#define VTP_CTRL_READY		(0x1 << 5)
#define VTP_CTRL_ENABLE		(0x1 << 6)
#define VTP_CTRL_LOCK_EN	(0x1 << 4)
#define VTP_CTRL_START_EN	(0x1)
#define DDR2_RATIO		0x80
#define CMD_FORCE		0x00
#define CMD_DELAY		0x00

#define EMIF_READ_LATENCY	0x05
#define EMIF_TIM1		0x0666B3D6
#define EMIF_TIM2		0x143731DA
#define EMIF_TIM3		0x00000347
#define EMIF_SDCFG		0x43805332
#define EMIF_SDREF		0x0000081a
#define DDR2_DLL_LOCK_DIFF	0x0
#define DDR2_RD_DQS		0x12
#define DDR2_PHY_FIFO_WE	0x80

#define DDR2_INVERT_CLKOUT	0x00
#define DDR2_WR_DQS		0x00
#define DDR2_PHY_WRLVL		0x00
#define DDR2_PHY_GATELVL	0x00
#define DDR2_PHY_WR_DATA	0x40
#define PHY_RANK0_DELAY		0x01
#define PHY_DLL_LOCK_DIFF	0x0
#define DDR_IOCTRL_VALUE	0x18B

/**
 * This structure represents the EMIF registers on AM33XX devices.
 */
struct emif_regs {
	unsigned int sdrrev;		/* offset 0x00 */
	unsigned int sdrstat;		/* offset 0x04 */
	unsigned int sdrcr;		/* offset 0x08 */
	unsigned int sdrcr2;		/* offset 0x0C */
	unsigned int sdrrcr;		/* offset 0x10 */
	unsigned int sdrrcsr;		/* offset 0x14 */
	unsigned int sdrtim1;		/* offset 0x18 */
	unsigned int sdrtim1sr;		/* offset 0x1C */
	unsigned int sdrtim2;		/* offset 0x20 */
	unsigned int sdrtim2sr;		/* offset 0x24 */
	unsigned int sdrtim3;		/* offset 0x28 */
	unsigned int sdrtim3sr;		/* offset 0x2C */
	unsigned int res1[2];
	unsigned int sdrmcr;		/* offset 0x38 */
	unsigned int sdrmcsr;		/* offset 0x3C */
	unsigned int res2[8];
	unsigned int sdritr;		/* offset 0x60 */
	unsigned int res3[32];
	unsigned int ddrphycr;		/* offset 0xE4 */
	unsigned int ddrphycsr;		/* offset 0xE8 */
	unsigned int ddrphycr2;		/* offset 0xEC */
};

/**
 * Encapsulates DDR PHY control and corresponding shadow registers.
 */
struct ddr_phy_control {
	unsigned long	reg;
	unsigned long	reg_sh;
	unsigned long	reg2;
};

/**
 * Encapsulates SDRAM timing and corresponding shadow registers.
 */
struct sdram_timing {
	unsigned long	time1;
	unsigned long	time1_sh;
	unsigned long	time2;
	unsigned long	time2_sh;
	unsigned long	time3;
	unsigned long	time3_sh;
};

/**
 * Encapsulates SDRAM configuration.
 * (Includes refresh control registers)  */
struct sdram_config {
	unsigned long	sdrcr;
	unsigned long	sdrcr2;
	unsigned long	refresh;
	unsigned long	refresh_sh;
};

/**
 * Configure SDRAM
 */
int config_sdram(struct sdram_config *cfg);

/**
 * Set SDRAM timings
 */
int set_sdram_timings(struct sdram_timing *val);

/**
 * Configure DDR PHY
 */
int config_ddr_phy(struct ddr_phy_control *cfg);

/**
 * This structure represents the DDR registers on AM33XX devices.
 */
struct ddr_regs {
	unsigned int resv0[7];
	unsigned int cm0csratio;	/* offset 0x01C */
	unsigned int cm0csforce;	/* offset 0x020 */
	unsigned int cm0csdelay;	/* offset 0x024 */
	unsigned int cm0dldiff;		/* offset 0x028 */
	unsigned int cm0iclkout;	/* offset 0x02C */
	unsigned int resv1[8];
	unsigned int cm1csratio;	/* offset 0x050 */
	unsigned int cm1csforce;	/* offset 0x054 */
	unsigned int cm1csdelay;	/* offset 0x058 */
	unsigned int cm1dldiff;		/* offset 0x05C */
	unsigned int cm1iclkout;	/* offset 0x060 */
	unsigned int resv2[8];
	unsigned int cm2csratio;	/* offset 0x084 */
	unsigned int cm2csforce;	/* offset 0x088 */
	unsigned int cm2csdelay;	/* offset 0x08C */
	unsigned int cm2dldiff;		/* offset 0x090 */
	unsigned int cm2iclkout;	/* offset 0x094 */
	unsigned int resv3[12];
	unsigned int dt0rdsratio0;	/* offset 0x0C8 */
	unsigned int dt0rdsratio1;	/* offset 0x0CC */
	unsigned int resv4[3];
	unsigned int dt0wdsratio0;	/* offset 0x0DC */
	unsigned int dt0wdsratio1;	/* offset 0x0E0 */
	unsigned int resv5[3];
	unsigned int dt0wiratio0;	/* offset 0x0F0 */
	unsigned int dt0wiratio1;	/* offset 0x0F4 */
	unsigned int dt0giratio0;	/* offset 0x0FC */
	unsigned int dt0giratio1;	/* offset 0x100 */
	unsigned int resv6[1];
	unsigned int dt0fwsratio0;	/* offset 0x108 */
	unsigned int dt0fwsratio1;	/* offset 0x10C */
	unsigned int resv7[4];
	unsigned int dt0wrsratio0;	/* offset 0x120 */
	unsigned int dt0wrsratio1;	/* offset 0x124 */
	unsigned int resv8[3];
	unsigned int dt0rdelays0;	/* offset 0x134 */
	unsigned int dt0dldiff0;	/* offset 0x138 */
	unsigned int resv9[39];
	unsigned int dt1rdelays0;	/* offset 0x1D8 */
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
	unsigned long datardsratio1;
	unsigned long datawdsratio0;
	unsigned long datawdsratio1;
	unsigned long datawiratio0;
	unsigned long datawiratio1;
	unsigned long datagiratio0;
	unsigned long datagiratio1;
	unsigned long datafwsratio0;
	unsigned long datafwsratio1;
	unsigned long datawrsratio0;
	unsigned long datawrsratio1;
	unsigned long datadldiff0;
};

/**
 * Configure DDR CMD control registers
 */
int config_cmd_ctrl(struct cmd_control *cmd);

/**
 * Configure DDR DATA registers
 */
int config_ddr_data(int data_macrono, struct ddr_data *data);

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
 * Encapsulates DDR CMD & DATA io control registers.
 */
struct ddr_ioctrl {
	unsigned long cmd1ctl;
	unsigned long cmd2ctl;
	unsigned long cmd3ctl;
	unsigned long data1ctl;
	unsigned long data2ctl;
};

/**
 * Configure DDR io control registers
 */
int config_io_ctrl(struct ddr_ioctrl *ioctrl);

struct ddr_ctrl {
	unsigned int ddrioctrl;
	unsigned int resv1[325];
	unsigned int ddrckectrl;
};

void config_ddr(void);

#endif  /* _DDR_DEFS_H */
