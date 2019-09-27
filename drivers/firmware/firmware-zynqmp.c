// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Zynq MPSoC Firmware driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>

#if defined(CONFIG_ZYNQMP_IPI)
#include <mailbox.h>
#include <asm/arch/sys_proto.h>

struct zynqmp_power {
	struct mbox_chan tx_chan;
	struct mbox_chan rx_chan;
} zynqmp_power;

static int zynqmp_power_probe(struct udevice *dev)
{
	int ret = 0;

	debug("%s, (dev=%p)\n", __func__, dev);

	ret = mbox_get_by_name(dev, "tx", &zynqmp_power.tx_chan);
	if (ret) {
		debug("%s, cannot tx mailbox\n", __func__);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &zynqmp_power.rx_chan);
	if (ret)
		debug("%s, cannot rx mailbox\n", __func__);

	return ret;
};

static const struct udevice_id zynqmp_power_ids[] = {
	{ .compatible = "xlnx,zynqmp-power" },
	{ }
};

U_BOOT_DRIVER(zynqmp_power) = {
	.name = "zynqmp_power",
	.id = UCLASS_FIRMWARE,
	.of_match = zynqmp_power_ids,
	.probe = zynqmp_power_probe,
};
#endif

static const struct udevice_id zynqmp_firmware_ids[] = {
	{ .compatible = "xlnx,zynqmp-firmware" },
	{ .compatible = "xlnx,versal-firmware"},
	{ }
};

U_BOOT_DRIVER(zynqmp_firmware) = {
	.id = UCLASS_FIRMWARE,
	.name = "zynqmp-firmware",
	.probe = dm_scan_fdt_dev,
	.of_match = zynqmp_firmware_ids,
};
