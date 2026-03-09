// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2024 - Analog Devices, Inc.
 *
 * Written by Timesys Corporation
 *
 */

#include <dm.h>
#include <syscon.h>

static const struct udevice_id adi_syscon_ids[] = {
	{ .compatible = "adi,reset-controller" },
	{ }
};

U_BOOT_DRIVER(syscon_sc5xx_rcu) = {
	.name = "sc5xx_rcu",
	.id = UCLASS_SYSCON,
	.of_match = adi_syscon_ids,
};
