/*
 *  Copyright (C) 2011 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <pmic.h>
#include <dialog_pmic.h>

int pmic_dialog_init(void)
{
	struct pmic *p = get_pmic();
	static const char name[] = "DIALOG_PMIC";

	p->name = name;
	p->number_of_regs = DIALOG_NUM_OF_REGS;

	p->interface = PMIC_I2C;
	p->hw.i2c.addr = CONFIG_SYS_DIALOG_PMIC_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = I2C_PMIC;

	return 0;
}
