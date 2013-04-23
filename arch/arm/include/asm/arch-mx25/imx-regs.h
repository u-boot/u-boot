/*
 * Copyright (C) 2009, DENX Software Engineering
 * Author: John Rigby <jcrigby@gmail.com
 *
 *   Based on arch-mx31/imx-regs.h
 *	Copyright (C) 2009 Ilya Yanok,
 *		Emcraft Systems <yanok@emcraft.com>
 *   and arch-mx27/imx-regs.h
 *	Copyright (C) 2007 Pengutronix,
 *		Sascha Hauer <s.hauer@pengutronix.de>
 *	Copyright (C) 2009 Ilya Yanok,
 *		Emcraft Systems <yanok@emcraft.com>
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

#ifndef _IMX_REGS_H
#define _IMX_REGS_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* Clock Control Module (CCM) registers */
struct ccm_regs {
	u32 mpctl;	/* Core PLL Control */
	u32 upctl;	/* USB PLL Control */
	u32 cctl;	/* Clock Control */
	u32 cgr0;	/* Clock Gating Control 0 */
	u32 cgr1;	/* Clock Gating Control 1 */
	u32 cgr2;	/* Clock Gating Control 2 */
	u32 pcdr[4];	/* PER Clock Dividers */
	u32 rcsr;	/* CCM Status */
	u32 crdr;	/* CCM Reset and Debug */
	u32 dcvr0;	/* DPTC Comparator Value 0 */
	u32 dcvr1;	/* DPTC Comparator Value 1 */
	u32 dcvr2;	/* DPTC Comparator Value 2 */
	u32 dcvr3;	/* DPTC Comparator Value 3 */
	u32 ltr0;	/* Load Tracking 0 */
	u32 ltr1;	/* Load Tracking 1 */
	u32 ltr2;	/* Load Tracking 2 */
	u32 ltr3;	/* Load Tracking 3 */
	u32 ltbr0;	/* Load Tracking Buffer 0 */
	u32 ltbr1;	/* Load Tracking Buffer 1 */
	u32 pcmr0;	/* Power Management Control 0 */
	u32 pcmr1;	/* Power Management Control 1 */
	u32 pcmr2;	/* Power Management Control 2 */
	u32 mcr;	/* Miscellaneous Control */
	u32 lpimr0;	/* Low Power Interrupt Mask 0 */
	u32 lpimr1;	/* Low Power Interrupt Mask 1 */
};

/* Enhanced SDRAM Controller (ESDRAMC) registers */
struct esdramc_regs {
	u32 ctl0; 	/* control 0 */
	u32 cfg0; 	/* configuration 0 */
	u32 ctl1; 	/* control 1 */
	u32 cfg1; 	/* configuration 1 */
	u32 misc; 	/* miscellaneous */
	u32 pad[3];
	u32 cdly1;	/* Delay Line 1 configuration debug */
	u32 cdly2;	/* delay line 2 configuration debug */
	u32 cdly3;	/* delay line 3 configuration debug */
	u32 cdly4;	/* delay line 4 configuration debug */
	u32 cdly5;	/* delay line 5 configuration debug */
	u32 cdlyl;	/* delay line cycle length debug */
};

/* General Purpose Timer (GPT) registers */
struct gpt_regs {
	u32 ctrl;   	/* control */
	u32 pre;    	/* prescaler */
	u32 stat;   	/* status */
	u32 intr;   	/* interrupt */
	u32 cmp[3]; 	/* output compare 1-3 */
	u32 capt[2];	/* input capture 1-2 */
	u32 counter;	/* counter */
};

/* Watchdog Timer (WDOG) registers */
struct wdog_regs {
	u16 wcr;	/* Control */
	u16 wsr;	/* Service */
	u16 wrsr;	/* Reset Status */
	u16 wicr;	/* Interrupt Control */
	u16 wmcr;	/* Misc Control */
};

