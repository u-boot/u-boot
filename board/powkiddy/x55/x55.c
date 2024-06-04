// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Chris Morgan <macromorgan@hotmail.com>
 */

#include <asm/io.h>

#define GPIO4_BASE		0xfe770000
#define GPIO_SWPORT_DR_L	0x0000
#define GPIO_SWPORT_DDR_L	0x0008
#define GPIO_B4			BIT(12)
#define GPIO_B5			BIT(13)
#define GPIO_B6			BIT(14)

#define GPIO_WRITEMASK(bits)	((bits) << 16)

/*
 * Start LED very early so user knows device is on. Set color
 * to red.
 */
void spl_board_init(void)
{
	/* Set GPIO4_B4, GPIO4_B5, and GPIO4_B6 to output. */
	writel(GPIO_WRITEMASK(GPIO_B6 | GPIO_B5 | GPIO_B4) | \
	       (GPIO_B6 | GPIO_B5 | GPIO_B4),
	       (GPIO4_BASE + GPIO_SWPORT_DDR_L));
	/* Set GPIO4_B5 and GPIO4_B6 to 0 and GPIO4_B4 to 1. */
	writel(GPIO_WRITEMASK(GPIO_B6 | GPIO_B5 | GPIO_B4) | GPIO_B4,
	       (GPIO4_BASE + GPIO_SWPORT_DR_L));
}

int rk_board_late_init(void)
{
	/* Turn off red LED and turn on orange LED. */
	writel(GPIO_WRITEMASK(GPIO_B6 | GPIO_B5 | GPIO_B4) | GPIO_B6,
	       (GPIO4_BASE + GPIO_SWPORT_DR_L));

	return 0;
}
