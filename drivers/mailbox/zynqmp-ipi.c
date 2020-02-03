// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx Zynq MPSoC Mailbox driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <dm/device_compat.h>
#include <mach/sys_proto.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <wait_bit.h>

/* IPI bitmasks, register base */
/* TODO: move reg base to DT */
#define IPI_BIT_MASK_PMU0     0x10000
#define IPI_INT_REG_BASE_APU  0xFF300000

struct ipi_int_regs {
	u32 trig; /* 0x0  */
	u32 obs;  /* 0x4  */
	u32 ist;  /* 0x8  */
	u32 imr;  /* 0xC  */
	u32 ier;  /* 0x10 */
	u32 idr;  /* 0x14 */
};

#define ipi_int_apu ((struct ipi_int_regs *)IPI_INT_REG_BASE_APU)

struct zynqmp_ipi {
	void __iomem *local_req_regs;
	void __iomem *local_res_regs;
	void __iomem *remote_req_regs;
	void __iomem *remote_res_regs;
};

static int zynqmp_ipi_send(struct mbox_chan *chan, const void *data)
{
	const struct zynqmp_ipi_msg *msg = (struct zynqmp_ipi_msg *)data;
	struct zynqmp_ipi *zynqmp = dev_get_priv(chan->dev);
	u32 ret;
	u32 *mbx = (u32 *)zynqmp->local_req_regs;

	for (size_t i = 0; i < msg->len; i++)
		writel(msg->buf[i], &mbx[i]);

	/* Write trigger interrupt */
	writel(IPI_BIT_MASK_PMU0, &ipi_int_apu->trig);

	/* Wait until observation bit is cleared */
	ret = wait_for_bit_le32(&ipi_int_apu->obs, IPI_BIT_MASK_PMU0, false,
				100, false);

	debug("%s, send %ld bytes\n", __func__, msg->len);
	return ret;
};

static int zynqmp_ipi_recv(struct mbox_chan *chan, void *data)
{
	struct zynqmp_ipi_msg *msg = (struct zynqmp_ipi_msg *)data;
	struct zynqmp_ipi *zynqmp = dev_get_priv(chan->dev);
	u32 *mbx = (u32 *)zynqmp->local_res_regs;

	for (size_t i = 0; i < msg->len; i++)
		msg->buf[i] = readl(&mbx[i]);

	debug("%s, recv %ld bytes\n", __func__, msg->len);
	return 0;
};

static int zynqmp_ipi_probe(struct udevice *dev)
{
	struct zynqmp_ipi *zynqmp = dev_get_priv(dev);
	struct resource res;
	ofnode node;

	debug("%s(dev=%p)\n", __func__, dev);

	/* Get subnode where the regs are defined */
	/* Note IPI mailbox node needs to be the first one in DT */
	node = ofnode_first_subnode(dev_ofnode(dev));

	if (ofnode_read_resource_byname(node, "local_request_region", &res)) {
		dev_err(dev, "No reg property for local_request_region\n");
		return -EINVAL;
	};
	zynqmp->local_req_regs = devm_ioremap(dev, res.start,
					      (res.start - res.end));

	if (ofnode_read_resource_byname(node, "local_response_region", &res)) {
		dev_err(dev, "No reg property for local_response_region\n");
		return -EINVAL;
	};
	zynqmp->local_res_regs = devm_ioremap(dev, res.start,
					      (res.start - res.end));

	if (ofnode_read_resource_byname(node, "remote_request_region", &res)) {
		dev_err(dev, "No reg property for remote_request_region\n");
		return -EINVAL;
	};
	zynqmp->remote_req_regs = devm_ioremap(dev, res.start,
					       (res.start - res.end));

	if (ofnode_read_resource_byname(node, "remote_response_region", &res)) {
		dev_err(dev, "No reg property for remote_response_region\n");
		return -EINVAL;
	};
	zynqmp->remote_res_regs = devm_ioremap(dev, res.start,
					       (res.start - res.end));

	return 0;
};

static const struct udevice_id zynqmp_ipi_ids[] = {
	{ .compatible = "xlnx,zynqmp-ipi-mailbox" },
	{ }
};

struct mbox_ops zynqmp_ipi_mbox_ops = {
	.send = zynqmp_ipi_send,
	.recv = zynqmp_ipi_recv,
};

U_BOOT_DRIVER(zynqmp_ipi) = {
	.name = "zynqmp-ipi",
	.id = UCLASS_MAILBOX,
	.of_match = zynqmp_ipi_ids,
	.probe = zynqmp_ipi_probe,
	.priv_auto_alloc_size = sizeof(struct zynqmp_ipi),
	.ops = &zynqmp_ipi_mbox_ops,
};
