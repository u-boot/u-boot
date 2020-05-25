// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <common.h>
#include <init.h>
#include <asm/arch/sys_proto.h>
#include "../common/stpmic1.h"

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_PMIC_STPMIC1) && CONFIG_IS_ENABLED(POWER_SUPPORT))
		stpmic1_init();

	return 0;
}
