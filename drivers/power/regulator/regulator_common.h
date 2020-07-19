// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Disruptive Technologies Research AS
 * Sven Schwermer <sven.svenschwermer@disruptive-technologies.com>
 */

#ifndef _REGULATOR_COMMON_H
#define _REGULATOR_COMMON_H

#include <asm/gpio.h>

struct regulator_common_platdata {
	struct gpio_desc gpio; /* GPIO for regulator enable control */
	unsigned int startup_delay_us;
	unsigned int off_on_delay_us;
};

int regulator_common_ofdata_to_platdata(struct udevice *dev,
	struct regulator_common_platdata *dev_pdata, const char *enable_gpio_name);
int regulator_common_get_enable(const struct udevice *dev,
	struct regulator_common_platdata *dev_pdata);
int regulator_common_set_enable(const struct udevice *dev,
	struct regulator_common_platdata *dev_pdata, bool enable);

#endif /* _REGULATOR_COMMON_H */
