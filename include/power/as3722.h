/*
 * Copyright (C) 2014 NVIDIA Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __POWER_AS3722_H__
#define __POWER_AS3722_H__

#include <asm/types.h>

#define AS3722_GPIO_OUTPUT_VDDH (1 << 0)
#define AS3722_GPIO_INVERT (1 << 1)

#define AS3722_DEVICE_ID 0x0c
#define AS3722_SD_VOLTAGE(n) (0x00 + (n))
#define AS3722_LDO_VOLTAGE(n) (0x10 + (n))
#define AS3722_SD_CONTROL 0x4d
#define AS3722_LDO_CONTROL 0x4e
#define AS3722_ASIC_ID1 0x90
#define AS3722_ASIC_ID2 0x91

struct udevice;

int as3722_init(struct udevice **devp);
int as3722_sd_enable(struct udevice *pmic, unsigned int sd);
int as3722_sd_set_voltage(struct udevice *pmic, unsigned int sd, u8 value);
int as3722_ldo_enable(struct udevice *pmic, unsigned int ldo);
int as3722_ldo_set_voltage(struct udevice *pmic, unsigned int ldo, u8 value);
int as3722_gpio_configure(struct udevice *pmic, unsigned int gpio,
			  unsigned long flags);
int as3722_gpio_direction_output(struct udevice *pmic, unsigned int gpio,
				 unsigned int level);
int as3722_read(struct udevice *pmic, u8 reg, u8 *value);
int as3722_write(struct udevice *pmic, u8 reg, u8 value);
int as3722_get(struct udevice **devp);

#endif /* __POWER_AS3722_H__ */
