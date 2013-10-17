/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __MV_UDC_H__
#define __MV_UDC_H__

#include <asm/byteorder.h>
#include <asm/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include "../../drivers/usb/host/ehci.h"

#define NUM_ENDPOINTS		6

/* Endpoint parameters */
#define MAX_ENDPOINTS		4

#define EP_MAX_PACKET_SIZE	0x200
#define EP0_MAX_PACKET_SIZE	64

struct mv_udc {
#define MICRO_8FRAME	0x8
#define USBCMD_ITC(x)	((((x) > 0xff) ? 0xff : x) << 16)
#define USBCMD_FS2	(1 << 15)
#define USBCMD_RST	(1 << 1)
#define USBCMD_RUN	(1)
	u32 usbcmd;		/* 0x140 */
#define STS_SLI		(1 << 8)
#define STS_URI		(1 << 6)
#define STS_PCI		(1 << 2)
#define STS_UEI		(1 << 1)
#define STS_UI		(1 << 0)
	u32 usbsts;		/* 0x144 */
	u32 pad1[3];
	u32 devaddr;		/* 0x154 */
	u32 epinitaddr;		/* 0x158 */
	u32 pad2[10];
#define PTS_ENABLE	2
#define PTS(x)		(((x) & 0x3) << 30)
#define PFSC		(1 << 24)
	u32 portsc;		/* 0x184 */
	u32 pad3[8];
#define USBMODE_DEVICE	2
	u32 usbmode;		/* 0x1a8 */
	u32 epstat;		/* 0x1ac */
#define EPT_TX(x)	(1 << (((x) & 0xffff) + 16))
#define EPT_RX(x)	(1 << ((x) & 0xffff))
	u32 epprime;		/* 0x1b0 */
	u32 epflush;		/* 0x1b4 */
	u32 pad4;
	u32 epcomp;		/* 0x1bc */
#define CTRL_TXE	(1 << 23)
#define CTRL_TXR	(1 << 22)
#define CTRL_RXE	(1 << 7)
#define CTRL_RXR	(1 << 6)
#define CTRL_TXT_BULK	(2 << 18)
#define CTRL_RXT_BULK	(2 << 2)
	u32 epctrl[16];		/* 0x1c0 */
};

struct mv_ep {
	struct usb_ep ep;
	struct list_head queue;
	const struct usb_endpoint_descriptor *desc;

	struct usb_request req;
	uint8_t *b_buf;
	uint32_t b_len;
	uint8_t b_fast[64] __aligned(ARCH_DMA_MINALIGN);
};

struct mv_drv {
	struct usb_gadget		gadget;
	struct usb_gadget_driver	*driver;
	struct ehci_ctrl		*ctrl;
	struct ept_queue_head		*epts;
	struct ept_queue_item		*items[2 * NUM_ENDPOINTS];
	uint8_t				*items_mem;
	struct mv_ep			ep[NUM_ENDPOINTS];
};

struct ept_queue_head {
	unsigned config;
	unsigned current;	/* read-only */

	unsigned next;
	unsigned info;
	unsigned page0;
	unsigned page1;
	unsigned page2;
	unsigned page3;
	unsigned page4;
	unsigned reserved_0;

	unsigned char setup_data[8];

	unsigned reserved_1;
	unsigned reserved_2;
	unsigned reserved_3;
	unsigned reserved_4;
};

#define CONFIG_MAX_PKT(n)	((n) << 16)
#define CONFIG_ZLT		(1 << 29)	/* stop on zero-len xfer */
#define CONFIG_IOS		(1 << 15)	/* IRQ on setup */

struct ept_queue_item {
	unsigned next;
	unsigned info;
	unsigned page0;
	unsigned page1;
	unsigned page2;
	unsigned page3;
	unsigned page4;
	unsigned reserved;
};

#define TERMINATE 1
#define INFO_BYTES(n)		((n) << 16)
#define INFO_IOC		(1 << 15)
#define INFO_ACTIVE		(1 << 7)
#define INFO_HALTED		(1 << 6)
#define INFO_BUFFER_ERROR	(1 << 5)
#define INFO_TX_ERROR		(1 << 3)

#endif /* __MV_UDC_H__ */
