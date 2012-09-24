/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _GP_PADCTRL_H_
#define _GP_PADCTRL_H_

/* APB_MISC_GP and padctrl registers */
struct apb_misc_gp_ctlr {
	u32	modereg;	/* 0x00: APB_MISC_GP_MODEREG */
	u32	hidrev;		/* 0x04: APB_MISC_GP_HIDREV */
	u32	reserved0[22];	/* 0x08 - 0x5C: */
	u32	emu_revid;	/* 0x60: APB_MISC_GP_EMU_REVID */
	u32	xactor_scratch;	/* 0x64: APB_MISC_GP_XACTOR_SCRATCH */
	u32	aocfg1;		/* 0x68: APB_MISC_GP_AOCFG1PADCTRL */
	u32	aocfg2;		/* 0x6c: APB_MISC_GP_AOCFG2PADCTRL */
	u32	atcfg1;		/* 0x70: APB_MISC_GP_ATCFG1PADCTRL */
	u32	atcfg2;		/* 0x74: APB_MISC_GP_ATCFG2PADCTRL */
	u32	cdevcfg1;	/* 0x78: APB_MISC_GP_CDEV1CFGPADCTRL */
	u32	cdevcfg2;	/* 0x7C: APB_MISC_GP_CDEV2CFGPADCTRL */
	u32	csuscfg;	/* 0x80: APB_MISC_GP_CSUSCFGPADCTRL */
	u32	dap1cfg;	/* 0x84: APB_MISC_GP_DAP1CFGPADCTRL */
	u32	dap2cfg;	/* 0x88: APB_MISC_GP_DAP2CFGPADCTRL */
	u32	dap3cfg;	/* 0x8C: APB_MISC_GP_DAP3CFGPADCTRL */
	u32	dap4cfg;	/* 0x90: APB_MISC_GP_DAP4CFGPADCTRL */
	u32	dbgcfg;		/* 0x94: APB_MISC_GP_DBGCFGPADCTRL */
	u32	lcdcfg1;	/* 0x98: APB_MISC_GP_LCDCFG1PADCTRL */
	u32	lcdcfg2;	/* 0x9C: APB_MISC_GP_LCDCFG2PADCTRL */
	u32	sdmmc2_cfg;	/* 0xA0: APB_MISC_GP_SDMMC2CFGPADCTRL */
	u32	sdmmc3_cfg;	/* 0xA4: APB_MISC_GP_SDMMC3CFGPADCTRL */
	u32	spicfg;		/* 0xA8: APB_MISC_GP_SPICFGPADCTRL */
	u32	uaacfg;		/* 0xAC: APB_MISC_GP_UAACFGPADCTRL */
	u32	uabcfg;		/* 0xB0: APB_MISC_GP_UABCFGPADCTRL */
	u32	uart2cfg;	/* 0xB4: APB_MISC_GP_UART2CFGPADCTRL */
	u32	uart3cfg;	/* 0xB8: APB_MISC_GP_UART3CFGPADCTRL */
	u32	vicfg1;		/* 0xBC: APB_MISC_GP_VICFG1PADCTRL */
	u32	vicfg2;		/* 0xC0: APB_MISC_GP_VICFG2PADCTRL */
	u32	xm2cfga;	/* 0xC4: APB_MISC_GP_XM2CFGAPADCTRL */
	u32	xm2cfgc;	/* 0xC8: APB_MISC_GP_XM2CFGCPADCTRL */
	u32	xm2cfgd;	/* 0xCC: APB_MISC_GP_XM2CFGDPADCTRL */
	u32	xm2clkcfg;	/* 0xD0: APB_MISC_GP_XM2CLKCFGPADCTRL */
	u32	memcomp;	/* 0xD4: APB_MISC_GP_MEMCOMPPADCTRL */
};

/* bit fields definitions for APB_MISC_GP_HIDREV register */
#define HIDREV_CHIPID_SHIFT		8
#define HIDREV_CHIPID_MASK		(0xff << HIDREV_CHIPID_SHIFT)
#define HIDREV_MAJORPREV_SHIFT		4
#define HIDREV_MAJORPREV_MASK		(0xf << HIDREV_MAJORPREV_SHIFT)

/* CHIPID field returned from APB_MISC_GP_HIDREV register */
#define CHIPID_TEGRA20				0x20

#endif
