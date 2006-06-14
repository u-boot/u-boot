/*
 * Copyright (C) 2006 Bryan O'Donoghue, CodeHermit
 * bodonoghue@codehermit.ie
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <commproc.h>

/* Mode Register */
#define USMOD_EN	0x01
#define USMOD_HOST	0x02
#define USMOD_TEST	0x04
#define USMOD_SFTE	0x08
#define USMOD_RESUME	0x40
#define USMOD_LSS	0x80

/* Endpoint Registers */
#define USEP_RHS_NORM	0x00
#define USEP_RHS_IGNORE	0x01
#define USEP_RHS_NAK	0x02
#define USEP_RHS_STALL	0x03

#define USEP_THS_NORM	0x00
#define USEP_THS_IGNORE	0x04
#define USEP_THS_NAK	0x08
#define USEP_THS_STALL	0x0C

#define USEP_RTE	0x10
#define USEP_MF		0x20

#define USEP_TM_CONTROL	0x00
#define USEP_TM_INT	0x100
#define USEP_TM_BULK	0x200
#define USEP_TM_ISO	0x300

/* Command Register */
#define USCOM_EP0	0x00
#define USCOM_EP1	0x01
#define USCOM_EP2	0x02
#define USCOM_EP3	0x03

#define USCOM_FLUSH	0x40
#define USCOM_STR	0x80

/* Event Register */
#define USB_E_RXB	0x0001
#define USB_E_TXB	0x0002
#define USB_E_BSY	0x0004
#define USB_E_SOF	0x0008
#define USB_E_TXE1	0x0010
#define USB_E_TXE2	0x0020
#define USB_E_TXE3	0x0040
#define USB_E_TXE4	0x0080
#define USB_TX_ERRMASK (USB_E_TXE1|USB_E_TXE2|USB_E_TXE3|USB_E_TXE4)
#define USB_E_IDLE	0x0100
#define USB_E_RESET	0x0200

/* Mask Register */
#define USBS_IDLE	0x01

/* RX Buffer Descriptor */
#define RX_BD_OV	0x02
#define RX_BD_CR	0x04
#define RX_BD_AB	0x08
#define RX_BD_NO	0x10
#define RX_BD_PID_DATA0	0x00
#define RX_BD_PID_DATA1	0x40
#define RX_BD_PID_SETUP	0x80
#define RX_BD_F		0x400
#define RX_BD_L		0x800
#define RX_BD_I		0x1000
#define RX_BD_W		0x2000
#define RX_BD_E		0x8000

/* Useful masks */
#define RX_BD_PID_BITMASK (RX_BD_PID_DATA1 | RX_BD_PID_SETUP)
#define STALL_BITMASK (USEP_THS_STALL | USEP_RHS_STALL)
#define NAK_BITMASK (USEP_THS_NAK | USEP_RHS_NAK)
#define CBD_TX_BITMASK (TX_BD_R | TX_BD_L | TX_BD_TC | TX_BD_I | TX_BD_CNF)

/* TX Buffer Descriptor */
#define TX_BD_UN	0x02
#define TX_BD_TO	0x04
#define TX_BD_NO_PID	0x00
#define TX_BD_PID_DATA0	0x80
#define TX_BD_PID_DATA1	0xC0
#define TX_BD_CNF	0x200
#define TX_BD_TC	0x400
#define TX_BD_L		0x800
#define TX_BD_I		0x1000
#define TX_BD_W		0x2000
#define TX_BD_R		0x8000

/* Implementation specific defines */

#define EP_MIN_PACKET_SIZE 0x08
#define MAX_ENDPOINTS	0x04
#define FIFO_SIZE	0x10
#define EP_MAX_PKT	FIFO_SIZE
#define TX_RING_SIZE	0x04
#define RX_RING_SIZE	0x06
#define USB_MAX_PKT	0x40
#define TOGGLE_TX_PID(x) x= ((~x)&0x40)|0x80
#define TOGGLE_RX_PID(x) x^= 0x40
#define EP_ATTACHED	0x01	/* Endpoint has a urb attached or not */
#define EP_SEND_ZLP	0x02	/* Send ZLP y/n ? */

