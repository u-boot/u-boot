// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 */

#include <env.h>
#include <misc.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>

/* used when CONFIG_DISPLAY_CPUINFO is activated */
int print_cpuinfo(void)
{
	char name[SOC_NAME_SIZE];

	get_soc_name(name);
	printf("CPU: %s\n", name);

	return 0;
}

int setup_serial_number(void)
{
	char serial_string[25];
	u32 otp[3] = {0, 0, 0 };
	struct udevice *dev;
	int ret;

	if (env_get("serial#"))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_SERIAL),
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	sprintf(serial_string, "%08X%08X%08X", otp[0], otp[1], otp[2]);
	env_set("serial#", serial_string);

	return 0;
}
