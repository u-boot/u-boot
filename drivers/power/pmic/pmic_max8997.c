/*
 *  Copyright (C) 2012 Samsung Electronics
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <power/pmic.h>
#include <power/max8997_pmic.h>
#include <i2c.h>
#include <errno.h>

unsigned char max8997_reg_ldo(int uV)
{
	unsigned char ret;
	if (uV <= 800000)
		return 0;
	if (uV >= 3950000)
		return MAX8997_LDO_MAX_VAL;
	ret = (uV - 800000) / 50000;
	if (ret > MAX8997_LDO_MAX_VAL) {
		printf("MAX8997 LDO SETTING ERROR (%duV) -> %u\n", uV, ret);
		ret = MAX8997_LDO_MAX_VAL;
	}

	return ret;
}

int pmic_init(unsigned char bus)
{
	static const char name[] = "MAX8997_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	puts("Board PMIC init\n");

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.addr = MAX8997_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}
