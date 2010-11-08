/*
 * Copyright (c) 2005, 2009 Freescale Semiconductor, Inc
 * Copyright (c) 2005 MontaVista Software
 * Copyright (c) 2008 Excito Elektronik i Sk=E5ne AB
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
 */

#ifndef _EHCI_FSL_H
#define _EHCI_FSL_H

#include <asm/processor.h>

/* Global offsets */
#define FSL_SKIP_PCI		0x100

/* offsets for the non-ehci registers in the FSL SOC USB controller */
#define FSL_SOC_USB_ULPIVP	0x170
#define FSL_SOC_USB_PORTSC1	0x184
#define PORT_PTS_MSK		(3 << 30)
#define PORT_PTS_UTMI		(0 << 30)
#define PORT_PTS_ULPI		(2 << 30)
#define PORT_PTS_SERIAL		(3 << 30)
#define PORT_PTS_PTW		(1 << 28)

/* USBMODE Register bits */
#define CM_IDLE			(0 << 0)
#define CM_RESERVED		(1 << 0)
#define CM_DEVICE		(2 << 0)
#define CM_HOST			(3 << 0)
#define USBMODE_RESERVED_2	(0 << 2)
#define SLOM			(1 << 3)
#define SDIS			(1 << 4)

/* CONTROL Register bits */
#define ULPI_INT_EN		(1 << 0)
#define WU_INT_EN		(1 << 1)
#define USB_EN			(1 << 2)
#define LSF_EN			(1 << 3)
#define KEEP_OTG_ON		(1 << 4)
#define OTG_PORT		(1 << 5)
#define REFSEL_12MHZ		(0 << 6)
#define REFSEL_16MHZ		(1 << 6)
#define REFSEL_48MHZ		(2 << 6)
#define PLL_RESET		(1 << 8)
#define UTMI_PHY_EN		(1 << 9)
#define PHY_CLK_SEL_UTMI	(0 << 10)
#define PHY_CLK_SEL_ULPI	(1 << 10)
#define CLKIN_SEL_USB_CLK	(0 << 11)
#define CLKIN_SEL_USB_CLK2	(1 << 11)
#define CLKIN_SEL_SYS_CLK	(2 << 11)
#define CLKIN_SEL_SYS_CLK2	(3 << 11)
#define RESERVED_18		(0 << 13)
#define RESERVED_17		(0 << 14)
#define RESERVED_16		(0 << 15)
#define WU_INT			(1 << 16)
#define PHY_CLK_VALID		(1 << 17)

#define FSL_SOC_USB_PORTSC2	0x188
#define FSL_SOC_USB_USBMODE	0x1a8
#define FSL_SOC_USB_SNOOP1	0x400	/* NOTE: big-endian */
#define FSL_SOC_USB_SNOOP2	0x404	/* NOTE: big-endian */
#define FSL_SOC_USB_AGECNTTHRSH	0x408	/* NOTE: big-endian */
#define FSL_SOC_USB_PRICTRL	0x40c	/* NOTE: big-endian */
#define FSL_SOC_USB_SICTRL	0x410	/* NOTE: big-endian */
#define FSL_SOC_USB_CTRL	0x500	/* NOTE: big-endian */
#define SNOOP_SIZE_2GB		0x1e

/* System Clock Control Register */
#define MPC83XX_SCCR_USB_MASK		0x00f00000
#define MPC83XX_SCCR_USB_DRCM_11	0x00300000
#define MPC83XX_SCCR_USB_DRCM_01	0x00100000
#define MPC83XX_SCCR_USB_DRCM_10	0x00200000

#if defined(CONFIG_MPC83xx)
#define CONFIG_SYS_MPC8xxx_USB_ADDR CONFIG_SYS_MPC83xx_USB_ADDR
#elif defined(CONFIG_MPC85xx)
#define CONFIG_SYS_MPC8xxx_USB_ADDR CONFIG_SYS_MPC85xx_USB_ADDR
#endif

/*
 * USB Registers
 */
struct usb_ehci {
	u8	res1[0x100];
	u16	caplength;	/* 0x100 - Capability Register Length */
	u16	hciversion;	/* 0x102 - Host Interface Version */
	u32	hcsparams;	/* 0x104 - Host Structural Parameters */
	u32	hccparams;	/* 0x108 - Host Capability Parameters */
	u8	res2[0x14];
	u32	dciversion;	/* 0x120 - Device Interface Version */
	u32	dciparams;	/* 0x124 - Device Controller Params */
	u8	res3[0x18];
	u32	usbcmd;		/* 0x140 - USB Command */
	u32	usbsts;		/* 0x144 - USB Status */
	u32	usbintr;	/* 0x148 - USB Interrupt Enable */
	u32	frindex;	/* 0x14C - USB Frame Index */
	u8	res4[0x4];
	u32	perlistbase;	/* 0x154 - Periodic List Base
					 - USB Device Address */
	u32	ep_list_addr;	/* 0x158 - Next Asynchronous List
					 - Endpoint Address */
	u8	res5[0x4];
	u32	burstsize;	/* 0x160 - Programmable Burst Size */
	u32	txfilltuning;	/* 0x164 - Host TT Transmit
					   pre-buffer packet tuning */
	u8	res6[0x8];
	u32	ulpi_viewpoint;	/* 0x170 - ULPI Reister Access */
	u8	res7[0xc];
	u32	config_flag;	/* 0x180 - Configured Flag Register */
	u32	portsc;		/* 0x184 - Port status/control */
	u8	res8[0x20];
	u32	usbmode;	/* 0x1a8 - USB Device Mode */
	u32	epsetupstat;	/* 0x1ac - Endpoint Setup Status */
	u32	epprime;	/* 0x1b0 - Endpoint Init Status */
	u32	epflush;	/* 0x1b4 - Endpoint De-initlialize */
	u32	epstatus;	/* 0x1b8 - Endpoint Status */
	u32	epcomplete;	/* 0x1bc - Endpoint Complete */
	u32	epctrl0;	/* 0x1c0 - Endpoint Control 0 */
	u32	epctrl1;	/* 0x1c4 - Endpoint Control 1 */
	u32	epctrl2;	/* 0x1c8 - Endpoint Control 2 */
	u32	epctrl3;	/* 0x1cc - Endpoint Control 3 */
	u32	epctrl4;	/* 0x1d0 - Endpoint Control 4 */
	u32	epctrl5;	/* 0x1d4 - Endpoint Control 5 */
	u8	res9[0x228];
	u32	snoop1;		/* 0x400 - Snoop 1 */
	u32	snoop2;		/* 0x404 - Snoop 2 */
	u32	age_cnt_limit;	/* 0x408 - Age Count Threshold */
	u32	prictrl;	/* 0x40c - Priority Control */
	u32	sictrl;		/* 0x410 - System Interface Control */
	u8	res10[0xEC];
	u32	control;	/* 0x500 - Control */
	u8	res11[0xafc];
};

#endif /* _EHCI_FSL_H */
