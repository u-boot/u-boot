// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2021
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

/* T20 Transformers derive from Ventana board */

#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/delay.h>

#define TPS6586X_I2C_ADDRESS	0x34
#define TPS6586X_SUPPLYENE	0x14
#define   EXITSLREQ_BIT		BIT(1)
#define   SLEEP_MODE_BIT	BIT(3)

#ifdef CONFIG_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp,
		int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	uchar data_buffer[1];
	int ret;

	ret = i2c_get_chip_for_busnum(0, TPS6586X_I2C_ADDRESS, 1, &dev);
	if (ret) {
		log_debug("cannot find PMIC I2C chip\n");
		return 0;
	}

	ret = dm_i2c_read(dev, TPS6586X_SUPPLYENE, data_buffer, 1);
	if (ret)
		return ret;

	data_buffer[0] &= ~EXITSLREQ_BIT;

	ret = dm_i2c_write(dev, TPS6586X_SUPPLYENE, data_buffer, 1);
	if (ret)
		return ret;

	data_buffer[0] |= SLEEP_MODE_BIT;

	ret = dm_i2c_write(dev, TPS6586X_SUPPLYENE, data_buffer, 1);
	if (ret)
		return ret;

	// wait some time and then print error
	mdelay(5000);
	printf("Failed to power off!!!\n");
	return 1;
}
#endif
