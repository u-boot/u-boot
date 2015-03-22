/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <asm/eth-raw-os.h>
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;


static int sb_eth_raw_start(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *interface;

	debug("eth_sandbox_raw: Start\n");

	interface = fdt_getprop(gd->fdt_blob, dev->of_offset,
					    "host-raw-interface", NULL);
	if (interface == NULL)
		return -EINVAL;

	return sandbox_eth_raw_os_start(interface, pdata->enetaddr, priv);
}

static int sb_eth_raw_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Send packet %d\n", length);

	return sandbox_eth_raw_os_send(packet, length, priv);
}

static int sb_eth_raw_recv(struct udevice *dev, uchar **packetp)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	int retval;
	int length;

	retval = sandbox_eth_raw_os_recv(net_rx_packets[0], &length, priv);

	if (!retval && length) {
		debug("eth_sandbox_raw: received packet %d\n",
		      length);
		*packetp = net_rx_packets[0];
		return length;
	}
	return retval;
}

static void sb_eth_raw_stop(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Stop\n");

	sandbox_eth_raw_os_stop(priv);
}

static const struct eth_ops sb_eth_raw_ops = {
	.start			= sb_eth_raw_start,
	.send			= sb_eth_raw_send,
	.recv			= sb_eth_raw_recv,
	.stop			= sb_eth_raw_stop,
};

static int sb_eth_raw_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = dev_get_addr(dev);
	return 0;
}

static const struct udevice_id sb_eth_raw_ids[] = {
	{ .compatible = "sandbox,eth-raw" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox_raw) = {
	.name	= "eth_sandbox_raw",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_raw_ids,
	.ofdata_to_platdata = sb_eth_raw_ofdata_to_platdata,
	.ops	= &sb_eth_raw_ops,
	.priv_auto_alloc_size = sizeof(struct eth_sandbox_raw_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
