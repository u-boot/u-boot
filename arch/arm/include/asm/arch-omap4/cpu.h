/*
 * (C) Copyright 2006-2010
 * Texas Instruments, <www.ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _CPU_H
#define _CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct gpmc_cs {
	u32 config1;		/* 0x00 */
	u32 config2;		/* 0x04 */
	u32 config3;		/* 0x08 */
	u32 config4;		/* 0x0C */
	u32 config5;		/* 0x10 */
	u32 config6;		/* 0x14 */
	u32 config7;		/* 0x18 */
	u32 nand_cmd;		/* 0x1C */
	u32 nand_adr;		/* 0x20 */
	u32 nand_dat;		/* 0x24 */
	u8 res[8];		/* blow up to 0x30 byte */
};

struct gpmc {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u8 res2[0x4];
	u32 irqstatus;		/* 0x18 */
	u32 irqenable;		/* 0x1C */
	u8 res3[0x20];
	u32 timeout_control;	/* 0x40 */
	u8 res4[0xC];
	u32 config;		/* 0x50 */
	u32 status;		/* 0x54 */
	u8 res5[0x8];	/* 0x58 */
	struct gpmc_cs cs[8];	/* 0x60, 0x90, .. */
	u8 res6[0x14];		/* 0x1E0 */
	u32 ecc_config;		/* 0x1F4 */
	u32 ecc_control;	/* 0x1F8 */
	u32 ecc_size_config;	/* 0x1FC */
	u32 ecc1_result;	/* 0x200 */
	u32 ecc2_result;	/* 0x204 */
	u32 ecc3_result;	/* 0x208 */
	u32 ecc4_result;	/* 0x20C */
	u32 ecc5_result;	/* 0x210 */
	u32 ecc6_result;	/* 0x214 */
	u32 ecc7_result;	/* 0x218 */
	u32 ecc8_result;	/* 0x21C */
	u32 ecc9_result;	/* 0x220 */
};

/* Used for board specific gpmc initialization */
extern struct gpmc *gpmc_cfg;

struct gptimer {
	u32 tidr;		/* 0x00 r */
	u8 res[0xc];
	u32 tiocp_cfg;		/* 0x10 rw */
	u32 tistat;		/* 0x14 r */
	u32 tisr;		/* 0x18 rw */
	u32 tier;		/* 0x1c rw */
	u32 twer;		/* 0x20 rw */
	u32 tclr;		/* 0x24 rw */
	u32 tcrr;		/* 0x28 rw */
	u32 tldr;		/* 0x2c rw */
	u32 ttgr;		/* 0x30 rw */
	u32 twpc;		/* 0x34 r */
	u32 tmar;		/* 0x38 rw */
	u32 tcar1;		/* 0x3c r */
	u32 tcicr;		/* 0x40 rw */
	u32 tcar2;		/* 0x44 r */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* enable sys_clk NO-prescale /1 */
#define GPT_EN			((0x0 << 2) | (0x1 << 1) | (0x1 << 0))

/* Watchdog */
#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct watchdog {
	u8 res1[0x34];
	u32 wwps;		/* 0x34 r */
	u8 res2[0x10];
	u32 wspr;		/* 0x48 rw */
};
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

#define WD_UNLOCK1		0xAAAA
#define WD_UNLOCK2		0x5555

#define SYSCLKDIV_1		(0x1 << 6)
#define SYSCLKDIV_2		(0x1 << 7)

#define CLKSEL_GPT1		(0x1 << 0)

#define EN_GPT1			(0x1 << 0)
#define EN_32KSYNC		(0x1 << 2)

#define ST_WDT2			(0x1 << 5)

#define RESETDONE		(0x1 << 0)

#define TCLR_ST			(0x1 << 0)
#define TCLR_AR			(0x1 << 1)
#define TCLR_PRE		(0x1 << 5)

/* GPMC BASE */
#define GPMC_BASE		(OMAP44XX_GPMC_BASE)

/* I2C base */
#define I2C_BASE1		(OMAP44XX_L4_PER_BASE + 0x70000)
#define I2C_BASE2		(OMAP44XX_L4_PER_BASE + 0x72000)
#define I2C_BASE3		(OMAP44XX_L4_PER_BASE + 0x60000)

/* MUSB base */
#define MUSB_BASE		(OMAP44XX_L4_CORE_BASE + 0xAB000)

/* OMAP4 GPIO registers */
#define OMAP_GPIO_REVISION		0x0000
#define OMAP_GPIO_SYSCONFIG		0x0010
#define OMAP_GPIO_SYSSTATUS		0x0114
#define OMAP_GPIO_IRQSTATUS1		0x0118
#define OMAP_GPIO_IRQSTATUS2		0x0128
#define OMAP_GPIO_IRQENABLE2		0x012c
#define OMAP_GPIO_IRQENABLE1		0x011c
#define OMAP_GPIO_WAKE_EN		0x0120
#define OMAP_GPIO_CTRL			0x0130
#define OMAP_GPIO_OE			0x0134
#define OMAP_GPIO_DATAIN		0x0138
#define OMAP_GPIO_DATAOUT		0x013c
#define OMAP_GPIO_LEVELDETECT0		0x0140
#define OMAP_GPIO_LEVELDETECT1		0x0144
#define OMAP_GPIO_RISINGDETECT		0x0148
#define OMAP_GPIO_FALLINGDETECT		0x014c
#define OMAP_GPIO_DEBOUNCE_EN		0x0150
#define OMAP_GPIO_DEBOUNCE_VAL		0x0154
#define OMAP_GPIO_CLEARIRQENABLE1	0x0160
#define OMAP_GPIO_SETIRQENABLE1		0x0164
#define OMAP_GPIO_CLEARWKUENA		0x0180
#define OMAP_GPIO_SETWKUENA		0x0184
#define OMAP_GPIO_CLEARDATAOUT		0x0190
#define OMAP_GPIO_SETDATAOUT		0x0194

/*
 * PRCM
 */

/* PRM */
#define PRM_BASE		0x4A306000
#define PRM_DEVICE_BASE		(PRM_BASE + 0x1B00)

#define PRM_RSTCTRL		PRM_DEVICE_BASE
#define PRM_RSTCTRL_RESET	0x01
#define PRM_RSTST		(PRM_DEVICE_BASE + 0x4)
#define PRM_RSTST_WARM_RESET_MASK	0x07EA

#endif /* _CPU_H */
