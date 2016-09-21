/*
 * (C) Copyright 2014 Texas Instruments Incorporated -  http://www.ti.com
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <linux/errno.h>
#include <power/pmic.h>
#include <power/tps62362.h>

/**
 * tps62362_voltage_update() - Function to change a voltage level, as this
 *			       is a multi-step process.
 * @reg:	Register address to write to
 * @volt_sel:	Voltage register value to write
 * @return:	0 on success, 1 on failure
 */
int tps62362_voltage_update(unsigned char reg, unsigned char volt_sel)
{
	if (reg > TPS62362_NUM_REGS)
		return 1;

	return i2c_write(TPS62362_I2C_ADDR, reg, 1, &volt_sel, 1);
}

int power_tps62362_init(unsigned char bus)
{
	static const char name[] = "TPS62362";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = TPS62362_NUM_REGS;
	p->hw.i2c.addr = TPS62362_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}