/* IIM control registers */
struct iim_regs {
	u32 iim_stat;
	u32 iim_statm;
	u32 iim_err;
	u32 iim_emask;
	u32 iim_fctl;
	u32 iim_ua;
	u32 iim_la;
	u32 iim_sdat;
	u32 iim_prev;
	u32 iim_srev;
	u32 iim_prg_p;
	u32 iim_scs0;
	u32 iim_scs1;
	u32 iim_scs2;
	u32 iim_scs3;
	u32 res1[0x1f1];
	struct fuse_bank {
		u32 fuse_regs[0x20];
		u32 fuse_rsvd[0xe0];
	} bank[3];
};

struct fuse_bank0_regs {
	u32 fuse0_7[8];
	u32 uid[8];
	u32 fuse16_25[0xa];
	u32 mac_addr[6];
};

struct fuse_bank1_regs {
	u32 fuse0_21[0x16];
	u32 usr5;
	u32 fuse23_29[7];
	u32 usr6[2];
};

/* Multi-Layer AHB Crossbar Switch (MAX) registers */
struct max_regs {
	u32 mpr0;
	u32 pad00[3];
	u32 sgpcr0;
	u32 pad01[59];
	u32 mpr1;
	u32 pad02[3];
	u32 sgpcr1;
	u32 pad03[59];
	u32 mpr2;
	u32 pad04[3];
	u32 sgpcr2;
	u32 pad05[59];
	u32 mpr3;
	u32 pad06[3];
	u32 sgpcr3;
	u32 pad07[59];
	u32 mpr4;
	u32 pad08[3];
	u32 sgpcr4;
	u32 pad09[251];
	u32 mgpcr0;
	u32 pad10[63];
	u32 mgpcr1;
	u32 pad11[63];
	u32 mgpcr2;
	u32 pad12[63];
	u32 mgpcr3;
	u32 pad13[63];
	u32 mgpcr4;
};

/* AHB <-> IP-Bus Interface (AIPS) */
struct aips_regs {
	u32 mpr_0_7;
	u32 mpr_8_15;
};

#endif

#define ARCH_MXC

/* AIPS 1 */
#define IMX_AIPS1_BASE		(0x43F00000)
#define IMX_MAX_BASE		(0x43F04000)
#define IMX_CLKCTL_BASE		(0x43F08000)
#define IMX_ETB_SLOT4_BASE	(0x43F0C000)
#define IMX_ETB_SLOT5_BASE	(0x43F10000)
#define IMX_ECT_CTIO_BASE	(0x43F18000)
#define IMX_I2C_BASE		(0x43F80000)
#define IMX_I2C3_BASE		(0x43F84000)
#define IMX_CAN1_BASE		(0x43F88000)
#define IMX_CAN2_BASE		(0x43F8C000)
#define UART1_BASE		(0x43F90000)
#define UART2_BASE		(0x43F94000)
#define IMX_I2C2_BASE		(0x43F98000)
#define IMX_OWIRE_BASE		(0x43F9C000)
#define IMX_CSPI1_BASE		(0x43FA4000)
#define IMX_KPP_BASE		(0x43FA8000)
#define IMX_IOPADMUX_BASE	(0x43FAC000)
#define IMX_IOPADCTL_BASE	(0x43FAC22C)
#define IMX_IOPADGRPCTL_BASE	(0x43FAC418)
#define IMX_IOPADINPUTSEL_BASE	(0x43FAC460)
#define IMX_AUDMUX_BASE		(0x43FB0000)
#define IMX_ECT_IP1_BASE	(0x43FB8000)
#define IMX_ECT_IP2_BASE	(0x43FBC000)

/* SPBA */
#define IMX_SPBA_BASE		(0x50000000)
#define IMX_CSPI3_BASE		(0x50004000)
#define UART4_BASE		(0x50008000)
#define UART3_BASE		(0x5000C000)
#define IMX_CSPI2_BASE		(0x50010000)
#define IMX_SSI2_BASE		(0x50014000)
#define IMX_ESAI_BASE		(0x50018000)
#define IMX_ATA_DMA_BASE	(0x50020000)
#define IMX_SIM1_BASE		(0x50024000)
#define IMX_SIM2_BASE		(0x50028000)
#define UART5_BASE		(0x5002C000)
#define IMX_TSC_BASE		(0x50030000)
#define IMX_SSI1_BASE		(0x50034000)
#define IMX_FEC_BASE		(0x50038000)
#define IMX_SPBA_CTRL_BASE	(0x5003C000)

