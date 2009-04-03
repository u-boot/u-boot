/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
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

#if !defined(CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS)
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	2
#endif

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
#define HCS_PPC(p)		((p) & (1 << 4))
#define HCS_INDICATOR(p)	((p) & (1 << 16)) /* Port indicators */
#define HCS_N_PORTS(p)		(((p) >> 0) & 0xf)
	uint32_t cr_hccparams;
	uint8_t cr_hcsp_portrt[8];
} __attribute__ ((packed));

struct ehci_hcor {
	uint32_t or_usbcmd;
#define CMD_PARK	(1 << 11)		/* enable "park" */
#define CMD_PARK_CNT(c)	(((c) >> 8) & 3)	/* how many transfers to park */
#define CMD_ASE		(1 << 5)		/* async schedule enable */
#define CMD_LRESET	(1 << 7)		/* partial reset */
#define CMD_IAAD	(1 << 5)		/* "doorbell" interrupt */
#define CMD_PSE		(1 << 4)		/* periodic schedule enable */
#define CMD_RESET	(1 << 1)		/* reset HC not bus */
#define CMD_RUN		(1 << 0)		/* start/stop HC */
	uint32_t or_usbsts;
#define	STD_ASS		(1 << 15)
#define STS_HALT	(1 << 12)
	uint32_t or_usbintr;
	uint32_t or_frindex;
	uint32_t or_ctrldssegment;
	uint32_t or_periodiclistbase;
	uint32_t or_asynclistaddr;
	uint32_t _reserved_[9];
	uint32_t or_configflag;
#define FLAG_CF		(1 << 0)	/* true:  we'll support "high speed" */
	uint32_t or_portsc[CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS];
	uint32_t or_systune;
} __attribute__ ((packed));

#define USBMODE		0x68		/* USB Device mode */
#define USBMODE_SDIS	(1 << 3)	/* Stream disable */
#define USBMODE_BE	(1 << 2)	/* BE/LE endiannes select */
#define USBMODE_CM_HC	(3 << 0)	/* host controller mode */
#define USBMODE_CM_IDLE	(0 << 0)	/* idle state */

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
#define	ehci_readl(x)		(*((volatile u32 *)(x)))
#define ehci_writel(a, b)	(*((volatile u32 *)(a)) = ((volatile u32)b))
#else
#define ehci_readl(x)		cpu_to_le32((*((volatile u32 *)(x))))
#define ehci_writel(a, b)	(*((volatile u32 *)(a)) = \
					cpu_to_le32(((volatile u32)b)))
#endif

#if defined CONFIG_EHCI_MMIO_BIG_ENDIAN
#define hc32_to_cpu(x)		be32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_be32((x))
#else
#define hc32_to_cpu(x)		le32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_le32((x))
#endif

#define EHCI_PS_WKOC_E		(1 << 22)	/* RW wake on over current */
#define EHCI_PS_WKDSCNNT_E	(1 << 21)	/* RW wake on disconnect */
#define EHCI_PS_WKCNNT_E	(1 << 20)	/* RW wake on connect */
#define EHCI_PS_PO		(1 << 13)	/* RW port owner */
#define EHCI_PS_PP		(1 << 12)	/* RW,RO port power */
#define EHCI_PS_LS		(3 << 10)	/* RO line status */
#define EHCI_PS_PR		(1 << 8)	/* RW port reset */
#define EHCI_PS_SUSP		(1 << 7)	/* RW suspend */
#define EHCI_PS_FPR		(1 << 6)	/* RW force port resume */
#define EHCI_PS_OCC		(1 << 5)	/* RWC over current change */
#define EHCI_PS_OCA		(1 << 4)	/* RO over current active */
#define EHCI_PS_PEC		(1 << 3)	/* RWC port enable change */
#define EHCI_PS_PE		(1 << 2)	/* RW port enable */
#define EHCI_PS_CSC		(1 << 1)	/* RWC connect status change */
#define EHCI_PS_CS		(1 << 0)	/* RO connect status */
#define EHCI_PS_CLEAR		(EHCI_PS_OCC | EHCI_PS_PEC | EHCI_PS_CSC)

#define EHCI_PS_IS_LOWSPEED(x)	(((x) & EHCI_PS_LS) == (1 << 10))

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
	/*
	 * Add dummy fill value to make the size of this struct
	 * aligned to 32 bytes
	 */
	uint8_t fill[16];
};

/* Low level init functions */
int ehci_hcd_init(void);
int ehci_hcd_stop(void);

#endif /* USB_EHCI_H */
