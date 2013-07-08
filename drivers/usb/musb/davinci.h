/*
 * TI's Davinci platform specific USB wrapper functions.
 *
 * Copyright (c) 2008 Texas Instruments
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Author: Thomas Abraham t-abraham@ti.com, Texas Instruments
 */

#ifndef __DAVINCI_USB_H__
#define __DAVINCI_USB_H__

#include <asm/arch/hardware.h>
#include "musb_core.h"

/* Base address of DAVINCI usb0 wrapper */
#define DAVINCI_USB0_BASE 0x01C64000

/* Base address of DAVINCI musb core */
#define MENTOR_USB0_BASE (DAVINCI_USB0_BASE+0x400)

/*
 * Davinci platform USB wrapper register overlay. Note: Only the required
 * registers are included in this structure. It can be expanded as required.
 */
struct davinci_usb_regs {
	u32	version;
	u32	ctrlr;
	u32	reserved[0x20];
	u32	intclrr;
	u32 	intmskr;
	u32 	intmsksetr;
};

#define DAVINCI_USB_TX_ENDPTS_MASK	0x1f /* ep0 + 4 tx */
#define DAVINCI_USB_RX_ENDPTS_MASK	0x1e /* 4 rx */
#define DAVINCI_USB_USBINT_SHIFT	16
#define DAVINCI_USB_TXINT_SHIFT 	0
#define DAVINCI_USB_RXINT_SHIFT 	8
#define DAVINCI_INTR_DRVVBUS		0x0100

#define DAVINCI_USB_USBINT_MASK 	0x01ff0000	/* 8 Mentor, DRVVBUS */
#define DAVINCI_USB_TXINT_MASK \
		(DAVINCI_USB_TX_ENDPTS_MASK << DAVINCI_USB_TXINT_SHIFT)
#define DAVINCI_USB_RXINT_MASK \
		(DAVINCI_USB_RX_ENDPTS_MASK << DAVINCI_USB_RXINT_SHIFT)
#define MGC_BUSCTL_OFFSET(_bEnd, _bOffset) \
		(0x80 + (8*(_bEnd)) + (_bOffset))

/* Integrated highspeed/otg PHY */
#define USBPHY_CTL_PADDR	(DAVINCI_SYSTEM_MODULE_BASE + 0x34)
#define USBPHY_PHY24MHZ 	(1 << 13)
#define USBPHY_PHYCLKGD 	(1 << 8)
#define USBPHY_SESNDEN		(1 << 7)	/* v(sess_end) comparator */
#define USBPHY_VBDTCTEN 	(1 << 6)	/* v(bus) comparator */
#define USBPHY_PHYPLLON 	(1 << 4)	/* override pll suspend */
#define USBPHY_CLKO1SEL 	(1 << 3)
#define USBPHY_OSCPDWN		(1 << 2)
#define USBPHY_PHYPDWN		(1 << 0)

/* Timeout for Davinci USB module */
#define DAVINCI_USB_TIMEOUT 0x3FFFFFF

/* IO Expander I2C address and VBUS enable mask */
#define IOEXP_I2C_ADDR 0x3A
#define IOEXP_VBUSEN_MASK 1

/* extern functions */
extern void lpsc_on(unsigned int id);
extern int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
extern void enable_vbus(void);
#endif	/* __DAVINCI_USB_H__ */
