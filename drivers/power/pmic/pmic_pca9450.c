// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pca9450.h>

static const char pca9450_name[] = "PCA9450";

int power_pca9450a_init(unsigned char bus)
{
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = pca9450_name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PCA9450_REG_NUM;
	p->hw.i2c.addr = 0x35;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

int power_pca9450b_init(unsigned char bus)
{
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = pca9450_name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PCA9450_REG_NUM;
	p->hw.i2c.addr = 0x25;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}
