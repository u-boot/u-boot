/******************************************************************************
*
*     Author: Xilinx, Inc.
*
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
*     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
*     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
*     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
*     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
*     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
*     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
*     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
*     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
*     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
*     FITNESS FOR A PARTICULAR PURPOSE.
*
*
*     Xilinx hardware products are not intended for use in life support
*     appliances, devices, or systems. Use in such applications is
*     expressly prohibited.
*
*
*     (c) Copyright 2002-2004 Xilinx Inc.
*     All rights reserved.
*
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/

#include <common.h>
#include <net.h>
#include "xparameters.h"
#include "xemac.h"

#if defined(XPAR_EMAC_0_DEVICE_ID)
/*
 * ENET_MAX_MTU and ENET_MAX_MTU_ALIGNED are set from
 * PKTSIZE and PKTSIZE_ALIGN (include/net.h)
 */

#define ENET_MAX_MTU           PKTSIZE
#define ENET_MAX_MTU_ALIGNED   PKTSIZE_ALIGN
#define ENET_ADDR_LENGTH       6

static XEmac Emac;
static char etherrxbuff[PKTSIZE_ALIGN];	/* Receive buffer */

/* hardcoded MAC address for the Xilinx EMAC Core when env is nowhere*/
#ifdef CFG_ENV_IS_NOWHERE
static u8 EMACAddr[ENET_ADDR_LENGTH] = { 0x00, 0x0a, 0x35, 0x00, 0x22, 0x01 };
#endif

static int initialized = 0;

void
eth_halt(void)
{
	if (initialized)
		(void) XEmac_Stop(&Emac);
}

int
eth_init(bd_t * bis)
{
	u32 Options;
	XStatus Result;

#ifdef DEBUG
	printf("EMAC Initialization Started\n\r");
#endif

	Result = XEmac_Initialize(&Emac, XPAR_EMAC_0_DEVICE_ID);
	if (Result != XST_SUCCESS) {
		return 0;
	}

	/* make sure the Emac is stopped before it is started */
	(void) XEmac_Stop(&Emac);

#ifdef CFG_ENV_IS_NOWHERE
	memcpy(bis->bi_enetaddr, EMACAddr, 6);
#endif

	Result = XEmac_SetMacAddress(&Emac, bis->bi_enetaddr);
	if (Result != XST_SUCCESS) {
		return 0;
	}

	Options =
	    (XEM_POLLED_OPTION | XEM_UNICAST_OPTION | XEM_BROADCAST_OPTION |
	     XEM_FDUPLEX_OPTION | XEM_INSERT_FCS_OPTION |
	     XEM_INSERT_PAD_OPTION);
	Result = XEmac_SetOptions(&Emac, Options);
	if (Result != XST_SUCCESS) {
		return 0;
	}

	Result = XEmac_Start(&Emac);
	if (Result != XST_SUCCESS) {
		return 0;
	}
#ifdef DEBUG
	printf("EMAC Initialization complete\n\r");
#endif

	initialized = 1;

	return (0);
}

/*-----------------------------------------------------------------------------+
+-----------------------------------------------------------------------------*/
int
eth_send(volatile void *ptr, int len)
{
	XStatus Result;

	if (len > ENET_MAX_MTU)
		len = ENET_MAX_MTU;

	Result = XEmac_PollSend(&Emac, (u8 *) ptr, len);
	if (Result == XST_SUCCESS) {
		return (1);
	} else {
		printf("Error while sending frame\n\r");
		return (0);
	}

}

int
eth_rx(void)
{
	u32 RecvFrameLength;
	XStatus Result;

	RecvFrameLength = PKTSIZE;
	Result = XEmac_PollRecv(&Emac, (u8 *) etherrxbuff, &RecvFrameLength);
	if (Result == XST_SUCCESS) {
		NetReceive((uchar *)etherrxbuff, RecvFrameLength);
		return (1);
	} else {
		return (0);
	}
}

#endif
