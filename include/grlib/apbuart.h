/* GRLIB APBUART definitions
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GRLIB_APBUART_H__
#define __GRLIB_APBUART_H__

/* APBUART Register map */
typedef struct {
	volatile unsigned int data;
	volatile unsigned int status;
	volatile unsigned int ctrl;
	volatile unsigned int scaler;
} ambapp_dev_apbuart;

/*
 *  The following defines the bits in the LEON UART Status Registers.
 */

#define APBUART_STATUS_DR   0x00000001	/* Data Ready */
#define APBUART_STATUS_TSE  0x00000002	/* TX Send Register Empty */
#define APBUART_STATUS_THE  0x00000004	/* TX Hold Register Empty */
#define APBUART_STATUS_BR   0x00000008	/* Break Error */
#define APBUART_STATUS_OE   0x00000010	/* RX Overrun Error */
#define APBUART_STATUS_PE   0x00000020	/* RX Parity Error */
#define APBUART_STATUS_FE   0x00000040	/* RX Framing Error */
#define APBUART_STATUS_ERR  0x00000078	/* Error Mask */

/*
 *  The following defines the bits in the LEON UART Ctrl Registers.
 */

#define APBUART_CTRL_RE     0x00000001	/* Receiver enable */
#define APBUART_CTRL_TE     0x00000002	/* Transmitter enable */
#define APBUART_CTRL_RI     0x00000004	/* Receiver interrupt enable */
#define APBUART_CTRL_TI     0x00000008	/* Transmitter interrupt enable */
#define APBUART_CTRL_PS     0x00000010	/* Parity select */
#define APBUART_CTRL_PE     0x00000020	/* Parity enable */
#define APBUART_CTRL_FL     0x00000040	/* Flow control enable */
#define APBUART_CTRL_LB     0x00000080	/* Loop Back enable */
#define APBUART_CTRL_DBG    (1<<11)	/* Debug Bit used by GRMON */

#endif
