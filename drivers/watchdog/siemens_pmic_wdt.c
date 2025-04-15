// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for a PMIC watchdog timer controlled via Siemens SCU firmware
 * extensions. Only useful on some Siemens i.MX8-based platforms as
 * special NXP SCFW is needed which provides the needed SCU API.
 *
 * Copyright (C) 2024 Siemens AG
 */

#include <dm.h>
#include <wdt.h>
#include <firmware/imx/sci/sci.h>

/* watchdog commands */
#define CMD_START_WDT 0x55
#define CMD_STOP_WDT  0x45
#define CMD_PING_WDT  0x35

static int scu_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	/* start external watchdog via Timer API */
	return sc_timer_control_siemens_pmic_wdog(-1, CMD_START_WDT);
}

static int scu_wdt_stop(struct udevice *dev)
{
	/* stop external watchdog via Timer API */
	return sc_timer_control_siemens_pmic_wdog(-1, CMD_STOP_WDT);
}

static int scu_wdt_reset(struct udevice *dev)
{
	return sc_timer_control_siemens_pmic_wdog(-1, CMD_PING_WDT);
}

static int scu_wdt_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);
	return 0;
}

static const struct wdt_ops scu_wdt_ops = {
	.reset		= scu_wdt_reset,
	.start		= scu_wdt_start,
	.stop		= scu_wdt_stop,
};

static const struct udevice_id scu_wdt_ids[] = {
	{ .compatible = "siemens,scu-wdt" },
	{ }
};

U_BOOT_DRIVER(scu_wdt) = {
	.name		= "scu_wdt",
	.id		= UCLASS_WDT,
	.of_match	= scu_wdt_ids,
	.probe		= scu_wdt_probe,
	.ops		= &scu_wdt_ops,
};
