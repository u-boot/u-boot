/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

enum axp209_reg {
	AXP209_POWER_STATUS = 0x00,
	AXP209_CHIP_VERSION = 0x03,
	AXP209_DCDC2_VOLTAGE = 0x23,
	AXP209_DCDC3_VOLTAGE = 0x27,
	AXP209_LDO24_VOLTAGE = 0x28,
	AXP209_LDO3_VOLTAGE = 0x29,
	AXP209_IRQ_ENABLE1 = 0x40,
	AXP209_IRQ_ENABLE2 = 0x41,
	AXP209_IRQ_ENABLE3 = 0x42,
	AXP209_IRQ_ENABLE4 = 0x43,
	AXP209_IRQ_ENABLE5 = 0x44,
	AXP209_IRQ_STATUS5 = 0x4c,
	AXP209_SHUTDOWN = 0x32,
	AXP209_GPIO0_CTRL = 0x90,
	AXP209_GPIO1_CTRL = 0x92,
	AXP209_GPIO2_CTRL = 0x93,
	AXP209_GPIO_STATE = 0x94,
	AXP209_GPIO3_CTRL = 0x95,
};

#define AXP209_POWER_STATUS_ON_BY_DC	(1 << 0)
#define AXP209_POWER_STATUS_VBUS_USABLE	(1 << 4)

#define AXP209_IRQ5_PEK_UP		(1 << 6)
#define AXP209_IRQ5_PEK_DOWN		(1 << 5)

#define AXP209_POWEROFF			(1 << 7)

#define AXP209_GPIO_OUTPUT_LOW		0x00 /* Drive pin low */
#define AXP209_GPIO_OUTPUT_HIGH		0x01 /* Drive pin high */
#define AXP209_GPIO_INPUT		0x02 /* Float pin */

/* GPIO3 is different from the others */
#define AXP209_GPIO3_OUTPUT_LOW		0x00 /* Drive pin low, Output mode */
#define AXP209_GPIO3_OUTPUT_HIGH	0x02 /* Float pin, Output mode */
#define AXP209_GPIO3_INPUT		0x06 /* Float pin, Input mode */

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