/* AIPS 2 */
#define IMX_AIPS2_BASE		(0x53F00000)
#define IMX_CCM_BASE		(0x53F80000)
#define IMX_GPT4_BASE		(0x53F84000)
#define IMX_GPT3_BASE		(0x53F88000)
#define IMX_GPT2_BASE		(0x53F8C000)
#define IMX_GPT1_BASE		(0x53F90000)
#define IMX_EPIT1_BASE		(0x53F94000)
#define IMX_EPIT2_BASE		(0x53F98000)
#define IMX_GPIO4_BASE		(0x53F9C000)
#define IMX_PWM2_BASE		(0x53FA0000)
#define IMX_GPIO3_BASE		(0x53FA4000)
#define IMX_PWM3_BASE		(0x53FA8000)
#define IMX_SCC_BASE		(0x53FAC000)
#define IMX_SCM_BASE		(0x53FAE000)
#define IMX_SMN_BASE		(0x53FAF000)
#define IMX_RNGD_BASE		(0x53FB0000)
#define IMX_MMC_SDHC1_BASE	(0x53FB4000)
#define IMX_MMC_SDHC2_BASE	(0x53FB8000)
#define IMX_LCDC_BASE		(0x53FBC000)
#define IMX_SLCDC_BASE		(0x53FC0000)
#define IMX_PWM4_BASE		(0x53FC8000)
#define IMX_GPIO1_BASE		(0x53FCC000)
#define IMX_GPIO2_BASE		(0x53FD0000)
#define IMX_SDMA_BASE		(0x53FD4000)
#define IMX_WDT_BASE		(0x53FDC000)
#define IMX_PWM1_BASE		(0x53FE0000)
#define IMX_RTIC_BASE		(0x53FEC000)
#define IMX_IIM_BASE		(0x53FF0000)
#define IIM_BASE_ADDR		IMX_IIM_BASE
#define IMX_USB_BASE		(0x53FF4000)
#define IMX_USB_PORT_OFFSET	0x200
#define IMX_CSI_BASE		(0x53FF8000)
#define IMX_DRYICE_BASE		(0x53FFC000)

#define IMX_ARM926_ROMPATCH	(0x60000000)
#define IMX_ARM926_ASIC		(0x68000000)

/* 128K Internal Static RAM */
#define IMX_RAM_BASE		(0x78000000)
#define IMX_RAM_SIZE		(128 * 1024)

/* SDRAM BANKS */
#define IMX_SDRAM_BANK0_BASE	(0x80000000)
#define IMX_SDRAM_BANK1_BASE	(0x90000000)

#define IMX_WEIM_CS0		(0xA0000000)
#define IMX_WEIM_CS1		(0xA8000000)
#define IMX_WEIM_CS2		(0xB0000000)
#define IMX_WEIM_CS3		(0xB2000000)
#define IMX_WEIM_CS4		(0xB4000000)
#define IMX_ESDRAMC_BASE	(0xB8001000)
#define IMX_WEIM_CTRL_BASE	(0xB8002000)
#define IMX_M3IF_CTRL_BASE	(0xB8003000)
#define IMX_EMI_CTRL_BASE	(0xB8004000)

/* NAND Flash Controller */
#define IMX_NFC_BASE		(0xBB000000)
#define NFC_BASE_ADDR		IMX_NFC_BASE

/* CCM bitfields */
#define CCM_PLL_MFI_SHIFT	10
#define CCM_PLL_MFI_MASK	0xf
#define CCM_PLL_MFN_SHIFT	0
#define CCM_PLL_MFN_MASK	0x3ff
#define CCM_PLL_MFD_SHIFT	16
#define CCM_PLL_MFD_MASK	0x3ff
#define CCM_PLL_PD_SHIFT	26
#define CCM_PLL_PD_MASK		0xf
#define CCM_CCTL_ARM_DIV_SHIFT	30
#define CCM_CCTL_ARM_DIV_MASK	3
#define CCM_CCTL_AHB_DIV_SHIFT	28
#define CCM_CCTL_AHB_DIV_MASK	3
#define CCM_CCTL_ARM_SRC	(1 << 14)
#define CCM_CGR1_GPT1		(1 << 19)
#define CCM_PERCLK_REG(clk)	(clk / 4)
#define CCM_PERCLK_SHIFT(clk)	(8 * (clk % 4))
#define CCM_PERCLK_MASK		0x3f
#define CCM_RCSR_NF_16BIT_SEL	(1 << 14)
#define CCM_RCSR_NF_PS(v)	((v >> 26) & 3)

