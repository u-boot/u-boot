/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 * Author(s): Gabriel Fernandez, <gabriel.fernandez@foss.st.com> for STMicroelectronics.
 */

#include <reset-uclass.h>

struct stm32_reset_cfg {
	u16 offset;
	u8 bit_idx;
	bool set_clr;
	bool inverted;
};

struct stm32_reset_data {
	const struct stm32_reset_cfg * (*get_reset_line)(struct reset_ctl *reset_ctl);
	u32 clear_offset;
	u32 reset_us;
};

struct stm32_reset_priv {
	fdt_addr_t base;
	struct stm32_reset_cfg reset_line;
	const struct stm32_reset_data *data;
};

extern const struct reset_ops stm32_reset_ops;

int stm32_reset_core_probe(struct udevice *dev,
			   const struct stm32_reset_data *data);
