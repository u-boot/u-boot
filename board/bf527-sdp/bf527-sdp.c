/*
 * U-Boot - main board file
 *
 * Copyright (c) 2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>
#include <asm/gpio.h>
#include <asm/mach-common/bits/pll.h>

int checkboard(void)
{
	printf("Board: ADI BF527 SDP board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");

	/* Enable access to parallel flash */
	gpio_request(GPIO_PG0, "parallel-flash");
	gpio_direction_output(GPIO_PG0, 0);

	return 0;
}

int misc_init_r(void)
{
	/* CLKIN Buffer Output Enable */
	bfin_write_VR_CTL(bfin_read_VR_CTL() | CLKBUFOE);

	return 0;
}