/* ESDRAM Controller register bitfields */
#define ESDCTL_PRCT(x)		(((x) & 0x3f) << 0)
#define ESDCTL_BL		(1 << 7)
#define ESDCTL_FP		(1 << 8)
#define ESDCTL_PWDT(x)		(((x) & 3) << 10)
#define ESDCTL_SREFR(x)		(((x) & 7) << 13)
#define ESDCTL_DSIZ_16_UPPER	(0 << 16)
#define ESDCTL_DSIZ_16_LOWER	(1 << 16)
#define ESDCTL_DSIZ_32		(2 << 16)
#define ESDCTL_COL8		(0 << 20)
#define ESDCTL_COL9		(1 << 20)
#define ESDCTL_COL10		(2 << 20)
#define ESDCTL_ROW11		(0 << 24)
#define ESDCTL_ROW12		(1 << 24)
#define ESDCTL_ROW13		(2 << 24)
#define ESDCTL_ROW14		(3 << 24)
#define ESDCTL_ROW15		(4 << 24)
#define ESDCTL_SP		(1 << 27)
#define ESDCTL_SMODE_NORMAL	(0 << 28)
#define ESDCTL_SMODE_PRECHARGE	(1 << 28)
#define ESDCTL_SMODE_AUTO_REF	(2 << 28)
#define ESDCTL_SMODE_LOAD_MODE	(3 << 28)
#define ESDCTL_SMODE_MAN_REF	(4 << 28)
#define ESDCTL_SDE		(1 << 31)

#define ESDCFG_TRC(x)		(((x) & 0xf) << 0)
#define ESDCFG_TRCD(x)		(((x) & 0x7) << 4)
#define ESDCFG_TCAS(x)		(((x) & 0x3) << 8)
#define ESDCFG_TRRD(x)		(((x) & 0x3) << 10)
#define ESDCFG_TRAS(x)		(((x) & 0x7) << 12)
#define ESDCFG_TWR		(1 << 15)
#define ESDCFG_TMRD(x)		(((x) & 0x3) << 16)
#define ESDCFG_TRP(x)		(((x) & 0x3) << 18)
#define ESDCFG_TWTR		(1 << 20)
#define ESDCFG_TXP(x)		(((x) & 0x3) << 21)

#define ESDMISC_RST		(1 << 1)
#define ESDMISC_MDDREN		(1 << 2)
#define ESDMISC_MDDR_DL_RST	(1 << 3)
#define ESDMISC_MDDR_MDIS	(1 << 4)
#define ESDMISC_LHD		(1 << 5)
#define ESDMISC_MA10_SHARE	(1 << 6)
#define ESDMISC_SDRAM_RDY	(1 << 31)

/* GPT bits */
#define GPT_CTRL_SWR		(1 << 15)	/* Software reset */
#define GPT_CTRL_FRR		(1 << 9)	/* Freerun / restart */
#define GPT_CTRL_CLKSOURCE_32	(4 << 6)	/* Clock source	*/
#define GPT_CTRL_TEN		1		/* Timer enable	*/

/* WDOG enable */
#define WCR_WDE 		0x04
#define WSR_UNLOCK1		0x5555
#define WSR_UNLOCK2		0xAAAA

/* Names used in GPIO driver */
#define GPIO1_BASE_ADDR		IMX_GPIO1_BASE
#define GPIO2_BASE_ADDR		IMX_GPIO2_BASE
#define GPIO3_BASE_ADDR		IMX_GPIO3_BASE
#define GPIO4_BASE_ADDR		IMX_GPIO4_BASE

#define CHIP_REV_1_0		0x10
#define CHIP_REV_1_1		0x11
#define CHIP_REV_1_2		0x12

#endif				/* _IMX_REGS_H */
