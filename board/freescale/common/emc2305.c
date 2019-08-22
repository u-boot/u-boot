// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/io.h>

#include "emc2305.h"

DECLARE_GLOBAL_DATA_PTR;

void set_fan_speed(u8 data)
{
	u8 index;
	u8 Fan[NUM_OF_FANS] = {I2C_EMC2305_FAN1,
			       I2C_EMC2305_FAN2,
			       I2C_EMC2305_FAN3,
			       I2C_EMC2305_FAN4,
			       I2C_EMC2305_FAN5};

	for (index = 0; index < NUM_OF_FANS; index++) {
#ifndef CONFIG_DM_I2C
		if (i2c_write(I2C_EMC2305_ADDR, Fan[index], 1, &data, 1) != 0) {
			printf("Error: failed to change fan speed @%x\n",
			       Fan[index]);
		}
#else
		struct udevice *dev;

		if (i2c_get_chip_for_busnum(0, I2C_EMC2305_ADDR, 1, &dev))
			continue;

		if (dm_i2c_write(dev, Fan[index], &data, 1) != 0) {
			printf("Error: failed to change fan speed @%x\n",
			       Fan[index]);
		}
#endif
	}
}

void emc2305_init(void)
{
	u8 data;

	data = I2C_EMC2305_CMD;
#ifndef CONFIG_DM_I2C
	if (i2c_write(I2C_EMC2305_ADDR, I2C_EMC2305_CONF, 1, &data, 1) != 0)
		printf("Error: failed to configure EMC2305\n");
#else
	struct udevice *dev;

	if (!i2c_get_chip_for_busnum(0, I2C_EMC2305_ADDR, 1, &dev))
		if (dm_i2c_write(dev, I2C_EMC2305_CONF, &data, 1))
			printf("Error: failed to configure EMC2305\n");
#endif

}
