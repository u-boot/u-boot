// SPDX-License-Identifier: GPL-2.0

#include <dm.h>

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
