// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <asm/arch-mvebu/thermal.h>
#include <fdtdec.h>
#include <thermal.h>
#include <dm/device-internal.h>

int thermal_sensor_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct udevice *dev;
	struct uclass *uc;
	int ret, temperature;

	ret = uclass_get(UCLASS_THERMAL, &uc);

	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		struct thermal_unit_config *thermal_cfg;

		ret = device_probe(dev);
		if (ret)
			continue;

		thermal_cfg = dev_get_priv(dev);
		thermal_get_temp(dev, &temperature);
		printf("Thermal.%8p = %d\n", thermal_cfg->regs_base,
		       temperature);
	}

	return 0;
}

U_BOOT_CMD(
	tsen, 1, 1, thermal_sensor_cmd,
	"tsen - Display the SoC temperature.\n",
	"\n\tDisplay the SoC temperature as read from the on chip thermal sensor.\n"
);
