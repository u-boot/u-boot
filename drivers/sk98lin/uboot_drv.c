/*
 * Driver for SysKonnect Gigabit Ethernet Server Adapters.
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI) && \
	defined(CONFIG_SK98)

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "u-boot_compat.h"


#define SKGE_MAX_CARDS	2


extern int skge_probe(struct eth_device **);
extern void SkGeIsr(int irq, void *dev_id, struct pt_regs *ptregs);
extern void SkGeIsrOnePort(int irq, void *dev_id, struct pt_regs *ptregs);
extern int SkGeOpen(struct eth_device *);
extern int SkGeClose(struct eth_device *);
extern int SkGeXmit(struct sk_buff *skb, struct eth_device *dev);
extern void ReceiveIrq(SK_AC *pAC, RX_PORT *pRxPort, SK_BOOL SlowPathLock);

static int skge_init(struct eth_device *dev, bd_t * bis);
static int skge_send(struct eth_device *dev, volatile void *packet, int length);
static int skge_recv(struct eth_device *dev);
static void skge_halt(struct eth_device *dev);

int skge_initialize(bd_t * bis)
{
	int numdev, i;
	struct eth_device *dev[SKGE_MAX_CARDS];

	numdev = skge_probe(&dev[0]);

	if (numdev > SKGE_MAX_CARDS)
	{
		printf("ERROR: numdev > SKGE_MAX_CARDS\n");
	}

	for (i = 0; i < numdev; i++)
	{
		sprintf (dev[i]->name, "SK98#%d", i);

		dev[i]->init = skge_init;
		dev[i]->halt = skge_halt;
		dev[i]->send = skge_send;
		dev[i]->recv = skge_recv;

		eth_register(dev[i]);
	}

	return numdev;
}


static int skge_init(struct eth_device *dev, bd_t * bis)
{
	int ret;
	SK_AC * pAC = ((DEV_NET*)dev->priv)->pAC;
	int i;

	ret = SkGeOpen(dev);

	while (pAC->Rlmt.Port[0].PortState != SK_RLMT_PS_GOING_UP)
	{
		SkGeIsrOnePort (0, pAC->dev[0], 0);
	}

	for (i = 0; i < 100; i ++)
	{
		udelay(1000);
	}

	return ret;
}


static void skge_halt(struct eth_device *dev)
{
	SkGeClose(dev);
}


static int skge_send(struct eth_device *dev, volatile void *packet,
						  int length)
{
	int ret = -1;
	struct sk_buff * skb = alloc_skb(length, 0);

	if (! skb)
	{
		printf("skge_send: failed to alloc skb\n");
		goto Done;
	}

	memcpy(skb->data, (void*)packet, length);
	ret = SkGeXmit(skb, dev);

Done:
	return ret;
}


static int skge_recv(struct eth_device *dev)
{
	DEV_NET		*pNet;
	SK_AC		*pAC;
	int		FromPort = 0;

	pNet = (DEV_NET*) dev->priv;
	pAC = pNet->pAC;

	ReceiveIrq(pAC, &pAC->RxPort[FromPort], SK_FALSE);

	return 0;
}


#endif	/* CONFIG_SK98 */
