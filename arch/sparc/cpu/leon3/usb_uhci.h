/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 *
 * Note: Part of this code has been derived from linux
 *
 */
#ifndef _USB_UHCI_H_
#define _USB_UHCI_H_

/* Command register */
#define USBCMD		0
#define   USBCMD_RS       0x0001	/* Run/Stop */
#define   USBCMD_HCRESET  0x0002	/* Host reset */
#define   USBCMD_GRESET   0x0004	/* Global reset */
#define   USBCMD_EGSM     0x0008	/* Global Suspend Mode */
#define   USBCMD_FGR      0x0010	/* Force Global Resume */
#define   USBCMD_SWDBG    0x0020	/* SW Debug mode */
#define   USBCMD_CF       0x0040	/* Config Flag (sw only) */
#define   USBCMD_MAXP     0x0080	/* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#define USBSTS		2
#define   USBSTS_USBINT   0x0001	/* Interrupt due to IOC */
#define   USBSTS_ERROR    0x0002	/* Interrupt due to error */
#define   USBSTS_RD       0x0004	/* Resume Detect */
#define   USBSTS_HSE      0x0008	/* Host System Error - basically PCI problems */
#define   USBSTS_HCPE     0x0010	/* Host Controller Process Error - the scripts were buggy */
#define   USBSTS_HCH      0x0020	/* HC Halted */

/* Interrupt enable register */
#define USBINTR		4
#define   USBINTR_TIMEOUT 0x0001	/* Timeout/CRC error enable */
#define   USBINTR_RESUME  0x0002	/* Resume interrupt enable */
#define   USBINTR_IOC     0x0004	/* Interrupt On Complete enable */
#define   USBINTR_SP      0x0008	/* Short packet interrupt enable */

#define USBFRNUM      6
#define USBFLBASEADD  8
#define USBSOF        12

/* USB port status and control registers */
#define USBPORTSC1	16
#define USBPORTSC2	18
#define   USBPORTSC_CCS   0x0001	/* Current Connect Status ("device present") */
#define   USBPORTSC_CSC   0x0002	/* Connect Status Change */
#define   USBPORTSC_PE    0x0004	/* Port Enable */
#define   USBPORTSC_PEC   0x0008	/* Port Enable Change */
#define   USBPORTSC_LS    0x0030	/* Line Status */
#define   USBPORTSC_RD    0x0040	/* Resume Detect */
#define   USBPORTSC_LSDA  0x0100	/* Low Speed Device Attached */
#define   USBPORTSC_PR    0x0200	/* Port Reset */
#define   USBPORTSC_SUSP  0x1000	/* Suspend */

/* Legacy support register */
#define USBLEGSUP 0xc0
#define USBLEGSUP_DEFAULT 0x2000	/* only PIRQ enable set */

#define UHCI_NULL_DATA_SIZE 0x7ff	/* for UHCI controller TD */
#define UHCI_PID            0xff	/* PID MASK */

#define UHCI_PTR_BITS       0x000F
#define UHCI_PTR_TERM       0x0001
#define UHCI_PTR_QH         0x0002
#define UHCI_PTR_DEPTH      0x0004

/* for TD <status>: */
#define TD_CTRL_SPD         (1 << 29)	/* Short Packet Detect */
#define TD_CTRL_C_ERR_MASK  (3 << 27)	/* Error Counter bits */
#define TD_CTRL_LS          (1 << 26)	/* Low Speed Device */
#define TD_CTRL_IOS         (1 << 25)	/* Isochronous Select */
#define TD_CTRL_IOC         (1 << 24)	/* Interrupt on Complete */
#define TD_CTRL_ACTIVE      (1 << 23)	/* TD Active */
#define TD_CTRL_STALLED     (1 << 22)	/* TD Stalled */
#define TD_CTRL_DBUFERR     (1 << 21)	/* Data Buffer Error */
#define TD_CTRL_BABBLE      (1 << 20)	/* Babble Detected */
#define TD_CTRL_NAK         (1 << 19)	/* NAK Received */
#define TD_CTRL_CRCTIMEO    (1 << 18)	/* CRC/Time Out Error */
#define TD_CTRL_BITSTUFF    (1 << 17)	/* Bit Stuff Error */
#define TD_CTRL_ACTLEN_MASK 0x7ff	/* actual length, encoded as n - 1 */

#define TD_CTRL_ANY_ERROR	(TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
				 TD_CTRL_BABBLE | TD_CTRL_CRCTIME | TD_CTRL_BITSTUFF)

#define TD_TOKEN_TOGGLE		19

/* ------------------------------------------------------------------------------------
   Virtual Root HUB
   ------------------------------------------------------------------------------------ */
/* destination of request */
#define RH_INTERFACE               0x01
#define RH_ENDPOINT                0x02
#define RH_OTHER                   0x03

#define RH_CLASS                   0x20
#define RH_VENDOR                  0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS           0x0080
#define RH_CLEAR_FEATURE        0x0100
#define RH_SET_FEATURE          0x0300
#define RH_SET_ADDRESS          0x0500
#define RH_GET_DESCRIPTOR       0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION    0x0880
#define RH_SET_CONFIGURATION    0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP               0x2000

/* Hub port features */
#define RH_PORT_CONNECTION         0x00
#define RH_PORT_ENABLE             0x01
#define RH_PORT_SUSPEND            0x02
#define RH_PORT_OVER_CURRENT       0x03
#define RH_PORT_RESET              0x04
#define RH_PORT_POWER              0x08
#define RH_PORT_LOW_SPEED          0x09
#define RH_C_PORT_CONNECTION       0x10
#define RH_C_PORT_ENABLE           0x11
#define RH_C_PORT_SUSPEND          0x12
#define RH_C_PORT_OVER_CURRENT     0x13
#define RH_C_PORT_RESET            0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER       0x00
#define RH_C_HUB_OVER_CURRENT      0x01

#define RH_DEVICE_REMOTE_WAKEUP    0x00
#define RH_ENDPOINT_STALL          0x01

/* Our Vendor Specific feature */
#define RH_REMOVE_EP               0x00

#define RH_ACK                     0x01
#define RH_REQ_ERR                 -1
#define RH_NACK                    0x00

/* Transfer descriptor structure */
typedef struct {
	unsigned long link;	/* next td/qh (LE) */
	unsigned long status;	/* status of the td */
	unsigned long info;	/* Max Lenght / Endpoint / device address and PID */
	unsigned long buffer;	/* pointer to data buffer (LE) */
	unsigned long dev_ptr;	/* pointer to the assigned device (BE) */
	unsigned long res[3];	/* reserved (TDs must be 8Byte aligned) */
} uhci_td_t, *puhci_td_t;

/* Queue Header structure */
typedef struct {
	unsigned long head;	/* Next QH (LE) */
	unsigned long element;	/* Queue element pointer (LE) */
	unsigned long res[5];	/* reserved */
	unsigned long dev_ptr;	/* if 0 no tds have been assigned to this qh */
} uhci_qh_t, *puhci_qh_t;

struct virt_root_hub {
	int devnum;		/* Address of Root Hub endpoint */
	int numports;		/* number of ports */
	int c_p_r[8];		/* C_PORT_RESET */
};

#endif				/* _USB_UHCI_H_ */
