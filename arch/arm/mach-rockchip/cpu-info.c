// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * (C) Copyright 2019 Amarula Solutions(India)
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>

int print_cpuinfo(void)
{
	printf("SoC: Rockchip %s\n", CONFIG_SYS_SOC);

	/* TODO print operating temparature and clock */

	return 0;
}
