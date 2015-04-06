/*
 * Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This file is a driver for Parade dP<->LVDS bridges. The original submission
 * is for the ps8625 chip.
 */
#include <config.h>
#include <common.h>
#include <i2c.h>
#include <fdtdec.h>
#include <asm/gpio.h>

/*
 * Initialization of the chip is a process of writing certaing values into
 * certain registers over i2c bus. The chip in fact responds to a range of
 * addresses on the i2c bus, so for each written value three parameters are
 * required: i2c address, register address and the actual value.
 *
 * The base address is derived from the device tree, only address offset is
 * stored in the table below.
 */
/**
 * struct reg_data() - data for a parade register write
 *
 * @addr_off        offset from the i2c base address for parade
 * @reg_addr        register address to write
 * @value           value to be written
 */
struct reg_data {
	uint8_t addr_off;
	uint8_t reg;
	uint8_t value;
} _packed;

#define END_OF_TABLE 0xff /* Ficticious offset */

static const struct reg_data parade_values[] = {
	{0x02, 0xa1, 0x01},  /* HPD low */
	 /*
	  * SW setting
	  * [1:0] SW output 1.2V voltage is lower to 96%
	  */
	{0x04, 0x14, 0x01},
	 /*
	  * RCO SS setting
	  * [5:4] = b01 0.5%, b10 1%, b11 1.5%
	  */
	{0x04, 0xe3, 0x20},
	{0x04, 0xe2, 0x80}, /* [7] RCO SS enable */
	 /*
	  *  RPHY Setting
	  * [3:2] CDR tune wait cycle before
	  * measure for fine tune b00: 1us,
	  * 01: 0.5us, 10:2us, 11:4us.
	  */
	{0x04, 0x8a, 0x0c},
	{0x04, 0x89, 0x08}, /* [3] RFD always on */
	 /*
	  * CTN lock in/out:
	  * 20000ppm/80000ppm. Lock out 2
	  * times.
	  */
	{0x04, 0x71, 0x2d},
	 /*
	  * 2.7G CDR settings
	  * NOF=40LSB for HBR CDR setting
	  */
	{0x04, 0x7d, 0x07},
	{0x04, 0x7b, 0x00},  /* [1:0] Fmin=+4bands */
	{0x04, 0x7a, 0xfd},  /* [7:5] DCO_FTRNG=+-40% */
	 /*
	  * 1.62G CDR settings
	  * [5:2]NOF=64LSB [1:0]DCO scale is 2/5
	  */
	{0x04, 0xc0, 0x12},
	{0x04, 0xc1, 0x92},  /* Gitune=-37% */
	{0x04, 0xc2, 0x1c},  /* Fbstep=100% */
	{0x04, 0x32, 0x80},  /* [7] LOS signal disable */
	 /*
	  * RPIO Setting
	  * [7:4] LVDS driver bias current :
	  * 75% (250mV swing)
	  */
	{0x04, 0x00, 0xb0},
	 /*
	  * [7:6] Right-bar GPIO output strength is 8mA
	  */
	{0x04, 0x15, 0x40},
	 /* EQ Training State Machine Setting */
	{0x04, 0x54, 0x10},  /* RCO calibration start */
	 /* [4:0] MAX_LANE_COUNT set to one lane */
	{0x01, 0x02, 0x81},
	 /* [4:0] LANE_COUNT_SET set to one lane */
	{0x01, 0x21, 0x81},
	{0x00, 0x52, 0x20},
	{0x00, 0xf1, 0x03},  /* HPD CP toggle enable */
	{0x00, 0x62, 0x41},
	 /* Counter number, add 1ms counter delay */
	{0x00, 0xf6, 0x01},
	 /*
	  * [6]PWM function control by
	  * DPCD0040f[7], default is PWM
	  * block always works.
	  */
	{0x00, 0x77, 0x06},
	 /*
	  * 04h Adjust VTotal tolerance to
	  * fix the 30Hz no display issue
	  */
	{0x00, 0x4c, 0x04},
	 /* DPCD00400='h00, Parade OUI = 'h001cf8 */
	{0x01, 0xc0, 0x00},
	{0x01, 0xc1, 0x1c},  /* DPCD00401='h1c */
	{0x01, 0xc2, 0xf8},  /* DPCD00402='hf8 */
	 /*
	  * DPCD403~408 = ASCII code
	  * D2SLV5='h4432534c5635
	  */
	{0x01, 0xc3, 0x44},
	{0x01, 0xc4, 0x32},  /* DPCD404 */
	{0x01, 0xc5, 0x53},  /* DPCD405 */
	{0x01, 0xc6, 0x4c},  /* DPCD406 */
	{0x01, 0xc7, 0x56},  /* DPCD407 */
	{0x01, 0xc8, 0x35},  /* DPCD408 */
	 /*
	  * DPCD40A, Initial Code major  revision
	  * '01'
	  */
	{0x01, 0xca, 0x01},
	 /* DPCD40B, Initial Code minor revision '05' */
	{0x01, 0xcb, 0x05},
	 /* DPCD720, Select internal PWM */
	{0x01, 0xa5, 0xa0},
	 /*
	  * FFh for 100% PWM of brightness, 0h for 0%
	  * brightness
	  */
	{0x01, 0xa7, 0xff},
	 /*
	  * Set LVDS output as 6bit-VESA mapping,
	  * single LVDS channel
	  */
	{0x01, 0xcc, 0x13},
	 /* Enable SSC set by register */
	{0x02, 0xb1, 0x20},
	 /*
	  * Set SSC enabled and +/-1% central
	  * spreading
	  */
	{0x04, 0x10, 0x16},
	 /* MPU Clock source: LC => RCO */
	{0x04, 0x59, 0x60},
	{0x04, 0x54, 0x14},  /* LC -> RCO */
	{0x02, 0xa1, 0x91},  /* HPD high */
	{END_OF_TABLE}
};

/**
 * Write values table into the Parade eDP bridge
 *
 * @return      0 on success, non-0 on failure
 */

static int parade_write_regs(int base_addr, const struct reg_data *table)
{
	int ret = 0;

	while (!ret && (table->addr_off != END_OF_TABLE)) {
		ret = i2c_write(base_addr + table->addr_off,
				table->reg, 1,
				(uint8_t *)&table->value,
				sizeof(table->value));
		table++;
	}
	return ret;
}

int parade_init(const void *blob)
{
	struct gpio_desc rst_gpio;
	struct gpio_desc slp_gpio;
	int bus, old_bus;
	int parent;
	int node;
	int addr;
	int ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_PARADE_PS8625);
	if (node < 0)
		return 0;

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Could not find parent i2c node\n", __func__);
		return -1;
	}
	addr = fdtdec_get_int(blob, node, "reg", -1);
	if (addr < 0) {
		debug("%s: Could not find i2c address\n", __func__);
		return -1;
	}

	gpio_request_by_name_nodev(blob, node, "sleep-gpio", 0, &slp_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	mdelay(10);

	gpio_request_by_name_nodev(blob, node, "reset-gpio", 0, &rst_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	bus = i2c_get_bus_num_fdt(parent);
	old_bus = i2c_get_bus_num();

	debug("%s: Using i2c bus %d\n", __func__, bus);

	/*
	 * TODO(sjg@chromium.org): Hmmm we seem to need some sort of delay
	 * here.
	 */
	mdelay(40);
	i2c_set_bus_num(bus);
	ret = parade_write_regs(addr, parade_values);

	i2c_set_bus_num(old_bus);

	return ret;
}
