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
} __attribute__ ((packed, aligned(4)));

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
#define STS_ASS		(1 << 15)
#define STS_HALT	(1 << 12)
	uint32_t or_usbintr;
#define INTR_UE         (1 << 0)                /* USB interrupt enable */
#define INTR_UEE        (1 << 1)                /* USB error interrupt enable */
#define INTR_PCE        (1 << 2)                /* Port change detect enable */
#define INTR_SEE        (1 << 4)                /* system error enable */
#define INTR_AAE        (1 << 5)                /* Interrupt on async adavance enable */
	uint32_t or_frindex;
	uint32_t or_ctrldssegment;
	uint32_t or_periodiclistbase;
	uint32_t or_asynclistaddr;
	uint32_t _reserved_0_;
	uint32_t or_burstsize;
	uint32_t or_txfilltuning;
#define TXFIFO_THRESH_MASK		(0x3f << 16)
#define TXFIFO_THRESH(p)		((p & 0x3f) << 16)
	uint32_t _reserved_1_[6];
	uint32_t or_configflag;
#define FLAG_CF		(1 << 0)	/* true:  we'll support "high speed" */
	uint32_t or_portsc[CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS];
#define PORTSC_PSPD(x)		(((x) >> 26) & 0x3)
#define PORTSC_PSPD_FS			0x0
#define PORTSC_PSPD_LS			0x1
#define PORTSC_PSPD_HS			0x2
	uint32_t or_systune;
} __attribute__ ((packed, aligned(4)));

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
	/* this part defined by EHCI spec */
	uint32_t qt_next;			/* see EHCI 3.5.1 */
#define	QT_NEXT_TERMINATE	1
	uint32_t qt_altnext;			/* see EHCI 3.5.2 */
	uint32_t qt_token;			/* see EHCI 3.5.3 */
#define QT_TOKEN_DT(x)		(((x) & 0x1) << 31)	/* Data Toggle */
#define QT_TOKEN_GET_DT(x)		(((x) >> 31) & 0x1)
#define QT_TOKEN_TOTALBYTES(x)	(((x) & 0x7fff) << 16)	/* Total Bytes to Transfer */
#define QT_TOKEN_GET_TOTALBYTES(x)	(((x) >> 16) & 0x7fff)
#define QT_TOKEN_IOC(x)		(((x) & 0x1) << 15)	/* Interrupt On Complete */
#define QT_TOKEN_CPAGE(x)	(((x) & 0x7) << 12)	/* Current Page */
#define QT_TOKEN_CERR(x)	(((x) & 0x3) << 10)	/* Error Counter */
#define QT_TOKEN_PID(x)		(((x) & 0x3) << 8)	/* PID Code */
#define QT_TOKEN_PID_OUT		0x0
#define QT_TOKEN_PID_IN			0x1
#define QT_TOKEN_PID_SETUP		0x2
#define QT_TOKEN_STATUS(x)	(((x) & 0xff) << 0)	/* Status */
#define QT_TOKEN_GET_STATUS(x)		(((x) >> 0) & 0xff)
#define QT_TOKEN_STATUS_ACTIVE		0x80
#define QT_TOKEN_STATUS_HALTED		0x40
#define QT_TOKEN_STATUS_DATBUFERR	0x20
#define QT_TOKEN_STATUS_BABBLEDET	0x10
#define QT_TOKEN_STATUS_XACTERR		0x08
#define QT_TOKEN_STATUS_MISSEDUFRAME	0x04
#define QT_TOKEN_STATUS_SPLITXSTATE	0x02
#define QT_TOKEN_STATUS_PERR		0x01
#define QT_BUFFER_CNT		5
	uint32_t qt_buffer[QT_BUFFER_CNT];	/* see EHCI 3.5.4 */
	uint32_t qt_buffer_hi[QT_BUFFER_CNT];	/* Appendix B */
	/* pad struct for 32 byte alignment */
	uint32_t unused[3];
};

#define EHCI_PAGE_SIZE		4096

/* Queue Head (QH). */
struct QH {
	uint32_t qh_link;
#define	QH_LINK_TERMINATE	1
#define	QH_LINK_TYPE_ITD	0
#define	QH_LINK_TYPE_QH		2
#define	QH_LINK_TYPE_SITD	4
#define	QH_LINK_TYPE_FSTN	6
	uint32_t qh_endpt1;
#define QH_ENDPT1_RL(x)		(((x) & 0xf) << 28)	/* NAK Count Reload */
#define QH_ENDPT1_C(x)		(((x) & 0x1) << 27)	/* Control Endpoint Flag */
#define QH_ENDPT1_MAXPKTLEN(x)	(((x) & 0x7ff) << 16)	/* Maximum Packet Length */
#define QH_ENDPT1_H(x)		(((x) & 0x1) << 15)	/* Head of Reclamation List Flag */
#define QH_ENDPT1_DTC(x)	(((x) & 0x1) << 14)	/* Data Toggle Control */
#define QH_ENDPT1_DTC_IGNORE_QTD_TD	0x0
#define QH_ENDPT1_DTC_DT_FROM_QTD	0x1
#define QH_ENDPT1_EPS(x)	(((x) & 0x3) << 12)	/* Endpoint Speed */
#define QH_ENDPT1_EPS_FS		0x0
#define QH_ENDPT1_EPS_LS		0x1
#define QH_ENDPT1_EPS_HS		0x2
#define QH_ENDPT1_ENDPT(x)	(((x) & 0xf) << 8)	/* Endpoint Number */
#define QH_ENDPT1_I(x)		(((x) & 0x1) << 7)	/* Inactivate on Next Transaction */
#define QH_ENDPT1_DEVADDR(x)	(((x) & 0x7f) << 0)	/* Device Address */
	uint32_t qh_endpt2;
#define QH_ENDPT2_MULT(x)	(((x) & 0x3) << 30)	/* High-Bandwidth Pipe Multiplier */
#define QH_ENDPT2_PORTNUM(x)	(((x) & 0x7f) << 23)	/* Port Number */
#define QH_ENDPT2_HUBADDR(x)	(((x) & 0x7f) << 16)	/* Hub Address */
#define QH_ENDPT2_UFCMASK(x)	(((x) & 0xff) << 8)	/* Split Completion Mask */
#define QH_ENDPT2_UFSMASK(x)	(((x) & 0xff) << 0)	/* Interrupt Schedule Mask */
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
