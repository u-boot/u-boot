/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright(C) 2024 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef _MAX8907_H_
#define _MAX8907_H_

#define MAX8907_LDO_NUM			20
#define MAX8907_SD_NUM			3

/* Drivers name */
#define MAX8907_LDO_DRIVER		"max8907_ldo"
#define MAX8907_SD_DRIVER		"max8907_sd"
#define MAX8907_RST_DRIVER		"max8907_rst"

/* MAX8907 register map */
#define MAX8907_REG_SDCTL1		0x04
#define MAX8907_REG_SDCTL2		0x07
#define MAX8907_REG_SDCTL3		0x0A

#define MAX8907_REG_LDOCTL16		0x10
#define MAX8907_REG_LDOCTL17		0x14
#define MAX8907_REG_LDOCTL1		0x18
#define MAX8907_REG_LDOCTL2		0x1C
#define MAX8907_REG_LDOCTL3		0x20
#define MAX8907_REG_LDOCTL4		0x24
#define MAX8907_REG_LDOCTL5		0x28
#define MAX8907_REG_LDOCTL6		0x2C
#define MAX8907_REG_LDOCTL7		0x30
#define MAX8907_REG_LDOCTL8		0x34
#define MAX8907_REG_LDOCTL9		0x38
#define MAX8907_REG_LDOCTL10		0x3C
#define MAX8907_REG_LDOCTL11		0x40
#define MAX8907_REG_LDOCTL12		0x44
#define MAX8907_REG_LDOCTL13		0x48
#define MAX8907_REG_LDOCTL14		0x4C
#define MAX8907_REG_LDOCTL15		0x50
#define MAX8907_REG_LDOCTL19		0x5C
#define MAX8907_REG_LDOCTL18		0x72
#define MAX8907_REG_LDOCTL20		0x9C

#define MAX8907_REG_RESET_CNFG		0x0F
#define   MASK_POWER_OFF		BIT(6)

/* MAX8907 configuration values */
#define MAX8907_CTL			0
#define MAX8907_SEQCNT			1
#define MAX8907_VOUT			2

/* mask bit fields */
#define MAX8907_MASK_LDO_SEQ		0x1C
#define MAX8907_MASK_LDO_EN		0x01

/* Step-Down (SD) Regulator calculations */
#define SD1_VOLT_MAX			2225000
#define SD1_VOLT_MIN			650000
#define SD1_VOLT_STEP			25000

#define SD2_VOLT_MAX			1425000
#define SD2_VOLT_MIN			637500
#define SD2_VOLT_STEP			12500

#define SD3_VOLT_MAX			3900000
#define SD3_VOLT_MIN			750000
#define SD3_VOLT_STEP			50000

/* Low-Dropout Linear (LDO) Regulator calculations */
#define LDO_750_VOLT_MAX		3900000
#define LDO_750_VOLT_MIN		750000
#define LDO_750_VOLT_STEP		50000

#define LDO_650_VOLT_MAX		2225000
#define LDO_650_VOLT_MIN		650000
#define LDO_650_VOLT_STEP		25000

#endif /* _MAX8907_H_ */
