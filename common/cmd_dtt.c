/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>

#include <dtt.h>
#include <i2c.h>
#include <tmu.h>

#if defined CONFIG_DTT_SENSORS
static unsigned long sensor_initialized;

static void _initialize_dtt(void)
{
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;

	for (i = 0; i < sizeof(sensors); i++) {
		if ((sensor_initialized & (1 << i)) == 0) {
			if (dtt_init_one(sensors[i]) != 0) {
				printf("DTT%d: Failed init!\n", i);
				continue;
			}
			sensor_initialized |= (1 << i);
		}
	}
}

void dtt_init(void)
{
	int old_bus;

	/* switch to correct I2C bus */
	old_bus = I2C_GET_BUS();
	I2C_SET_BUS(CONFIG_SYS_DTT_BUS_NUM);

	_initialize_dtt();

	/* switch back to original I2C bus */
	I2C_SET_BUS(old_bus);
}
#endif

int dtt_i2c(void)
{
#if defined CONFIG_DTT_SENSORS
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;
	int old_bus;

	/* Force a compilation error, if there are more then 32 sensors */
	BUILD_BUG_ON(sizeof(sensors) > 32);
	/* switch to correct I2C bus */
#ifdef CONFIG_SYS_I2C
	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_DTT_BUS_NUM);
#else
	old_bus = I2C_GET_BUS();
	I2C_SET_BUS(CONFIG_SYS_DTT_BUS_NUM);
#endif

	_initialize_dtt();

	/*
	 * Loop through sensors, read
	 * temperature, and output it.
	 */
	for (i = 0; i < sizeof(sensors); i++)
		printf("DTT%d: %i C\n", i + 1, dtt_get_temp(sensors[i]));

	/* switch back to original I2C bus */
#ifdef CONFIG_SYS_I2C
	i2c_set_bus_num(old_bus);
#else
	I2C_SET_BUS(old_bus);
#endif
#endif

	return 0;
}

int dtt_tmu(void)
{
#if defined CONFIG_TMU_CMD_DTT
	int cur_temp;

	/* Sense and return latest thermal info */
	if (tmu_monitor(&cur_temp) == TMU_STATUS_INIT) {
		puts("TMU is in unknown state, temperature is invalid\n");
		return -1;
	}
	printf("Current temperature: %u degrees Celsius\n", cur_temp);
#endif
	return 0;
}

int do_dtt(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int err = 0;

	err |= dtt_i2c();
	err |= dtt_tmu();

	return err;
}	/* do_dtt() */

/***************************************************/

U_BOOT_CMD(
	  dtt,	1,	1,	do_dtt,
	  "Read temperature from Digital Thermometer and Thermostat",
	  ""
);
