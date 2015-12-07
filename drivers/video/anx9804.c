/*
 * (C) 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Support for the ANX9804 bridge chip, which can take pixel data coming
 * from a parallel LCD interface and translate it on the flight into a DP
 * interface for driving eDP TFT displays.
 */

#include <common.h>
#include <i2c.h>
#include "anx9804.h"

/* Registers at i2c address 0x38 */

#define ANX9804_HDCP_CONTROL_0_REG				0x01

#define ANX9804_SYS_CTRL2_REG					0x81
#define ANX9804_SYS_CTRL2_CHA_STA				0x04

#define ANX9804_SYS_CTRL3_REG					0x82
#define ANX9804_SYS_CTRL3_VALID_CTRL				BIT(0)
#define ANX9804_SYS_CTRL3_F_VALID				BIT(1)
#define ANX9804_SYS_CTRL3_HPD_CTRL				BIT(4)
#define ANX9804_SYS_CTRL3_F_HPD					BIT(5)

#define ANX9804_LINK_BW_SET_REG					0xa0
#define ANX9804_LANE_COUNT_SET_REG				0xa1
#define ANX9804_TRAINING_PTN_SET_REG				0xa2
#define ANX9804_TRAINING_LANE0_SET_REG				0xa3
#define ANX9804_TRAINING_LANE1_SET_REG				0xa4
#define ANX9804_TRAINING_LANE2_SET_REG				0xa5
#define ANX9804_TRAINING_LANE3_SET_REG				0xa6

#define ANX9804_LINK_TRAINING_CTRL_REG				0xa8
#define ANX9804_LINK_TRAINING_CTRL_EN				BIT(0)

#define ANX9804_LINK_DEBUG_REG					0xb8
#define ANX9804_PLL_CTRL_REG					0xc7	
#define ANX9804_ANALOG_POWER_DOWN_REG				0xc8

/* Registers at i2c address 0x39 */

#define ANX9804_DEV_IDH_REG					0x03

#define ANX9804_POWERD_CTRL_REG					0x05
#define ANX9804_POWERD_AUDIO					BIT(4)

#define ANX9804_RST_CTRL_REG					0x06

#define ANX9804_RST_CTRL2_REG					0x07
#define ANX9804_RST_CTRL2_AUX					BIT(2)
#define ANX9804_RST_CTRL2_AC_MODE				BIT(6)

#define ANX9804_VID_CTRL1_REG					0x08
#define ANX9804_VID_CTRL1_VID_EN				BIT(7)
#define ANX9804_VID_CTRL1_EDGE					BIT(0)

#define ANX9804_VID_CTRL2_REG					0x09
#define ANX9804_ANALOG_DEBUG_REG1				0xdc
#define ANX9804_ANALOG_DEBUG_REG3				0xde
#define ANX9804_PLL_FILTER_CTRL1				0xdf
#define ANX9804_PLL_FILTER_CTRL3				0xe1
#define ANX9804_PLL_FILTER_CTRL					0xe2
#define ANX9804_PLL_CTRL3					0xe6

/**
 * anx9804_init() - Init anx9804 parallel lcd to edp bridge chip
 *
 * This function will init an anx9804 parallel lcd to dp bridge chip
 * using the passed in parameters.
 *
 * @i2c_bus:	Number of the i2c bus to which the anx9804 is connected.
 * @lanes:	Number of displayport lanes to use
 * @data_rate:	Register value for the bandwidth reg 0x06: 1.62G, 0x0a: 2.7G
 * @bpp:	Bits per pixel, must be 18 or 24
 */
