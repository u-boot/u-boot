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
#include <power/battery.h>
#include <power/max8997_pmic.h>
#include <errno.h>

static struct battery battery_trats;

static int power_battery_charge(struct pmic *bat)
{
	struct power_battery *p_bat = bat->pbat;
	struct battery *battery = p_bat->bat;
	int k;

	if (bat->chrg->chrg_state(p_bat->chrg, CHARGER_ENABLE, 450))
		return -1;

	for (k = 0; bat->chrg->chrg_bat_present(p_bat->chrg) &&
		     bat->chrg->chrg_type(p_bat->muic) &&
		     battery->state_of_chrg < 100; k++) {
		udelay(10000000);
		puts(".");
		bat->fg->fg_battery_update(p_bat->fg, bat);

		if (k == 100) {
			debug(" %d [V]", battery->voltage_uV);
			puts("\n");
			k = 0;
		}

	}

	bat->chrg->chrg_state(p_bat->chrg, CHARGER_DISABLE, 0);

	return 0;
}

static int power_battery_init_trats(struct pmic *bat_,
				    struct pmic *fg_,
				    struct pmic *chrg_,
				    struct pmic *muic_)
{
	bat_->pbat->fg = fg_;
	bat_->pbat->chrg = chrg_;
	bat_->pbat->muic = muic_;

	bat_->fg = fg_->fg;
	bat_->chrg = chrg_->chrg;
	bat_->chrg->chrg_type = muic_->chrg->chrg_type;
	return 0;
}

static struct power_battery power_bat_trats = {
	.bat = &battery_trats,
	.battery_init = power_battery_init_trats,
	.battery_charge = power_battery_charge,
};

int power_bat_init(unsigned char bus)
{
	static const char name[] = "BAT_TRATS";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	debug("Board BAT init\n");

	p->interface = PMIC_NONE;
	p->name = name;
	p->bus = bus;

	p->pbat = &power_bat_trats;
	return 0;
}
