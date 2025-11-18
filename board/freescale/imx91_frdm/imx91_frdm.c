// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <env.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <dm.h>

#define TCPC_ALERT 0x10
#define TCPC_ALERT_MASK 0x12
#define TCPC_FAULT_STATUS_MASK 0x15
#define USB_I2C_BUS 2
#define USB_I2C_ADDR 0x50

static int clear_pd_alert(void)
{
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;
	u8 buffer_0[2] = {0, 0};
	u8 buffer_1[2] = {0xff, 0xff};

	ret = uclass_get_device_by_seq(UCLASS_I2C, USB_I2C_BUS, &bus);
	if (ret) {
		printf("Failed to get I2C bus %d\n", USB_I2C_BUS);
		return ret;
	}

		ret = dm_i2c_probe(bus, USB_I2C_ADDR, 0, &i2c_dev);
		if (ret)
			printf("Can't find USB PD device at 0x%02x\n", USB_I2C_ADDR);

		/* Mask all alert status*/
		ret = dm_i2c_write(i2c_dev, TCPC_ALERT_MASK, buffer_0, 2);
		if (ret) {
			printf("%s dm_i2c_write failed: %d\n", __func__, ret);
			return 0;
		}

		ret = dm_i2c_write(i2c_dev, TCPC_FAULT_STATUS_MASK, buffer_0, 2);
		if (ret) {
			printf("%s dm_i2c_write failed: %d\n", __func__, ret);
			return 0;
		}

		ret = dm_i2c_write(i2c_dev, TCPC_ALERT, buffer_1, 2);
		if (ret) {
			printf("%s dm_i2c_write failed: %d\n", __func__, ret);
			return 0;
		}

	return 0;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_IS_IN_MMC))
		board_late_mmc_env_init();

	env_set("sec_boot", "no");

	if (IS_ENABLED(CONFIG_AHAB_BOOT))
		env_set("sec_boot", "yes");

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "11X11_FRDM");
		env_set("board_rev", "iMX91");
	}

	clear_pd_alert();
	return 0;
}
