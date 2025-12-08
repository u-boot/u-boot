// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2025, Renesas Electronics Corporation.
 */

#include <asm/io.h>
#include <dm.h>
#include <linux/delay.h>
#include <mailbox-uclass.h>

#define COM		0x0
#define IIR		BIT(0)

struct mfis_priv {
	void __iomem	*tx_base;
};

static int mfis_send(struct mbox_chan *chan, const void *data)
{
	struct mfis_priv *mfis = dev_get_priv(chan->dev);

	writel(IIR, mfis->tx_base + COM);

	/* Give the remote side some time. */
	mdelay(1);

	writel(0, mfis->tx_base + COM);

	return 0;
}

struct mbox_ops mfis_mbox_ops = {
	.send		= mfis_send,
};

static int mfis_mbox_probe(struct udevice *dev)
{
	struct mfis_priv *mbox = dev_get_priv(dev);

	mbox->tx_base = dev_read_addr_index_ptr(dev, 0);
	if (!mbox->tx_base)
		return -ENODEV;

	return 0;
}

static const struct udevice_id mfis_mbox_of_match[] = {
	{ .compatible = "renesas,mfis-mbox", },
	{},
};

U_BOOT_DRIVER(renesas_mfis) = {
	.name		= "renesas-mfis",
	.id		= UCLASS_MAILBOX,
	.of_match	= mfis_mbox_of_match,
	.probe		= mfis_mbox_probe,
	.priv_auto	= sizeof(struct mfis_priv),
	.ops		= &mfis_mbox_ops,
};
