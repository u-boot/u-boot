// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <common.h>
#include <asm/arch/sys_proto.h>
#include "../common/stpmic1.h"

/* board early initialisation in board_f: need to use global variable */
static u32 opp_voltage_mv __section(".data");

void board_vddcore_init(u32 voltage_mv)
{
	if (IS_ENABLED(CONFIG_PMIC_STPMIC1) && CONFIG_IS_ENABLED(POWER))
		opp_voltage_mv = voltage_mv;
}

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_PMIC_STPMIC1) && CONFIG_IS_ENABLED(POWER))
		stpmic1_init(opp_voltage_mv);

	return 0;
}