#define PROFF_USB	0x00000000
#define CPM_USB_BASE	0x00000A00

/* UDC device defines */
#define EP0_MAX_PACKET_SIZE	EP_MAX_PKT
#define UDC_OUT_ENDPOINT	0x02
#define UDC_OUT_PACKET_SIZE	EP_MIN_PACKET_SIZE
#define UDC_IN_ENDPOINT		0x03
#define UDC_IN_PACKET_SIZE	EP_MIN_PACKET_SIZE
#define UDC_INT_ENDPOINT	0x01
#define UDC_INT_PACKET_SIZE	UDC_IN_PACKET_SIZE
#define UDC_BULK_PACKET_SIZE	EP_MIN_PACKET_SIZE

struct mpc8xx_ep {
	struct urb * urb;
	unsigned char pid;
	unsigned char sc;
	volatile cbd_t * prx;
};

typedef struct mpc8xx_usb{
	char usmod;	/* Mode Register */
	char usaddr;	/* Slave Address Register */
	char uscom;	/* Command Register */
	char res1;	/* Reserved */
	ushort usep[4];
	ulong res2;	/* Reserved */
	ushort usber;	/* Event Register */
	ushort res3;	/* Reserved */
	ushort usbmr;	/* Mask Register */
	char res4;	/* Reserved */
	char usbs;	/* Status Register */
	char res5[8];	/* Reserved */
}usb_t;

typedef struct mpc8xx_parameter_ram{
	ushort ep0ptr;	/* Endpoint Pointer Register 0 */
	ushort ep1ptr;	/* Endpoint Pointer Register 1 */
	ushort ep2ptr;	/* Endpoint Pointer Register 2 */
	ushort ep3ptr;	/* Endpoint Pointer Register 3 */
	uint rstate;	/* Receive state */
	uint rptr;	/* Receive internal data pointer */
	ushort frame_n;	/* Frame number */
	ushort rbcnt;	/* Receive byte count */
	uint rtemp;	/* Receive temp cp use only */
	uint rxusb;	/* Rx Data Temp */
	ushort rxuptr;	/* Rx microcode return address temp */
}usb_pram_t;

typedef struct endpoint_parameter_block_pointer{
	ushort rbase;	/* RxBD base address */
	ushort tbase;	/* TxBD base address */
	char rfcr;	/* Rx Function code */
	char tfcr;	/* Tx Function code */
	ushort mrblr;	/* Maximum Receive Buffer Length */
	ushort rbptr; 	/* RxBD pointer Next Buffer Descriptor */
	ushort tbptr;	/* TxBD pointer Next Buffer Descriptor  */
	ulong tstate;	/* Transmit internal state */
	ulong tptr;	/* Transmit internal data pointer */
	ushort tcrc;	/* Transmit temp CRC */
	ushort tbcnt;	/* Transmit internal bye count */
	ulong ttemp;	/* Tx temp */
	ushort txuptr;	/* Tx microcode return address */
	ushort res1;	/* Reserved */
}usb_epb_t;

typedef enum mpc8xx_udc_state{
	STATE_NOT_READY,
	STATE_ERROR,
	STATE_READY,
}mpc8xx_udc_state_t;

/* Declarations */
int udc_init(void);
void udc_irq(void);
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);
void udc_setup_ep(struct usb_device_instance *device, unsigned int ep,
		  struct usb_endpoint_instance *endpoint);
void udc_connect(void);
void udc_disconnect(void);
void udc_enable(struct usb_device_instance *device);
void udc_disable(void);
void udc_startup_events(struct usb_device_instance *device);

/* Flow control */
void udc_set_nak(int epid);
void udc_unset_nak (int epid);
