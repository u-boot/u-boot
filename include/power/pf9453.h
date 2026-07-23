/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2024 NXP
 */

#ifndef PF9453_H_
#define PF9453_H_

#define PF9453_REGULATOR_DRIVER "pf9453_regulator"

enum {
	PF9453_REG_DEV_ID          = 0x00,
	PF9453_OTP_VER             = 0x01,
	PF9453_INT1                = 0x02,
	PF9453_INT1_MSK            = 0x03,
	PF9453_INT1_STATUS         = 0x04,
	PF9453_VRFLT1_INT          = 0x05,
	PF9453_VRFLT1_MASK         = 0x06,
	PF9453_PWRON_STAT          = 0x07,
	PF9453_RESET_CTRL          = 0x08,
	PF9453_SW_RST              = 0x09,
	PF9453_PWR_CTRL            = 0x0a,
	PF9453_CONFIG1             = 0x0b,
	PF9453_CONFIG2             = 0x0c,
	PF9453_32K_CONFIG          = 0x0d,
	PF9453_BUCK1CTRL           = 0x10,
	PF9453_BUCK1OUT            = 0x11,
	PF9453_BUCK2CTRL           = 0x14,
	PF9453_BUCK2OUT            = 0x15,
	PF9453_BUCK2OUT_STBY       = 0x1D,
	PF9453_BUCK2OUT_MAX_LIMIT  = 0x1F,
	PF9453_BUCK2OUT_MIN_LIMIT  = 0x20,
	PF9453_BUCK3CTRL           = 0x21,
	PF9453_BUCK3OUT            = 0x22,
	PF9453_BUCK4CTRL           = 0x2e,
	PF9453_BUCK4OUT            = 0x2f,
	PF9453_LDO1OUT_L          = 0x36,
	PF9453_LDO1CFG            = 0x37,
	PF9453_LDO1OUT_H          = 0x38,
	PF9453_LDOSNVS_CFG1        = 0x39,
	PF9453_LDOSNVS_CFG2        = 0x3a,
	PF9453_LDO2CFG            = 0x3b,
	PF9453_LDO2OUT            = 0x3c,
	PF9453_BUCK_POK            = 0x3d,
	PF9453_LSW_CTRL1           = 0x40,
	PF9453_LSW_CTRL2           = 0x41,
	PF9453_REG_LOCK            = 0x4e,
	PF9453_REG_NUM,
};

int power_pf9453_init(unsigned char bus, unsigned char addr);

enum {
	NXP_CHIP_TYPE_PF9453 = 0,
	NXP_CHIP_TYPE_AMOUNT
};

#define PF9453_UNLOCK_KEY		0x5c
#define PF9453_LOCK_KEY			0x0

#define PF9453_EN_MODE_MASK		0x3
#define PF9453_BUCK_RUN_MASK		0x7f
#define PF9453_LDO1_MASK		0x7f
#define PF9453_LDO2_MASK		0x3f
#define PF9453_LDOSNVS_MASK		0x7f

#define PF9453_PMIC_RESET_WDOG_B_CFG_MASK		0xc0
#define PF9453_PMIC_RESET_WDOG_B_CFG_WARM		0x40
#define PF9453_PMIC_RESET_WDOG_B_CFG_COLD		0x80

#endif
