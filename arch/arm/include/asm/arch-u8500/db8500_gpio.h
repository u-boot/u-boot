/*
 * Structures and registers for GPIO access in the Nomadik SoC
 *
 * Code ported from Nomadik GPIO driver in ST-Ericsson Linux kernel code.
 * The purpose is that GPIO config found in kernel should work by simply
 * copy-paste it to U-boot.
 *
 * Ported to U-boot by:
 * Copyright (C) 2010 Joakim Axelsson <joakim.axelsson AT stericsson.com>
 * Copyright (C) 2008 STMicroelectronics
 *     Author: Prafulla WADASKAR <prafulla.wadaskar@st.com>
 * Copyright (C) 2009 Alessandro Rubini <rubini@unipv.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DB8500_GPIO_H__
#define __DB8500_GPIO_H__

/* Alternate functions: function C is set in hw by setting both A and B */
enum db8500_gpio_alt {
	DB8500_GPIO_ALT_GPIO = 0,
	DB8500_GPIO_ALT_A = 1,
	DB8500_GPIO_ALT_B = 2,
	DB8500_GPIO_ALT_C = (DB8500_GPIO_ALT_A | DB8500_GPIO_ALT_B)
};

enum db8500_gpio_pull {
	DB8500_GPIO_PULL_NONE,
	DB8500_GPIO_PULL_UP,
	DB8500_GPIO_PULL_DOWN
};

void db8500_gpio_set_pull(unsigned gpio, enum db8500_gpio_pull pull);
void db8500_gpio_make_input(unsigned gpio);
int db8500_gpio_get_input(unsigned gpio);
void db8500_gpio_make_output(unsigned gpio, int val);
void db8500_gpio_set_output(unsigned gpio, int val);

#endif /* __DB8500_GPIO_H__ */
