/*
 * fec.h -- Fast Ethernet Controller definitions
 *
 * Some definitions copied from commproc.h for MPC8xx:
 * MPC8xx Communication Processor Module.
 * Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
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
 */

#ifndef	fec_h
#define	fec_h

/* Buffer descriptors used FEC.
*/
typedef struct cpm_buf_desc {
	ushort	cbd_sc;		/* Status and Control */
	ushort	cbd_datlen;	/* Data length in buffer */
	uint	cbd_bufaddr;	/* Buffer address in host memory */
} cbd_t;

#define BD_SC_EMPTY	((ushort)0x8000)	/* Recieve is empty */
#define BD_SC_READY	((ushort)0x8000)	/* Transmit is ready */
#define BD_SC_WRAP	((ushort)0x2000)	/* Last buffer descriptor */
#define BD_SC_INTRPT	((ushort)0x1000)	/* Interrupt on change */
#define BD_SC_LAST	((ushort)0x0800)	/* Last buffer in frame */
#define BD_SC_TC	((ushort)0x0400)	/* Transmit CRC */
#define BD_SC_CM	((ushort)0x0200)	/* Continous mode */
#define BD_SC_ID	((ushort)0x0100)	/* Rec'd too many idles */
#define BD_SC_P		((ushort)0x0100)	/* xmt preamble */
#define BD_SC_BR	((ushort)0x0020)	/* Break received */
#define BD_SC_FR	((ushort)0x0010)	/* Framing error */
#define BD_SC_PR	((ushort)0x0008)	/* Parity error */
#define BD_SC_OV	((ushort)0x0002)	/* Overrun */
#define BD_SC_CD	((ushort)0x0001)	/* Carrier Detect lost */

/* Buffer descriptor control/status used by Ethernet receive.
*/
#define BD_ENET_RX_EMPTY	((ushort)0x8000)
#define BD_ENET_RX_WRAP		((ushort)0x2000)
#define BD_ENET_RX_INTR		((ushort)0x1000)
#define BD_ENET_RX_LAST		((ushort)0x0800)
#define BD_ENET_RX_FIRST	((ushort)0x0400)
#define BD_ENET_RX_MISS		((ushort)0x0100)
#define BD_ENET_RX_LG		((ushort)0x0020)
#define BD_ENET_RX_NO		((ushort)0x0010)
#define BD_ENET_RX_SH		((ushort)0x0008)
#define BD_ENET_RX_CR		((ushort)0x0004)
#define BD_ENET_RX_OV		((ushort)0x0002)
#define BD_ENET_RX_CL		((ushort)0x0001)
#define BD_ENET_RX_STATS	((ushort)0x013f)	/* All status bits */

/* Buffer descriptor control/status used by Ethernet transmit.
*/
#define BD_ENET_TX_READY	((ushort)0x8000)
#define BD_ENET_TX_PAD		((ushort)0x4000)
#define BD_ENET_TX_WRAP		((ushort)0x2000)
#define BD_ENET_TX_INTR		((ushort)0x1000)
#define BD_ENET_TX_LAST		((ushort)0x0800)
#define BD_ENET_TX_TC		((ushort)0x0400)
#define BD_ENET_TX_DEF		((ushort)0x0200)
#define BD_ENET_TX_HB		((ushort)0x0100)
#define BD_ENET_TX_LC		((ushort)0x0080)
#define BD_ENET_TX_RL		((ushort)0x0040)
#define BD_ENET_TX_RCMASK	((ushort)0x003c)
#define BD_ENET_TX_UN		((ushort)0x0002)
#define BD_ENET_TX_CSL		((ushort)0x0001)
#define BD_ENET_TX_STATS	((ushort)0x03ff)	/* All status bits */

#endif	/* fec_h */
