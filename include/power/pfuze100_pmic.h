/*
 *  Copyright (C) 2014 Gateworks Corporation
 *  Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __PFUZE100_PMIC_H_
#define __PFUZE100_PMIC_H_

/* PFUZE100 registers */
enum {
	PFUZE100_DEVICEID	= 0x00,
	PFUZE100_REVID		= 0x03,
	PFUZE100_FABID		= 0x04,

	PFUZE100_SW1ABVOL	= 0x20,
	PFUZE100_SW1CVOL	= 0x2e,
	PFUZE100_SW2VOL		= 0x35,
	PFUZE100_SW3AVOL	= 0x3c,
	PFUZE100_SW3BVOL	= 0x43,
	PFUZE100_SW4VOL		= 0x4a,
	PFUZE100_SWBSTCON1	= 0x66,
	PFUZE100_VREFDDRCON	= 0x6a,
	PFUZE100_VSNVSVOL	= 0x6b,
	PFUZE100_VGEN1VOL	= 0x6c,
	PFUZE100_VGEN2VOL	= 0x6d,
	PFUZE100_VGEN3VOL	= 0x6e,
	PFUZE100_VGEN4VOL	= 0x6f,
	PFUZE100_VGEN5VOL	= 0x70,
	PFUZE100_VGEN6VOL	= 0x71,

	PMIC_NUM_OF_REGS	= 0x7f,
};

/*
 * LDO Configuration
 */

/* VGEN1/2 Voltage Configuration */
#define LDOA_0_80V	0
#define LDOA_0_85V	1
#define LDOA_0_90V	2
#define LDOA_0_95V	3
#define LDOA_1_00V	4
#define LDOA_1_05V	5
#define LDOA_1_10V	6
#define LDOA_1_15V	7
#define LDOA_1_20V	8
#define LDOA_1_25V	9
#define LDOA_1_30V	10
#define LDOA_1_35V	11
#define LDOA_1_40V	12
#define LDOA_1_45V	13
#define LDOA_1_50V	14
#define LDOA_1_55V	15

/* VGEN3/4/5/6 Voltage Configuration */
#define LDOB_1_80V	0
#define LDOB_1_90V	1
#define LDOB_2_00V	2
#define LDOB_2_10V	3
#define LDOB_2_20V	4
#define LDOB_2_30V	5
#define LDOB_2_40V	6
#define LDOB_2_50V	7
#define LDOB_2_60V	8
#define LDOB_2_70V	9
#define LDOB_2_80V	10
#define LDOB_2_90V	11
#define LDOB_3_00V	12
#define LDOB_3_10V	13
#define LDOB_3_20V	14
#define LDOB_3_30V	15

#define LDO_VOL_MASK	0xf
#define LDO_EN		4

/*
 * Boost Regulator
 */

/* SWBST Output Voltage */
#define SWBST_5_00V	0
#define SWBST_5_05V	1
#define SWBST_5_10V	2
#define SWBST_5_15V	3

#define SWBST_VOL_MASK	0x3
#define SWBST_MODE_MASK	0x6
#define SWBST_MODE_OFF	(2 << 0)
#define SWBST_MODE_PFM	(2 << 1)
#define SWBST_MODE_AUTO	(2 << 2)
#define SWBST_MODE_APS	(2 << 3)

#endif
