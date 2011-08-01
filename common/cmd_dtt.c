/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <config.h>
#include <command.h>

#include <dtt.h>
#include <i2c.h>

static unsigned long sensor_initialized;

int do_dtt (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;
	int old_bus;

	/* Force a compilation error, if there are more then 32 sensors */
	BUILD_BUG_ON(sizeof(sensors) > 32);
	/* switch to correct I2C bus */
	old_bus = I2C_GET_BUS();
	I2C_SET_BUS(CONFIG_SYS_DTT_BUS_NUM);

	/*
	 * Loop through sensors, read
	 * temperature, and output it.
	 */
	for (i = 0; i < sizeof(sensors); i++) {
		if ((sensor_initialized & (1 << i)) == 0) {
			if (dtt_init_one(sensors[i]) != 0) {
				printf("DTT%d: Failed init!\n", i);
				continue;
			}
			sensor_initialized |= (1 << i);
		}
		printf("DTT%d: %i C\n", i + 1, dtt_get_temp(sensors[i]));
	}

	/* switch back to original I2C bus */
	I2C_SET_BUS(old_bus);

	return 0;
}	/* do_dtt() */

/***************************************************/

U_BOOT_CMD(
	  dtt,	1,	1,	do_dtt,
	  "Read temperature from Digital Thermometer and Thermostat",
	  ""
);
