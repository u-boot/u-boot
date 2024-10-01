// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <asm/eth.h>
#include <asm/global_data.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

static int sb_lwip_eth_start(struct udevice *dev)
{
	debug("eth_sandbox_lwip: Start\n");

	return 0;
}

static int sb_lwip_eth_send(struct udevice *dev, void *packet, int length)
{
	debug("eth_sandbox_lwip: Send packet %d\n", length);

	return -ENOTSUPP;
}

static int sb_lwip_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	return -EAGAIN;
}

static int sb_lwip_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	return 0;
}

static void sb_lwip_eth_stop(struct udevice *dev)
{
}

static int sb_lwip_eth_write_hwaddr(struct udevice *dev)
{
	return 0;
}

static const struct eth_ops sb_eth_ops = {
	.start			= sb_lwip_eth_start,
	.send			= sb_lwip_eth_send,
	.recv			= sb_lwip_eth_recv,
	.free_pkt		= sb_lwip_eth_free_pkt,
	.stop			= sb_lwip_eth_stop,
	.write_hwaddr		= sb_lwip_eth_write_hwaddr,
};

static int sb_lwip_eth_remove(struct udevice *dev)
{
	return 0;
}

static int sb_lwip_eth_of_to_plat(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id sb_eth_ids[] = {
	{ .compatible = "sandbox,eth" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox) = {
	.name	= "eth_lwip_sandbox",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_ids,
	.of_to_plat = sb_lwip_eth_of_to_plat,
	.remove	= sb_lwip_eth_remove,
	.ops	= &sb_eth_ops,
	.priv_auto	= 0,
	.plat_auto	= sizeof(struct eth_pdata),
};
