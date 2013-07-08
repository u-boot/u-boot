/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _TEGRA_GPIO_H_
#define _TEGRA_GPIO_H_

#define MAX_NUM_GPIOS           (TEGRA_GPIO_PORTS * TEGRA_GPIO_BANKS * 8)
#define GPIO_NAME_SIZE		20	/* gpio_request max label len */

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_FULLPORT(x)	((x) >> 3)
#define GPIO_BIT(x)		((x) & 0x7)

/*
 * Tegra-specific GPIO API
 */

void gpio_info(void);

#define gpio_status()	gpio_info()
#endif	/* TEGRA_GPIO_H_ */
