/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

static int sb_eth_start(struct udevice *dev)
{
	debug("eth_sandbox: Start\n");

	return 0;
}

static int sb_eth_send(struct udevice *dev, void *packet, int length)
{
	debug("eth_sandbox: Send packet %d\n", length);

	return 0;
}

static int sb_eth_recv(struct udevice *dev, uchar **packetp)
{
	return 0;
}

static void sb_eth_stop(struct udevice *dev)
{
	debug("eth_sandbox: Stop\n");
}

static int sb_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	debug("eth_sandbox %s: Write HW ADDR - %pM\n", dev->name,
	      pdata->enetaddr);
	return 0;
}

static const struct eth_ops sb_eth_ops = {
	.start			= sb_eth_start,
	.send			= sb_eth_send,
	.recv			= sb_eth_recv,
	.stop			= sb_eth_stop,
	.write_hwaddr		= sb_eth_write_hwaddr,
};

static int sb_eth_remove(struct udevice *dev)
{
	return 0;
}

static int sb_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = dev_get_addr(dev);
	return 0;
}

static const struct udevice_id sb_eth_ids[] = {
	{ .compatible = "sandbox,eth" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox) = {
	.name	= "eth_sandbox",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_ids,
	.ofdata_to_platdata = sb_eth_ofdata_to_platdata,
	.remove	= sb_eth_remove,
	.ops	= &sb_eth_ops,
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
