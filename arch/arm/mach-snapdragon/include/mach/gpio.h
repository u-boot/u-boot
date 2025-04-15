/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Qualcomm common pin control data.
 *
 * Copyright (C) 2023 Linaro Ltd.
 */
#ifndef _QCOM_GPIO_H_
#define _QCOM_GPIO_H_

#include <asm/types.h>
#include <stdbool.h>

struct msm_special_pin_data {
	char *name;

	u32 ctl_reg;
	u32 io_reg;

	unsigned pull_bit:5;
	unsigned drv_bit:5;

	unsigned oe_bit:5;
	unsigned in_bit:5;
	unsigned out_bit:5;
};

struct msm_pin_data {
	int pin_count;
	const unsigned int *pin_offsets;
	unsigned int special_pins_start;
	const struct msm_special_pin_data *special_pins_data;
};

static inline u32 qcom_pin_offset(const unsigned int *offs, unsigned int selector)
{
	u32 out = (selector * 0x1000);

	if (offs)
		return out + offs[selector];

	return out;
}

static inline bool qcom_is_special_pin(const struct msm_pin_data *pindata, unsigned int pin)
{
	return pindata->special_pins_start && pin >= pindata->special_pins_start;
}

struct udevice;

/**
 * msm_pinctrl_is_reserved() - Check if a pin lies in a reserved range
 *
 * @dev: pinctrl device
 * @pin: Pin number
 *
 * Returns: true if pin is reserved, otherwise false
 *
 * Call using dev_get_parent() from the GPIO device, it is a child of
 * the pinctrl device.
 */
bool msm_pinctrl_is_reserved(struct udevice *dev, unsigned int pin);

#endif /* _QCOM_GPIO_H_ */
