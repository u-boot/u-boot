// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Enforce Nodebox v3 FAN-daughterboard enable.
 *
 * Note: it tequires CONFIG_DM_REGULATOR_FIXED
 */

#include <dm.h>
#include <event.h>
#include <power/regulator.h>

/* `regulator-name = "+3V3_FAN"` in the nbxv3 DTS. */
#define NBXV3_FAN_REGULATOR_NAME	"+3V3_FAN"

static int nbxv3_fan_power_last_stage_init(void)
{
	struct udevice *reg = NULL;
	int ret;

	if (regulator_get_by_platname(NBXV3_FAN_REGULATOR_NAME, &reg) || !reg)
		return 0;

	ret = regulator_set_enable_if_allowed(reg, true);
	if (ret)
		printf("nbxv3: %s enable failed (%d)\n",
		       NBXV3_FAN_REGULATOR_NAME, ret);

	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, nbxv3_fan_power_last_stage_init);
