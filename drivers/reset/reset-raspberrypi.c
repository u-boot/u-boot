// SPDX-License-Identifier: GPL-2.0
/*
 * Raspberry Pi 4 firmware reset driver
 *
 * Copyright (C) 2020 Nicolas Saenz Julienne <nsaenzjulienne@suse.de>
 */
#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <asm/arch/msg.h>
#include <dt-bindings/reset/raspberrypi,firmware-reset.h>

static int raspberrypi_reset_request(struct reset_ctl *reset_ctl)
{
	if (reset_ctl->id >= RASPBERRYPI_FIRMWARE_RESET_NUM_IDS)
		return -EINVAL;

	return 0;
}

static int raspberrypi_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int raspberrypi_reset_assert(struct reset_ctl *reset_ctl)
{
	switch (reset_ctl->id) {
	case RASPBERRYPI_FIRMWARE_RESET_ID_USB:
		bcm2711_notify_vl805_reset();
		return 0;
	default:
		return -EINVAL;
	}
}

static int raspberrypi_reset_deassert(struct reset_ctl *reset_ctl)
{
	return 0;
}

struct reset_ops raspberrypi_reset_ops = {
	.request = raspberrypi_reset_request,
	.rfree = raspberrypi_reset_free,
	.rst_assert = raspberrypi_reset_assert,
	.rst_deassert = raspberrypi_reset_deassert,
};

static const struct udevice_id raspberrypi_reset_ids[] = {
	{ .compatible = "raspberrypi,firmware-reset" },
	{ }
};

U_BOOT_DRIVER(raspberrypi_reset) = {
	.name = "raspberrypi-reset",
	.id = UCLASS_RESET,
	.of_match = raspberrypi_reset_ids,
	.ops = &raspberrypi_reset_ops,
};
