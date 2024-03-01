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

struct msm_pin_data {
	int pin_count;
	const unsigned int *pin_offsets;
	/* Index of first special pin, these are ignored for now */
	unsigned int special_pins_start;
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

#endif /* _QCOM_GPIO_H_ */
