// SPDX-License-Identifier: GPL-2.0-or-later

#include <dm.h>

static const struct udevice_id uniphier_system_bus_match[] = {
	{ .compatible = "socionext,uniphier-system-bus" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_system_bus_driver) = {
	.name	= "uniphier-system-bus",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = uniphier_system_bus_match,
};
