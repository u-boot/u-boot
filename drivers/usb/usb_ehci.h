/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
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

#ifndef USB_EHCI_H
#define USB_EHCI_H

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define DeviceOutRequest \
	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define InterfaceRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointRequest \
	((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointOutRequest \
	((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

/*
 * Register Space.
 */
struct ehci_hccr {
	uint32_t cr_capbase;
#define HC_LENGTH(p)		(((p) >> 0) & 0x00ff)
#define HC_VERSION(p)		(((p) >> 16) & 0xffff)
	uint32_t cr_hcsparams;
	uint32_t cr_hccparams;
	uint8_t cr_hcsp_portrt[8];
};

struct ehci_hcor {
	uint32_t or_usbcmd;
#define CMD_ASE		(1 << 5)
	uint32_t or_usbsts;
#define	STD_ASS		(1 << 15)
	uint32_t or_usbintr;
	uint32_t or_frindex;
	uint32_t or_ctrldssegment;
	uint32_t or_periodiclistbase;
	uint32_t or_asynclistaddr;
	uint32_t _reserved_[9];
	uint32_t or_configflag;
	uint32_t or_portsc[2];
	uint32_t or_systune;
};

/* Interface descriptor */
struct usb_linux_interface_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bInterfaceNumber;
	unsigned char	bAlternateSetting;
	unsigned char	bNumEndpoints;
	unsigned char	bInterfaceClass;
	unsigned char	bInterfaceSubClass;
	unsigned char	bInterfaceProtocol;
	unsigned char	iInterface;
} __attribute__ ((packed));

/* Configuration descriptor information.. */
struct usb_linux_config_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned short	wTotalLength;
	unsigned char	bNumInterfaces;
	unsigned char	bConfigurationValue;
	unsigned char	iConfiguration;
	unsigned char	bmAttributes;
	unsigned char	MaxPower;
} __attribute__ ((packed));

#if defined CONFIG_EHCI_DESC_BIG_ENDIAN
#define	ehci_readl(x)		(x)
#define ehci_writel(a, b)	(a) = (b)
#else
#define ehci_readl(x)		cpu_to_le32((x))
#define ehci_writel(a, b)	(a) = cpu_to_le32((b))
#endif

#if defined CONFIG_EHCI_MMIO_BIG_ENDIAN
#define hc32_to_cpu(x)		be32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_be32((x))
#else
#define hc32_to_cpu(x)		le32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_le32((x))
#endif

#define EHCI_PS_WKOC_E		0x00400000	/* RW wake on over current */
#define EHCI_PS_WKDSCNNT_E	0x00200000	/* RW wake on disconnect */
#define EHCI_PS_WKCNNT_E	0x00100000	/* RW wake on connect */
#define EHCI_PS_PTC		0x000f0000	/* RW port test control */
#define EHCI_PS_PIC		0x0000c000	/* RW port indicator control */
#define EHCI_PS_PO		0x00002000	/* RW port owner */
#define EHCI_PS_PP		0x00001000	/* RW,RO port power */
#define EHCI_PS_LS		0x00000c00	/* RO line status */
#define EHCI_PS_PR		0x00000100	/* RW port reset */
#define EHCI_PS_SUSP		0x00000080	/* RW suspend */
#define EHCI_PS_FPR		0x00000040	/* RW force port resume */
#define EHCI_PS_OCC		0x00000020	/* RWC over current change */
#define EHCI_PS_OCA		0x00000010	/* RO over current active */
#define EHCI_PS_PEC		0x00000008	/* RWC port enable change */
#define EHCI_PS_PE		0x00000004	/* RW port enable */
#define EHCI_PS_CSC		0x00000002	/* RWC connect status change */
#define EHCI_PS_CS		0x00000001	/* RO connect status */
#define EHCI_PS_CLEAR		(EHCI_PS_OCC | EHCI_PS_PEC | EHCI_PS_CSC)

#define EHCI_PS_IS_LOWSPEED(x)	(((x) & EHCI_PS_LS) == 0x00000400)

/*
 * Schedule Interface Space.
 *
 * IMPORTANT: Software must ensure that no interface data structure
 * reachable by the EHCI host controller spans a 4K page boundary!
 *
 * Periodic transfers (i.e. isochronous and interrupt transfers) are
 * not supported.
 */

/* Queue Element Transfer Descriptor (qTD). */
struct qTD {
	uint32_t qt_next;
#define	QT_NEXT_TERMINATE	1
	uint32_t qt_altnext;
	uint32_t qt_token;
	uint32_t qt_buffer[5];
};

/* Queue Head (QH). */
struct QH {
	uint32_t qh_link;
#define	QH_LINK_TERMINATE	1
#define	QH_LINK_TYPE_ITD	0
#define	QH_LINK_TYPE_QH		2
#define	QH_LINK_TYPE_SITD	4
#define	QH_LINK_TYPE_FSTN	6
	uint32_t qh_endpt1;
	uint32_t qh_endpt2;
	uint32_t qh_curtd;
	struct qTD qh_overlay;
};

/* Low level intit functions */

int ehci_hcd_init(void);
int ehci_hcd_stop(void);
#endif /* USB_EHCI_H */
