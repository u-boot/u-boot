/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define AXP_GPIO

extern int axp209_set_dcdc2(int mvolt);
extern int axp209_set_dcdc3(int mvolt);
extern int axp209_set_ldo2(int mvolt);
extern int axp209_set_ldo3(int mvolt);
extern int axp209_set_ldo4(int mvolt);
extern int axp209_init(void);
extern int axp209_poweron_by_dc(void);
extern int axp209_power_button(void);

extern int axp_gpio_direction_input(unsigned int pin);
extern int axp_gpio_direction_output(unsigned int pin, unsigned int val);
extern int axp_gpio_get_value(unsigned int pin);
extern int axp_gpio_set_value(unsigned int pin, unsigned int val);
