// SPDX-License-Identifier: GPL-2.0+
/*
 * TI LM74 temperature sensor driver
 *
 * Copyright (C) 2024 CS GROUP France
 *
 */

#include <dm.h>
#include <thermal.h>
#include <spi.h>

static int ti_lm74_get_temp(struct udevice *dev, int *temp)
{
	char buf[2];
	s16 raw;
	int ret;

	ret = dm_spi_claim_bus(dev);
	if (ret)
		return ret;

	ret = dm_spi_xfer(dev, 16, NULL, buf, SPI_XFER_BEGIN | SPI_XFER_END);

	dm_spi_release_bus(dev);
	if (ret)
		return ret;

	raw = ((buf[0] << 8) + buf[1]) >> 3;

	*temp = (((int)raw * 125) + 1000) / 2000;

	return 0;
}

static struct dm_thermal_ops ti_lm74_ops = {
	.get_temp	= ti_lm74_get_temp,
};

static const struct udevice_id of_ti_lm74_match[] = {
	{
		.compatible = "ti,lm74",
	},
	{},
};

U_BOOT_DRIVER(ti_bandgap_thermal) = {
	.name	= "ti_lm74_thermal",
	.id	= UCLASS_THERMAL,
	.ops	= &ti_lm74_ops,
	.of_match = of_ti_lm74_match,
};
