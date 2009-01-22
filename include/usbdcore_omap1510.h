/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * Based on
 * linux/drivers/usb/device/bi/omap.h
 * Register definitions for TI OMAP1510 USB bus interface driver
 *
 * Author: MontaVista Software, Inc.
 *	   source@mvista.com
 *
 * 2003 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#ifndef __USBDCORE_OMAP1510_H__
#define __USBDCORE_OMAP1510_H__


/*
 * 13.2 MPU Register Map
 */

/* Table 13-1. USB Function Module Registers (endpoint) */
#define UDC_BASE		     0xFFFB4000
#define UDC_OFFSET(offset)	     (UDC_BASE + (offset))
#define UDC_REV			     UDC_OFFSET(0x0)	/* Revision */
#define UDC_EP_NUM		     UDC_OFFSET(0x4)	/* Endpoint selection */
#define UDC_DATA		     UDC_OFFSET(0x08)	/* Data */
#define UDC_CTRL		     UDC_OFFSET(0x0C)	/* Control */
#define UDC_STAT_FLG		     UDC_OFFSET(0x10)	/* Status flag */
#define UDC_RXFSTAT		     UDC_OFFSET(0x14)	/* Receive FIFO status */
#define UDC_SYSCON1		     UDC_OFFSET(0x18)	/* System configuration 1 */
#define UDC_SYSCON2		     UDC_OFFSET(0x1C)	/* System configuration 2 */
#define UDC_DEVSTAT		     UDC_OFFSET(0x20)	/* Device status */
#define UDC_SOF			     UDC_OFFSET(0x24)	/* Start of frame */
#define UDC_IRQ_EN		     UDC_OFFSET(0x28)	/* Interrupt enable */
#define UDC_DMA_IRQ_EN		     UDC_OFFSET(0x2C)	/* DMA interrupt enable */
#define UDC_IRQ_SRC		     UDC_OFFSET(0x30)	/* Interrupt source */
#define UDC_EPN_STAT		     UDC_OFFSET(0x34)	/* Endpoint interrupt status */
#define UDC_DMAN_STAT		     UDC_OFFSET(0x3C)	/* DMA endpoint interrupt status */

/* IRQ_EN register fields */
#define UDC_Sof_IE		     (1 << 7)	/* Start-of-frame interrupt enabled */
#define UDC_EPn_RX_IE		     (1 << 5)	/* Receive endpoint interrupt enabled */
#define UDC_EPn_TX_IE		     (1 << 4)	/* Transmit endpoint interrupt enabled */
#define UDC_DS_Chg_IE		     (1 << 3)	/* Device state changed interrupt enabled */
#define UDC_EP0_IE		     (1 << 0)	/* EP0 transaction interrupt enabled */

/* IRQ_SRC register fields */
#define UDC_TXn_Done		     (1 << 10)	/* Transmit DMA channel n done */
#define UDC_RXn_Cnt		     (1 << 9)	/* Receive DMA channel n transactions count */
#define UDC_RXn_EOT		     (1 << 8)	/* Receive DMA channel n end of transfer */
#define UDC_SOF_Flg		     (1 << 7)	/* Start-of-frame interrupt flag */
#define UDC_EPn_RX		     (1 << 5)	/* Endpoint n OUT transaction */
#define UDC_EPn_TX		     (1 << 4)	/* Endpoint n IN transaction */
#define UDC_DS_Chg		     (1 << 3)	/* Device state changed */
#define UDC_Setup		     (1 << 2)	/* Setup transaction */
#define UDC_EP0_RX		     (1 << 1)	/* EP0 OUT transaction */
#define UDC_EP0_TX		     (1 << 0)	/* EP0 IN transaction */

/* DEVSTAT register fields, 14.2.9 */
#define UDC_R_WK_OK		     (1 << 6)	/* Remote wakeup granted */
#define UDC_USB_Reset		     (1 << 5)	/* USB reset signalling is active */
#define UDC_SUS			     (1 << 4)	/* Suspended state */
#define UDC_CFG			     (1 << 3)	/* Configured state */
#define UDC_ADD			     (1 << 2)	/* Addressed state */
#define UDC_DEF			     (1 << 1)	/* Default state */
#define UDC_ATT			     (1 << 0)	/* Attached state */

/* SYSCON1 register fields */
#define UDC_Cfg_Lock		     (1 << 8)	/* Device configuration locked */
#define UDC_Nak_En		     (1 << 4)	/* NAK enable */
#define UDC_Self_Pwr		     (1 << 2)	/* Device is self-powered */
#define UDC_Soff_Dis		     (1 << 1)	/* Shutoff disabled */
#define UDC_Pullup_En		     (1 << 0)	/* External pullup enabled */

/* SYSCON2 register fields */
#define UDC_Rmt_Wkp		     (1 << 6)	/* Remote wakeup */
#define UDC_Stall_Cmd		     (1 << 5)	/* Stall endpoint */
#define UDC_Dev_Cfg		     (1 << 3)	/* Device configured */
#define UDC_Clr_Cfg		     (1 << 2)	/* Clear configured */

/*
 * Select and enable endpoints
 */

