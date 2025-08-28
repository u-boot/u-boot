// SPDX-License-Identifier: GPL-2.0
/*
 * Microchip's PolarFire SoC (MPFS) Mailbox Driver
 *
 * Copyright (C) 2024 Microchip Technology Inc. All rights reserved.
 *
 * Author: Jamie Gibbons <jamie.gibbons@microchip.com>
 *
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/ofnode.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <log.h>
#include <mailbox-uclass.h>
#include <malloc.h>
#include <mpfs-mailbox.h>

#define SERVICES_CR_OFFSET 0x50u
#define SERVICES_SR_OFFSET 0x54u

#define SERVICE_CR_REQ_MASK 0x1u
#define SERVICE_SR_BUSY_MASK 0x2u
#define SERVICE_SR_STATUS_SHIFT 16
#define SERVICE_CR_COMMAND_SHIFT 16
#define MASK_8BIT 0xFF

struct mpfs_mbox {
	struct udevice *dev;
	void __iomem *ctrl_base;
	void __iomem *mbox_base;
	struct mbox_chan *chan;
};

static bool mpfs_mbox_busy(struct mbox_chan *chan)
{
	struct mpfs_mbox *mbox = dev_get_priv(chan->dev);
	uint16_t status;

	status = readl(mbox->ctrl_base + SERVICES_SR_OFFSET);

	return status & SERVICE_SR_BUSY_MASK;
}

static int mpfs_mbox_send(struct mbox_chan *chan, const void *data)
{
	struct mpfs_mbox *mbox = dev_get_priv(chan->dev);
	struct mpfs_mss_msg *msg = (struct mpfs_mss_msg *)data;
	u32 mailbox_val, cmd_shifted, value;
	u8 *byte_buf;
	u8 idx, byte_idx, byte_offset;

	u32 *word_buf = (u32 *)msg->cmd_data;

	if (mpfs_mbox_busy(chan))
		return -EBUSY;

	for (idx = 0; idx < (msg->cmd_data_size / BYTES_4); idx++)
		writel(word_buf[idx], mbox->mbox_base + msg->mbox_offset + idx * BYTES_4);

	if ((msg->cmd_data_size % BYTES_4) > 0) {
		byte_offset = (msg->cmd_data_size / BYTES_4) * BYTES_4;
		byte_buf = (u8 *)(msg->cmd_data + byte_offset);
		mailbox_val = readl(mbox->mbox_base + msg->mbox_offset + idx * BYTES_4);

		for (byte_idx = 0; byte_idx < (msg->cmd_data_size % BYTES_4); byte_idx++) {
			mailbox_val &= ~(MASK_8BIT << (byte_idx * 0x8u));
			mailbox_val |= (u32)byte_buf[byte_idx] << (byte_idx * 0x8u);
		}
		writel(mailbox_val, mbox->mbox_base + msg->mbox_offset + idx * BYTES_4);
	}

	cmd_shifted = msg->cmd_opcode << SERVICE_CR_COMMAND_SHIFT;
	cmd_shifted |= SERVICE_CR_REQ_MASK;
	writel(cmd_shifted, mbox->ctrl_base + SERVICES_CR_OFFSET);

	do {
		value = readl(mbox->ctrl_base + SERVICES_CR_OFFSET);
	} while (SERVICE_CR_REQ_MASK == (value & SERVICE_CR_REQ_MASK));

	do {
		value = readl(mbox->ctrl_base + SERVICES_SR_OFFSET);
	} while (SERVICE_SR_BUSY_MASK == (value & SERVICE_SR_BUSY_MASK));

	msg->response->resp_status = (value >> SERVICE_SR_STATUS_SHIFT);
	if (msg->response->resp_status)
		return -EBADMSG;

	return 0;
}

static int mpfs_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct mpfs_mbox *mbox = dev_get_priv(chan->dev);
	struct mpfs_mss_msg *msg = data;
	struct mpfs_mss_response *response = msg->response;
	u8 idx;

	if (!response->resp_msg) {
		dev_err(chan->dev, "failed to assign memory for response %d\n", -ENOMEM);
		return -EINVAL;
	}

	if (mpfs_mbox_busy(chan)) {
		dev_err(chan->dev, "mailbox is busy\n");
		response->resp_status = 0xDEAD;
		return -EINVAL;
	}

	for (idx = 0; idx < response->resp_size; idx++)
		*((u8 *)(response->resp_msg) + idx) = readb(mbox->mbox_base + msg->resp_offset  + idx);

	return 0;
}

static const struct mbox_ops mpfs_mbox_ops = {
	.send = mpfs_mbox_send,
	.recv = mpfs_mbox_recv,
};

static int mpfs_mbox_probe(struct udevice *dev)
{
	struct mpfs_mbox *mbox;
	struct resource regs;
	ofnode node;
	int ret;

	node = dev_ofnode(dev);

	mbox = devm_kzalloc(dev, sizeof(*mbox), GFP_KERNEL);
	if (!mbox)
		return -ENOMEM;

	ret = ofnode_read_resource(node, 0, &regs);
	if (ret) {
		dev_err(dev, "No reg property for controller base\n");
		return ret;
	};

	mbox->ctrl_base = devm_ioremap(dev, regs.start, regs.start - regs.end);

	ret = ofnode_read_resource(node, 2, &regs);
	if (ret) {
		dev_err(dev, "No reg property for mailbox base\n");
		return ret;
	};

	mbox->mbox_base = devm_ioremap(dev, regs.start, regs.start - regs.end);

	mbox->dev = dev;
	dev_set_priv(dev, mbox);
	mbox->chan->con_priv = mbox;

	return 0;
}

static const struct udevice_id mpfs_mbox_ids[] = {
	{.compatible = "microchip,mpfs-mailbox"},
	{ }
};

U_BOOT_DRIVER(mpfs_mbox) = {
	.name = "mpfs-mbox",
	.id = UCLASS_MAILBOX,
	.of_match = mpfs_mbox_ids,
	.probe = mpfs_mbox_probe,
	.priv_auto = sizeof(struct mpfs_mbox),
	.ops = &mpfs_mbox_ops,
};
