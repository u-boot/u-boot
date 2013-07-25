/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * FIFO sub-controller
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * CREDITS: tsec driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [F]:	[0]/ip_documentation/xps_ll_fifo.pdf
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */

#include <config.h>
#include <common.h>
#include <net.h>

#include <asm/types.h>
#include <asm/io.h>

#include "xilinx_ll_temac.h"
#include "xilinx_ll_temac_fifo.h"

int ll_temac_reset_fifo(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct fifo_ctrl *fifo_ctrl = (void *)ll_temac->ctrladdr;

	out_be32(&fifo_ctrl->tdfr, LL_FIFO_TDFR_KEY);
	out_be32(&fifo_ctrl->rdfr, LL_FIFO_RDFR_KEY);
	out_be32(&fifo_ctrl->isr, ~0UL);
	out_be32(&fifo_ctrl->ier, 0);

	return 0;
}

int ll_temac_recv_fifo(struct eth_device *dev)
{
	int i, length = 0;
	u32 *buf = (u32 *)NetRxPackets[0];
	struct ll_temac *ll_temac = dev->priv;
	struct fifo_ctrl *fifo_ctrl = (void *)ll_temac->ctrladdr;

	if (in_be32(&fifo_ctrl->isr) & LL_FIFO_ISR_RC) {

		/* reset isr */
		out_be32(&fifo_ctrl->isr, ~0UL);

		/*
		 * MAYBE here:
		 *   while (fifo_ctrl->isr);
		 */

		/*
		 * The length is written (into RLR) by the XPS LL FIFO
		 * when the packet is received across the RX LocalLink
		 * interface and the receive data FIFO had enough
		 * locations that all of the packet data has been saved.
		 * The RLR should only be read when a receive packet is
		 * available for processing (the receive occupancy is
		 * not zero). Once the RLR is read, the receive packet
		 * data should be read from the receive data FIFO before
		 * the RLR is read again.
		 *
		 * [F] page 17, Receive Length Register (RLR)
		 */
		if (in_be32(&fifo_ctrl->rdfo) & LL_FIFO_RDFO_MASK) {
			length = in_be32(&fifo_ctrl->rlf) & LL_FIFO_RLF_MASK;
		} else {
			printf("%s: Got error, no receive occupancy\n",
					__func__);
			return -1;
		}

		if (length > PKTSIZE_ALIGN) {
			printf("%s: Got error, receive package too big (%i)\n",
					__func__, length);
			ll_temac_reset_fifo(dev);
			return -1;
		}

		for (i = 0; i < length; i += 4)
			*buf++ = in_be32(&fifo_ctrl->rdfd);

		NetReceive(NetRxPackets[0], length);
	}

	return 0;
}

int ll_temac_send_fifo(struct eth_device *dev, void *packet, int length)
{
	int i;
	u32 *buf = (u32 *)packet;
	struct ll_temac *ll_temac = dev->priv;
	struct fifo_ctrl *fifo_ctrl = (void *)ll_temac->ctrladdr;

	if (length < LL_FIFO_TLF_MIN) {
		printf("%s: Got error, transmit package too small (%i)\n",
				__func__, length);
		return -1;
	}

	if (length > LL_FIFO_TLF_MAX) {
		printf("%s: Got error, transmit package too big (%i)\n",
				__func__, length);
		return -1;
	}

	for (i = 0; i < length; i += 4)
		out_be32(&fifo_ctrl->tdfd, *buf++);

	/*
	 * Once the packet length is written to the TLR it is
	 * automatically moved to the transmit data FIFO with
	 * the packet data freeing up the TLR for another value.
	 * The packet length must be written to the TLR after
	 * the packet data is written to the transmit data FIFO.
	 * It is not valid to write data for multiple packets
	 * to the transmit data FIFO before writing the packet
	 * length values.
	 *
	 * [F] page 17, Transmit Length Register (TLR)
	 */
	out_be32(&fifo_ctrl->tlf, length);

	return 0;
}