void anx9804_init(unsigned int i2c_bus, u8 lanes, u8 data_rate, int bpp)
{
	unsigned int orig_i2c_bus = i2c_get_bus_num();
	u8 c, colordepth;
	int i;

	i2c_set_bus_num(i2c_bus);

	if (bpp == 18)
		colordepth = 0x00; /* 6 bit */
	else
		colordepth = 0x10; /* 8 bit */

	/* Reset */
	i2c_reg_write(0x39, ANX9804_RST_CTRL_REG, 1);
	mdelay(100);
	i2c_reg_write(0x39, ANX9804_RST_CTRL_REG, 0);

	/* Write 0 to the powerdown reg (powerup everything) */
	i2c_reg_write(0x39, ANX9804_POWERD_CTRL_REG, 0);

	c = i2c_reg_read(0x39, ANX9804_DEV_IDH_REG);
	if (c != 0x98) {
		printf("Error anx9804 chipid mismatch\n");
		i2c_set_bus_num(orig_i2c_bus);
		return;
	}

	for (i = 0; i < 100; i++) {
		c = i2c_reg_read(0x38, ANX9804_SYS_CTRL2_REG);
		i2c_reg_write(0x38, ANX9804_SYS_CTRL2_REG, c);
		c = i2c_reg_read(0x38, ANX9804_SYS_CTRL2_REG);
		if ((c & ANX9804_SYS_CTRL2_CHA_STA) == 0)
			break;

		mdelay(5);
	}
	if (i == 100)
		printf("Error anx9804 clock is not stable\n");

	i2c_reg_write(0x39, ANX9804_VID_CTRL2_REG, colordepth);
	
	/* Set a bunch of analog related register values */
	i2c_reg_write(0x38, ANX9804_PLL_CTRL_REG, 0x07); 
	i2c_reg_write(0x39, ANX9804_PLL_FILTER_CTRL3, 0x19); 
	i2c_reg_write(0x39, ANX9804_PLL_CTRL3, 0xd9); 
	i2c_reg_write(0x39, ANX9804_RST_CTRL2_REG, ANX9804_RST_CTRL2_AC_MODE);
	i2c_reg_write(0x39, ANX9804_ANALOG_DEBUG_REG1, 0xf0);
	i2c_reg_write(0x39, ANX9804_ANALOG_DEBUG_REG3, 0x99);
	i2c_reg_write(0x39, ANX9804_PLL_FILTER_CTRL1, 0x7b);
	i2c_reg_write(0x38, ANX9804_LINK_DEBUG_REG, 0x30);
	i2c_reg_write(0x39, ANX9804_PLL_FILTER_CTRL, 0x06);

	/* Force HPD */
	i2c_reg_write(0x38, ANX9804_SYS_CTRL3_REG,
		      ANX9804_SYS_CTRL3_F_HPD | ANX9804_SYS_CTRL3_HPD_CTRL);

	/* Power up and configure lanes */
	i2c_reg_write(0x38, ANX9804_ANALOG_POWER_DOWN_REG, 0x00);
	i2c_reg_write(0x38, ANX9804_TRAINING_LANE0_SET_REG, 0x00);
	i2c_reg_write(0x38, ANX9804_TRAINING_LANE1_SET_REG, 0x00);
	i2c_reg_write(0x38, ANX9804_TRAINING_LANE2_SET_REG, 0x00);
	i2c_reg_write(0x38, ANX9804_TRAINING_LANE3_SET_REG, 0x00);

	/* Reset AUX CH */
	i2c_reg_write(0x39, ANX9804_RST_CTRL2_REG,
		      ANX9804_RST_CTRL2_AC_MODE | ANX9804_RST_CTRL2_AUX);
	i2c_reg_write(0x39, ANX9804_RST_CTRL2_REG,
		      ANX9804_RST_CTRL2_AC_MODE);

	/* Powerdown audio and some other unused bits */
	i2c_reg_write(0x39, ANX9804_POWERD_CTRL_REG, ANX9804_POWERD_AUDIO);
	i2c_reg_write(0x38, ANX9804_HDCP_CONTROL_0_REG, 0x00);
	i2c_reg_write(0x38, 0xa7, 0x00);

	/* Set data-rate / lanes */
	i2c_reg_write(0x38, ANX9804_LINK_BW_SET_REG, data_rate);
	i2c_reg_write(0x38, ANX9804_LANE_COUNT_SET_REG, lanes);

	/* Link training */	
	i2c_reg_write(0x38, ANX9804_LINK_TRAINING_CTRL_REG,
		      ANX9804_LINK_TRAINING_CTRL_EN);
	mdelay(5);
	for (i = 0; i < 100; i++) {
		c = i2c_reg_read(0x38, ANX9804_LINK_TRAINING_CTRL_REG);
		if ((c & 0x01) == 0)
			break;

		mdelay(5);
	}
	if(i == 100) {
		printf("Error anx9804 link training timeout\n");
		i2c_set_bus_num(orig_i2c_bus);
		return;
	}

	/* Enable */
	i2c_reg_write(0x39, ANX9804_VID_CTRL1_REG,
		      ANX9804_VID_CTRL1_VID_EN | ANX9804_VID_CTRL1_EDGE);
	/* Force stream valid */
	i2c_reg_write(0x38, ANX9804_SYS_CTRL3_REG,
		      ANX9804_SYS_CTRL3_F_HPD | ANX9804_SYS_CTRL3_HPD_CTRL |
		      ANX9804_SYS_CTRL3_F_VALID | ANX9804_SYS_CTRL3_VALID_CTRL);

	i2c_set_bus_num(orig_i2c_bus);
}