/* Table 13-1. USB Function Module Registers (endpoint configuration) */
#define UDC_EPBASE		     UDC_OFFSET(0x80)	/* Endpoints base address */
#define UDC_EP0			     UDC_EPBASE /* Control endpoint configuration */
#define UDC_EP_RX_BASE		     UDC_OFFSET(0x84)	/* Receive endpoints base address */
#define UDC_EP_RX(endpoint)	     (UDC_EP_RX_BASE + ((endpoint) - 1) * 4)
#define UDC_EP_TX_BASE		     UDC_OFFSET(0xC4)	/* Transmit endpoints base address */
#define UDC_EP_TX(endpoint)	     (UDC_EP_TX_BASE + ((endpoint) - 1) * 4)

/* EP_NUM register fields */
#define UDC_Setup_Sel		     (1 << 6)	/* Setup FIFO select */
#define UDC_EP_Sel		     (1 << 5)	/* TX/RX FIFO select */
#define UDC_EP_Dir		     (1 << 4)	/* Endpoint direction */

/* CTRL register fields */
#define UDC_Clr_Halt		     (1 << 7)	/* Clear halt endpoint */
#define UDC_Set_Halt		     (1 << 6)	/* Set halt endpoint */
#define UDC_Set_FIFO_En		     (1 << 2)	/* Set FIFO enable */
#define UDC_Clr_EP		     (1 << 1)	/* Clear endpoint */
#define UDC_Reset_EP		     (1 << 0)	/* Reset endpoint */

/* STAT_FLG register fields */
#define UDC_Miss_In		     (1 << 14)
#define UDC_Data_Flush		     (1 << 13)
#define UDC_ISO_Err		     (1 << 12)
#define UDC_ISO_FIFO_Empty	     (1 << 9)
#define UDC_ISO_FIFO_Full	     (1 << 8)
#define UDC_EP_Halted		     (1 << 6)
#define UDC_STALL		     (1 << 5)
#define UDC_NAK			     (1 << 4)
#define UDC_ACK			     (1 << 3)
#define UDC_FIFO_En		     (1 << 2)
#define UDC_Non_ISO_FIFO_Empty	     (1 << 1)
#define UDC_Non_ISO_FIFO_Full	     (1 << 0)

/* EPn_RX register fields */
#define UDC_EPn_RX_Valid	     (1 << 15)	/* valid */
#define UDC_EPn_RX_Db		     (1 << 14)	/* double-buffer */
#define UDC_EPn_RX_Iso		     (1 << 11)	/* isochronous */

/* EPn_TX register fields */
#define UDC_EPn_TX_Valid	     (1 << 15)	/* valid */
#define UDC_EPn_TX_Db		     (1 << 14)	/* double-buffer */
#define UDC_EPn_TX_Iso		     (1 << 11)	/* isochronous */

#define EP0_PACKETSIZE		     0x40

/* physical to logical endpoint mapping
 * Physical endpoints are an index into device->bus->endpoint_array.
 * Logical endpoints are endpoints 0 to 15 IN and OUT as defined in
 * the USB specification.
 *
 *	physical ep	logical ep	direction	endpoint_address
 *	0		0		IN and OUT	0x00
 *	1 to 15		1 to 15		OUT		0x01 to 0x0f
 *	16 to 30	1 to 15		IN		0x81 to 0x8f
 */
#define PHYS_EP_TO_EP_ADDR(ep) (((ep) < 16) ? (ep) : (((ep) - 15) | 0x80))
#define EP_ADDR_TO_PHYS_EP(a) (((a) & 0x80) ? (((a) & ~0x80) + 15) : (a))

/* MOD_CONF_CTRL_0 bits (FIXME: move to board hardware.h ?) */
#define CONF_MOD_USB_W2FC_VBUS_MODE_R (1 << 17)

/* Other registers (may be) related to USB */

#define CLOCK_CTRL	    (0xFFFE0830)
#define APLL_CTRL	    (0xFFFE084C)
#define DPLL_CTRL	    (0xFFFE083C)
#define SOFT_REQ	    (0xFFFE0834)
#define STATUS_REQ	    (0xFFFE0840)

/* FUNC_MUX_CTRL_0 bits related to USB */
#define UDC_VBUS_CTRL	    (1 << 19)
#define UDC_VBUS_MODE	    (1 << 18)

/* OMAP Endpoint parameters */
#define EP0_MAX_PACKET_SIZE 64
#define UDC_OUT_ENDPOINT 2
#define UDC_OUT_PACKET_SIZE 64
#define UDC_IN_ENDPOINT	1
#define UDC_IN_PACKET_SIZE 64
#define UDC_INT_ENDPOINT 5
#define UDC_INT_PACKET_SIZE	16
#define UDC_BULK_PACKET_SIZE 16

void udc_irq (void);
/* Flow control */
void udc_set_nak(int epid);
void udc_unset_nak (int epid);

/* Higher level functions for abstracting away from specific device */
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);

int  udc_init (void);

void udc_enable(struct usb_device_instance *device);
void udc_disable(void);

void udc_connect(void);
void udc_disconnect(void);

void udc_startup_events(struct usb_device_instance *device);
void udc_setup_ep(struct usb_device_instance *device, unsigned int ep, struct usb_endpoint_instance *endpoint);

#endif
