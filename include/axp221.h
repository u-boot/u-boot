/*
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 *
 * X-Powers AXP221 Power Management IC driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define AXP221_CHIP_ADDR 0x68
#define AXP221_CTRL_ADDR 0x3e
#define AXP221_INIT_DATA 0x3e

#define AXP221_CHIP_ID		0x03
#define AXP221_OUTPUT_CTRL1	0x10
#define AXP221_OUTPUT_CTRL1_ALDO1_EN	(1 << 6)
#define AXP221_OUTPUT_CTRL1_ALDO2_EN	(1 << 7)
#define AXP221_OUTPUT_CTRL2	0x12
#define AXP221_OUTPUT_CTRL2_DLDO1_EN	(1 << 3)
#define AXP221_OUTPUT_CTRL2_DLDO2_EN	(1 << 4)
#define AXP221_OUTPUT_CTRL2_DLDO3_EN	(1 << 5)
#define AXP221_OUTPUT_CTRL2_DLDO4_EN	(1 << 6)
#define AXP221_OUTPUT_CTRL2_DCDC1_EN	(1 << 7)
#define AXP221_OUTPUT_CTRL3	0x13
#define AXP221_OUTPUT_CTRL3_ALDO3_EN	(1 << 7)
#define AXP221_DLDO1_CTRL	0x15
#define AXP221_DLDO2_CTRL	0x16
#define AXP221_DLDO3_CTRL	0x17
#define AXP221_DLDO4_CTRL	0x18
#define AXP221_DCDC1_CTRL	0x21
#define AXP221_DCDC2_CTRL	0x22
#define AXP221_DCDC3_CTRL	0x23
#define AXP221_DCDC4_CTRL	0x24
#define AXP221_DCDC5_CTRL	0x25
#define AXP221_ALDO1_CTRL	0x28
#define AXP221_ALDO2_CTRL	0x29
#define AXP221_ALDO3_CTRL	0x2a

int axp221_set_dcdc1(unsigned int mvolt);
int axp221_set_dcdc2(unsigned int mvolt);
int axp221_set_dcdc3(unsigned int mvolt);
int axp221_set_dcdc4(unsigned int mvolt);
int axp221_set_dcdc5(unsigned int mvolt);
int axp221_set_dldo1(unsigned int mvolt);
int axp221_set_dldo2(unsigned int mvolt);
int axp221_set_dldo3(unsigned int mvolt);
int axp221_set_dldo4(unsigned int mvolt);
int axp221_set_aldo1(unsigned int mvolt);
int axp221_set_aldo2(unsigned int mvolt);
int axp221_set_aldo3(unsigned int mvolt);
int axp221_init(void);
