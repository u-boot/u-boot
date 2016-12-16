/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm/device.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id at91_sckc_match[] = {
	{ .compatible = "atmel,at91sam9x5-sckc" },
	{}
};

U_BOOT_DRIVER(at91_sckc) = {
	.name = "at91-sckc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = at91_sckc_match,
};
